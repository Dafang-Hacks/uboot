/*
 * #endif
 * Ingenic burner function Code (Vendor Private Protocol)
 *
 * Copyright (c) 2014 jykang <jykang@ingenic.cn>
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
#include <efuse.h>
#include <ingenic_soft_i2c.h>
#include <ingenic_nand_mgr/nand_param.h>
#include <linux/compiler.h>
#include <asm/arch/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <config.h>
#include <serial.h>

#include <asm/jz_uart.h>
#include <asm/arch/base.h>


char *ptemp[16];
char stateok[32]="state ok\n";
char *data_addr;
char getdata[1024*1024]={0};
char serial_buf[1024] = "";
int write_back_chk;

DECLARE_GLOBAL_DATA_PTR;

static struct jz_uart *uart __attribute__ ((section(".data")));

#define BURNNER_DEBUG 1

#define MMC_ERASE_ALL	1
#define MMC_ERASE_PART	2
#define MMC_ERASE_CNT_MAX	10

extern enum medium_type {
	MEMORY = 0,
	       NAND,
	       MMC,
	       I2C,
	       EFUSE,
	       REGISTER
};

extern enum data_type {
	RAW = 0,
	    OOB,
	    IMAGE,
};

extern struct i2c_args {
	int clk;
	int data;
	int device;
	int value_count;
	int value[0];
};

extern struct mmc_erase_range {
	uint32_t start;
	uint32_t end;
};

extern struct arguments {
	int efuse_gpio;
	int use_nand_mgr;
	int use_mmc;

	int nand_erase;
	int nand_erase_count;
	unsigned int offsets[32];

	int mmc_open_card;
	int mmc_erase;
	uint32_t mmc_erase_range_count;
	struct mmc_erase_range mmc_erase_range[MMC_ERASE_CNT_MAX];

	int transfer_data_chk;
	int write_back_chk;

	PartitionInfo PartInfo;
	int nr_nand_args;
	nand_flash_param nand_params[0];
};

extern union cmd {
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

struct serial_cloner {
	union cmd *cmd;
	int cmd_type;
	uint32_t buf_size;
	int ack;
	struct arguments *args;
	int inited;
};

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

static void local_serial_setbrg(void)
{
	u32 baud_div, tmp;

	baud_div = CONFIG_SYS_EXTAL / 16 / CONFIG_BAUDRATE;

	tmp = readb(&uart->lcr);
	tmp |= UART_LCR_DLAB;
	writeb(tmp, &uart->lcr);

	writeb((baud_div >> 8) & 0xff, &uart->dlhr_ier);
	writeb(baud_div & 0xff, &uart->rbr_thr_dllr);

	tmp &= ~UART_LCR_DLAB;
	writeb(tmp, &uart->lcr);
}

static int local_serial_init(void)
{
	uart = (struct jz_uart *)(UART0_BASE + 3 * 0x1000);

	/* Disable port interrupts while changing hardware */
	writeb(0, &uart->dlhr_ier);

	/* Disable UART unit function */
	writeb(~UART_FCR_UUE, &uart->iir_fcr);

	/* Set both receiver and transmitter in UART mode (not SIR) */
	writeb(~(SIRCR_RSIRE | SIRCR_TSIRE), &uart->isr);

	/*
	 * Set databits, stopbits and parity.
	 * (8-bit data, 1 stopbit, no parity)
	 */
	writeb(UART_LCR_WLEN_8 | UART_LCR_STOP_1, &uart->lcr);

	/* Set baud rate */
	local_serial_setbrg();

	/* Enable UART unit, enable and clear FIFO */
	writeb(UART_FCR_UUE | UART_FCR_FE | UART_FCR_TFLS | UART_FCR_RFLS,
	       &uart->iir_fcr);

	return 0;
}


static int local_serial_tstc(void)
{
	if (readb(&uart->lsr) & UART_LSR_DR)
		return 1;

	return 0;
}

