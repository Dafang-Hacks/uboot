/**
 * nand_info.c
 **/

#include <os_clib.h>
#include "nand_info.h"
#include "nand_debug.h"
#include "ndcommand.h"
#include "nand_io.h"

extern void (*__clear_rb_state)(rb_item *);
extern int (*__wait_rb_timeout) (rb_item *, int); /* return 0 -> ok; !0 -> timeout */
extern int (*__try_wait_rb) (rb_item *, int);

static void dump_retry_parms(retry_parms *retryparms)
{
	int i;
	unsigned char *data = (unsigned char *)(retryparms->data);

	ndd_debug("\ndump retry parms, recycle = %d, regcnt = %d",
		  retryparms->cycle, retryparms->regcnt);
	for (i = 0; i < retryparms->cycle * retryparms->regcnt; i++) {
		if (!(i % retryparms->regcnt))
			ndd_debug("\n %d:", i / retryparms->regcnt);
		ndd_debug(" %02x", data[i]);
	}
	ndd_debug("\n\n");
}

#define TRY_RB_DELAY 1 // 1ms
static rb_item* wait_unknown_rb_timeout(rb_info *rbinfo, int timeout)
{
	int i, wait_times = 0;

	for (i = 0; wait_times < timeout; i++)
	{
		if (__try_wait_rb(&(rbinfo->rbinfo_table[i % rbinfo->totalrbs]), TRY_RB_DELAY))
			break;
		wait_times += TRY_RB_DELAY;
	}

	if (wait_times >= timeout)
		return NULL;
	else
		return &(rbinfo->rbinfo_table[i % rbinfo->totalrbs]);
}

static void clear_all_rb_state(rb_info *rbinfo)
{
	int rb_index;

	for (rb_index = 0; rb_index < rbinfo->totalrbs; rb_index++)
		__clear_rb_state(&rbinfo->rbinfo_table[rb_index]);
}

rb_item* get_rbitem(nfi_base *base, unsigned int cs_id, rb_info *rbinfo)
{
	int context, ret;
	rb_item *rbitem = NULL;

	context = nand_io_open(base, NULL);
	if (!context)
		RETURN_ERR(NULL, "nand io open error");

	ret = nand_io_chip_select(context, cs_id);
	if (ret)
		RETURN_ERR(NULL, "nand io chip select error, cs = [%d]", cs_id);

	clear_all_rb_state(rbinfo);

	ret = nand_io_send_cmd(context, CMD_RESET_1ST, 300);
	if (ret)
		RETURN_ERR(NULL, "nand io sent cmd error, cs = [%d], cmd = [%d]", cs_id, CMD_RESET_1ST);

	rbitem = wait_unknown_rb_timeout(rbinfo, 8);
	ret = nand_io_chip_deselect(context, cs_id);
	if (ret)
		RETURN_ERR(NULL, "nand io chip deselect error, cs = [%d]", cs_id);

	nand_io_close(context);

	return rbitem;
}

/**
 * early_nand_prepare: nand will be reset.
 *
 * @return: 0: success, !0: fail
 **/
int early_nand_prepare(nfi_base *base, unsigned int cs_id)
{
	int context, ret;

	context = nand_io_open(base, NULL);
	if (!context)
		RETURN_ERR(ENAND, "nand io open error");

	ret = nand_io_chip_select(context, cs_id);
	if (ret)
		RETURN_ERR(ENAND, "nand io chip select error, cs = [%d]", cs_id);

	ret = nand_io_send_cmd(context, CMD_RESET_1ST, 300);
	if (ret)
		RETURN_ERR(ENAND, "nand io sent cmd error, cs = [%d], cmd = [%d]", cs_id, CMD_RESET_1ST);

	ndd_ndelay(5 * 1000 * 1000);
	nand_io_chip_deselect(context, cs_id);
	nand_io_close(context);
	return SUCCESS;
}

