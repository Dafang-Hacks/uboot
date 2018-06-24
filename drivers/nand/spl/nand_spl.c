#include <asm/arch/base.h>
#include <asm/arch/cpm.h>
#include <asm/arch/gpio.h>
#include <linux/types.h>
#include <asm/arch/clk.h>
#include <nand_info.h>
#include <nand_bch.h>
#include <nand_io.h>
#include <cpu_trans.h>
#include <transadaptor.h>
#include "nand_basic.h"

#define NAND_PARAM_BASE	 (0x80000000 + 512 *1024)
#define UBOOT_AUTO_MAX_SIZE (0x100000 * 10)

typedef struct __nand_io {
	nfi_base *base;
	void *dataport;
	void *cmdport;
	void *addrport;
	transadaptor trans;
	chip_info *cinfo;
#ifndef CONFIG_NAND_NFI
	const nand_timing_com *timing;
#endif
	unsigned int copy_context;
} nand_io;

typedef struct __nand_bch {
	bch_base *base;
	unsigned int eccbit;
	unsigned int eccsize;
	unsigned int eccbytes;
	transadaptor trans;
	unsigned int copy_context;
} nand_bch;

cpu_copy_info *cpinfo;
nand_bch *nandbch ;
bch_base *bchbase ;
nfi_base *nfibase;
nand_io *nandio;
chip_info *cinfo;

struct image_header;

static unsigned char *oob_space;
static unsigned char *data_space;

static unsigned int pagesize;
static unsigned int buswidth;
static unsigned int rowcycles;
static unsigned int eccbit;
static unsigned int blocksize;
static unsigned int oob_size;
static unsigned int ecc_block;
static unsigned int ecc_count;
static unsigned int parity_size;

static unsigned int spl_read_align = 0;

void (*ndd_ndelay) (unsigned long nsecs);
int (*ndd_printf)(const char *fmt, ...);
extern int printf(const char* fmt, ...);
static int nand_read_page(unsigned int pageaddr,unsigned char *data_buf,unsigned char *oob_buf);
static int nand_read_spl_page(int page_addr, unsigned char *data_buf, unsigned char *oob_buf,
		struct spl_basic_param *params, unsigned int offset,unsigned int bytes);
#define __raw_readl(reg)     \
	    *((volatile unsigned int *)(reg))
#define __raw_writel(value,reg)  \
	    *((volatile unsigned int *)(reg)) = (value)

static void test_delay(unsigned long nsecs)
{
	unsigned int loops = (unsigned int)(nsecs + 999) / 1000 ;
	udelay(loops + 1);
}

/*
 *   * nand_wait_ready - NAND Wait R/B
 */
static inline void nand_wait_ready(void)
{
	volatile unsigned int timeout = 200;

//#ifndef CONFIG_NAND_NFI // no define

#if 1

	while ((*((volatile unsigned int *)GPIO_PXPIN(0)) & 0x00100000) && timeout--);
	while (!((*(volatile unsigned int *)GPIO_PXPIN(0)) & 0x00100000));
#else

	while(!(*(volatile unsigned int *)(0xb341000c) & (1 << 16)) && (timeout--)); //NFBC
	if(timeout > 0)
		*(volatile unsigned int *)(0xb341000c) |= (1 << 16);
	else
		printf("-------- WARNING: wait rb timeout ---------\n");

#endif //no def  CONFIG_NAND_NFI
}

static int nfi_readl(int reg)
{
	void *iomem = nandio->base->iomem;

	return __raw_readl(iomem + reg);
}

static void nfi_writel(int reg, int val)
{
	void *iomem = nandio->base->iomem;

	__raw_writel((val), iomem + reg);
}

static int bch_readl(int reg)
{
	void *iomem = nandbch->base->iomem;

	return __raw_readl(iomem + reg);
}

static void bch_writel(int reg, int val)
{
	void *iomem = nandbch->base->iomem;

	__raw_writel((val), iomem + reg);
}

