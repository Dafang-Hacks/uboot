#include <os_clib.h>
#include <nand_debug.h>
#include <nand_io.h>
#include <nand_info.h>
#include <ndcommand.h>
#include <cpu_trans.h>

#include <soc/jz_nfi.h>

#define NAND_DEFAULT_BUS	NAND_BUSWIDTH_8BIT
#define NAND_DEFAULT_MODE	NAND_IF_COMMON
#define COMMON_EDO_NAND		NAND_IF_EDO
#define TOGGLE_DDR_NAND		NAND_IF_TOGGLE
#define ONFI_DDR_NAND		NAND_IF_ONFI

#define NAND_EDO_TEST_BYTES	32

#define NFI_COMMON_CS_ENABLE_MODE
/******  the operation of nemc registers  ******/
static int ref_cnt = 0;
extern int (*nand_auto_adapt_edo)(int, chip_info *, rb_item *);
extern int (*nand_adjust_to_toggle)(int, chip_info *);
extern int (*nand_adjust_to_onfi)(int, chip_info *);

typedef struct __nand_io {
	nfi_base *base;
	void *dataport;
	void *cmdport;
	void *addrport;
	transadaptor trans;
	chip_info *cinfo;
	unsigned int copy_context;
} nand_io;
static int edo_timing = 0;
#if 0
/* set NFI busy control register */
#define __nfi_unmark_rb(n)	(REG_NAND_NFCR &= ~(NAND_NFCR_BUSY_MASK(n)))
#define __nfi_mark_rb(n)	(REG_NAND_NFCR |= (NAND_NFCR_BUSY_MASK(n)))
#define __nfi_wait_rb(n) \
	do{ \
		while(REG_NAND_NFCR & NAND_NFCR_BUSY(n));			\
		REG_NAND_NFCR |= NAND_NFCR_BUSY(n);				\
	}while(0)
#define __nfi_set_rb_type(r) \
	do{ \
		unsigned int tmp;						 \
		tmp = REG_NAND_NFCR;						 \
		tmp &= ~(NAND_NFCR_BUSYMOD_MASK);				 \
		tmp |= NAND_NFCR_BUSYMOD(r);					 \
		REG_NAND_NFCR = tmp;						 \
	}while(0)
#endif
extern int (*__wait_rb_timeout) (rb_item *, int);
void nand_io_setup_timing_default(int context);
int nand_io_setup_timing_optimize(int context, chip_info *cinfo);
/******  the operation of nfi registers  ******/
void pn_enable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NAND_PNCR);
	io->base->writel(NAND_PNCR, (tmp | NAND_PNCR_PNRST | NAND_PNCR_PNEN));
}

void pn_disable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NAND_PNCR);
	tmp &= ~(NAND_PNCR_PNEN);
	io->base->writel(NAND_PNCR, tmp);
}
void nand_io_counter0_enable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NAND_PNCR);
	tmp &= ~(NAND_PNCR_BIT_MASK);
	tmp |= NAND_PNCR_BITRST | NAND_PNCR_BIT0 | NAND_PNCR_BITEN;
	io->base->writel(NAND_PNCR, tmp);
}

void nand_io_counter1_enable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NAND_PNCR);
	tmp &= ~(NAND_PNCR_BIT_MASK);
	tmp |= NAND_PNCR_BITRST | NAND_PNCR_BIT1 | NAND_PNCR_BITEN;
	io->base->writel(NAND_PNCR, tmp);
}

unsigned int nand_io_read_counter(int context)
{
	nand_io *io = (nand_io *)context;

	return io->base->readl(NAND_BCNT);
}

void nand_io_counter_disable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NAND_PNCR);
	tmp &= ~(NAND_PNCR_BITEN);
	io->base->writel(NAND_PNCR, tmp);
}
static void nand_nfi_enable(nfi_base *base)
{
	unsigned int reg;
	reg = base->readl(NAND_NFCR);
	reg &= ~(NAND_NFCR_CSEN_MASK);
	reg |= NAND_NFCR_CSMOD | NAND_NFCR_INIT | NAND_NFCR_EN;
	base->writel(NAND_NFCR, reg);
	ndd_ndelay(100);
}
static void nand_nfi_disable(nfi_base *base)
{
	unsigned int reg;
	reg = base->readl(NAND_NFCR);
	reg &= ~(NAND_NFCR_INIT | NAND_NFCR_EN);
	base->writel(NAND_NFCR, reg);
}