struct hy_rr_msg {
	unsigned int offset;
        unsigned char addr1[2];
        unsigned char wdata[2];
        unsigned char addr2[5];
        unsigned char setaddr[8];
};

struct mt_rr_ada_msg{
	unsigned int offset;
	unsigned int feature_addr;
	unsigned int set_value[8];
};

struct mt_rr_ada_msg mt_rr_29f_32g_ada = {
	0x00,
	0x89,
	{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07}
};

struct hy_rr_msg hy_rr_f26_32g = {
	0x00,
	{0x00, 0x00},
	{0x00, 0x00},
	{0xa7, 0xad, 0xae, 0xaf, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

struct hy_rr_msg hy_rr_f20_64g_a = {
	2,
	{0xff, 0xcc},
	{0x40, 0x4d},
	{0x00, 0x00, 0x00, 0x02, 0x00},
	{0xcc, 0xbf, 0xaa, 0xab, 0xcd, 0xad, 0xae, 0xaf},
};

struct hy_rr_msg hy_rr_f20_64g_b = {
	2,
	{0xae, 0xb0},
	{0x00, 0x4d},
	{0x00, 0x00, 0x00, 0x02, 0x00},
	{0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7},
};

struct hy_rr_msg hy_rr_f20_32g = {
	2,
	{0xae, 0xb0},
	{0x00, 0x4d},
	{0x00, 0x00, 0x00, 0x02, 0x00},
	{0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7},
};

struct hy_rr_msg hy_rr_f1y_64g = {
	16,
	/**
	 * 0xff is not legal cmd in this area,
	 * because 0xff is RESET CMD, so here we
	 * use to indicate invalid value
	 **/
	{0x38, -1},
	{0x52, -1},
	{0x00, 0x00, 0x00, 0x021, 0x00},
	{0x38, 0x39, 0x3a, 0x3b, 0xff, 0xff, 0xff, 0xff},
};
static int set_retry_f26(int context, retry_parms *retryparms, struct hy_rr_msg *msg, int index)
{
	int i = 0;
	unsigned char *oridata = (unsigned char *)(retryparms->data);
	unsigned char setf26data[4] = {0, 0, 0, 0};
	unsigned char f26mlc[7][4] = {
		{0x00, 0x00, 0x00, 0x00},
		{0x00, 0x06, 0x0a, 0x06},
		{0xff, 0x03, 0x07, 0x08},
		{0xff, 0x06, 0x0d, 0x0f},
		{0xff, 0x09, 0x14, 0x17},
		{0xff, 0xff, 0x1a, 0x1e},
		{0xff, 0xff, 0x20, 0x25},
		};
	/* retryparms->cycle should be 7. retryparms->regcnt should be 4. */
	index = (index % retryparms->cycle);
	switch(index){
		case 0:
		case 1:
			for(i = 0; i < 4; i++)
				setf26data[i] = oridata[i] + f26mlc[index][i];
			break;
		default:
			for(i = 0; i < 4; i++)
				setf26data[i] = ((f26mlc[index][i] == 0xff) ? 0x00 : (oridata[i] - f26mlc[index][i]));
			break;
	}
	nand_io_send_cmd(context, 0x36, 300);
	for(i = 0;  i < 4; i++){
		nand_io_send_spec_addr(context, msg->addr2[i], 1, 300);
		nand_io_send_data(context, &(setf26data[i]), 1);
	}
	nand_io_send_cmd(context, 0x16, 300);
	return 0;
}
static int set_retry_othermode(int context, retry_parms *retryparms, struct hy_rr_msg *msg, int index)
{
	int i = 0;
	unsigned char regcnt = retryparms->regcnt;
	unsigned char *oridata = (unsigned char *)(retryparms->data);
	nand_io_send_cmd(context, 0x36, 300);
	for(i = 0;  i < regcnt; i++){
		nand_io_send_spec_addr(context, msg->setaddr[i], 1, 300);
		nand_io_send_data(context, &(oridata[index * regcnt + i]), 1);
	}
	nand_io_send_cmd(context, 0x16, 300);
	return 0;
}
static int set_retry_mt_ada(int data,int context,struct mt_rr_ada_msg *msg,int index,int cs_id)
{

	nand_data *ndata = (nand_data *)data;
	io_base *iobase = ndata->base;
	int ret;
	int timeout = 10;
	int timeout_ns = timeout * 1000 * 1000;
	unsigned char feature_value[4] = {0};

	ndata->clear_rb(cs_id);

	nand_io_send_cmd(context, CMD_SET_FEATURES, 0);
	nand_io_send_spec_addr(context,msg->feature_addr , 1, 300);

	feature_value[0] = msg->set_value[index];
	nand_io_send_data(context,&feature_value, 4);

	ret = ndata->wait_rb(cs_id, timeout);
	if(ret < 0)
		RETURN_ERR(ENAND, "set retry feature ,wait rb error !! \n");

	return 0;
}

int set_hynix_retry_feature(int data, unsigned int cs_id, int cycle)
{
	int ret, context;
	static unsigned char cs_retry[4] = {0, 0, 0, 0};
	nand_data *ndata = (nand_data *)data;
	chip_info *cinfo = ndata->cinfo;
	io_base *iobase = ndata->base;
	retry_parms *retryparms = cinfo->retryparms;

	/* io open */
	context = nand_io_open(&(iobase->nfi), cinfo);
	if (!context)
		RETURN_ERR(ENAND, "nand io open error");

	/* chip select */
	ret = nand_io_chip_select(context, cs_id);
	if (ret)
		GOTO_ERR(chip_select_setretry);
	switch (retryparms->mode) {
		case HY_RR_F26_32G_MLC:
			cs_retry[cs_id] = (cs_retry[cs_id] + cycle) % retryparms->cycle;
			ret = set_retry_f26(context, retryparms, &hy_rr_f26_32g, cs_retry[cs_id]);
			break;
		case HY_RR_F20_64G_MLC_A:
			cs_retry[cs_id] = (cs_retry[cs_id] + cycle) % retryparms->cycle;
			ret = set_retry_othermode(context, retryparms, &hy_rr_f20_64g_a, cs_retry[cs_id]);
			break;
		case HY_RR_F20_64G_MLC_B:
			cs_retry[cs_id] = (cs_retry[cs_id] + cycle) % retryparms->cycle;
			ret = set_retry_othermode(context, retryparms, &hy_rr_f20_64g_b, cs_retry[cs_id]);
			break;
		case HY_RR_F20_32G_MLC_C:
			cs_retry[cs_id] = (cs_retry[cs_id] + cycle) % retryparms->cycle;
			ret = set_retry_othermode(context, retryparms, &hy_rr_f20_32g, cs_retry[cs_id]);
			break;
		case HY_RR_F1Y_64G_MLC:
			cs_retry[cs_id] = (cs_retry[cs_id] + cycle) % retryparms->cycle;
			ret = set_retry_othermode(context, retryparms, &hy_rr_f1y_64g, cs_retry[cs_id]);
			break;
		default:
			nand_io_chip_deselect(context, cs_id);
			nand_io_close(context);
			RETURN_ERR(ENAND, "unknown hynix set retry feature mode");
	}

	nand_io_chip_deselect(context, cs_id);
ERR_LABLE(chip_select_setretry):
	nand_io_close(context);
	return ret;
}
int set_micron_retry_feature(int data, unsigned int cs_id, int cycle)
{
	int ret, context;
	static unsigned char cs_retry[4] = {0, 0, 0, 0};
	nand_data *ndata = (nand_data *)data;
	chip_info *cinfo = ndata->cinfo;
	io_base *iobase = ndata->base;
	retry_parms *retryparms = cinfo->retryparms;

	/* io open */
	context = nand_io_open(&(iobase->nfi), cinfo);
	if (!context)
		RETURN_ERR(ENAND, "nand io open error");

	/* chip select */
	ret = nand_io_chip_select(context, cs_id);
	if (ret)
		GOTO_ERR(chip_select_setretry);
	switch (retryparms->mode) {
		case MT_RR_29F_32G_MLC_ADA:
			cs_retry[cs_id] = (cs_retry[cs_id] + cycle) % retryparms->cycle;
			ret = set_retry_mt_ada(data,context,&mt_rr_29f_32g_ada,cs_retry[cs_id],cs_id);
			break;
		default:
			nand_io_chip_deselect(context, cs_id);
			nand_io_close(context);
			RETURN_ERR(ENAND, "unknown hynix set retry feature mode");
	}

	nand_io_chip_deselect(context, cs_id);
	ERR_LABLE(chip_select_setretry):
	nand_io_close(context);
	return ret;
}

int set_retry_feature(int data, unsigned int cs_id, int cycle)
{
	int ret;
	nand_data *ndata = (nand_data *)data;
	chip_info *cinfo = ndata->cinfo;

	switch(cinfo->manuf){
		case NAND_MFR_MICRON:
			ret = set_micron_retry_feature(data,cs_id,cycle);
			break;
		case NAND_MFR_HYNIX:
			ret = set_hynix_retry_feature(data,cs_id,cycle);
			break;
		default:
			RETURN_ERR(ENAND, "unknownset retry feature mode");

	}
	return ret;
}
int get_retry_f26_data(int context, retry_parms *retryparms, struct hy_rr_msg *msg)
{
	unsigned char *buf = (unsigned char *)(retryparms->data);
	unsigned int i = 0;
	nand_io_send_cmd(context, 0x37, 300);
	for (i = 0; i < retryparms->regcnt; i++) {
		nand_io_send_spec_addr(context, msg->addr2[i], 1, 300);
		nand_io_receive_data(context, buf + i, 1);
	}
	return 0;
}
static int get_hynix_retry_parms(nfi_base *base, unsigned int cs_id, rb_info *rbinfo, retry_parms *retryparms)
{
	int i, ret, context, datasize;
	int retry_flag, retrycnt = 0;
	struct hy_rr_msg *msg = NULL;
	unsigned char inversed_val[RETRY_DATA_SIZE];
	unsigned char *buf = (unsigned char *)(retryparms->data);
        unsigned char cmd[4] = {0x16, 0x17, 0x04, 0x19};

	switch (retryparms->mode) {
		case HY_RR_F26_32G_MLC:
			retryparms->cycle = 7;
			retryparms->regcnt = 4;
			msg = &hy_rr_f26_32g;
			break;
		case HY_RR_F20_64G_MLC_A:
			retryparms->cycle = 8;
			retryparms->regcnt = 8;
			msg = &hy_rr_f20_64g_a;
			break;
		case HY_RR_F20_64G_MLC_B:
			retryparms->cycle = 8;
			retryparms->regcnt = 8;
			msg = &hy_rr_f20_64g_b;
			break;
		case HY_RR_F20_32G_MLC_C:
			retryparms->cycle = 8;
			retryparms->regcnt = 8;
			msg = &hy_rr_f20_32g;
			break;
		case HY_RR_F1Y_64G_MLC:
			retryparms->cycle = 8;
			retryparms->regcnt = 4;
			msg = &hy_rr_f1y_64g;
			break;
		default:
			RETURN_ERR(ENAND, "unknown hynix retry mode");
	}

	/* io open */
	context = nand_io_open(base, NULL);
	if (!context)
		RETURN_ERR(ENAND, "nand io open error");

	/* chip select */
	ret = nand_io_chip_select(context, cs_id);
	if (ret)
		GOTO_ERR(chip_select);

	/* reset */
#if 0
	clear_all_rb_state(rbinfo);
	nand_io_send_cmd(context, CMD_RESET_1ST, 300);
	if ((wait_unknown_rb_timeout(rbinfo, 8)) == NULL) {
		ret = TIMEOUT;
		GOTO_ERR(get_data);
	}
#endif
	/* send read otp cmd and data */
	if (retryparms->mode == HY_RR_F26_32G_MLC) {
		ret = get_retry_f26_data(context, retryparms, msg);
		goto retry_finish;
	}
	nand_io_send_cmd(context, 0x36, 300);
	for (i = 0; i < 2; i++) {
		if (msg->addr1[i] != ((unsigned char)(-1))) {
			nand_io_send_spec_addr(context, msg->addr1[i], 1, 300);
			nand_io_send_data(context, &(msg->wdata[i]), 1);
		}
	}
        for (i = 0; i < 4; i++)
                nand_io_send_cmd(context, cmd[i], 300);

	/* sent read cmd */
	nand_io_send_cmd(context, CMD_PAGE_READ_1ST, 300);
	for (i = 0; i < 5; i++)
		nand_io_send_spec_addr(context, msg->addr2[i], 1, 300);

	clear_all_rb_state(rbinfo);
	nand_io_send_cmd(context, CMD_PAGE_READ_2ND, 300);
	if ((wait_unknown_rb_timeout(rbinfo, 8)) == NULL) {
		ret = TIMEOUT;
		GOTO_ERR(get_data);
	}

	/* ignore recycle and regcnt data */
	nand_io_receive_data(context, buf, (msg->offset - 0));

	/* calc datasize */
	datasize = retryparms->cycle * retryparms->regcnt;

	/* receive and check data */
retry:
	retry_flag = 0;
	nand_io_receive_data(context, buf, datasize);
	nand_io_receive_data(context, inversed_val, datasize);

	for (i = 0; i < datasize; i++) {
		if (buf[i] != (unsigned char)(~(inversed_val[i]))) {
			ndd_debug("buf[%d] = %02x, ~(inversed_val) = %02x\n", i, buf[i], ~(inversed_val[i]));
			retry_flag = 1;
		}
	}

	if (retry_flag) {
		if (retrycnt++ < 8) {
			goto retry;
		} else {
			ret = IO_ERROR;
			GOTO_ERR(get_data);
		}
	}

	/* reset */
	clear_all_rb_state(rbinfo);
	nand_io_send_cmd(context, CMD_RESET_1ST, 300);
	if ((wait_unknown_rb_timeout(rbinfo, 8)) == NULL) {
		ret = TIMEOUT;
		GOTO_ERR(get_data);
	}

	if (retryparms->mode == HY_RR_F1Y_64G_MLC) {
		unsigned char tmp = 0x00;
		nand_io_send_cmd(context, 0x36, 300);
		nand_io_send_spec_addr(context, 0x38, 1, 300);
		nand_io_send_data(context, &tmp, 1);
		nand_io_send_cmd(context, 0x16, 300);
		/* read any page */
		nand_io_send_cmd(context, CMD_PAGE_READ_1ST, 300);
		/* we can't sure that addr is 5 cycles or 1 cycle, so that the step is dangerous. */
#if 1
		nand_io_send_spec_addr(context, msg->addr2[0], 1, 300);
#else
		for (i = 0; i < 5; i++)
			nand_io_send_spec_addr(context, msg->addr2[i], 1, 300);
#endif
		clear_all_rb_state(rbinfo);
		nand_io_send_cmd(context, CMD_PAGE_READ_2ND, 300);
	} else {
		clear_all_rb_state(rbinfo);
		nand_io_send_cmd(context, 0x38, 300);
	}

	if ((wait_unknown_rb_timeout(rbinfo, 8)) == NULL) {
		ret = TIMEOUT;
		GOTO_ERR(get_data);
	}

	ret = SUCCESS;
	dump_retry_parms(retryparms);

retry_finish:
ERR_LABLE(get_data):
	nand_io_chip_deselect(context, cs_id);
ERR_LABLE(chip_select):
	nand_io_close(context);

	return ret;
}

static int get_micron_retry_params(retry_parms *retryparms)
{
	switch(retryparms->mode){
		case MT_RR_29F_32G_MLC_ADA:
			retryparms->cycle = 8;
			break;
		default:
			RETURN_ERR(ENAND, "unknown micron retry mode");

	}
	return 0;
}

int get_retry_parms(nfi_base *base,chip_info *cinfo, unsigned int cs_id, rb_info *rbinfo, retry_parms *retryparms)
{

	int ret;

	switch (cinfo->manuf) {
		case NAND_MFR_MICRON:
			ret = get_micron_retry_params(retryparms);
			break;
		case NAND_MFR_HYNIX:
			ret = get_hynix_retry_parms(base,cs_id,rbinfo,retryparms);
			break;
		default:
			RETURN_ERR(ENAND, "unknown the Manufacturer of nand !!!\n");
	}

	return ret;
}
int __set_features(int io_context, rb_item *rbitem, nand_ops_timing *timing,
		   unsigned char addr, unsigned char *data, int len)
{
	nand_io_send_cmd(io_context, CMD_SET_FEATURES, 0);
	nand_io_send_spec_addr(io_context, addr, 1, timing->tADL);
	nand_io_send_data(io_context, data, len);
	ndd_ndelay(timing->tWB);
	if (__wait_rb_timeout(rbitem, 500) < 0)
		RETURN_ERR(TIMEOUT, "nand io wait rb timeout");

	return 0;
}

int __get_features(int io_context, rb_item *rbitem, nand_ops_timing *timing,
		   unsigned char addr, unsigned char *data, int len)
{
	nand_io_send_cmd(io_context, CMD_GET_FEATURES, 0);
	nand_io_send_spec_addr(io_context, addr, 1, timing->tWB);
	if (__wait_rb_timeout(rbitem, 500) < 0)
		RETURN_ERR(TIMEOUT, "nand io wait rb timeout");
	ndd_ndelay(timing->tRR);
	nand_io_receive_data(io_context, data, len);

	return 0;
}

static int __nand_set_features(int io_context, rb_item *rbitem, nand_ops_timing *timing,
			unsigned char addr, unsigned char *data, int len)
{
	int i, ret;
	unsigned char rdata[4] = {0x00};

	ret = __set_features(io_context, rbitem, timing, addr, data, len);
	if (ret)
		RETURN_ERR(ENAND, "set Nand feature faild, addr = 0x%02x", addr);

	ret = __get_features(io_context, rbitem, timing, addr, rdata, 4);
	if (ret)
		RETURN_ERR(ENAND, "get Nand feature faild, addr = 0x%02x", addr);

	for (i = 0; i < len; i++) {
		if (data[i] != rdata[i])
			RETURN_ERR(ENAND, "set Nand feature error, addr = 0x%02x,\n"
				   " data[%d] = 0x%02x, rata[%d] = 0x%02x\n", addr, i, data[i], i, rdata[i]);
	}

	return 0;
}

int nand_set_features(nfi_base *base, unsigned int cs_id, rb_item *rbitem, chip_info *cinfo)
{
	int io_context, ret;
	unsigned char data[4] = {0x00};
	nand_ops_timing *timing = &cinfo->ops_timing;

	io_context = nand_io_open(base, cinfo);
	if (!io_context)
		RETURN_ERR(ENAND, "nand io open error");

	ret = nand_io_chip_select(io_context, cs_id);
	if (ret)
		RETURN_ERR(ENAND, "nand io chip select error, cs = [%d]", cs_id);

	/* set timimg mode */
	if (SUPPROT_TIMING_MODE(cinfo) && TIMING_MODE(cinfo) != 0x0f) {
		data[0] = TIMING_MODE(cinfo);
		data[1] = TIMING_MODE(cinfo) >> 8;
		data[2] = TIMING_MODE(cinfo) >> 16;
		data[3] = TIMING_MODE(cinfo) >> 24;
		ret = __nand_set_features(io_context, rbitem, timing, 0x01, data, 4);
		if (ret)
			RETURN_ERR(ENAND, "set Nand flash timing mode faild!");
	}

	/* set driver strength */
	if (SUPPROT_DRIVER_STRENGTH(cinfo)) {
		if (cinfo->drv_strength != DRV_STRENGTH_DEFAULT) {
			if (cinfo->manuf == NAND_MFR_MICRON) {
				if (cinfo->drv_strength == DRV_STRENGTH_LEVEL0)
					data[0] = MR_DRIVER_STRENGTH_UNDER;
				else if (cinfo->drv_strength == DRV_STRENGTH_LEVEL1)
					data[0] = MR_DRIVER_STRENGTH_NORMAL;
				else if (cinfo->drv_strength == DRV_STRENGTH_LEVEL2)
					data[0] = MR_DRIVER_STRENGTH_OVER1;
				else if (cinfo->drv_strength == DRV_STRENGTH_LEVEL3)
					data[0] = MR_DRIVER_STRENGTH_OVER2;
				else
					RETURN_ERR(ENAND, "unsupport driver strength!");

				data[1] = 0x00 >> 8;
				data[2] = 0x00 >> 16;
				data[3] = 0x00 >> 24;
				ret = __nand_set_features(io_context, rbitem, timing, 0x10, data, 4);
				if (ret)
					RETURN_ERR(ENAND, "set Nand flash driver strength faild!");
			}
		}
	}

	/* set rb pull_down strength */
	if (SUPPROT_RB_PULL_DOWN_STRENGTH(cinfo)) {
		data[0] = rbitem->pulldown_strength;
		data[1] = 0x00 >> 8;
		data[2] = 0x00 >> 16;
		data[3] = 0x00 >> 24;
		ret = __nand_set_features(io_context, rbitem, timing, 0x81, data, 4);
		if (ret)
			RETURN_ERR(ENAND, "set Nand flash pull_down strength faild!");
	}

	ret = nand_io_chip_deselect(io_context, cs_id);
	if (ret)
		RETURN_ERR(ret, "nand io chip deselect error, cs = [%d]", cs_id);

	nand_io_close(io_context);

	return 0;
}

/**
 * get_nand_id: get nand id
 *
 * @cid: contain nand id and extid
 * @return: 0: success, !0: fail
 **/
int get_nand_id(nfi_base *base, nand_flash_id *id, unsigned int cs_id)
{
	int context, ret;
	unsigned char nand_id[6];

	context = nand_io_open(base, NULL);
	if (!context)
		RETURN_ERR(ENAND, "nand io open error");

	ret = nand_io_chip_select(context, cs_id);
	if (ret)
		RETURN_ERR(ENAND, "nand io chip select error, cs = [%d]", cs_id);

	ret = nand_io_send_cmd(context, CMD_RESET_1ST, 300);
	if (ret)
		RETURN_ERR(ret, "nand io sent cmd error, cs = [%d], cmd = [%d]", cs_id, CMD_RESET_1ST);

	ndd_ndelay(5 * 1000 * 1000);
	ret = nand_io_send_cmd(context, CMD_READ_ID_1ST, 300);
	if (ret)
		RETURN_ERR(ret, "nand io sent cmd error, cs = [%d], cmd = [%d]", cs_id, CMD_READ_ID_1ST);

	nand_io_send_spec_addr(context, 0x00, 1, 1 * 1000 * 1000);

	ret = nand_io_receive_data(context, nand_id, sizeof(nand_id));
	if (ret)
		RETURN_ERR(ret, "nand io receive data error, cs = [%d]", cs_id);

	ret = nand_io_chip_deselect(context, cs_id);
	if (ret)
		RETURN_ERR(ret, "nand io chip deselect error, cs = [%d]", cs_id);

	nand_io_close(context);

	id->id = ((nand_id[0] << 8) | nand_id[1]);
	id->extid = ((nand_id[4] << 16) | (nand_id[3] << 8) | nand_id[2]);

	ndd_debug("get nand io chip[%d] id: id = [%04x], extid = [%08x]!\n", cs_id, id->id, id->extid);

	return 0;
}
