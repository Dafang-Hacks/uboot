/*
 * #endif
 * Ingenic burner function Code (Vendor Private Protocol)
 *
 * Copyright (c) 2013 cli <cli@ingenic.cn>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <errno.h>
#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <rtc.h>
#include <part.h>
#include <spi.h>
#include <spi_flash.h>
#include <efuse.h>
#include <ingenic_soft_i2c.h>
#include <ingenic_soft_spi.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/compiler.h>
#include <linux/usb/composite.h>

#include <ingenic_nand_mgr/nand_param.h>

#define ARGS_LEN (1024*1024)
#define BURNNER_DEBUG 0


#define SSI_IDX 0

struct spi spi;
/*bootrom stage request*/
#define VR_GET_CPU_INFO		0x00
#define VR_SET_DATA_ADDR	0x01
#define VR_SET_DATA_LEN		0x02
#define VR_FLUSH_CACHE		0x03
#define VR_PROG_STAGE1		0x04
#define VR_PROG_STAGE2		0x05
/*firmware stage request*/
#define VR_GET_ACK		0x10
#define VR_INIT			0x11
#define VR_WRITE		0x12
#define VR_READ			0x13
#define VR_UPDATE_CFG		0x14
#define VR_SYNC_TIME		0x15
#define VR_REBOOT		0x16

#define MMC_ERASE_ALL	1
#define MMC_ERASE_PART	2
#define MMC_ERASE_CNT_MAX	10

#define SPI_NO_ERASE	0
#define SPI_ERASE_PART	1
#ifndef CONFIG_SF_DEFAULT_SPEED
# define CONFIG_SF_DEFAULT_SPEED    20000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
# define CONFIG_SF_DEFAULT_MODE     SPI_MODE_3
#endif
#ifndef CONFIG_SF_DEFAULT_CS
# define CONFIG_SF_DEFAULT_CS       0
#endif
#ifndef CONFIG_SF_DEFAULT_BUS
# define CONFIG_SF_DEFAULT_BUS      0
#endif

enum medium_type {
	MEMORY = 0,
	NAND,
	MMC,
	I2C,
	EFUSE,
	REGISTER,
	SPI
};

enum data_type {
	RAW = 0,
	OOB,
	IMAGE,
};

struct i2c_args {
	int clk;
	int data;
	int device;
	int value_count;
	int value[0];
};

struct mmc_erase_range {
	uint32_t start;
	uint32_t end;
};

struct spi_args {
	uint32_t clk;
	uint32_t data_in;
	uint32_t data_out;
	uint32_t enable;
	uint32_t rate;
};

struct spi_erase_range{
	uint32_t blocksize;
	uint32_t blockcount;
};

struct jz_spi_support{
	uint8_t id;
	char name[32];
	int page_size;
	int sector_size;
	int size;
};

struct arguments {
	int efuse_gpio;
	int use_nand_mgr;
	int use_mmc;
	int use_spi;

	int nand_erase;
	int nand_erase_count;
	unsigned int offsets[32];

	int mmc_open_card;
	int mmc_erase;
	uint32_t mmc_erase_range_count;
	struct mmc_erase_range mmc_erase_range[MMC_ERASE_CNT_MAX];

	struct spi_args spi_args;
	uint32_t spi_erase;
	struct jz_spi_support jz_spi_support_table;
	struct spi_erase_range spi_erase_range;

	int transfer_data_chk;
	int write_back_chk;

	PartitionInfo PartInfo;
	int nr_nand_args;
	nand_flash_param nand_params[0];

};

union cmd {
	struct update {
		uint32_t length;
	}update;

	struct write {
		uint64_t partation;
		uint32_t ops;
		uint32_t offset;
		uint32_t length;
		uint32_t crc;
	}write;

	struct read {
		uint64_t partation;
		uint32_t ops;
		uint32_t offset;
		uint32_t length;
	}read;

	struct rtc_time rtc;
};

struct cloner {
	struct usb_function usb_function;
	struct usb_composite_dev *cdev;		/*Copy of config->cdev*/
	struct usb_gadget *gadget;	/*Copy of cdev->gadget*/
	struct usb_ep *ep0;		/*Copy of gadget->ep0*/
	struct usb_request *ep0req;	/*Copy of cdev->req*/

