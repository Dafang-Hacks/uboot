/*
 *Ingenic mensa boot android system command
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Martin <czhu@ingenic.cn>
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

#include <stdarg.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <errno.h>
#include <div64.h>
#include <common.h>
#include <command.h>
#include <config.h>
#include <mmc.h>
#include <boot_img.h>
#include <asm/gpio.h>
#include <asm/arch/mmc.h>

#define BULK_OUT_BUF_SIZE 0x4000//0x21000      //buffer size :
#define BULK_IN_BUF_SIZE 0x4000//0x21000       // too

struct boot_img_hdr bootimginfo;
struct mmc *mmc;

extern void flush_cache_all(void);
extern void setup_tlb(void);


/*change unsigned int to string*/
void uint2str(unsigned int pword, unsigned char* str)
{
	char alpha[] = "0123456789ABCDEF";
	unsigned int i;

	for (i = 0; i < 8; i++, pword <<= 4) {
		str[i] = (alpha[pword >> 28]);
        }
	str[i] = 0;
}

#ifdef CONFIG_SPL_MMC_SUPPORT
/*find a mmc device and init */
void mmc_ready(unsigned int mmc_select)
{
	int err;
	mmc = find_mmc_device(mmc_select);
	if (!mmc) {
		printf("UBoot: mmc device not found!!\n");
		hang();
	}

	err = mmc_init(mmc);
	if (err) {
		printf("UBoot: mmc init failed: err - %d\n", err);
		hang();
	}
	printf("mmc ready done!\n");
}
#else/* CONFIG_SPL_MMC_SUPPORT */
/*find a nand device and init */
void nand_ready(void)
{
	return;
}
#endif/* !CONFIG_SPL_MMC_SUPPORT */

/* convert a boot_image at kernel_addr into a kernel + ramdisk + tags */
int ready_for_jump(unsigned char* data_buf, unsigned int data_size)
{
	int i;
	unsigned kernel_actual;
	unsigned page_mask;
	char initrd_param[64];
	unsigned int pos;
	unsigned long dsize = 0;

	unsigned char *bulk_data_buf;
	static unsigned long bulk_data_size = 0;

	static u32      *param_addr = 0;
	static u8       *tmpbuf = 0;
#ifndef CONFIG_BURNER
	static u8       cmdline[256] = CONFIG_BOOTARGS;
#else
	static u8       cmdline[256] = {0,};
#endif

	if (data_buf == NULL) {
		return -EINVAL;
	}
	bulk_data_buf = data_buf;
	if (data_size != 0) {
		dsize = data_size;
	} else {
		dsize = bulk_data_size;
	}

	memcpy(&bootimginfo, bulk_data_buf, 2048);

	for (i = 0; i < dsize; i++) {
		bulk_data_buf[i] = bulk_data_buf[2048 + i];
	}

	page_mask = bootimginfo.page_size - 1;
	kernel_actual = (bootimginfo.kernel_size + page_mask) & (~page_mask);
	bootimginfo.kernel_addr = (unsigned int)bulk_data_buf;
	bootimginfo.ramdisk_addr = bootimginfo.kernel_addr + kernel_actual;

	memcpy((u8 *)CONFIG_RAMDISK_DST, (char *)bootimginfo.ramdisk_addr, bootimginfo.ramdisk_size);

	/* Prepare kernel parameters and environment */
	param_addr = (u32 *)CONFIG_PARAM_BASE;
	/* might be address of ascii-z string: "memsize" */
	param_addr[0] = 0;
	/* might be address of ascii-z string: "0x01000000" */
	param_addr[1] = 0;
	param_addr[2] = 0;
	param_addr[3] = 0;
	param_addr[4] = 0;
	param_addr[5] = CONFIG_PARAM_BASE + 32;
	param_addr[6] = (u32)data_buf;
	tmpbuf = (u8 *)(CONFIG_PARAM_BASE + 32);

	memset(initrd_param, 0, 40);
	strcpy((char *)initrd_param, " rd_start=0x");

	pos = strlen(initrd_param);
	uint2str(CONFIG_RAMDISK_DST, (unsigned char *)(initrd_param + pos));
	pos = strlen(initrd_param);

	strcpy((char *)(initrd_param + pos), " rd_size=0x");
	pos = strlen(initrd_param);
	uint2str(bootimginfo.ramdisk_size, (unsigned char *)(initrd_param + pos));

	pos = strlen((char *)cmdline);
	strcpy((char *)(cmdline + pos), initrd_param);

	for (i = 0; i < 256; i++)
		tmpbuf[i] = cmdline[i];

	printf("cmdline: %s\n",(char *)cmdline);

	return 0;
}

/*boot.img has been in memory already. just call init_boot_linux() and jump to kernel.*/
void jump_kernel(unsigned long mem_address,unsigned long size)
{
	void (*kernel)(int, char **, char *);

	if (ready_for_jump((unsigned char*)mem_address, size) == 0) {
		printf("Jump to kernel start Addr 0x%lx\n\n",mem_address);
		kernel = (void (*)(int, char **, char *))mem_address;
		flush_cache_all();
		/*Jump to kernel image*/
		(*kernel)(2, (char **)(CONFIG_PARAM_BASE + 16), (char *)CONFIG_PARAM_BASE);
		printf("We should not come here ... \n");
	}
}

