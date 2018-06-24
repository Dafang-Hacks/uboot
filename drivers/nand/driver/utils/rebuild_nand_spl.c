/**
 * rebuild nand spl
 **/
#include "nand_chip.h"
#include "rebuild_nand_spl.h"

static void *__memcpy(void *dst, const void *src, unsigned int count)
{
	char *tmp = dst;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;

	return dst;
}

static void *__memset(void *s, int c, unsigned int count)
{
	char *xs = s;

	while (count--)
		*xs++ = c;
	return s;
}

/**
 * JZ4775: nand boot specification area
 * 64:   0 ~  63: BusWidth_flag
 * 64:  64 ~ 127: NandType_flag
 * 32: 128 ~ 159: RowCycle_flag
 * 32: 160 ~ 191: PageSize_flag2
 * 32: 192 ~ 223: PageSize_flag1
 * 32: 224 ~ 255: PageSize_flag0
 **/
static int gen_spl_header_4775(unsigned int page_size,
			       unsigned int bus_width,
			       unsigned int row_cycle,
			       unsigned int nand_type,
			       void *buf)
{
	int pagesize_flag;
	struct parm_buf {
		void *bw_buf;
		void *tp_buf;
		void *rc_buf;
		void *pf2_buf;
		void *pf1_buf;
		void *pf0_buf;
	} parm_buf = {buf, buf + 64, buf + 128,	buf + 160, buf + 192, buf + 224};

	switch (page_size) {
	case 512:
		pagesize_flag = 0;
		break;
	case 2048:
		pagesize_flag = 2;
		break;
	case 4096:
		pagesize_flag = 4;
		break;
	case 8192:
		pagesize_flag = 6;
		break;
	case 16384:
		pagesize_flag = 7;
		break;
	default:
		return -1;
	}

	__memset(parm_buf.bw_buf, (bus_width == 16) ? 0xAA : 0x55, 64);
	__memset(parm_buf.tp_buf, (nand_type == NAND_TYPE_TOGGLE) ? 0xAA : 0x55, 64);
	__memset(parm_buf.rc_buf, (row_cycle == 3) ? 0xAA : 0x55, 32);
	__memset(parm_buf.pf2_buf, (pagesize_flag >> 2 & 1) ? 0xAA : 0x55, 32);
	__memset(parm_buf.pf1_buf, (pagesize_flag >> 1 & 1) ? 0xAA : 0x55, 32);
	__memset(parm_buf.pf0_buf, (pagesize_flag >> 0 & 1) ? 0xAA : 0x55, 32);

	return 0;
}

/**
 * JZ4780: nand boot specification area
 * 64:   0 ~  63: NandType_flag  (0x55: common nand, 0xaa: toggle nand)
 * 32:  64 ~  95: RowCycle_flag  (0x55: 2-byte row cycles, 0xaa: 3-byte row cycles)
 * 32:  96 ~ 127: PageSize_flag2
 * 32: 128 ~ 159: PageSize_flag1
 * 32: 160 ~ 191: PageSize_flag0
 **/
static int gen_spl_header_4780(unsigned int page_size,
			       unsigned int bus_width,
			       unsigned int row_cycle,
			       unsigned int nand_type,
			       void *buf)
{
	int pagesize_flag;
	struct parm_buf {
		void *tp_buf;
		void *rc_buf;
		void *pf2_buf;
		void *pf1_buf;
		void *pf0_buf;
	} parm_buf = {buf, buf + 64, buf + 96, buf + 128, buf + 160};

	switch (page_size) {
	case 512:
		pagesize_flag = 0;
		break;
	case 2048:
		pagesize_flag = 2;
		break;
	case 4096:
		pagesize_flag = 4;
		break;
	case 8192:
		pagesize_flag = 6;
		break;
	case 16384:
		pagesize_flag = 7;
		break;
	default:
		return -1;
	}

	__memset(parm_buf.tp_buf, (nand_type == NAND_TYPE_TOGGLE) ? 0xAA : 0x55, 64);
	__memset(parm_buf.rc_buf, (row_cycle == 3) ? 0xAA : 0x55, 32);
	__memset(parm_buf.pf2_buf, (pagesize_flag >> 2 & 1) ? 0xAA : 0x55, 32);
	__memset(parm_buf.pf1_buf, (pagesize_flag >> 1 & 1) ? 0xAA : 0x55, 32);
	__memset(parm_buf.pf0_buf, (pagesize_flag >> 0 & 1) ? 0xAA : 0x55, 32);

	return 0;
}