	struct usb_ep *ep_in;
	struct usb_ep *ep_out;
	struct usb_request *write_req;
	struct usb_request *args_req;
	struct usb_request *read_req;

	union cmd *cmd;
	int cmd_type;
	uint32_t buf_size;
	int ack;
	struct arguments *args;
	int inited;
};

static const char burntool_name[] = "INGENIC VENDOR BURNNER";

static struct usb_string burner_intf_string_defs[] = {
	[0].s = burntool_name,
	{}
};

static struct usb_gadget_strings  burn_intf_string = {
	.language = 0x0409, /* en-us */
	.strings = burner_intf_string_defs,
};

static struct usb_gadget_strings  *burn_intf_string_tab[] = {
	&burn_intf_string,
	NULL,
};

static struct usb_interface_descriptor intf_desc = {
	.bLength =              sizeof(intf_desc),
	.bDescriptorType =      USB_DT_INTERFACE,
	.bNumEndpoints =        2,
};

static struct usb_endpoint_descriptor fs_bulk_in_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN|0x1,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor fs_bulk_out_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_OUT|0x1,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor hs_bulk_in_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN|0x1,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_bulk_out_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_OUT|0x1,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(512),
};

static struct usb_descriptor_header *fs_intf_descs[] = {
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &fs_bulk_out_desc,
	(struct usb_descriptor_header *) &fs_bulk_in_desc,
	NULL,
};

static struct usb_descriptor_header *hs_intf_descs[] = {
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &hs_bulk_out_desc,
	(struct usb_descriptor_header *) &hs_bulk_in_desc,
	NULL,
};

static inline struct cloner *func_to_cloner(struct usb_function *f)
{
	return container_of(f, struct cloner, usb_function);
}

static uint32_t crc_table[] = {
	/* CRC polynomial 0xedb88320 */
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static uint32_t local_crc32(uint32_t crc,unsigned char *buffer, uint32_t size)
{
	uint32_t i;
	for (i = 0; i < size; i++) {
		crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
	}
	return crc ;
}

int i2c_program(struct cloner *cloner)
{
	int i = 0;
	struct i2c_args *i2c_arg = (struct i2c_args *)cloner->write_req->buf;
	struct i2c i2c;
	i2c.scl = i2c_arg->clk;
	i2c.sda = i2c_arg->data;
	i2c_init(&i2c);

	for(i=0;i<i2c_arg->value_count;i++) {
		char reg = i2c_arg->value[i] >> 16;
		unsigned char value = i2c_arg->value[i] & 0xff;
		i2c_write(&i2c,i2c_arg->device,reg,1,&value,1);
	}
	return 0;
}

#define MMC_BYTE_PER_BLOCK 512
extern ulong mmc_erase_t(struct mmc *mmc, ulong start, lbaint_t blkcnt);
static int mmc_erase(struct cloner *cloner)
{
	int curr_device = 0;
	struct mmc *mmc = find_mmc_device(0);
	uint32_t blk, blk_end, blk_cnt;
	uint32_t erase_cnt = 0;
	int timeout = 30000;
	int i;
	int ret;

	if (!mmc) {
		printf("no mmc device at slot %x\n", curr_device);
		return -ENODEV;
	}

	mmc_init(mmc);
	if(get_mmc_csd_perm_w_protect()){
		printf("ERROR: MMC Init error ,can not be erase !!!!!!!!!\n");
		return -EPERM;
	}

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return -EPERM;
	}
	if (cloner->args->mmc_erase == MMC_ERASE_ALL) {
		blk = 0;
		blk_cnt = mmc->capacity / MMC_BYTE_PER_BLOCK;

		printf("MMC erase: dev # %d, start block # %d, count %u ... \n",
				curr_device, blk, blk_cnt);

		ret = mmc_erase_t(mmc, blk, blk_cnt);
		if (ret) {
			printf("mmc erase error\n");
			return ret;
		}
		ret = mmc_send_status(mmc, timeout);
		if(ret){
			printf("mmc erase error\n");
			return ret;
		}

		printf("mmc all erase ok, blocks %d\n", blk_cnt);
		return 0;
	} else if (cloner->args->mmc_erase != MMC_ERASE_PART) {
		return -EINVAL;
	}

	/*mmc part erase */
	erase_cnt = (cloner->args->mmc_erase_range_count >MMC_ERASE_CNT_MAX) ?
		MMC_ERASE_CNT_MAX : cloner->args->mmc_erase_range_count;

	for (i = 0; erase_cnt > 0; i++, erase_cnt--) {
		blk = cloner->args->mmc_erase_range[i].start / MMC_BYTE_PER_BLOCK;
		blk_end = cloner->args->mmc_erase_range[i].end / MMC_BYTE_PER_BLOCK;
		blk_cnt = blk_end - blk + 1;

		printf("MMC erase: dev # %d, start block # 0x%x, count 0x%x ... \n",
				curr_device, blk, blk_cnt);

		if ((blk % mmc->erase_grp_size) || (blk_cnt % mmc->erase_grp_size)) {
			printf("\n\nCaution! Your devices Erase group is 0x%x\n"
					"The erase block range would be change to "
					"0x" LBAF "~0x" LBAF "\n\n",
					mmc->erase_grp_size, blk & ~(mmc->erase_grp_size - 1),
					((blk + blk_cnt + mmc->erase_grp_size)
					 & ~(mmc->erase_grp_size - 1)) - 1);
		}

		ret = mmc_erase_t(mmc, blk, blk_cnt);
		if (ret) {
			printf("mmc erase error\n");
			return ret;
		}
		ret = mmc_send_status(mmc, timeout);
		if(ret){
			printf("mmc erase error\n");
			return ret;
		}

		printf("mmc part erase, part %d ok\n", i);
	}
	printf("mmc erase ok\n");
	return 0;
}