static void local_serial_putc(const char c)
{
	writeb((u8)c, &uart->rbr_thr_dllr);

	/* Wait for fifo to shift out some bytes */
	while (!((readb(&uart->lsr) & (UART_LSR_TDRQ | UART_LSR_TEMT)) == 0x60))		;
}

static void local_serial_puts(const char *s)
{
	while(*s)
		local_serial_putc(*s++);
}

static int local_serial_getc(void)
{
	while (!local_serial_tstc())
		;

	return readb(&uart->rbr_thr_dllr);
}

static void local_serial_gpio_init()
{
	gpio_set_func(GPIO_PORT_A,GPIO_FUNC_1 ,1<<31 );
}

static void local_serial_clk_init()
{
	unsigned int clkgr = cpm_inl(CPM_CLKGR);
	clkgr &= ~(0xfff<<12);
	cpm_outl(clkgr, CPM_CLKGR);
}

static int atoi(char *pstr)
{
	int value = 0;
	int sign = 1;
	int radix;

	if(*pstr == '-'){
		sign = -1;
		pstr++;
	}
	if(*pstr == '0' && (*(pstr+1) == 'x' || *(pstr+1) == 'X')){
		radix = 16;
		pstr += 2;
	}
	else
		radix = 10;
	while(*pstr){
		if(radix == 16){
			if(*pstr >= '0' && *pstr <= '9')
				value = value * radix + *pstr - '0';
			else if(*pstr >= 'A' && *pstr <= 'F')
				value = value * radix + *pstr - 'A' + 10;
			else if(*pstr >= 'a' && *pstr <= 'f')
				value = value * radix + *pstr - 'a' + 10;
		}
		else
			value = value * radix + *pstr - '0';
		pstr++;
	}
	return sign*value;
}

static uint32_t local_crc32(uint32_t crc,unsigned char *buffer, uint32_t size)
{
	uint32_t i;
	for (i = 0; i < size; i++) {
		crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
	}
	return crc ;
}