static void fill_nfi_base(void)
{
	nandio->base = nfibase;
	nandio->base->iomem = (void*)NEMC_BASE;
	nandio->base->cs_iomem[0] = 0xbb000000;
	nandio->base->readl = nfi_readl;
	nandio->base->writel = nfi_writel;
	nandio->cinfo = cinfo;
#ifndef CONFIG_NAND_NFI
	nandio->timing = 0x0;
#endif
	nandio->copy_context = (unsigned int)(cpinfo);

	nandio->trans.prepare_memcpy = cpu_prepare_memcpy;
	nandio->trans.finish_memcpy = cpu_finish_memcpy;
}

static void fill_bch_base(void)
{
	nandbch->base = bchbase;
	nandbch->base->iomem = (void*)BCH_BASE;
	nandbch->base->readl = bch_readl;
	nandbch->base->writel = bch_writel;
	nandbch->copy_context = (unsigned int)(cpinfo);

	nandbch->trans.prepare_memcpy = cpu_prepare_memcpy;
	nandbch->trans.finish_memcpy = cpu_finish_memcpy;
}

static int parse_flag(unsigned char *flag_buf)
{
	int cnt_55 = 0, cnt_aa = 0;
	int i;

	for (i = 0; i < 32; i++) {
		switch (flag_buf[i]) {
			case 0x55 :
				cnt_55++;
				break;
			case 0xaa :
				cnt_aa++;
				break;
			default :
				break;
		}
	}

	if ((cnt_55 - cnt_aa) > 7)
		return 0x55;
	else if ((cnt_aa - cnt_55) > 7)
		return 0xaa;
	else
		return 0;
}

#if (defined(CONFIG_JZ4775) || defined(CONFIG_NAND_NFI))
static inline void get_nand_buswidth(unsigned char *buswidth, unsigned char *flag_buf)
{
	int flag = parse_flag(flag_buf + BUSWIDTH_FLAG_OFFSET);
	if(flag == FLAG_BUSWIDTH_8BIT)
		*buswidth = 8;
	else if(flag == FLAG_BUSWIDTH_16BIT)
		*buswidth = 16;
	else
		*buswidth = -1;
}
#endif //endif CONFIG_JZ4775 || CONFIG_NAND_NFI

static inline void get_nand_nandtype(unsigned char *nandtype, unsigned char *flag_buf)
{
	int flag = parse_flag(flag_buf + NANDTYPE_FLAG_OFFSET);
	if(flag == FLAG_NANDTYPE_COMMON)
		*nandtype = 0;
	else if(flag == FLAG_NANDTYPE_TOGGLE)
		*nandtype = 1;
	else
		*nandtype = -1;
}
#ifndef CONFIG_NAND_NFI
static inline void get_nand_rowcycles(unsigned char *rowcycles, unsigned char *flag_buf)
{
	int flag = parse_flag(flag_buf + ROWCYCLE_FLAG_OFFSET);
	if(flag == FLAG_ROWCYCLE_3)
		*rowcycles = 3;
	else if(flag == FLAG_ROWCYCLE_2)
		*rowcycles = 2;
	else
		*rowcycles = -1;
}


static inline void get_nand_pagesize(unsigned int *pagesize, unsigned char *flag_buf)
{
	int pagesize_flag2 = parse_flag(flag_buf + PAGESIZE_FLAG2_OFFSET);
	int pagesize_flag1 = parse_flag(flag_buf + PAGESIZE_FLAG1_OFFSET);
	int pagesize_flag0 = parse_flag(flag_buf + PAGESIZE_FLAG0_OFFSET);

	switch(pagesize_flag2 << 16 | pagesize_flag1 << 8 | pagesize_flag0)
	{
		case FLAG_PAGESIZE_512:
			*pagesize = 512;
			break;
		case FLAG_PAGESIZE_2K:
			*pagesize = 2048;
			break;
		case FLAG_PAGESIZE_4K:
			*pagesize = 4096;
			break;
		case FLAG_PAGESIZE_8K:
			*pagesize = 8192;
			break;
		case FLAG_PAGESIZE_16K:
			*pagesize = 16384;
			break;
		default:
			*pagesize = -1;
	}
}
#endif //no define CONFIG_NAND_NFI