static void set_nand_timing_default(nfi_base *base)
{
	//ndd_debug("##### base->writel = %p, base->iomem = 0x%08x\n",base->writel,(unsigned int)base->iomem);
	base->writel(NAND_NFIT0, (NAND_NFIT0_SWE(0x3) | NAND_NFIT0_WWE(0x9)));//030
	base->writel(NAND_NFIT1, (NAND_NFIT1_HWE(0x3) | NAND_NFIT1_SRE(0x3)));
	base->writel(NAND_NFIT2, (NAND_NFIT2_WRE(0x9) | NAND_NFIT2_HRE(0x3)));//030
	base->writel(NAND_NFIT3, (NAND_NFIT3_SCS(0xf) | NAND_NFIT3_WCS(0xf)));//21
	base->writel(NAND_NFIT4, (NAND_NFIT4_BUSY(0xff) | NAND_NFIT4_EDO(0xf)));

}
static void set_nand_timing_optimize(nfi_base *base, nand_timing_com *timing)
{



	//dsqiu ---------------------

	unsigned long cycle = base->cycle;  //unit: ps
	unsigned int lbit,hbit;
	hbit = ((timing->tWH - timing->tDH) * 1000) / cycle;
	lbit = ((timing->tWP) * 1000 + cycle - 1) / cycle;
	base->writel(NAND_NFIT0, (NAND_NFIT0_SWE(hbit) | NAND_NFIT0_WWE(lbit)));

	hbit = ((timing->tDH) * 1000) / cycle;
	lbit = ((timing->tREH - timing->tDH ) * 1000 + cycle - 1) / cycle;
	base->writel(NAND_NFIT1, (NAND_NFIT1_HWE(hbit) | NAND_NFIT1_SRE(lbit)));

	hbit = ((timing->tRP) * 1000 + cycle - 1) / cycle;
	lbit = ((timing->tDH) * 1000) / cycle;
	base->writel(NAND_NFIT2, (NAND_NFIT2_WRE(hbit) | NAND_NFIT2_HRE(lbit)));

	hbit = ((timing->tCS) * 1000 + cycle - 1) / cycle;
	lbit = ((timing->tCH) * 1000 + cycle - 1) / cycle;
	base->writel(NAND_NFIT3, (NAND_NFIT3_SCS(hbit) | NAND_NFIT3_WCS(lbit)));

	hbit = ((timing->tRR) * 1000 + cycle - 1) / cycle;
	lbit = (1 * 1000 + cycle - 1) / cycle ;
	base->writel(NAND_NFIT4, (NAND_NFIT4_BUSY(hbit) | NAND_NFIT4_EDO(lbit)));
/*
	base->writel(NAND_NFIT0, (NAND_NFIT0_SWE(0x0) | NAND_NFIT0_WWE(0x2)));
	base->writel(NAND_NFIT1, (NAND_NFIT1_HWE(0x0) | NAND_NFIT1_SRE(0x0)));
	base->writel(NAND_NFIT2, (NAND_NFIT2_WRE(0x3) | NAND_NFIT2_HRE(0x0)));
	base->writel(NAND_NFIT3, (NAND_NFIT3_SCS(0x7) | NAND_NFIT3_WCS(0x7)));
	base->writel(NAND_NFIT4, (NAND_NFIT4_BUSY(0xf) | NAND_NFIT4_EDO(0xf)));


    ndd_print(NDD_DEBUG,"NNNNNNNNNNNN******** T0 = %08x\n",base->readl(NAND_NFIT0));
    ndd_print(NDD_DEBUG,"NNNNNNNNNNNN******** T1 = %08x\n",base->readl(NAND_NFIT1));
    ndd_print(NDD_DEBUG,"NNNNNNNNNNNN******** T2 = %08x\n",base->readl(NAND_NFIT2));
    ndd_print(NDD_DEBUG,"NNNNNNNNNNNN******** T3 = %08x\n",base->readl(NAND_NFIT3));
    ndd_print(NDD_DEBUG,"NNNNNNNNNNNN******** T4 = %08x\n",base->readl(NAND_NFIT4));
*/
}
static void recalc_writecycle(nfi_base *base,chip_info *cinfo)
{
	int tset,twait,thold;
	int t0,t1,t;
	unsigned long cycle = base->cycle;  //unit: ps
	t0 = base->readl(NAND_NFIT0);
	tset = (t0 >> NAND_NFIT0_SWE_BIT) & 0xffff;
	tset = tset + 1;
	twait = t0 & 0xffff;

	t1 = base->readl(NAND_NFIT1);
	thold = t1 >> NAND_NFIT1_HWE_BIT;
	thold = thold + 1;
	t = tset + twait + thold;
	cinfo->ops_timing.tWC = (t * cycle + 999) / 1000;
	//ndd_print(NDD_DEBUG,"%s(%s) %d tWC  = %d\n",__FUNCTION__,__FILE__,__LINE__,cinfo->ops_timing.tWC);

}
/*
   set the buswidth and mode of nand;
   @context:
   @bus: it is the buswidth of nand; 0 -> 8bit, 1 -> 16bit.
   @mode: it is the type of nand;	0 -> common nand
   1 -> common nand EDO mode
   2 -> toggle DDR nand
   3 -> onfi DDR nand
 */