static int i2c_program_serial(struct serial_cloner *serial_cloner)
{
	int i = 0;
	struct i2c_args *i2c_arg = (struct i2c_args *)data_addr;
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
static int mmc_erase(struct serial_cloner *serial_cloner)
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

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return -EPERM;
	}
	if (serial_cloner->args->mmc_erase == MMC_ERASE_ALL) {
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
	} else if (serial_cloner->args->mmc_erase != MMC_ERASE_PART) {
		return -EINVAL;
	}

	/*mmc part erase */
	erase_cnt = (serial_cloner->args->mmc_erase_range_count >MMC_ERASE_CNT_MAX) ?
		MMC_ERASE_CNT_MAX : serial_cloner->args->mmc_erase_range_count;

	for (i = 0; erase_cnt > 0; i++, erase_cnt--) {
		blk = serial_cloner->args->mmc_erase_range[i].start / MMC_BYTE_PER_BLOCK;
		blk_end = serial_cloner->args->mmc_erase_range[i].end / MMC_BYTE_PER_BLOCK;
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

static int cloner_init_serial(struct serial_cloner *serial_cloner)
{
	if(serial_cloner->args->use_nand_mgr) {
#ifdef CONFIG_JZ_NAND_MGR
		nand_probe_burner(&(serial_cloner->args->PartInfo),
				&(serial_cloner->args->nand_params[0]),
				serial_cloner->args->nr_nand_args,
				serial_cloner->args->nand_erase,serial_cloner->args->offsets,serial_cloner->args->nand_erase_count);
#endif
	}

	if (serial_cloner->args->use_mmc) {
		if (serial_cloner->args->mmc_erase) {
			mmc_erase(serial_cloner);
		}
	}
}

static int nand_program_serial(struct serial_cloner *serial_cloner)
{
#ifdef CONFIG_JZ_NAND_MGR
	int curr_device = 0;
	u32 startaddr = atoi(ptemp[4]) + atoi(ptemp[6]);
	u32 length = atoi(ptemp[8]);
	void *databuf = (void *)data_addr;

	printf("=========++++++++++++>   NAND PROGRAM:startaddr = %d P offset = %d P length = %d \n",startaddr,length);
	do_nand_request(startaddr, databuf, length,atoi(ptemp[6]));

	return 0;
#else
	return -ENODEV;
#endif
}

static int mmc_program_serial(struct serial_cloner *serial_cloner,int mmc_index)
{
#define MMC_BYTE_PER_BLOCK 512
	int curr_device = 0;
	struct mmc *mmc = find_mmc_device(mmc_index);
	u32 blk = (atoi(ptemp[4]) + atoi(ptemp[6]))/MMC_BYTE_PER_BLOCK;
	u32 cnt = (atoi(ptemp[8]) + MMC_BYTE_PER_BLOCK - 1)/MMC_BYTE_PER_BLOCK;
	void *addr = (void *)data_addr;
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

//	if (serial_cloner->args->write_back_chk) {

	if(write_back_chk){
		mmc->block_dev.block_read(curr_device, blk,
				cnt, addr);
		debug_cond(BURNNER_DEBUG,"%d blocks read: %s\n",n, (n == cnt) ? "OK" : "ERROR");
		if (n != cnt)
			return -EIO;

	//	uint32_t tmp_crc = local_crc32(0xffffffff,addr,ptemp[8]);
		//debug_cond(BURNNER_DEBUG,"%d blocks check: %s\n",n,(serial_cloner->cmd->write.crc == tmp_crc) ? "OK" : "ERROR");
		//if (serial_cloner->cmd->write.crc != tmp_crc) {
		//	printf("src_crc32 = %08x , dst_crc32 = %08x\n",serial_cloner->cmd->write.crc,tmp_crc);
		//	return -EIO;
	//	}
	}
	return 0;
}

static int efuse_program_serial(struct serial_cloner *serial_cloner)
{
	static int enabled = 0;
	if(!enabled) {
		efuse_init(serial_cloner->args->efuse_gpio);
		enabled = 1;
	}
	u32 partation = atoi(ptemp[4]);
	u32 length = atoi(ptemp[8]);
	void *addr = (void *)data_addr;
	u32 r = 0;

	if (r = efuse_write(addr, length, partation)) {
		printf("efuse write error\n");
		return r;
	}
	return r;
}

static void serial_read_cmd(int *success,struct serial_cloner *serial_cloner)
{
	char *temp = ":,";
	char *p;
	int index = 0;
	printf("serial_read_cmd wb:%d\n",serial_cloner->args->write_back_chk);
	unsigned int length;
	memset(p,0,1024);
	printf("%s\n",strtok(serial_buf,temp));
	while((p = strtok(NULL, temp))){
		printf("%s\n", p);
		ptemp[index] = p;
		printf("ptemp[%d]:%s\n",index,ptemp[index]);
		index ++;
	}
	if(index < 2){
		if(!strncmp(ptemp[0],"get_info",8)){
		}else if(!strncmp(ptemp[0],"reset",5)){
		}
	}else{
		if(!strncmp(ptemp[0],"write",5)){
			if(!strncmp(ptemp[2],"memory",6)){
			}else if(!strncmp(ptemp[2],"cache",5)){
			}else if(!strncmp(ptemp[2],"nand",4)){
				printf("++++++++++++++++++++++==>>nand program\n");
				nand_program_serial(serial_cloner);
				printf(stateok);
				local_serial_puts(stateok);
			}else if(!strncmp(ptemp[2],"emmc",4)){
				mmc_program_serial(serial_cloner,0);
				printf(stateok);
				local_serial_puts(stateok);
				*success = 1;
			}else if(!strncmp(ptemp[2],"efuse",5)){
				efuse_program_serial(serial_cloner);
				printf(stateok);
				local_serial_puts(stateok);
			}
		}else if(!strncmp(ptemp[0],"read",4)){
			if(!strncmp(ptemp[2],"memery",6)){
			}else if(!strncmp(ptemp[2],"cache",5)){
			}else if(!strncmp(ptemp[2],"nand",4)){
			}else if(!strncmp(ptemp[2],"emmc",4)){
			}else if(!strncmp(ptemp[2],"efuse",5)){
			}
		}else if(!strncmp(ptemp[0],"run",3)){
		}else if(!strncmp(ptemp[0],"erase",5)){
			if(!strncmp(ptemp[2],"memery",6)){
			}else if(!strncmp(ptemp[2],"cache",5)){
			}else if(!strncmp(ptemp[2],"nand",4)){
			}else if(!strncmp(ptemp[2],"emmc",4)){
				mmc_erase(serial_cloner);
			}else if(!strncmp(ptemp[2],"efuse",5)){
			}
		}
	}
}

static int analysis_cmd(char *strcmd)
{
	char *temp = ":,";
	char *p;
	char *cmd_temp[16];
	int index = 0;
	unsigned int length;
	memset(p,0,1024);
	printf("a---%s\n",strtok(strcmd,temp));
	while((p = strtok(NULL, temp))){
		cmd_temp[index] = p;
		printf("a---cmd_temp[%d]:%s\n",index,cmd_temp[index]);
		index ++;
	}
	length = atoi(cmd_temp[8]);
	return length;
}

static int serial_receive_data(int *p,int *q)
{
	int index_cmd = 0;
	int index_data = 0;
	char cmd[1024];
	while(1){
		while(local_serial_tstc()){
			char cdata = local_serial_getc();
			cmd[index_cmd] = cdata;
			serial_putc(cmd[index_cmd]);
			index_cmd ++;
		}
		if(cmd[index_cmd-1] == '\n'){
			local_serial_puts(stateok);
			printf("cmdstatok\n");
			*p = 1;
			strcpy(serial_buf,cmd);
			break;
		}
	}
	int len = analysis_cmd(cmd);
	while(1){
		while(local_serial_tstc()){
			char cdata = local_serial_getc();
			getdata[index_data] = cdata;
			index_data ++;
		}


		if((index_data-1) == len){
			printf("index_data:%d\n",index_data);
			*q = 1;
			break;
		}
	}
	data_addr = getdata;
	return 0;
}

static char *serial_get_args(int *p,struct serial_cloner *serial_cloner)
{
	struct arguments *argument;
	argument = malloc(1024*1024);
	int index = 0;
	char *dataargs;
	memset(dataargs,0,1024*1024);
	while(1){
		while(local_serial_tstc()){
			char cdata = local_serial_getc();
			dataargs[index] = cdata;
			index ++;
		}
		argument = (struct arguments *)dataargs;
		if(argument->nr_nand_args){
			if(index >= sizeof(struct arguments)+(argument->nr_nand_args*sizeof(nand_flash_param))){
				printf("index_args:%d\n",index);
				break;
			}
		}
	}

	serial_cloner->args = argument;
	cloner_init_serial(serial_cloner);
	local_serial_puts(stateok);
	printf("args:%s\n",stateok);
	*p = 1;

	return dataargs;
}

int start_serial_cloner()
{
	int success = 0;
	int success_args = 0;
	int success_cmd = 0,success_data = 0;
	local_serial_gpio_init();
	local_serial_clk_init();
	local_serial_init();

	struct serial_cloner serial_cloner;

	struct serial_cloner *pser;

	serial_cloner.args = (struct arguments *)serial_get_args(&success_args,pser);//args

	write_back_chk = serial_cloner.args->write_back_chk;
	pser = &serial_cloner;
	while(1){
		serial_receive_data(&success_cmd,&success_data);//cmd,data
		serial_read_cmd(&success,pser);
	}
}