#ifdef CONFIG_NAND_NFI
static inline void get_nand_otherparam(struct spl_basic_param *param,unsigned char *flag_buf)
{
	struct nand_otherflag *otherparms = (struct nand_otherflag *)(flag_buf + NAND_OTHER_FLAG_OFFSET);

	param->rowcycles = otherparms->rowcycle;
	param->pagesize = otherparms->pagesize;
}
#endif //CONFIG_NAND_NFI

/* Bootrom had read the first 256 bytes of spl, and put the nandinfo */
/* in address 0xf4000800. Now we read out them, and parse the nandinfo*/
static inline void get_nand_spl_basic_param(struct spl_basic_param *param)
{
#ifdef CONFIG_NAND_NFI // nfi
	unsigned char *spl_flag = (unsigned char *)0x80001000;
#else // nemc
	unsigned char *spl_flag = (unsigned char *)0xf4000800;
#endif

#ifndef CONFIG_NAND_NFI // no define

#ifdef CONFIG_JZ4775
	get_nand_buswidth(&param->buswidth, spl_flag);
#endif
	get_nand_nandtype(&param->nandtype, spl_flag);
	get_nand_rowcycles(&param->rowcycles, spl_flag);
	get_nand_pagesize(&param->pagesize, spl_flag);

#else //nfi

	get_nand_buswidth(&param->buswidth, spl_flag);

	get_nand_nandtype(&param->nandtype, spl_flag);
	get_nand_otherparam(param,spl_flag);
#endif //CONFIG_NAND_NFI
}

static void spl_bch_init(int ecc_leavel,int eccsize)
{
	nandbch->eccsize = eccsize;
}

static void send_read_start_cmd(unsigned int page_addr,unsigned int offset,int delay)
{

	if ((buswidth == 8)&&(offset == 512)){
		nand_io_send_cmd((int)(nandio),NAND_CMD_READ_OOB_512,delay);
		nand_io_send_addr((int)(nandio),0,page_addr,delay);
	}else{
		nand_io_send_cmd((int)(nandio),NAND_CMD_READ0,delay);
		nand_io_send_addr((int)(nandio),offset,page_addr,delay);
	}
	if(pagesize != 512)
		nand_io_send_cmd((int)(nandio),NAND_CMD_READSTART,delay);

	nand_wait_ready();
}

static void dump_data(unsigned char *buf,unsigned int len)
{
	int k;
	for(k=0;k<len/4;k++){
		if(!(k % 8))
			printf("\n");
		printf("%x ",((unsigned int *)buf)[k]);
	}
}

static void dump_nand_common_params(nand_sharing_params *nandparams)
{
	printf("id = %x\n",nandparams->nandinfo.id);
	printf("extid = %x\n",nandparams->nandinfo.extid);
	printf("pagesize = %d\n",nandparams->nandinfo.pagesize);
	printf("oobsize = %d\n",nandparams->nandinfo.oobsize);
	printf("blocksize = %d\n",nandparams->nandinfo.blocksize);
	printf("eccbit = %d\n",nandparams->nandinfo.eccbit);
	printf("buswidth = %d\n",nandparams->nandinfo.buswidth);
	printf("realplanenum = %d\n",nandparams->nandinfo.realplanenum);
	printf("badblockpos = %d\n",nandparams->nandinfo.badblockpos);
	printf("rowcycles = %d\n",nandparams->nandinfo.rowcycles);
	printf("options = %d\n",nandparams->nandinfo.options);
	printf("kernel_offset = %x\n",nandparams->kernel_offset);
	printf("nand_manager_version = %x\n",nandparams->nand_manager_version);

}

static inline void fill_nand_io_cinfo(struct spl_basic_param *param)
{
	nandio->cinfo->rowcycles = param->rowcycles;
	nandio->cinfo->pagesize = param->pagesize;
	nandio->base->cycle = param->rowcycles;
}

static void get_nand_manager_version(nand_sharing_params *nandparams)
{
	if(nandparams->nand_manager_version == 0xffffffff) // nand_manager_version lower than 1.9.0
		spl_read_align = 0;
	else if(nandparams->nand_manager_version >= 190) // nand manager version highter than 1.9.0
		spl_read_align = 1;
	else
		printf("can't parse nand manager version , spl read uboot will be error !!!\n");

}