static int set_nand_bus_mode(nfi_base *base, unsigned int bus, unsigned int mode)
{
	unsigned int reg;
	reg = base->readl(NAND_NFCR);
	while(!(reg & NAND_NFCR_EMPTY))
		reg = base->readl(NAND_NFCR);
	reg &= ~(NAND_NFCR_SEL_MASK | NAND_NFCR_BUSWIDTH_MASK);
	reg |=	NAND_NFCR_BUSWIDTH(bus) | NAND_NFCR_SEL(mode);
	base->writel(NAND_NFCR, reg);
	return 0;
}
#if 0
static int set_nfi_busy(void)
{
	return 0;
}
static int set_nfi_busy_detector(void)
{
	return 0;
}
#endif
static void __send_cmd_to_nand(void *cmdport, unsigned char cmd, unsigned int dly_f,unsigned int dly_b, unsigned char busy)
{
	unsigned int reg = NAND_NFCM_DLYF(dly_f) | NAND_NFCM_DLYB(dly_b) | NAND_NFCM_CMD(cmd);
	if(busy)
		reg |= NAND_NFCM_BUSY;
	*(volatile unsigned int *)(cmdport) = reg;
}
static void __send_addr_to_nand(void *addrport, unsigned char addr, unsigned int dly_f,unsigned int dly_b, unsigned char busy)
{
	unsigned int reg = NAND_NFAD_DLYF(dly_f) | NAND_NFAD_DLYB(dly_b) | NAND_NFAD_ADDR(addr);
	if(busy)
		reg |= NAND_NFAD_BUSY;
	*(volatile unsigned int *)(addrport) = reg;
}
/*
   config the delay timing about the mode of nand's EDO; the timing is undefined,
   so that we should be set a appropriate value.
return : 0 -> ok
1 -> fail
 */
static int compare_data_ok(int context, transadaptor *trans, void *src)
{
	unsigned char dst[NAND_EDO_TEST_BYTES];
	unsigned int i = 0, ret = SUCCESS;
	ret = trans->prepare_memcpy(context,src,dst,NAND_EDO_TEST_BYTES,DSTADD);
	if(ret < 0)
		RETURN_ERR(ENAND, "prepare memcpy error");

	ret = trans->finish_memcpy(context);
	if(ret < 0)
		RETURN_ERR(ENAND, "finish_memcpy error");
	for(i = 0; i < NAND_EDO_TEST_BYTES; i++){
		if(dst[i] != 0x55 && dst[i] != 0xaa){
			ret = ENAND;
			break;
		}
	}
	return ret;
}
/*
return : 0 -> ok
!0 -> fail
 */