static int spi_erase(struct cloner *cloner)
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	int blk_cnt = cloner->args->spi_erase_range.blockcount;
	int blk_size = cloner->args->spi_erase_range.blocksize;
	int curr_device = 0;
	uint32_t erase_cnt = 0;
	struct spi_flash *flash;
	int timeout = 30000;
	int offset = 0;
	int i;
	int ret;
#ifdef CONFIG_JZ_SPI
	spi_init();
#endif
#ifdef CONFIG_INGENIC_SOFT_SPI
	spi_init_jz(&spi);
#endif

	if(flash == NULL){
		flash = spi_flash_probe(bus, cs, spi.rate, mode);
		if (!flash) {
			printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
			return 1;
		}
	}

	for (i = 0;i < blk_cnt; i++) {
		ret = spi_flash_erase(flash, offset, blk_size);
		printf("SF: %zu bytes @ %#x Erased: %s\n", blk_size, (u32)offset,
				ret ? "ERROR" : "OK");
		offset += blk_size;
	}
	printf("spi erase ok\n");
	return 0;
}




int cloner_init(struct cloner *cloner)
{
	if(cloner->args->use_nand_mgr) {
#ifdef CONFIG_JZ_NAND_MGR
		nand_probe_burner(&(cloner->args->PartInfo),
				&(cloner->args->nand_params[0]),
				cloner->args->nr_nand_args,
				cloner->args->nand_erase,cloner->args->offsets,cloner->args->nand_erase_count);
#endif
	}

	if (cloner->args->use_mmc) {
		if (cloner->args->mmc_erase) {
			mmc_erase(cloner);
		}
	}

	if (cloner->args->use_spi) {
		if (cloner->args->spi_erase == SPI_ERASE_PART) {
			spi_erase(cloner);
		}
	}

	if(cloner->args->use_spi){
		printf("cloner->args->spi_args.clk:%d\n",cloner->args->spi_args.clk);
		printf("cloner->args->spi_args.data_in:%d\n",cloner->args->spi_args.data_in);
		printf("cloner->args->spi_args.data_out:%d\n",cloner->args->spi_args.data_out);
		printf("cloner->args->spi_args.enable:%d\n",cloner->args->spi_args.enable);

		printf("cloner->args->count:%d\n",cloner->args->spi_erase_range.blockcount);
		printf("cloner->args->size:%d\n",cloner->args->spi_erase_range.blocksize);
		printf("cloner->args->jz_spi_support_table.size:%d\n",cloner->args->jz_spi_support_table.size);
		printf("name:%s\n",cloner->args->jz_spi_support_table.name);
	}


}