static struct nand_sharing_params *get_nand_basic_params_emc()
{
	struct spl_basic_param param;
	int page_addr;
	int ret = 0, bakup_num = 0;

	get_nand_spl_basic_param(&param);

	fill_nand_io_cinfo(&param);

	memset(data_space,0x00,NAND_PARAMS_LEN);

	page_addr = (SPL_SIZE / param.pagesize) * 2;

	do{
		ret = nand_read_spl_page(page_addr,data_space, oob_space, &param, 0, NAND_PARAMS_LEN);
		page_addr += 256;
		bakup_num++;
	}while((ret < 0) && (bakup_num < SPL_BACKUP_NUM));

	if(bakup_num >= SPL_BACKUP_NUM){
		printf("SPL ERROR!");
	}

	memcpy(NAND_SHARING_PARMS_ADDR,data_space,NAND_PARAMS_LEN);

	//dump_nand_common_params((nand_sharing_params *)NAND_SHARING_PARMS_ADDR);
	get_nand_manager_version((nand_sharing_params *)NAND_SHARING_PARMS_ADDR);
	return (nand_sharing_params *)NAND_SHARING_PARMS_ADDR;
}
static inline void init_param_addr()
{

	cpinfo = (cpu_copy_info *)(NAND_PARAM_BASE);
	nandbch = (nand_bch *)(cpinfo + 1);
	bchbase = (bch_base *)(nandbch + 1);
	nfibase = (nfi_base *)(bchbase + 1);
	nandio = (nand_io *)(nfibase + 1);
	cinfo = (chip_info *)(nandio + 1);

	oob_space = (unsigned char *)(cinfo + 1);
	data_space = (unsigned char *)(oob_space + 256);
}

static inline void init_nand_basic_info()
{
	nand_sharing_params *nandparams = (nand_sharing_params *)NAND_SHARING_PARMS_ADDR;

	pagesize = nandparams->nandinfo.pagesize;
	buswidth = nandparams->nandinfo.buswidth;
	rowcycles = nandparams->nandinfo.rowcycles;
	eccbit = nandparams->nandinfo.eccbit;
	blocksize = nandparams->nandinfo.blocksize;
	oob_size = nandparams->nandinfo.oobsize;

	ecc_block = (pagesize == 512) ? 512 : 1024;
	ecc_count = pagesize / ecc_block;
	parity_size = eccbit * 14 / 8;

	cinfo->pagesize = pagesize;
	cinfo->rowcycles = rowcycles;
	cinfo->buswidth = buswidth;
}

static void nand_init(void)
{
	ndd_ndelay = test_delay;
	ndd_printf = printf;

	init_param_addr();
	fill_nfi_base();
	fill_bch_base();

#ifdef CONFIG_NAND_NFI
	clk_set_rate(BCH,clk_get_rate(H2CLK));

	nand_io_setup_timing_default((int)nandio);
#endif

	nand_io_chip_select((int)nandio,0);

	get_nand_basic_params_emc();

	init_nand_basic_info();
	spl_bch_init(eccbit,ecc_block);
}

static void nand_read_oob(unsigned int page,unsigned char *oob_buf,unsigned int oob_size)
{
	int col_addr;

	if(pagesize != 512){
		if(buswidth == 8)
			col_addr = pagesize;
		else
			col_addr = pagesize / 2;
	}else
		col_addr = 512;

	send_read_start_cmd(page,col_addr,1);

	nand_io_receive_data((int)(nandio),oob_buf,oob_size);
}

static int nand_check_bad_blk(unsigned int pageid)
{
	unsigned int m,i,k,bits = 0;

	for(m = 0; m < 4; m++){
		nand_read_oob(pageid + m,oob_space,oob_size);
		for(i=0; i<4;i++) {
			if (oob_space[oob_size -4 + i] != 0xff) {
				for(k=0;k<8;k++) {
					if(!(0x01 & (oob_space[oob_size - 4 + i]>>k)))
						bits++;
				}
			}
		}
	}
	if(bits > 64)
		return 1;
	else
		return 0;
}

