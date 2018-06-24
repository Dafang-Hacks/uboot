#include <os_clib.h>
#include <nand_debug.h>
#include <nand_bch.h>
#include <cpu_trans.h>

#include <soc/jz_bch.h>
/*****  the operation of bch registers  *****/
static int ref_cnt = 0;

typedef struct __nand_bch {
	bch_base *base;
	unsigned int eccbit;
	unsigned int eccsize;
	unsigned int eccbytes;
	transadaptor trans;
	unsigned int copy_context;
} nand_bch;

/*
static void dump_regisers(void)
{
	ndd_debug("BHCR: 0x%08x\n", *(unsigned int *)0xb34d0000);
	ndd_debug("BHCNT: 0x%08x\n", *(unsigned int *)0xb34d000c);
	ndd_debug("BHINT: 0x%08x\n", *(unsigned int *)0xb34d0184);
	ndd_debug("BHINTE: 0x%08x\n", *(unsigned int *)0xb34d0190);
}
*/

/*
static void bch_encoding_nbit(nand_bch *ndbch,unsigned int n)
{
	unsigned int tmp = (BCH_CR_BSEL(n) | BCH_CR_ENCE | BCH_CR_BCHE |BCH_CR_INIT);

	ndbch->base->writel(BCH_CRS, tmp);
}

static void bch_decoding_nbit(nand_bch *ndbch,unsigned int n)
{
	unsigned int tmp = (BCH_CR_BSEL(n) | BCH_CR_DECE | BCH_CR_BCHE |BCH_CR_INIT);

	ndbch->base->writel(BCH_CRS, tmp);
}
*/

static void bch_disable(nand_bch *ndbch)
{
	ndbch->base->writel(BCH_CRC, BCH_CR_BCHE);
}

static void bch_encode_sync(nand_bch *ndbch)
{
	int (*readl)(int) = ndbch->base->readl;
	while(!(readl(BCH_INTS) & BCH_INTS_ENCF));
}

static void bch_decode_sync(nand_bch *ndbch)
{
	int (*readl)(int) = ndbch->base->readl;
	while(!(readl(BCH_INTS) & BCH_INTS_DECF));
}

/*
static void bch_decode_sdmf(nand_bch *ndbch)
{
	unsigned int tmp;
	int (*readl)(int) = ndbch->base->readl;

	tmp = readl(BCH_INTS);
	while(!(tmp & BCH_INTS_SDMF))
		tmp = readl(BCH_INTS);
}
*/
static void bch_encints_clear(nand_bch *ndbch)
{
	unsigned int tmp;

	tmp = ndbch->base->readl(BCH_INTS);
	tmp |= BCH_INTS_ENCF;
	ndbch->base->writel(BCH_INTS, tmp);
}

static void bch_decints_clear(nand_bch *ndbch)
{
	unsigned int tmp;

	tmp = ndbch->base->readl(BCH_INTS);
	tmp |= BCH_INTS_DECF;
	ndbch->base->writel(BCH_INTS, tmp);
}

/* blk = ECC_BLOCK_SIZE, par = ECC_PARITY_SIZE */
static void bch_cnt_set(nand_bch *ndbch)
{
	unsigned int tmp;

	tmp = ndbch->base->readl(BCH_CNT);
	tmp &= ~(BCH_CNT_PARITY_MASK | BCH_CNT_BLOCK_MASK);
	tmp |= ((ndbch->eccbytes) << BCH_CNT_PARITY_BIT | (ndbch->eccsize) << BCH_CNT_BLOCK_BIT);
	ndbch->base->writel(BCH_CNT, tmp);
}

