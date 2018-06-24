#include <os_clib.h>
#include <nand_debug.h>
#include <nand_io.h>
#include <nand_info.h>
#include <cpu_trans.h>

#include <soc/jz_nemc.h>

/******  the operation of nemc registers  ******/
static int ref_cnt = 0;

typedef struct __nand_io {
	nfi_base *base;
	void *dataport;
	void *cmdport;
	void *addrport;
	transadaptor trans;
	chip_info *cinfo;
	const nand_timing_com *timing;
	unsigned int copy_context;
} nand_io;

void pn_enable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NEMC_PNCR);
	io->base->writel(NEMC_PNCR, (tmp | NEMC_PNCR_PNRST | NEMC_PNCR_PNEN));
}

void pn_disable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NEMC_PNCR);
	tmp &= ~(NEMC_PNCR_PNEN);
	io->base->writel(NEMC_PNCR, tmp);
}

void nand_io_counter0_enable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NEMC_PNCR);
	tmp &= ~(NEMC_PNCR_BIT_MASK);
	tmp |= NEMC_PNCR_BITRST | NEMC_PNCR_BIT0 | NEMC_PNCR_BITEN;
	io->base->writel(NEMC_PNCR, tmp);
}

void nand_io_counter1_enable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NEMC_PNCR);
	tmp &= ~(NEMC_PNCR_BIT_MASK);
	tmp |= NEMC_PNCR_BITRST | NEMC_PNCR_BIT1 | NEMC_PNCR_BITEN;
	io->base->writel(NEMC_PNCR, tmp);
}

unsigned int nand_io_read_counter(int context)
{
	nand_io *io = (nand_io *)context;

	return io->base->readl(NEMC_BITCNT);
}

void nand_io_counter_disable(int context)
{
	nand_io *io = (nand_io *)context;
	unsigned int tmp = io->base->readl(NEMC_PNCR);
	tmp &= ~(NEMC_PNCR_BITEN);
	io->base->writel(NEMC_PNCR, tmp);
}
static inline void init_nandchip_smcr_n(nfi_base *base, unsigned int cs, unsigned int value)
{
	base->writel(NEMC_SMCR(cs), value);
}

static inline void nand_enable(nand_io *io, unsigned int cs)
{
	const nand_timing_com *timing = io->timing;
	ndd_ndelay(timing ? timing->tALH : 500);
	io->base->writel(NEMC_NFCSR, (NEMC_NFCSR_NFE(cs) | NEMC_NFCSR_NFCE(cs)));
	ndd_ndelay(timing ? timing->tCS : 500);
}

static void nand_disable(nand_io *io, unsigned int cs)
{
	const nand_timing_com *timing = io->timing;
	ndd_ndelay(timing ? timing->tALH : 500);
	io->base->writel(NEMC_NFCSR, (NEMC_NFCSR_NFEC(cs) | NEMC_NFCSR_NFCEC(cs)));
}

static void setup_timing(nfi_base *base, int val)
{
	int cs_index;

	for(cs_index = 0; cs_index < CS_PER_NFI; cs_index++)
		init_nandchip_smcr_n(base, cs_index, val);

	ndd_debug("--NEMC_SMCR: value = 0x%08x \n", *(volatile unsigned int *)0xb3410014);
}

static void nand_io_setup_default(nfi_base *base)
{
	setup_timing(base, SMCR_DEFAULT_VAL);
}

static void nand_io_setup_optimize(nfi_base *base, const nand_timing_com *timing, int buswidth)
{
	int valume, smcr = 0;
	int cycle = 1000000000 / (base->rate / 1000);  //unit: ps

	/* NEMC.TAS */
	valume = (timing->tALS * 1000 + cycle - 1) / cycle;
	/**
	 * here we reduce one cycle, because that,
	 * IC maybe designed as add one cycle at
	 * TAS & TAH, but here we can't set TAS & TAH
	 * as '0', because set '0' can cause bugs of
	 * bitcount, the bitcount may count less that
	 * the really count if '0' be set at TAS & TAH
	*/
	valume -= (valume > 1) ? 1 : 0;
	smcr |= (valume & NEMC_SMCR_TAS_MASK) << NEMC_SMCR_TAS_BIT;
	/* NEMC.TAH */
	valume = (timing->tALH * 1000 + cycle -1) / cycle;
	valume -= (valume > 1) ? 1 : 0;
	smcr |= (valume & NEMC_SMCR_TAH_MASK) << NEMC_SMCR_TAH_BIT;
	/* NEMC.TBP */
	valume = (timing->tWP * 1000 + cycle - 1) / cycle;
	smcr |= (valume & NEMC_SMCR_TBP_MASK) << NEMC_SMCR_TBP_BIT;
	/* NEMC.TAW */
	valume = (timing->tRP * 1000 + cycle -1) / cycle;
	smcr |= (valume & NEMC_SMCR_TAW_MASK) << NEMC_SMCR_TAW_BIT;
	/* NEMC.STRV */
	valume = (timing->tRHW * 1000 + cycle - 1) / cycle;
	smcr |= (valume & NEMC_SMCR_STRV_MASK) << NEMC_SMCR_STRV_BIT;

	if (buswidth == 16)
		smcr |= 1 << NEMC_SMCR_BW_BIT;

	ndd_debug("INFO: tals=%d talh=%d twp=%d trp=%d smcr=0x%08x\n"
                        , timing->tALS, timing->tALH, timing->tWP, timing->tRP, smcr);

	setup_timing(base, smcr);
}