static int check_edo_timing(nand_io *io, chip_info *cinfo, rb_item *rb)
{
	int ret = SUCCESS;
	nand_ops_timing *timing = &cinfo->ops_timing;

	nand_io_send_cmd((int)io,CMD_PAGE_READ_1ST, 0);
	nand_io_send_spec_addr((int)io,0,cinfo->rowcycles, timing->tADL);
	if(cinfo->pagesize != 512)
		    nand_io_send_cmd((int)io, CMD_PAGE_READ_2ND, 100);
	if(__wait_rb_timeout){
			ret = __wait_rb_timeout(rb, 500);
		if(!ret)
			ret = TIMEOUT;
		else
			ret = compare_data_ok(io->copy_context, &(io->trans), io->dataport);
	}else{
		ret = ENAND;
	}
	return ret;
}
/*
   we will set a appropriate value of delay timing of EDO, the zone of nand flag is 0x55 or 0xaa at beginning
   of the first nand.
 */
int auto_adapt_edo_nand(int context, chip_info *cinfo, rb_item *rbitem)
{
	nand_io *io = (nand_io *)context;
	nfi_base *base = io->base;
	unsigned int reg;
	unsigned int b_ok,f_ok,max;
	static unsigned int edo_t = 1;	//the min value is 1;
	unsigned int ret = SUCCESS;
	unsigned int cycle = base->cycle;  //unit: ps
	nand_timing_com *timing = (nand_timing_com *)cinfo->ops_timing.io_timing;
	int nfcr = base->readl(NAND_NFCR);
	reg = base->readl(NAND_NFIT4);
	b_ok = f_ok = 0;
	base->writel(NAND_NFCR, nfcr | NAND_NFCR_EDO_EN);

#ifdef EDO_PROBE
	if(edo_t ==1 ){
		max = ((timing->tRP + timing->tREH) * 1000 + cycle - 1) / cycle;
		edo_t = (reg & NAND_NFIT4_EDO_MASK) >> NAND_NFIT4_EDO_BIT;
		reg &= ~(NAND_NFIT4_EDO_MASK);
		while(1){
			if(edo_t >= max){
				edo_t -= 2; // larger than max ,have add 2,should sub 2 to adjust
				ret = ENAND;
				break;
			}
			base->writel(NAND_NFIT4, reg | NAND_NFIT4_EDO(edo_t));
			ret = check_edo_timing(io, cinfo, rbitem);
			if(ret == TIMEOUT){
				ret = ENAND;
				break;
			}else if(ret == SUCCESS){
				if(!b_ok)
					b_ok = edo_t;
				else
					f_ok = edo_t;
			}else if(f_ok != 0)
				break;
			edo_t += 2;
		}
		if(ret != ENAND && f_ok != 0)
			edo_t = (b_ok + f_ok) / 2;
	}
	reg &= ~(NAND_NFIT4_EDO_MASK);
#else
	edo_t = (timing->tREH * 1000 + cycle - 1) / cycle;
#endif //EDO_PROBE
	base->writel(NAND_NFIT4, reg | NAND_NFIT4_EDO(edo_t));
	edo_timing = base->readl(NAND_NFIT4) & 0x0000ffff;
	return ret;
}
static int auto_adapt_dqs_delay(nfi_base *base)
{
	volatile unsigned int reg;
	base->writel(NAND_NFDL, 0x07c00100);
	reg = base->readl(NAND_NFDL);
	while(!(reg & NAND_NFDL_DONE_MASK))
		reg = base->readl(NAND_NFDL);
	if(reg & NAND_NFDL_ERR_MASK)
		return ENAND;
	else
		return SUCCESS;
}
int auto_adapt_toggle_nand(int context, chip_info *cinfo)
{
	nand_io *io = (nand_io *)context;
	unsigned int reg = 0, tmp;
	nfi_base *base = io->base;
	nand_timing_com *timing = (nand_timing_com *)cinfo->ops_timing.io_timing;
	nfi_toggle_timing *t_timing = (nfi_toggle_timing *)cinfo->ops_timing.io_etiming;
	unsigned int cycle = base->cycle;  //unit: ps

	tmp = ((t_timing->tRPRE + timing->tDH - timing->tREH) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFITG0_FRE(tmp);
	tmp = ((t_timing->tWPRE) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFITG0_FDQS(tmp);
	tmp = ((t_timing->tDS) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFITG0_SDQS(tmp);
	tmp = ((timing->tDH) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFITG0_HDQS(tmp);
	base->writel(NAND_NFITG0, reg);
	reg = 0;
	tmp = ((t_timing->tWPST) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFITG1_DQS2IDLE(tmp);
	tmp = ((t_timing->tDQSRE) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFITG1_DQSRE(tmp);
	base->writel(NAND_NFITG1, reg);
	return auto_adapt_dqs_delay(base);
}
int auto_adapt_onfi_nand(int context, chip_info *cinfo)
{
	nand_io *io = (nand_io *)context;
	unsigned int reg = 0, tmp;
	nfi_base *base = io->base;
//	const nand_timing *timing = cinfo->timing;
        nfi_onfi_timing *o_timing = (nfi_onfi_timing *)cinfo->ops_timing.io_etiming;
	unsigned int cycle = base->cycle;  //unit: ps

	tmp = ((o_timing->tDQSD > 20 ? o_timing->tDQSD : 20) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFIOF0_WR2CLE(tmp);
	tmp = ((o_timing->tDQSS) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFIOF0_DQSS(tmp);
	tmp = (((o_timing->tCK + 1) / 2) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFIOF0_SCLK(tmp);
	tmp = (((o_timing->tCK + 1) / 2 ) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFIOF0_HCLK(tmp);
	base->writel(NAND_NFIOF0, reg);
	reg = 0;
	tmp = ((o_timing->tDQSHZ) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFIOF1_DQS2IDLE(tmp);
	tmp = ((o_timing->tDQSCK + o_timing->tCK) * 1000 + cycle - 1) / cycle;
	reg |= NAND_NFIOF1_R2IDLE(tmp);
	base->writel(NAND_NFIOF1, reg);
	return auto_adapt_dqs_delay(base);
}
#if 0
static int toggle_nand_data_sync(nfi_base *base)
{
	unsigned int reg = base->readl(NAND_NFCS);
	base->writel(NAND_NFCS, 0);
	base->writel(NAND_NFCS, reg);

	return 0;
}

static int onfi_nand_data_sync(nfi_base *base)
{
	unsigned int reg = base->readl(NAND_NFCS);
	base->writel(NAND_NFCS, reg);

	return 0;
}
#endif

int nand_io_chip_select(int context, int cs)
{
	nand_io *io = (nand_io *)context;

	if((cs >= NFI_CS_COUNT) || (cs < 0))
		RETURN_ERR(ENAND, "chip select number out of limit, cs = [%d]", cs);

	io->dataport = io->base->cs_iomem[0] - cs * 0x01000000;
	io->cmdport = io->dataport + NAND_CMD_OFFSET;
	io->addrport = io->dataport + NAND_ADDR_OFFSET;
//	ndd_debug("cs = %d, csmemory = %p, dataport = %p\n",
//		  cs, io->base->cs_iomem[0], io->dataport);
#ifdef NFI_COMMON_CS_ENABLE_MODE
	io->base->writel(NAND_NFCS, NAND_NFCS_CS(cs));
//	ndd_debug("-------NAND_CS = %x-------\n", *((volatile unsigned int *)(0xB3410000 + (NAND_NFCS))));
#endif
	return SUCCESS;
}

int nand_io_chip_deselect(int context, int cs)
{
	nand_io *io = (nand_io *)context;
	unsigned int reg = io->base->readl(NAND_NFCS);
	reg &= ~NAND_NFCS_CS(cs);
	io->base->writel(NAND_NFCS, reg);
	return 0;
}
static inline void read_status(unsigned int *val, void *addr)
{
	*val = *(volatile unsigned int *)addr;
}

static int nand_io_init(nfi_base *base)
{
	if (ref_cnt++ == 0){
		base->clk_enable();
		base->cycle = 1000000000 / (base->rate / 1000);  //unit: ps
		nand_nfi_enable(base);
	}

	return 0;
}

static void nand_io_deinit(nfi_base *base)
{
	if (--ref_cnt == 0){
		base->clk_disable();
		nand_nfi_disable(base);
	}
}
int nand_io_open(nfi_base *base, chip_info *cinfo)
{
	nand_io *nandio = NULL;

	nandio = ndd_alloc(sizeof(nand_io));
	if(!nandio)
		RETURN_ERR(ENAND, "alloc io memory error");

	nandio->base = base;
	nandio->cinfo = cinfo;
	nand_io_init(nandio->base);
	if (cinfo) {
		nandio->cinfo = cinfo;
		nand_io_setup_timing_optimize((int)nandio, nandio->cinfo);
		recalc_writecycle(base, cinfo);
	} else {
		nandio->cinfo = NULL;
		nand_io_setup_timing_default((int)nandio);
	}
	nandio->copy_context = cpu_move_init(&nandio->trans);
	if (nandio->copy_context < 0) {
		ndd_free(nandio);
		RETURN_ERR(0, "cpu move init error");
	}

	return (int)nandio;
}

void nand_io_close(int context)
{
	nand_io *io = (nand_io *)context;

	cpu_move_deinit(io->copy_context, &io->trans);
	nand_io_deinit(io->base);
	ndd_free(io);
}
void nand_io_setup_timing_default(int context)
{
	nand_io *io = (nand_io *)context;
	set_nand_bus_mode(io->base, NAND_DEFAULT_BUS, NAND_DEFAULT_MODE);
	set_nand_timing_default(io->base);
	io->base->writel(NAND_NFRB, 0x7);
	ndd_debug("default NFCR = 0x%08x, NFRB = 0x%08x\n",
			*(volatile unsigned int *)(0xb3410010),*(volatile unsigned int *)(0xb3410054));
}
void nfi_set_edo_mode(nfi_base *base)
{
	int nfcr,t4;

	nfcr = base->readl(NAND_NFCR);
	nfcr |= NAND_NFCR_EDO_EN;
	base->writel(NAND_NFCR,nfcr);

	t4 = base->readl(NAND_NFIT4);
	t4 &= 0xffff0000;
	t4 |= (edo_timing << 0);
	base->writel(NAND_NFIT4,t4);
}

int nand_io_setup_timing_optimize(int context, chip_info *cinfo)
{
	nand_io *io = (nand_io *)context;
	unsigned int bus = NAND_BUSWIDTH_8BIT;
	int ret = SUCCESS;
	if(cinfo->buswidth == 16){
		bus = NAND_BUSWIDTH_16BIT;
#if defined(CONFIG_NAND_BUS_WIDTH_8) || defined(CONFIG_SOC_4780)
		cinfo->buswidth = 8;
		ndd_print(NDD_ERROR,"%s:the buswidth is wrong,please modify it; next step is while(1)\n",
				__func__);
		while(1);
#endif
	}
	set_nand_bus_mode(io->base, bus, GET_NAND_TYPE(cinfo));
	set_nand_timing_optimize(io->base, (nand_timing_com *)cinfo->ops_timing.io_timing);
	if(edo_timing)
		nfi_set_edo_mode(io->base);

	if(GET_NAND_TYPE(cinfo) == COMMON_EDO_NAND && nand_auto_adapt_edo == NULL){
		nand_auto_adapt_edo = auto_adapt_edo_nand;
	}else if(GET_NAND_TYPE(cinfo) == TOGGLE_DDR_NAND){
		nand_adjust_to_toggle = auto_adapt_toggle_nand;
	}else if(GET_NAND_TYPE(cinfo) == ONFI_DDR_NAND){
		nand_adjust_to_onfi = auto_adapt_onfi_nand;
	}else{
		ret = SUCCESS;
		ndd_debug("%s the type of the nand is common !\n",__func__);
	}

	ndd_debug("%s time0 = 0x%08x, time1 = 0x%08x \n time2 = 0x%08x, time3 = 0x%08x, time4 = 0x%08x\n",
			__func__, *(volatile unsigned int *)0xb3410028,*(volatile unsigned int *)0xb341002c,
			*(volatile unsigned int *)0xb3410030, *(volatile unsigned int *)0xb3410034,
			*(volatile unsigned int *)0xb3410038);
	return ret;
}

int nand_io_send_cmd(int context, unsigned char command, unsigned int delay)
{
	int ret = 0;
	nand_io *io = (nand_io *)context;
	unsigned long cycle = io->base->cycle;  //unit: ps

	delay = (delay * 1000 + cycle - 1) / cycle;


	__send_cmd_to_nand(io->cmdport,command,0,delay,0);

	if(command == CMD_READ_STATUS_1ST){
		read_status(&ret, io->dataport);
	}

	return ret;
}

void nand_io_send_addr(int context, int col_addr, int row_addr, unsigned int delay)
{
	nand_io *io = (nand_io *)context;

	int rowcycle = io->cinfo->rowcycles;
	int i;
	unsigned int cycle = io->base->cycle;  //unit: ps

	delay = (delay * 1000 + cycle - 1) / cycle;

	if(col_addr >= 0){
		if(io->cinfo->pagesize != 512){
			__send_addr_to_nand(io->addrport, (col_addr & 0xff), 0, 0, 0);
			col_addr >>= 8;
		}
		__send_addr_to_nand(io->addrport, (col_addr & 0xff), 0, 0, 0);
	}

	if(row_addr >= 0){
		for(i=0; i<rowcycle - 1; i++){
			__send_addr_to_nand(io->addrport, (row_addr & 0xff), 0, 0, 0);
			row_addr >>= 8;
		}
		__send_addr_to_nand(io->addrport, (row_addr & 0xff), 0, delay, 0);
	}
}
void nand_io_send_spec_addr(int context, int addr, unsigned int rowcycle, unsigned int delay)
{
	nand_io *io = (nand_io *)context;
	unsigned int cycle = io->base->cycle;  //unit: ps

	delay = (delay * 1000 + cycle - 1) / cycle;
	while(rowcycle > 1){
		__send_addr_to_nand(io->addrport, (addr & 0xff), 0, 0, 0);
		addr = addr >> 0x08;
		rowcycle--;
	}
	if(rowcycle)
		__send_addr_to_nand(io->addrport, (addr & 0xff), 0, delay, 0);
}

int nand_io_send_data(int context, unsigned char *src, unsigned int len)
{
	int ret = 0;
	nand_io *io = (nand_io *)context;
	unsigned char *dst = (unsigned char *)(io->dataport);

	ret = io->trans.prepare_memcpy(io->copy_context,src,dst,len,SRCADD);
	if(ret < 0)
		RETURN_ERR(ENAND, "prepare memcpy error");

	ret = io->trans.finish_memcpy(io->copy_context);
	if(ret < 0)
		RETURN_ERR(ENAND, "finish_memcpy error");

	return 0;
}

int nand_io_receive_data(int context, unsigned char *dst, unsigned int len)
{
	int ret = 0;
	nand_io *io = (nand_io *)context;
	unsigned char *src = (unsigned char *)(io->dataport);

	ret = io->trans.prepare_memcpy(io->copy_context,src,dst,len,DSTADD);
	if(ret < 0)
		RETURN_ERR(ENAND, "prepare memcpy error");

	ret = io->trans.finish_memcpy(io->copy_context);
	if(ret < 0)
		RETURN_ERR(ENAND, "prepare memcpy error");

	return 0;
}
int nand_io_send_waitcomplete(int context, chip_info *cinfo) {
	    ndd_ndelay(cinfo->ops_timing.tWC * 64);
	return 0;
}
void nand_io_setup_default_16bit(nfi_base *base)
{
	set_nand_bus_mode(base, NAND_BUSWIDTH_16BIT, NAND_DEFAULT_MODE);
}

int nand_io_suspend(void)
{
	return 0;
}

int nand_io_resume(void)
{
	return 0;
}

//------------------------------------------------------------------------------
static void convert2opstiming(nand_ops_timing *ops_timing, const nand_timing *nandtiming) {
	const nand_timing_com *timing = &nandtiming->timing;
#define assign(member) ops_timing->member = timing->member
	assign(tRP);
	assign(tWP);
	assign(tWHR);
	assign(tWHR2);
	assign(tRR);
	assign(tWB);
	assign(tADL);
	assign(tCWAW);
	assign(tCS);
#undef assign
	ops_timing->tWC = timing->tWP + timing->tWH;
	ops_timing->tCLH = timing->tCH;
	ops_timing->io_timing = (void *)timing;
}
void nand_controll_adpt(chip_info *cinfo, const nand_flash *ndflash) {
	convert2opstiming(&cinfo->ops_timing, &ndflash->timing);
	cinfo->ops_timing.io_etiming = &ndflash->nand_extra;
}

char* nand_io_get_clk_name(void) {
	return NAND_IO_CLK_NAME;
}