static void bch_enable(nand_bch *ndbch, unsigned int mode, unsigned char eccbit)
{
	unsigned int tmp = BCH_CR_BCHE;
	int (*readl)(int) = ndbch->base->readl;
	void (*writel)(int, int) = ndbch->base->writel;

	ndbch->eccbit = eccbit;
	ndbch->eccbytes = __bch_cale_eccbytes(eccbit);
	if (mode)
		tmp |= BCH_CR_ENCE;
	bch_cnt_set(ndbch);
	tmp |= BCH_CR_BSEL(eccbit);

	writel(BCH_CRC, (~tmp));
	writel(BCH_CRS, tmp);
	writel(BCH_CRS, BCH_CR_MZEB(eccbit > 12 ? 7 : 2));
	writel(BCH_CRS, BCH_CR_INIT);
	writel(BCH_INTS, 0xffffffff);

	/* if clear ints faild, clear again */
	if (readl(BCH_INTS) & 0x003f) {
		ndd_print(NDD_WARNING, "WARNING: bch clear BHINT faild!\n");
		writel(BCH_INTS, 0xffffffff);
	}
}

static inline void bch_decode_enable(nand_bch *ndbch, unsigned char eccbit)
{
	bch_enable(ndbch,BCH_DECODE, eccbit);
}

static inline void bch_encode_enable(nand_bch *ndbch, unsigned char eccbit)
{
	bch_enable(ndbch,BCH_ENCODE, eccbit);
}

/**
 * bch_correct
 * @dat:        data to be corrected
 * @idx:        the index of error bit in an eccsize
 */
static void bch_correct(unsigned char *dat, unsigned int idx, unsigned int eccsize)
{
	int i, bits;		/* the 'bits' of i unsigned short is error */
	unsigned short *ptmp = (unsigned short *)dat;

	i = idx & BCH_ERR_INDEX_MASK;
	bits = (idx & BCH_ERR_MASK_MASK)>>BCH_ERR_MASK_BIT;

	if (i < (eccsize>>1))
		ptmp[i] ^= bits;
}

static int bch_decode_correct(nand_bch *ndbch, PipeNode *pipe)
{
	int i;
	volatile unsigned int *errs;
	unsigned int stat, errbits = 0, errcnt = 0;

	/* Check decoding */
	stat = ndbch->base->readl(BCH_INTS);

	if(stat & BCH_INTS_ALLf) {
		ndd_print(NDD_DEBUG, "ERROR: BITCOUNT is in use, BCH should not get ALL_FF status!\n");
		return ALL_FF;
	}

	if (stat & BCH_INTS_UNCOR) {
		ndd_print(NDD_DEBUG, "Uncorrectable ECC error -- stat = 0x%08x", stat);
		return ECC_ERROR;
	} else {
		if (stat & BCH_INTS_ERR) {
			/* Error occurred */
			errcnt = (stat & BCH_INTS_ERRC_MASK) >> BCH_INTS_ERRC_BIT;
			errbits = (stat & BCH_INTS_TERRC_MASK) >> BCH_INTS_TERRC_BIT;

			/*begin at the second DWORD*/
			errs = (volatile unsigned int *)(ndbch->base->iomem+BCH_ERR0);
			for (i = 0; i < errcnt; i++)
			{
				bch_correct(pipe->data, errs[i], ndbch->eccsize);
			}
		}
	}

	return errbits;
}

static int nand_bch_init(bch_base *base)
{
	if (ref_cnt++ == 0) {
		if (base->clk_enable())
			RETURN_ERR(ECC_ERROR, "bch clock enable error!\n");
	}

	return 0;
}

static void nand_bch_deinit(bch_base *base)
{
	if (--ref_cnt == 0)
		base->clk_disable();
}

int nand_bch_open(bch_base *base, int eccsize)
{
	nand_bch *ndbch;

	ndbch = ndd_alloc(sizeof(nand_bch));
	if(!ndbch)
		RETURN_ERR(ENAND, "alloc bch memory error");

	ndbch->base = base;
	nand_bch_init(ndbch->base);

	ndbch->eccsize = eccsize;
	ndbch->copy_context = cpu_move_init(&ndbch->trans);
	if(ndbch->copy_context < 0) {
		ndd_free(ndbch);
		RETURN_ERR(0, "cpu move init  error");
	}

	return (int)ndbch;
}