static inline void io_send(int cmd, void *port)
{
	*(volatile unsigned char *)port = cmd;
}

static inline void read_status(int *val, void *addr)
{
	*val = *(volatile unsigned char *)addr;
}

static void nand_send_addr(nand_io *io, int col_addr, int row_addr)
{
	int rowcycle = io->cinfo->rowcycles;
	int i;

	if(col_addr >= 0){
		if(io->cinfo->pagesize != 512){
			io_send(col_addr & 0xff, io->addrport);
			col_addr >>= 8;
		}
		io_send(col_addr & 0xff,io->addrport);
	}

	if(row_addr >= 0){
		for(i=0; i<rowcycle; i++){
			io_send(row_addr & 0xff, io->addrport);
			row_addr >>= 8;
		}
	}
}

static int nand_io_init(nfi_base *base)
{
	if (ref_cnt++ == 0)
		base->clk_enable();

	return 0;
}

static void nand_io_deinit(nfi_base *base)
{
	if (--ref_cnt == 0)
		base->clk_disable();
}

void nand_io_setup_default_16bit(nfi_base *base)
{
	setup_timing(base, SMCR_DEFAULT_VAL | (0x1 << 6));
}

int nand_io_open(nfi_base *base, chip_info *cinfo)
{
	nand_io *nandio = NULL;

	nandio = ndd_alloc(sizeof(nand_io));
	if(!nandio)
		RETURN_ERR(ENAND, "alloc io memory error");

	nandio->base = base;
	nand_io_init(nandio->base);
	if (cinfo) {
		nandio->cinfo = cinfo;
		nandio->timing = (const nand_timing_com *)(cinfo->ops_timing.io_timing);
		nand_io_setup_optimize(nandio->base, nandio->timing, cinfo->buswidth);
	} else {
		nandio->cinfo = NULL;
		nandio->timing = NULL;
		/*setup default 8bit buswidth, if you want to use 16bit,
		  you should call <nand_io_setup_default_16bit> after <nand_io_open>*/
		nand_io_setup_default(nandio->base);
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

int nand_io_chip_select(int context, int cs)
{
	nand_io *io = (nand_io *)context;

	io->dataport = io->base->cs_iomem[0] - cs * 0x01000000;
	io->cmdport = io->dataport + NAND_CMD_OFFSET;
	io->addrport = io->dataport + NAND_ADDR_OFFSET;
	//ndd_debug("cs = %d, csmemory = %p, dataport = %p\n",
	//	  cs, io->base->cs_iomem[0], io->dataport);
	nand_enable(io, cs);
	return 0;
}

int nand_io_chip_deselect(int context, int cs)
{
	nand_io *io = (nand_io *)context;

	nand_disable(io, cs);
	return 0;
}

int nand_io_send_cmd(int context, unsigned char command, unsigned int delay)
{
	int ret = 0;
	nand_io *io = (nand_io *)context;

	io_send(command,io->cmdport);
	ndd_ndelay(delay);

	if (command == NAND_CMD_STATUS)
		read_status(&ret, io->dataport);

	return ret;
}

void nand_io_send_addr(int context, int offset, int pageid, unsigned int delay)
{
	nand_io *io = (nand_io *)context;

	nand_send_addr(io, offset, pageid);
	ndd_ndelay(delay);
}

void nand_io_send_spec_addr(int context, int addr, unsigned int cycle, unsigned int delay)
{
	nand_io *io = (nand_io *)context;
	ndd_debug("%s addrport=%p, addr=0x%08x\n", __func__, io->addrport,addr);
	while(cycle--){
		io_send((addr & 0xff), io->addrport);
		addr = addr >> 0x08;
	}
	ndd_ndelay(delay);
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

int nand_io_send_waitcomplete(int context, chip_info *cinfo)
{
	ndd_ndelay(cinfo->ops_timing.tWC * 64);
	return 0;
}

int nand_io_suspend(void)
{
	return 0;
}

int nand_io_resume(void)
{
	return 0;
}

char* nand_io_get_clk_name(void) {
	return NAND_IO_CLK_NAME;
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
	assign(tCLH);
#undef assign
	ops_timing->tWC = timing->tWP + timing->tALS + timing->tALH;
	ops_timing->io_timing = (void *)timing;
}

void nand_controll_adpt(chip_info *cinfo, const nand_flash *ndflash) {
	convert2opstiming(&cinfo->ops_timing, &ndflash->timing);
	cinfo->ops_timing.io_etiming = &ndflash->nand_extra;
}