static int nand_read_spl_page(int page_addr, unsigned char *data_buf, unsigned char *oob_buf, struct spl_basic_param *params, unsigned int      offset,unsigned int bytes)
{
	int i;
	int spl_ecc_bytes = SPL_BCH_BIT * 14 / 8;
	int ecccnt = bytes / SPL_ECC_SIZE;
	int oob_size = spl_ecc_bytes * ecccnt;

	PipeNode pipenode;

	send_read_start_cmd(page_addr,offset,0xf);

	pn_enable((int)nandio);
	nand_io_receive_data((int)(nandio),data_buf,bytes);
	pn_disable((int)(nandio));

	send_read_start_cmd(page_addr + 1,0,0xf);

	pn_enable((int)nandio);
	nand_io_receive_data((int)(nandio),oob_buf,oob_size);
	pn_disable((int)(nandio));

	pipenode.data = data_buf;
	pipenode.parity = oob_buf;

	spl_bch_init(SPL_BCH_BIT,256);
	nand_bch_decode_prepare((int)(nandbch),&pipenode,SPL_BCH_BIT);
	nand_bch_decode_complete((int)(nandbch),&pipenode);
	return 0;
}

static int nand_read_page(unsigned int pageaddr,unsigned char *data_buf,unsigned char *oob_buf)
{
	int i;
	unsigned int bit0cnt;
	PipeNode pipenode;

	if(spl_read_align)
		parity_size = (parity_size + 3) / 4 * 4;

	send_read_start_cmd(pageaddr,0,1);

	for(i = 0; i < ecc_count; i++){
		bit0cnt = 0;
		nand_io_counter0_enable((int)nandio);

		pn_enable((int)nandio);
		nand_io_receive_data((int)(nandio),data_buf + i * ecc_block,ecc_block);
		nand_io_receive_data((int)(nandio),oob_buf,parity_size);
		pn_disable((int)nandio);

		bit0cnt = nand_io_read_counter((int)nandio);
		nand_io_counter_disable((int)nandio);

		if(bit0cnt < eccbit){
			printf("--- read over last pageaddr is %d\n",pageaddr);
			return -1;
		}

		pipenode.data = data_buf + i * ecc_block;
		pipenode.parity = oob_buf;

		nand_bch_decode_prepare((int)(nandbch),&pipenode,eccbit);
		nand_bch_decode_complete((int)(nandbch),&pipenode);

	}
	return 0;
}

static int nand_spl_load_image(long  offs, long size, void *dst)
{

	int page, pagecopy_cnt = 0;
	int page_per_blk;
	int blk;
	int ret;
	int ubootoffs;
	page_per_blk = blocksize / pagesize;

	if (pagesize < 4096){
		/*+pagesize for params *2 for bch */
		ubootoffs = ((SPL_SIZE + pagesize) / pagesize * 2 + page_per_blk -1) / (page_per_blk);
	}else{
		ubootoffs = 2;
	}
	if(offs < 0){
		page = (page_per_blk < 128 ? ((128 / page_per_blk) + ubootoffs) : ubootoffs) * page_per_blk;
	}else
		page = offs / pagesize;

	if(size < 0)
		size = UBOOT_AUTO_MAX_SIZE;

	while (pagecopy_cnt * pagesize < size) {
		if (page % page_per_blk == 0){
			if(nand_check_bad_blk(page)){
				page += page_per_blk;
				continue;
			}
		}
		ret = nand_read_page(page,(unsigned char *)dst,oob_space);
		if(ret != 0)
			break;

		dst += pagesize;
		page++;
		pagecopy_cnt++;

	}
	return 0;
}

static void nand_deselect(void)
{
	nand_io_chip_deselect((int)nandio,0);
}

void spl_nand_load_image(void)
{
	struct image_header *header;

	nand_init();
	// offset -1 is auto offset mode, size -1 is auto size mode
	nand_spl_load_image(-1,-1, (void *)CONFIG_SYS_TEXT_BASE);

	header = (struct image_header *)CONFIG_SYS_TEXT_BASE;
	spl_parse_image_header(header);

	nand_deselect();
}


