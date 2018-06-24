#include <os_clib.h>
#include "bad_block.h"

#define NO_DELAY        0

struct bad_block_ops {
        int current_cs;
        int current_io;
        chip_info *cinfo;
	nand_data *nddata;
};

struct bad_block_ops bad_block = {-1, -1, NULL, NULL};

static void nand_busy_clear(void)
{
	bad_block.nddata->clear_rb(bad_block.current_cs);
}

static int wait_nand_busy(void)
{
	int ret = 0;

	ret = bad_block.nddata->wait_rb(bad_block.current_cs, 5000);

	ndd_ndelay(bad_block.cinfo->ops_timing.tRR);

	return ret;
}

static int nand_prepare(struct nandops_info *ops, struct task_msg *msg)
{
	nand_data *data = ops->nanddata;

	if (bad_block.current_io == -1) {
		bad_block.current_io = nand_io_open(&(data->base->nfi), data->cinfo);
		bad_block.cinfo = data->cinfo;
		bad_block.nddata = data;
	}

	bad_block.current_cs = msg->ops.bits.chipsel;

	return 0;
}

static void enable_nand_cs(void)
{
	nand_data *data = bad_block.nddata;
	int cs_index = bad_block.current_cs;
	int cs = data->csinfo->csinfo_table[cs_index].id;

	nand_io_chip_select(bad_block.current_io, cs);
}

static void disable_nand_cs(void)
{
	nand_data *data = bad_block.nddata;
	int cs_index = bad_block.current_cs;
	int cs = data->csinfo->csinfo_table[cs_index].id;

	nand_io_chip_deselect(bad_block.current_io, cs);
}

/*
static int sent_two_plane_read(struct task_msg *msg, int pageid)
{
	int ret = 0;

	nand_io_send_cmd(bad_block.current_io, CMD_2P_PAGE_READ_1ST, 1000);
	nand_io_send_addr(bad_block.current_io, -1, pageid, 1000);
	nand_io_send_cmd(bad_block.current_io, CMD_2P_PAGE_READ_2ND, 1000);
	nand_io_send_addr(bad_block.current_io, -1, pageid + bad_block.cinfo->ppblock, 1000);
	nand_busy_clear();
	nand_io_send_cmd(bad_block.current_io, CMD_2P_PAGE_READ_3RD, 1000);
	ret = wait_nand_busy();

	return ret;
}

static void start_two_plane_read(int pageid)
{
	nand_io_send_cmd(bad_block.current_io, NAND_CMD_READ0, 1000);
	nand_io_send_addr(bad_block.current_io, 0, pageid, 1000);
	nand_io_send_cmd(bad_block.current_io, NAND_CMD_RNDOUT, 1000);
	nand_io_send_addr(bad_block.current_io, bad_block.cinfo->pagesize + bad_block.cinfo->badblkpos
			, -1, 1000);
	nand_io_send_cmd(bad_block.current_io, NAND_CMD_RNDOUTSTART, 1000);
}
*/

static int one_plane_read(struct task_msg *msg, int pageid)
{
	int offset;
	chip_info *cinfo = bad_block.cinfo;

	if (cinfo->pagesize != 512) {
		/* (cinfo->buswidth / 8): bytes of column */
		offset = (cinfo->pagesize + cinfo->badblkpos) / (cinfo->buswidth / 8);
		nand_io_send_cmd(bad_block.current_io, NAND_CMD_READ0, NO_DELAY);
	} else {
		offset = cinfo->badblkpos / (cinfo->buswidth / 8);
		nand_io_send_cmd(bad_block.current_io, CMD_OOB_READ_1ST_512, NO_DELAY);
	}

	nand_busy_clear();
	nand_io_send_addr(bad_block.current_io, offset, pageid, NO_DELAY);

	if (cinfo->pagesize != 512)
		nand_io_send_cmd(bad_block.current_io, NAND_CMD_READSTART, bad_block.cinfo->ops_timing.tWB);

	return wait_nand_busy();
}

static int check_badblock_buf(unsigned char *buf, int bufsize)
{
	int i, k;
	int bit0_count = 0;

	for (i = 0; i < bufsize; i++) {
		if(buf[i] != 0xff) {
			ndd_debug("%02x ",buf[i]);
			for(k = 0; k < 8; k++)
				if(!(0x01 & (buf[i] >> k)))
					bit0_count++;
			ndd_debug("\n");
		}
	}

	if (bit0_count)
		ndd_debug("%s bit0_count = %d \n",__func__, bit0_count);

	return bit0_count;
}