/**
 * M200: nand boot specification area
 * 64:   0 ~  63: BusWidth_flag
 * 64:  64 ~ 127: NandType_flag
 * 4: 128 ~ 131: RowCycle
 * 4: 132 ~ 135: PageSize
 * 24: 136 ~ 159: reserved
 * 96: 160 ~ 255: reserved for nand basic params
 **/
static int gen_spl_header_m200(unsigned int page_size,
			       unsigned int bus_width,
			       unsigned int row_cycle,
			       unsigned int nand_type,
			       void *buf)
{
	struct parm_buf {
		void *bw_buf;
		void *tp_buf;
		unsigned int *rowcycle;
		unsigned int *pagesize;
	} parm_buf = {buf, buf + 64, buf + 128, buf + 132};

	__memset(parm_buf.bw_buf, (bus_width == 16) ? 0xAA : 0x55, 64);
	__memset(parm_buf.tp_buf, (nand_type == NAND_TYPE_TOGGLE) ? 0xAA : 0x55, 64);
	*(parm_buf.rowcycle) = row_cycle;
	*(parm_buf.pagesize) = page_size;

	return 0;
}

/**
 * rebuild nand Boot Specification area and patch nand basic params.
 * NOTE1: spl_sbuf and spl_dbuf can use the same buf.
 * NOTE2: size of the spl_dbuf must >= nand_spl.bin + 256.
 **/
#define NAND_SPL_SIZE_4775		(16 * 1024)
#define NAND_PARAMS_OFFSET_4775		NAND_SPL_SIZE_4775
int rebuild_nand_spl_4775(nand_params *ndparams, void *spl_sbuf, void *spl_dbuf) {
	int ret;
	nand_basic_info *ndinfo = &ndparams->ndbaseinfo;

	if (spl_sbuf != spl_dbuf)
		__memcpy(spl_dbuf, spl_sbuf, NAND_SPL_SIZE_4775);

	ret = gen_spl_header_4775(ndinfo->pagesize, ndinfo->buswidth,
				  ndinfo->rowcycles, REBUILD_GET_NAND_TYPE(ndinfo->options), spl_dbuf);
	if (ret)
		return -1;

	__memcpy(spl_dbuf + NAND_PARAMS_OFFSET_4775, ndparams, sizeof(nand_params));

	return 0;
}

/**
 * rebuild nand Boot Specification area and patch nand basic params.
 * NOTE1: spl_sbuf and spl_dbuf can use the same buf.
 * NOTE2: size of the spl_dbuf must >= nand_spl.bin + 256.
 **/
#define NAND_SPL_SIZE_4780		(16 * 1024)
#define NAND_PARAMS_OFFSET_4780		NAND_SPL_SIZE_4780
int rebuild_nand_spl_4780(nand_params *ndparams, void *spl_sbuf, void *spl_dbuf) {
	int ret;
	nand_basic_info *ndinfo = &ndparams->ndbaseinfo;

	if (spl_sbuf != spl_dbuf)
		__memcpy(spl_dbuf, spl_sbuf, NAND_SPL_SIZE_4780);

	ret = gen_spl_header_4780(ndinfo->pagesize, ndinfo->buswidth,
				  ndinfo->rowcycles, REBUILD_GET_NAND_TYPE(ndinfo->options), spl_dbuf);
	if (ret)
		return -1;

	__memcpy(spl_dbuf + NAND_PARAMS_OFFSET_4780, ndparams, sizeof(nand_params));

	return 0;
}

/**
 * rebuild nand Boot Specification area and patch nand basic params.
 * NOTE1: spl_sbuf and spl_dbuf can use the same buf.
 * NOTE2: size of the spl_dbuf must >= nand_spl.bin.
 **/
#define NAND_SPL_SIZE_M200 (32 * 1024)
#define NAND_PARAMS_OFFSET_M200	(128 + 32)
int rebuild_nand_spl_m200(nand_params *ndparams, void *spl_sbuf, void *spl_dbuf) {
	int ret;
	nand_basic_info *ndinfo = &ndparams->ndbaseinfo;

	if (spl_sbuf != spl_dbuf)
		__memcpy(spl_dbuf, spl_sbuf, NAND_SPL_SIZE_M200);

	ret = gen_spl_header_m200(ndinfo->pagesize, ndinfo->buswidth,
				  ndinfo->rowcycles, REBUILD_GET_NAND_TYPE(ndinfo->options), spl_dbuf);
	if (ret)
		return -1;

	__memcpy(spl_dbuf + NAND_PARAMS_OFFSET_M200, ndparams, sizeof(nand_params));

	return 0;
}