int nand_program(struct cloner *cloner)
{
#ifdef CONFIG_JZ_NAND_MGR
	int curr_device = 0;
	u32 startaddr = cloner->cmd->write.partation + (cloner->cmd->write.offset);
	u32 length = cloner->cmd->write.length;
	void *databuf = (void *)cloner->write_req->buf;

	printf("=========++++++++++++>   NAND PROGRAM:startaddr = %d P offset = %d P length = %d \n",startaddr,cloner->cmd->write.offset,length);
	do_nand_request(startaddr, databuf, length,cloner->cmd->write.offset);

	return 0;
#else
	return -ENODEV;
#endif
}

int mmc_program(struct cloner *cloner,int mmc_index)
{
#define MMC_BYTE_PER_BLOCK 512
	int curr_device = 0;
	struct mmc *mmc = find_mmc_device(mmc_index);
	u32 blk = (cloner->cmd->write.partation + cloner->cmd->write.offset)/MMC_BYTE_PER_BLOCK;
	u32 cnt = (cloner->cmd->write.length + MMC_BYTE_PER_BLOCK - 1)/MMC_BYTE_PER_BLOCK;
	void *addr = (void *)cloner->write_req->buf;
	u32 n;

	if (!mmc) {
		printf("no mmc device at slot %x\n", curr_device);
		return -ENODEV;
	}

	//debug_cond(BURNNER_DEBUG,"\nMMC write: dev # %d, block # %d, count %d ... ",
	printf("MMC write: dev # %d, block # %d, count %d ... ",
			curr_device, blk, cnt);

	mmc_init(mmc);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return -EPERM;
	}

	n = mmc->block_dev.block_write(curr_device, blk,
			cnt, addr);
	//debug_cond(BURNNER_DEBUG,"%d blocks write: %s\n",n, (n == cnt) ? "OK" : "ERROR");
	printf("%d blocks write: %s\n",n, (n == cnt) ? "OK" : "ERROR");

	if (n != cnt)
		return -EIO;

	if (cloner->args->write_back_chk) {
		mmc->block_dev.block_read(curr_device, blk,
				cnt, addr);
		debug_cond(BURNNER_DEBUG,"%d blocks read: %s\n",n, (n == cnt) ? "OK" : "ERROR");
		if (n != cnt)
			return -EIO;

		uint32_t tmp_crc = local_crc32(0xffffffff,addr,cloner->cmd->write.length);
		debug_cond(BURNNER_DEBUG,"%d blocks check: %s\n",n,(cloner->cmd->write.crc == tmp_crc) ? "OK" : "ERROR");
		if (cloner->cmd->write.crc != tmp_crc) {
			printf("src_crc32 = %08x , dst_crc32 = %08x\n",cloner->cmd->write.crc,tmp_crc);
			return -EIO;
		}
	}
	return 0;
}

int efuse_program(struct cloner *cloner)
{
	static int enabled = 0;
	if(!enabled) {
		efuse_init(cloner->args->efuse_gpio);
		enabled = 1;
	}
	u32 partation = cloner->cmd->write.partation;
	u32 length = cloner->cmd->write.length;
	void *addr = (void *)cloner->write_req->buf;
	u32 r = 0;

	if (r = efuse_write(addr, length, partation)) {
		printf("efuse write error\n");
		return r;
	}
	return r;
}

int spi_program(struct cloner *cloner)
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	u32 offset = cloner->cmd->write.partation + cloner->cmd->write.offset;
	u32 length = cloner->cmd->write.length;
	int blk_cnt = cloner->args->spi_erase_range.blockcount;
	int blk_size = cloner->args->spi_erase_range.blocksize;
	void *addr = (void *)cloner->write_req->buf;
	struct spi_args *spi_arg = &cloner->args->spi_args;
	unsigned int ret;
	int len = 0;
	struct spi_flash *flash;
	spi.enable = spi_arg->enable;
	spi.clk   = spi_arg->clk;
	spi.data_in  = spi_arg->data_in;
	spi.data_out  = spi_arg->data_out;
	spi.rate  = spi_arg->rate * 1000000;


#ifdef CONFIG_JZ_SPI
	spi_init();