void nand_bch_close(int context)
{
	nand_bch *bch = (nand_bch *)context;

	cpu_move_deinit(bch->copy_context,&bch->trans);
	nand_bch_deinit(bch->base);
	ndd_free(bch);
}

int nand_bch_suspend(void)
{
	return 0;
}

int nand_bch_resume(void)
{
	return 0;
}

int nand_bch_encode_prepare(int context, PipeNode *pipe, unsigned char eccbit)
{
	int ret = 0;
	nand_bch *bch = (nand_bch *)context;
	unsigned int eccsize = bch->eccsize;

	bch_encode_enable(bch, eccbit);
	//dump_regisers();
	ret = bch->trans.prepare_memcpy(bch->copy_context,
					pipe->data,
					(unsigned char *)(bch->base->iomem+BCH_DR),
					eccsize,
					SRCADD);
	if(ret < 0)
		RETURN_ERR(ENAND, "prepare memcpy error");

	ret = bch->trans.finish_memcpy(bch->copy_context);
	if(ret < 0)
		RETURN_ERR(ENAND, "finish memcpy error");

	return 0;
}

int nand_bch_encode_complete(int context, PipeNode *pipe)
{
	int ret = 0;
	nand_bch *bch = (nand_bch *)context;
	unsigned int eccbytes = bch->eccbytes;
	unsigned char *paraddr = (unsigned char *)(bch->base->iomem+BCH_PAR0);

	bch_encode_sync(bch);

	ret = bch->trans.prepare_memcpy(bch->copy_context,
					paraddr,
					pipe->parity,
					eccbytes,
					SRC_AND_DST_ADD);
	if(ret < 0)
		RETURN_ERR(ENAND, "prepare memcpy error");

	ret = bch->trans.finish_memcpy(bch->copy_context);
	if(ret < 0)
		RETURN_ERR(ENAND, "finish memcpy error");

	bch_encints_clear(bch);
	bch_disable(bch);

	return 0;
}

int nand_bch_decode_prepare(int context, PipeNode *pipe, unsigned char eccbit)
{
	int ret = 0;
	nand_bch *bch = (nand_bch *)context;

	bch_decode_enable(bch, eccbit);
	//dump_regisers();

	/*write data and parity to BCH_DR*/
	ret = bch->trans.prepare_memcpy(bch->copy_context,
					pipe->data,
					(unsigned char *)(bch->base->iomem+BCH_DR),
					bch->eccsize,
					SRCADD);
	if(ret < 0)
		RETURN_ERR(ENAND, "prepare memcpy error");

	ret = bch->trans.finish_memcpy(bch->copy_context);
	if(ret < 0)
		RETURN_ERR(ENAND, "finish memcpy error");

	ret = bch->trans.prepare_memcpy(bch->copy_context,
					pipe->parity,
					(unsigned char *)(bch->base->iomem+BCH_DR),
					bch->eccbytes,
					SRCADD);
	if(ret < 0)
		RETURN_ERR(ENAND, "prepare memcpy error");

	ret = bch->trans.finish_memcpy(bch->copy_context);
	if(ret < 0)
		RETURN_ERR(ENAND, "finish memcpy error");

	return 0;
}

int nand_bch_decode_complete(int context, PipeNode *pipe)
{
	nand_bch *bch = (nand_bch *)context;
	int errcnt = 0;

	/* Wait for completion */
	bch_decode_sync(bch);
	errcnt = bch_decode_correct(bch,pipe);
	bch_decints_clear(bch);
	bch_disable(bch);

	return errcnt;
}

int get_parity_size(unsigned char eccbit)
{
	return __bch_cale_eccbytes(eccbit);
}
char *nand_bch_get_clk_name(void) {
	return BCH_CLK_NAME;
}
char *nand_bch_get_cgu_clk_name(void) {
	return BCH_CGU_CLK_NAME;
}