static int read_badblock(struct task_msg *msg, int pageid)
{
	int ret = 0;
	unsigned char badblockbuf[4] = {0xff};

	enable_nand_cs();
	ret = one_plane_read(msg, pageid);
	if (ret < 0) {
		ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
		goto err;
	}
	ret = nand_io_receive_data(bad_block.current_io, &badblockbuf[0], 4);
	if (ret < 0) {
		ndd_print(NDD_INFO, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
		goto err;
	}
	disable_nand_cs();
	ret = check_badblock_buf(badblockbuf, 4);
err:
	return ret;
}

static int write_badblock(struct task_msg *msg, int pageid)
{
	int ret = 0;
	unsigned char badblockbuf[4] = {0x00};

	enable_nand_cs();
	if (msg->msgdata.badblock.planes == 2) {
#if 0
		nand_io_send_cmd(bad_block.current_io, CMD_2P_PAGE_PROGRAM_1ST, 1000);
		nand_io_send_addr(bad_block.current_io, bad_block.cinfo->pagesize  + bad_block.cinfo->badblkpos
				, pageid, 1000);
		nand_io_send_data(bad_block.current_io, badblockbuf, 4);
		nand_busy_clear();
		nand_io_send_cmd(bad_block.current_io, CMD_2P_PAGE_PROGRAM_2ND, 1000);
		ret = wait_nand_busy();
		if (ret < 0) {
			ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
			goto err;
		}

		nand_io_send_cmd(bad_block.current_io, CMD_2P_PAGE_PROGRAM_3RD, 1000);
		nand_io_send_addr(bad_block.current_io, bad_block.cinfo->pagesize + bad_block.cinfo->badblkpos
				, pageid + bad_block.cinfo->ppblock, 1000);
		nand_io_send_data(bad_block.current_io, badblockbuf, 4);
		nand_busy_clear();
		nand_io_send_cmd(bad_block.current_io, CMD_2P_PAGE_PROGRAM_4TH, 1000);
		ret = wait_nand_busy();
		if (ret < 0) {
			ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
			goto err;
		}
#else
		nand_io_send_cmd(bad_block.current_io, CMD_PAGE_PROGRAM_1ST, NO_DELAY);
		nand_io_send_addr(bad_block.current_io, bad_block.cinfo->pagesize + bad_block.cinfo->badblkpos
				, pageid, bad_block.cinfo->ops_timing.tADL);
		nand_io_send_data(bad_block.current_io, badblockbuf, 4);
		nand_busy_clear();
		nand_io_send_cmd(bad_block.current_io, CMD_PAGE_PROGRAM_2ND
				, bad_block.cinfo->ops_timing.tWB);
		ret = wait_nand_busy();
		if (ret < 0) {
			ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
			goto err;
		}
		nand_io_send_cmd(bad_block.current_io, CMD_PAGE_PROGRAM_1ST, NO_DELAY);
		nand_io_send_addr(bad_block.current_io, bad_block.cinfo->pagesize + bad_block.cinfo->badblkpos
				, pageid + bad_block.cinfo->ppblock, bad_block.cinfo->ops_timing.tADL);
		nand_io_send_data(bad_block.current_io, badblockbuf, 4);
		nand_busy_clear();
		nand_io_send_cmd(bad_block.current_io, CMD_PAGE_PROGRAM_2ND
				, bad_block.cinfo->ops_timing.tWB);
		ret = wait_nand_busy();
		if (ret < 0) {
			ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
			goto err;
		}
#endif
	} else {
		nand_io_send_cmd(bad_block.current_io, CMD_PAGE_PROGRAM_1ST, NO_DELAY);
		nand_io_send_addr(bad_block.current_io, bad_block.cinfo->pagesize + bad_block.cinfo->badblkpos
				, pageid, bad_block.cinfo->ops_timing.tADL);
		nand_io_send_data(bad_block.current_io, badblockbuf, 4);
		nand_busy_clear();
		nand_io_send_cmd(bad_block.current_io, CMD_PAGE_PROGRAM_2ND
				, bad_block.cinfo->ops_timing.tWB);
		ret = wait_nand_busy();
		if (ret < 0) {
			ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
			goto err;
		}
	}

	ret = nand_io_send_cmd(bad_block.current_io, CMD_READ_STATUS_1ST, bad_block.cinfo->ops_timing.tWHR);
	if (ret & NAND_STATUS_FAIL)
		ret = -1;
	else if(!(ret & NAND_STATUS_WP))
		ret = -2;
	else
		ret = 0;
	disable_nand_cs();
err:
	return ret;
}

int is_bad_block(struct nandops_info *ops, Nand_Task *nt)
{
	int i, planes, ret = 0, count = 0, pageid = 0;
	struct task_msg *msg = nt->msg;

	ret = nand_prepare(ops, msg);
	if (ret < 0) {
		ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
		return ret;
	}

	planes = msg->msgdata.badblock.planes;
	pageid = msg->msgdata.badblock.blockid * bad_block.cinfo->ppblock;

	while (planes--) {
		for (i = 0; i < 4; i++) {
			ret = read_badblock(msg, pageid + i);
			if (ret < 0) {
				ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
				return ret;
			} else
				count += ret;
		}
		if (count > 64)
			return -1;
		count = 0;
		if(planes){
			pageid += bad_block.cinfo->ppblock;
		}
	}

	return 0;
}

int mark_bad_block(struct nandops_info *ops, Nand_Task *nt)
{
	int i, ret = 0;
	struct task_msg *msg = nt->msg;

	ret = nand_prepare(ops, msg);
	if (ret < 0) {
		ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
		return ret;
	}

	for (i = 0; i < 4; i++) {
		ret = write_badblock(msg, msg->msgdata.badblock.blockid * bad_block.cinfo->ppblock + i);
		if (ret < 0) {
			ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
			return ret;
		}
	}

	ndd_debug("%s [%d] \n", __func__, msg->msgdata.badblock.blockid);

	return ret;
}