#endif
#ifdef CONFIG_INGENIC_SOFT_SPI
	spi_init_jz(&spi);
#endif


	if(flash == NULL){
		flash = spi_flash_probe(bus, cs, spi.rate, mode);
		if (!flash) {
			printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
			return 1;
		}
	}

	debug("the offset = %x\n",offset);
	debug("the length = %x\n",length);


	if (length%blk_size == 0){
		len = length;
		printf("the length = %x\n",length);
	}
	else{
		printf("the length = %x, is no enough %x\n",length,blk_size);
		len = (length/blk_size)*blk_size + blk_size;
	}
	if (cloner->args->spi_erase == SPI_NO_ERASE) {
		ret = spi_flash_erase(flash, offset, len);
		printf("SF: %zu bytes @ %#x Erased: %s\n", (size_t)len, (u32)offset,
				ret ? "ERROR" : "OK");
	}
	ret = spi_flash_write(flash, offset, len, addr);
	printf("SF: %zu bytes @ %#x write: %s\n", (size_t)len, (u32)offset,
			ret ? "ERROR" : "OK");

#if debug
	int buf_debug[8*1024*1024];
	if (spi_flash_read(flash, 1024, /*len*/2048, buf_debug)) {
		printf("read failed\n");
		return -1;
	}
	int i = 0;
	for(i=0;i<4096;i++){
		printf("the debug[%d] = %x\n",i,buf_debug[i]);
	}

#endif
	return 0;
}

void handle_read(struct usb_ep *ep,struct usb_request *req)
{
}

void handle_write(struct usb_ep *ep,struct usb_request *req)
{
	struct cloner *cloner = req->context;

	if(req->status == -ECONNRESET) {
		cloner->ack = -ECONNRESET;
		return;
	}

	if (req->actual != req->length) {
		printf("write transfer length is errr,actual=%08x,length=%08x\n",req->actual,req->length);
		cloner->ack = -EIO;
		return;
	}

	if(cloner->cmd_type == VR_UPDATE_CFG) {
		cloner->ack = 0;
		printf("nand_erase:%d\n",cloner->args->nand_erase);
		printf("mmc_erase:%d\n",cloner->args->mmc_erase);
		printf("mmc_open_card:%d\n",cloner->args->mmc_open_card);
		return;
	}

	if (cloner->args->transfer_data_chk) {
		uint32_t tmp_crc = local_crc32(0xffffffff,req->buf,req->actual);
		if (cloner->cmd->write.crc != tmp_crc) {
			printf("crc is errr! src crc=%08x crc=%08x\n",cloner->cmd->write.crc,tmp_crc);
			cloner->ack = -EINVAL;
			return;
		}
	}
#define OPS(x,y) ((x<<16)|(y&0xffff))
	switch(cloner->cmd->write.ops) {
		case OPS(I2C,RAW):
			cloner->ack = i2c_program(cloner);
			break;
		case OPS(NAND,IMAGE):
			cloner->ack = nand_program(cloner);
			break;
		case OPS(MMC,0):
		case OPS(MMC,1):
		case OPS(MMC,2):
			cloner->ack = mmc_program(cloner,cloner->cmd->write.ops & 0xffff);
			break;
		case OPS(MEMORY,RAW):
			cloner->ack = 0;
			break;
		case OPS(EFUSE,RAW):
			cloner->ack = efuse_program(cloner);
			break;
		case OPS(REGISTER,RAW):
			{
				volatile unsigned int *tmp = cloner->cmd->write.partation;
				if(tmp > 0xb0000000 && tmp < 0xb8000000) {
					*tmp = *((int*)cloner->write_req->buf);
					cloner->ack = 0;
				} else {
					printf("OPS(REGISTER,RAW): not supported address.");
					cloner->ack = -ENODEV;
				}
			}
			break;
		case OPS(SPI,RAW):
			cloner->ack = spi_program(cloner);
			break;
		default:
			printf("ops %08x not support yet.\n",cloner->cmd->write.ops);
	}
#undef OPS
}