/* boot the android system form the memory directly.*/
int mem_boot (unsigned int mem_address)
{
	struct boot_img_hdr *bootimginfo;
	int kernel_actual,ramdisk_actual,size;
	unsigned int page_mask;
	unsigned int page_size;

	printf("Enter mem_boot routine ...\n");

	if (2048 < sizeof(struct boot_img_hdr)) {
		printf("size too small!\n");
        }

	bootimginfo = (struct boot_img_hdr *)mem_address;

	if (memcmp(bootimginfo->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE))
		return -1;

	page_size = bootimginfo->page_size;
	page_mask = page_size - 1;

	kernel_actual = (bootimginfo->kernel_size + page_mask) & (~page_mask);
	ramdisk_actual = (bootimginfo->ramdisk_size + page_mask) & (~page_mask);
	size = kernel_actual + ramdisk_actual + page_size;
	printf("Prepare kernel parameters ...\n");
	jump_kernel(mem_address,size);
	return 0;
}

/* load .img file form mmc,analyze the header of boot.img and then jump to the kernel*/
#ifdef CONFIG_SPL_MMC_SUPPORT
void msc_boot(unsigned int mmc_select,unsigned int mem_address,unsigned int sectors)
{
	struct boot_img_hdr *bootimginfo;
	int kernel_actual;
	int ramdisk_actual;
	unsigned int page_mask;
	unsigned int page_size;
        unsigned int offset, boffset, bsize, size;

	printf("Enter msc_boot routine ...\n");
        offset = sectors*512;
	/*512 Bytes =1 block*/
	boffset = (offset%512)?(offset/512+1):(offset/512);
	/* Load header*/
	mmc->block_dev.block_read(mmc_select, boffset, 4, (void *)mem_address);

	if(2048 < sizeof(struct boot_img_hdr)){
		printf("size too small!\n");
	}

	bootimginfo = (struct boot_img_hdr *)mem_address;

	if (memcmp(bootimginfo->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE))
		return;

	page_size = bootimginfo->page_size;
	page_mask = page_size - 1;

	kernel_actual = (bootimginfo->kernel_size + page_mask) & (~page_mask);
	ramdisk_actual = (bootimginfo->ramdisk_size + page_mask) & (~page_mask);
	size = kernel_actual + ramdisk_actual + page_size;
	bsize = (size % 512)?(size/512+1):(size/512);

	printf("Load offset = 0x%x\n",offset);
	printf("Load size   = 0x%x\n",size);
	printf("Load kernel from MSC ...\n");

	/* Load kernel and ramdisk */
	mmc->block_dev.block_read(mmc_select, boffset+4, bsize, (void*)(mem_address+2048));
	printf("Load kernel and ramdisk done!\n");

	printf("Prepare kernel parameters ...\n");
	jump_kernel(mem_address,size);
}
#else/* CONFIG_SPL_MMC_SUPPORT */
/* load .img flie form nand,analyze the header of boot.img and then jump to the kernel*/
void nand_boot(unsigned int mem_address,unsigned int sectors)
{

}
#endif/* !CONFIG_SPL_MMC_SUPPORT */

static int do_boota(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long mmc_select,mem_address,sectors;
	/* Consume 'boota' */
        argc--; argv++;
	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp("mem",argv[0])) {
		mem_address=simple_strtoul(argv[1], NULL, 16);
		printf("mem boot start\n");
		mem_boot(mem_address);
		printf("mem boot error\n");
	} else if (!strcmp("mmc",argv[0])) {
		mmc_select=simple_strtoul(argv[1], NULL, 10);
		mem_address=simple_strtoul(argv[2], NULL, 16);
		sectors=simple_strtoul(argv[3], NULL, 10);

#ifdef CONFIG_SPL_MMC_SUPPORT
		printf("MSC boot start\n");
		mmc_ready(mmc_select);
		msc_boot(mmc_select,mem_address,sectors);
		printf("MSC boot error\n");
		return 0;
#else /*!CONFIG_SPL_MMC_SUPPORT */
	} else if (!strcmp("nand",argv[0])) {
		mem_address=simple_strtoul(argv[1], NULL, 16);
		sectors=simple_strtoul(argv[2], NULL, 10);
		printf("Nand boot start\n");
		nand_ready();
		nand_boot(mem_address,sectors);
		printf("Nand boot error\n");
		return 0;
#endif/*!CONFIG_SPL_MMC_SUPPORT*/
	} else {
		printf("%s boot unsupport\n", argv[0]);
                return CMD_RET_USAGE;
	}
	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char boota_help_text[] =
        "[[way],[mmc_select],[mem_address],[sectors]]\n"
        "- boot Android system....\n"
        "\tThe argument [way] means the way of booting boot.img.[way]='mem'/'mmc'/'nand'.\n"
        "\tThe argument [mmc_select] means the channel of mmc.[mmc_select]='0'/'1'/'2'.\n"
        "\tThe argument [mem_address] means the start position of boot.img in memory.\n"
        "\tThe argument [sectors] means the position of boot.img in MMC/NAND.\n"
        "";
#endif

U_BOOT_CMD(
	boota, 5, 1, do_boota,
	"boot android system",boota_help_text
);