#ifdef CONFIG_FPGA
extern int do_udc_reset(void);
#endif
void handle_cmd(struct usb_ep *ep,struct usb_request *req)
{
	struct cloner *cloner = req->context;
	if(req->status == -ECONNRESET) {
		cloner->ack = -ECONNRESET;
		return;
	}

	if (req->actual != req->length) {
		printf("cmd transfer length is err req->actual = %d, req->length = %d\n",
				req->actual,req->length);
		cloner->ack = -EIO;
		return;
	}

	union cmd *cmd = req->buf;
	debug_cond(BURNNER_DEBUG,"handle_cmd type=%x\n",cloner->cmd_type);
	switch(cloner->cmd_type) {
		case VR_UPDATE_CFG:
			cloner->args_req->length = cmd->update.length;
			usb_ep_queue(cloner->ep_out, cloner->args_req, 0);
			break;
		case VR_WRITE:
			if(cloner->buf_size < cmd->write.length) {
				cloner->buf_size = cmd->write.length;
				cloner->write_req->buf = realloc(cloner->write_req->buf,cloner->buf_size);
			}
			cloner->write_req->length = cmd->write.length;
			usb_ep_queue(cloner->ep_out, cloner->write_req, 0);
			break;
		case VR_INIT:
			if(!cloner->inited) {
				cloner->ack = -EBUSY;
				cloner_init(cloner);
				cloner->inited = 1;
				cloner->ack = 0;
			}
		case VR_READ:
			break;
		case VR_SYNC_TIME:
			cloner->ack = rtc_set(&cloner->cmd->rtc);
			break;
		case VR_GET_ACK:
		case VR_GET_CPU_INFO:
			break;
		case VR_REBOOT:
#ifdef CONFIG_FPGA
			mdelay(1000);
			do_udc_reset();
			mdelay(10000);
#endif
			do_reset(NULL,0,0,NULL);
			break;
	}
}

int f_cloner_setup_handle(struct usb_function *f,
		const struct usb_ctrlrequest *ctlreq)
{
	struct cloner *cloner = f->config->cdev->req->context;
	struct usb_request *req = cloner->ep0req;

	debug_cond(BURNNER_DEBUG,"vendor bRequestType %x,bRequest %x wLength %d\n",
			ctlreq->bRequestType,
			ctlreq->bRequest,
			ctlreq->wLength);

	if ((ctlreq->bRequestType & USB_TYPE_MASK) != USB_TYPE_VENDOR) {
		printf("Unkown RequestType 0x%x \n",ctlreq->bRequestType);
		cloner->ack = -ENOSYS;
		return -ENOSYS;
	}

	usb_ep_dequeue(cloner->ep0, cloner->ep0req);
	usb_ep_dequeue(cloner->ep_in, cloner->read_req);
	usb_ep_dequeue(cloner->ep_out, cloner->write_req);

	cloner->cmd_type = ctlreq->bRequest;
	req->length = ctlreq->wLength;
	req->complete = handle_cmd;

	switch (ctlreq->bRequest) {
		case VR_GET_CPU_INFO:
			strcpy(cloner->ep0req->buf,"BOOT47XX");
			break;
		case VR_GET_ACK:
			memcpy(cloner->ep0req->buf,&cloner->ack,sizeof(int));
			break;
		case VR_INIT:
			break;
		case VR_UPDATE_CFG:
		case VR_WRITE:
			cloner->ack = -EBUSY;
	}

	return usb_ep_queue(cloner->ep0, cloner->ep0req, 0);
}

int f_cloner_bind(struct usb_configuration *c,
		struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct cloner *cloner = func_to_cloner(f);

	debug_cond(BURNNER_DEBUG,"f_cloner_bind\n");

	intf_desc.bInterfaceNumber = usb_interface_id(c, f);
	if(intf_desc.bInterfaceNumber < 0 )
		return intf_desc.bInterfaceNumber;

	cloner->ep0 = cdev->gadget->ep0;
	cloner->ep0req = cdev->req;
	cloner->gadget = cdev->gadget;
	cloner->ack = 0;
	cloner->cdev = cdev;

	cloner->cmd = (union cmd *)cloner->ep0req->buf;

	if (gadget_is_dualspeed(cdev->gadget)) {
		hs_bulk_in_desc.bEndpointAddress =
			fs_bulk_in_desc.bEndpointAddress;
		hs_bulk_out_desc.bEndpointAddress =
			fs_bulk_out_desc.bEndpointAddress;
	}

	cdev->req->context = cloner;

	cloner->ep_in = usb_ep_autoconfig(cdev->gadget, &fs_bulk_in_desc);
	cloner->ep_out = usb_ep_autoconfig(cdev->gadget, &fs_bulk_out_desc);

	cloner->write_req = usb_ep_alloc_request(cloner->ep_out,0);
	cloner->args_req = usb_ep_alloc_request(cloner->ep_out,0);
	cloner->read_req = usb_ep_alloc_request(cloner->ep_in,0);

	cloner->buf_size = 1024*1024;
	cloner->write_req->complete = handle_write;
	cloner->write_req->buf = malloc(1024*1024);
	cloner->write_req->length = 1024*1024;
	cloner->write_req->context = cloner;

	cloner->args_req->complete = handle_write;
	cloner->args_req->buf = cloner->args;
	cloner->args_req->length = ARGS_LEN;
	cloner->args_req->context = cloner;

	cloner->read_req->complete = handle_read;
	cloner->read_req->buf = malloc(1024*1024);
	cloner->read_req->length = 1024*1024;
	cloner->read_req->context = cloner;

	return 0;
}

int f_cloner_set_alt(struct usb_function *f,
		unsigned interface, unsigned alt)
{
	struct cloner *cloner = func_to_cloner(f);
	const struct usb_endpoint_descriptor *epin_desc,*epout_desc;
	int status = 0;

	debug_cond(BURNNER_DEBUG,"set interface %d alt %d\n",interface,alt);
	epin_desc = ep_choose(cloner->gadget,&hs_bulk_in_desc,&fs_bulk_in_desc);
	epout_desc = ep_choose(cloner->gadget,&hs_bulk_out_desc,&fs_bulk_out_desc);

	status += usb_ep_enable(cloner->ep_in,epin_desc);
	status += usb_ep_enable(cloner->ep_out,epout_desc);

	if (status < 0) {
		printf("usb enable ep in failed\n");
		goto failed;
	}

	cloner->ep_in->driver_data = cloner;
	cloner->ep_out->driver_data = cloner;
failed:
	return status;
}

void f_cloner_unbind(struct usb_configuration *c,struct usb_function *f)
{
}

void f_cloner_disable(struct usb_function *f)
{
	struct cloner *cloner = func_to_cloner(f);
	int status = 0;
	status += usb_ep_disable(cloner->ep_in);
	status += usb_ep_disable(cloner->ep_out);
	if (status < 0)
		printf("usb disable ep failed");
	return;
}

int cloner_function_bind_config(struct usb_configuration *c)
{
	int status = 0;
	struct cloner *cloner = calloc(sizeof(struct cloner),1);

	if (!cloner)
		return -ENOMEM;

	cloner->usb_function.name = "vendor burnner interface";
	cloner->usb_function.bind = f_cloner_bind;
	cloner->usb_function.hs_descriptors = hs_intf_descs;
	cloner->usb_function.descriptors = fs_intf_descs;
	cloner->usb_function.set_alt = f_cloner_set_alt;
	cloner->usb_function.setup = f_cloner_setup_handle;
	cloner->usb_function.strings= burn_intf_string_tab;
	cloner->usb_function.disable = f_cloner_disable;
	cloner->usb_function.unbind = f_cloner_unbind;

	cloner->args = malloc(ARGS_LEN);
	cloner->args->transfer_data_chk = 1;
	cloner->args->write_back_chk = 1;

	cloner->inited = 0;

	INIT_LIST_HEAD(&cloner->usb_function.list);
	bitmap_zero(cloner->usb_function.endpoints,32);

	status =  usb_add_function(c,&cloner->usb_function);
	if (status)
		free(cloner);
	return status;
}

int jz_cloner_add(struct usb_configuration *c)
{
	int id;

	id = usb_string_id(c->cdev);
	if (id < 0)
		return id;
	burner_intf_string_defs[0].id = id;
	intf_desc.iInterface = id;

	debug_cond(BURNNER_DEBUG,"%s: cdev: 0x%p gadget:0x%p gadget->ep0: 0x%p\n", __func__,
			c->cdev, c->cdev->gadget, c->cdev->gadget->ep0);

	return cloner_function_bind_config(c);
}

