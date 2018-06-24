#include <os_clib.h>
#include "cpu_msg_handler.h"

//#define FILL_UP_WRITE_PAGE
#define CFG_NAND_USE_PN

/**
 * for vfat fs, we should use copy
 * to backup data before do bch.
 * to avoid the pDate be modified
 * after we have got bch parity but
 * send to nemc have not been done
 **/
//#define WRITE_NOT_USE_COPY

#define AA_BUF_SIZE	1024
#define FF_BUF_SIZE	4
#define NDD_SECTOR_SIZE	512

struct cpu_msg_ops {
        nand_data *nddata;
        chip_info *cinfo;
	cs_item *csinfo_table;
	int par_offset;
        int par_size;
	int eccblock_cnt;
	unsigned long long bitmap;
	unsigned long long bitmap_mask;
	int oobsize;
	int free_oobsize;
	unsigned char eccpos;
	unsigned char eccbit;
	unsigned short eccsize;
        int msg_cnt;
        int current_cs;
	int cs_enabled[CS_PER_NFI];
        int ret_index;
        int io_context;
        int bch_context;
        unsigned char *bch_buf;
        unsigned char *data_buf;
	unsigned char *aa_buf;
	unsigned char *ff_buf;
        unsigned char *ret_addr;
        PipeNode pipe;
};

extern int cpu_trans_align;

static void nand_busy_clear(struct cpu_msg_ops *cpu_msg)
{
	nand_data *nddata = cpu_msg->nddata;

	nddata->clear_rb(cpu_msg->current_cs);
}

static int wait_nand_busy(struct cpu_msg_ops *cpu_msg, struct task_msg *msg)
{
	int ret = SUCCESS;
	nand_data *nddata = cpu_msg->nddata;

	ret = nddata->wait_rb(cpu_msg->current_cs, 500);

	ndd_ndelay(cpu_msg->cinfo->ops_timing.tRR);

	return ret;
}

static int set_retVal(struct cpu_msg_ops *cpu_msg, int ret)
{
	char value = 0;
	switch (ret) {
		case SUCCESS:
			value = MSG_RET_SUCCESS;
			break;
		case ENAND:
		case TIMEOUT:
		case ECC_ERROR:
			value = MSG_RET_FAIL;
			break;
		case WRITE_PROTECT:
			value = MSG_RET_WP;
			break;
		case BLOCK_MOVE:
			value = MSG_RET_MOVE;
			break;
		case ALL_FF:
			value = MSG_RET_EMPTY;
			break;
		default:
			break;
	}
	set_ret_value(cpu_msg->ret_addr, cpu_msg->ret_index++, value);

	return value;
}

static int nand_prepare(struct cpu_msg_ops *cpu_msg, struct task_msg *msg)
{
	cpu_msg->msg_cnt = msg->msgdata.prepare.totaltasknum;
	cpu_msg->eccbit = msg->msgdata.prepare.eccbit;
	cpu_msg->par_size = get_parity_size(cpu_msg->eccbit);
	if(cpu_trans_align)
		cpu_msg->par_size = (cpu_msg->par_size + 3 ) / 4 * 4;
	cpu_msg->free_oobsize = cpu_msg->oobsize - (cpu_msg->eccblock_cnt * cpu_msg->par_size);
	if (cpu_msg->free_oobsize < FF_BUF_SIZE)
		RETURN_ERR(ENAND, "free oobsize [%d] is less than badblock flag size\n", cpu_msg->free_oobsize);

	cpu_msg->ret_index = 0;

	return 0;
}

/**
 * WARNING: cs state maybe changed by DMA ops at anytime
 **/
static void enable_nand_cs(struct cpu_msg_ops *cpu_msg, int cs_index)
{
	if (1/*cpu_msg->cs_enabled[cs_index] == 0*/) {
		int cs = cpu_msg->csinfo_table[cs_index].id;
		cpu_msg->current_cs = cs_index;
		nand_io_chip_select(cpu_msg->io_context, cs);
		cpu_msg->cs_enabled[cs_index] = 1;
	}
}

static void disable_nand_cs(struct cpu_msg_ops *cpu_msg, int cs_index)
{
	int cs = cpu_msg->csinfo_table[cs_index].id;
	nand_io_chip_deselect(cpu_msg->io_context, cs);
	cpu_msg->cs_enabled[cs_index] = 0;
}

static void send_read_random(struct cpu_msg_ops *cpu_msg, int offset)
{
	int io_context = cpu_msg->io_context;
	chip_info *cinfo = cpu_msg->cinfo;
	nand_ops_timing *timing = &cinfo->ops_timing;
	// (cinfo->buswidth / 8): bytes of column
	int fix_offset = offset / (cinfo->buswidth / 8);

	nand_io_send_cmd(io_context, NAND_CMD_RNDOUT, 0);
	nand_io_send_addr(io_context, fix_offset, -1, 0);
	nand_io_send_cmd(io_context, NAND_CMD_RNDOUTSTART, timing->tWHR2);
}

/*
static void send_prog_random(struct cpu_msg_ops *cpu_msg, int offset)
{
	int io_context = cpu_msg->io_context;
	chip_info *cinfo = cpu_msg->cinfo;
	const nand_timing *timing = cinfo->timing;
	// (cinfo->buswidth / 8): bytes of column
	int fix_offset = offset / (cinfo->buswidth / 8);


	nand_io_send_cmd(io_context, NAND_CMD_RNDIN, timing->tCWAW);
	nand_io_send_addr(io_context, offset, -1, timing->tADL);
}
*/

/* ######################### nand_msg_cmd ######################### */
static inline void fill_ff(struct cpu_msg_ops *cpu_msg);
extern int __set_features(int io_context, rb_info *rbinfo, const nand_timing *timing,
		   unsigned char addr, unsigned char *data, int len);
static int nand_msg_cmd(struct cpu_msg_ops *cpu_msg, struct task_msg *msg)
{
	int ret = 0, tmp = -1;
	int cs_index = msg->ops.bits.chipsel;
	int io_context = cpu_msg->io_context;
	int command = msg->msgdata.cmd.command;
	int cmddelay = msg->msgdata.cmd.cmddelay;
	int addrdelay = msg->msgdata.cmd.addrdelay;
	int offset = msg->msgdata.cmd.offset;
	int pageid = msg->msgdata.cmd.pageid;
	int eccsize = cpu_msg->eccsize;
	int model = msg->ops.bits.model;
	struct msgdata_cmd cmd;
	cmd.offset = -1;
	cmd.pageid = -1;

	enable_nand_cs(cpu_msg, cs_index);

	/* if pagesize == 512, write 0x00 to reset pointer to 0 before program */
	if ((cpu_msg->cinfo->pagesize == 512) && (command == CMD_PAGE_PROGRAM_1ST))
		nand_io_send_cmd(io_context, CMD_PAGE_READ_1ST, 0);

	if (command == CMD_PAGE_PROGRAM_2ND) {
#ifdef DEBUG_WRITE_PAGE_FULL
		if (cpu_msg->bitmap != cpu_msg->bitmap_mask) {
			ndd_print(NDD_ERROR, "ERROR: page data is not full, page data bitmap = 0x%08x, mask = 0x%08x\n",
				  cpu_msg->bitmap, cpu_msg->bitmap_mask);
			while (1);
		}
#endif
#ifdef FILL_UP_WRITE_PAGE
		fill_ff(cpu_msg);
#endif
	}

	/* if pagesize == 512, do not send 0x30 when read */
	if (!((cpu_msg->cinfo->pagesize == 512) && (command == CMD_PAGE_READ_2ND))) {
		if (model == MCU_WITH_RB)
			nand_busy_clear(cpu_msg);
		ret = nand_io_send_cmd(io_context, command, cmddelay);
	}

	if (!(offset == cmd.offset && pageid == cmd.pageid)) {
		if (offset != cmd.offset)
			tmp = offset * 512 / eccsize * eccsize;
		nand_io_send_addr(io_context, tmp, pageid, addrdelay);
	}

	if (command == NAND_CMD_STATUS) {
		disable_nand_cs(cpu_msg, cs_index);
		if (ret & NAND_STATUS_FAIL)
			ret = ENAND;
		else if(!(ret & NAND_STATUS_WP))
			ret = WRITE_PROTECT;
		else
			ret = SUCCESS;
	}

	if (model == MCU_WITH_RB) {
		ret = wait_nand_busy(cpu_msg, msg);
		if (ret < 0)
			RETURN_ERR(ret, "send cmd wait rb timeout\n");
	}

	return ret;
}

/* ######################### nand_msg_data ######################### */
static inline void fill_ff(struct cpu_msg_ops *cpu_msg)
{
	int io_context = cpu_msg->io_context;
	int free_oobsize = cpu_msg->free_oobsize;

	while (free_oobsize > 0) {
		if (free_oobsize > (AA_BUF_SIZE + FF_BUF_SIZE)) {
			nand_io_send_data(io_context, cpu_msg->aa_buf, AA_BUF_SIZE);
			free_oobsize -= AA_BUF_SIZE;
		} else {
			nand_io_send_data(io_context, cpu_msg->aa_buf, free_oobsize - FF_BUF_SIZE);
			nand_io_send_data(io_context, cpu_msg->ff_buf, FF_BUF_SIZE);
			free_oobsize -= free_oobsize;
		}
	}
}

static int pipe_is_full(struct cpu_msg_ops *cpu_msg, int offset, int bytes, int eccsize)
{
#ifdef CHECK_PIPE_DATA
	int i, ret = 0;
	unsigned long long bitmap = cpu_msg->bitmap;
	unsigned long long bitmap_mask = cpu_msg->bitmap_mask;
	int sector_cnt = bytes / NDD_SECTOR_SIZE;
	int sector_offset = offset / NDD_SECTOR_SIZE;
	unsigned long long check_mask = (1LL << (eccsize / NDD_SECTOR_SIZE)) - 1;
	int check_offset = (offset / eccsize) * (eccsize / NDD_SECTOR_SIZE);

	if (bitmap == bitmap_mask)
		bitmap = 0LL;

	for (i = 0; i < sector_cnt; i++) {
		if (bitmap & (1LL << (sector_offset + i))) {
			ndd_print(NDD_ERROR, "ERROR: page data is repeat, page data bitmap = %x, offset = %d\n",
				  bitmap, offset);
			while (1);
		} else
			bitmap |= 1LL << (sector_offset + i);
	}

	if ((unsigned int)(bitmap >> check_offset) == check_mask)
		ret = 1;
	else if ((unsigned int)(bitmap >> check_offset) < check_mask) {
		ndd_print(NDD_ERROR, "ERROR: page data is not in rules, page data bitmap >> %d = 0x%x, offset = %d\n",check_offset,
			  (int)(bitmap >> check_offset), offset);
		while (1);
	}

	cpu_msg->bitmap = bitmap;

	return ret;
#else
	return 1;
#endif
}

static int nand_write_data(struct cpu_msg_ops *cpu_msg, struct task_msg *msg)
{
	int ret = 0;
	int cs_index = msg->ops.bits.chipsel;
	int io_context = cpu_msg->io_context;
	int bch_context = cpu_msg->bch_context;
	int offset = msg->msgdata.data.offset;
	int bytes = msg->msgdata.data.bytes;
	int eccbit = cpu_msg->eccbit;
	int eccsize = cpu_msg->eccsize;
	void *pdata = (void *)get_vaddr(msg->msgdata.data.pdata);
	PipeNode *pipe = &cpu_msg->pipe;

#ifdef WRITE_NOT_USE_COPY
	if (bytes == eccsize)
		pipe->data = pdata;
	else
		ndd_memcpy(pipe->data + (offset % eccsize), pdata, bytes);
#else
	ndd_memcpy(pipe->data + (offset % eccsize), pdata, bytes);
#endif
	if (pipe_is_full(cpu_msg, offset, bytes, eccsize)) {
		enable_nand_cs(cpu_msg, cs_index);
		ret = nand_bch_encode_prepare(bch_context, pipe, eccbit);
		if (ret)
			RETURN_ERR(ret, "bch decode prepare error");

		ret = nand_bch_encode_complete(bch_context, pipe);
		if (ret)
			RETURN_ERR(ret, "bch encode complete error");

#ifdef CFG_NAND_USE_PN
		pn_enable(io_context);
#endif
		nand_io_send_data(io_context, pipe->data, eccsize);
		nand_io_send_data(io_context, pipe->parity, cpu_msg->par_size);

		ret = nand_io_send_waitcomplete(io_context, cpu_msg->cinfo);
		if (ret)
			ndd_print(NDD_ERROR, "wait data nocomplete error!\n");
#ifdef CFG_NAND_USE_PN
		pn_disable(io_context);
#endif
	}

#ifdef WRITE_NOT_USE_COPY
	if (bytes == eccsize)
		pipe->data = cpu_msg->data_buf;
#endif
	return ret;
}

static int nand_read_data(struct cpu_msg_ops *cpu_msg, struct task_msg *msg)
{
	int ret = SUCCESS;
	int offset = msg->msgdata.data.offset;
	int bytes = msg->msgdata.data.bytes;
	int eccbit = cpu_msg->eccbit;
	int eccsize = cpu_msg->eccsize;
	int unitsize = eccsize + cpu_msg->par_size;
	int cs_index = msg->ops.bits.chipsel;
	int io_context = cpu_msg->io_context;
	int bch_context = cpu_msg->bch_context;
	void *pdata = (void *)get_vaddr(msg->msgdata.data.pdata);
	PipeNode *pipe = &cpu_msg->pipe;
	unsigned int bitcount = 0;

	if (bytes == eccsize)
		pipe->data = pdata;

	enable_nand_cs(cpu_msg, cs_index);

	/* if pagesize == 512, do not send random read command */
	if (cpu_msg->cinfo->pagesize != 512)
		send_read_random(cpu_msg, (offset / eccsize) * unitsize);

	nand_io_counter0_enable(io_context);
#ifdef CFG_NAND_USE_PN
	pn_enable(io_context);
#endif
	nand_io_receive_data(io_context, pipe->data, eccsize);
	nand_io_receive_data(io_context, pipe->parity, cpu_msg->par_size);
	/* if pagesize == 512, read all data of page and wait rb */
	if (cpu_msg->cinfo->pagesize == 512) {
		unsigned char spare_buf[cpu_msg->free_oobsize];
		nand_io_receive_data(io_context, spare_buf, cpu_msg->free_oobsize);
	}
#ifdef CFG_NAND_USE_PN
	pn_disable(io_context);
#endif
	bitcount = nand_io_read_counter(io_context);
	nand_io_counter_disable(io_context);
	if(bitcount < eccbit)
		ret = ALL_FF;

	disable_nand_cs(cpu_msg, cs_index);

	if (ret != ALL_FF) {
		ret = nand_bch_decode_prepare(bch_context, pipe, eccbit);
		if (ret < 0)
			RETURN_ERR(ret, "bch decode prepare error");

		ret = nand_bch_decode_complete(bch_context, pipe);
		if (ret > 0)
			ret = (ret > eccbit / 3) ? BLOCK_MOVE : SUCCESS;
#ifdef DEBUG_ECCERROR
		if (ret == ECC_ERROR) {
			int i;
			ndd_print(NDD_DEBUG, "====== data: data_offset = %d, par_offset = %d",
				  (offset / eccsize) * unitsize, ((offset / eccsize) * unitsize + eccsize));
			for (i = 0; i < eccsize; i++) {
				if (!(i % 16))
					ndd_print(NDD_DEBUG, "\n%d: ", i / 16);
				ndd_print(NDD_DEBUG, " %02x", (unsigned char)pipe->data[i]);
			}
			ndd_print(NDD_DEBUG, "\n====== parity:");
			for (i = 0; i < cpu_msg->par_size; i++) {
				if (!(i % 16))
					ndd_print(NDD_DEBUG, "\n%d: ", i / 16);
				ndd_print(NDD_DEBUG, " %02x", (unsigned char)pipe->parity[i]);
			}
			ndd_print(NDD_DEBUG, "\n");
		}
#endif
	}

	if (bytes == eccsize)
		pipe->data = cpu_msg->data_buf;
	else if (ret != ALL_FF)
		ndd_memcpy(pdata, pipe->data + offset % eccsize, bytes);

	return ret;
}

static int nand_msg_data(struct cpu_msg_ops *cpu_msg, struct task_msg *msg)
{
	if (msg->ops.bits.model == MCU_READ_DATA)
		return nand_read_data(cpu_msg, msg);
	else
		return nand_write_data(cpu_msg, msg);
}

/* ######################### nand_msg_block ######################### */
static int sent_two_plane_read(struct cpu_msg_ops *cpu_msg, struct task_msg *msg, int pageid)
{
	int io_context = cpu_msg->io_context;
	int ppblock = cpu_msg->cinfo->ppblock;

	nand_io_send_cmd(io_context, CMD_2P_PAGE_READ_1ST, 1000);
	nand_io_send_addr(io_context, -1, pageid, 1000);
	nand_io_send_cmd(io_context, CMD_2P_PAGE_READ_2ND, 1000);
	nand_io_send_addr(io_context, -1, pageid + ppblock, 1000);
	nand_busy_clear(cpu_msg);
	nand_io_send_cmd(io_context, CMD_2P_PAGE_READ_3RD, 1000);

	return wait_nand_busy(cpu_msg, msg);
}

static void start_two_plane_read(struct cpu_msg_ops *cpu_msg, int pageid)
{
	int io_context = cpu_msg->io_context;
	int pagesize = cpu_msg->cinfo->pagesize;

	nand_io_send_cmd(io_context, NAND_CMD_READ0, 1000);
	nand_io_send_addr(io_context, 0, pageid, 1000);
	nand_io_send_cmd(io_context, NAND_CMD_RNDOUT, 1000);
	nand_io_send_addr(io_context, pagesize, -1, 1000);
	nand_io_send_cmd(io_context, NAND_CMD_RNDOUTSTART, 1000);
}

static int one_plane_read(struct cpu_msg_ops *cpu_msg, struct task_msg *msg, int pageid)
{
	int io_context = cpu_msg->io_context;
	chip_info *cinfo = cpu_msg->cinfo;
	int offset = cinfo->pagesize / (cinfo->buswidth / 8);
	nand_ops_timing *timing = &cinfo->ops_timing;

	nand_io_send_cmd(io_context, NAND_CMD_READ0, 0);
	nand_io_send_addr(io_context, offset, pageid, 0);
	nand_busy_clear(cpu_msg);
	nand_io_send_cmd(io_context, NAND_CMD_READSTART, timing->tWB);

	return wait_nand_busy(cpu_msg, msg);
}

static int check_badblock_buf(unsigned char *buf, int bufsize)
{
	int i, j;
	int bit0_count = 0;

	for (i = 0; i < bufsize; i++) {
		if(buf[i] != 0xff)
			for(j = 0; j < 8; j++)
				if(!(0x01 & (buf[i] >> j)))
					bit0_count++;
	}

	if (bit0_count)
		ndd_debug("%s bit0_count = %d \n",__func__, bit0_count);

	return bit0_count;
}

static int read_badblock(struct cpu_msg_ops *cpu_msg, struct task_msg *msg, int pageid)
{
	int ret = SUCCESS;
	unsigned char eccpos = cpu_msg->eccpos;
	int io_context = cpu_msg->io_context;
	int ppblock = cpu_msg->cinfo->ppblock;

	if (msg->msgdata.badblock.planes == 2) {
		unsigned char badblockbuf[8] = {0xff};
		ret = sent_two_plane_read(cpu_msg, msg, pageid);
		if (ret < 0)
			RETURN_ERR(ret, "sent two plane read\n");
		start_two_plane_read(cpu_msg, pageid);
		nand_io_receive_data(io_context, &badblockbuf[0], eccpos);
		start_two_plane_read(cpu_msg, pageid + ppblock);
		nand_io_receive_data(io_context, &badblockbuf[eccpos], eccpos);
		ret = check_badblock_buf(badblockbuf, eccpos * 2);
		ret = ret / 2;
	} else {
		unsigned char badblockbuf[4] = {0xff};
		ret = one_plane_read(cpu_msg, msg, pageid);
		if (ret < 0)
			RETURN_ERR(ret, "one plane read\n");
		nand_io_receive_data(io_context, &badblockbuf[0], eccpos);
		ret = check_badblock_buf(badblockbuf, eccpos);
	}

	return ret;
}

static int write_badblock(struct cpu_msg_ops *cpu_msg, struct task_msg *msg, int pageid)
{
	int ret = SUCCESS;
	unsigned char eccpos = cpu_msg->eccpos;
	unsigned char badblockbuf[4] = {0x00};
	int io_context = cpu_msg->io_context;
	int pagesize = cpu_msg->cinfo->pagesize;
	int ppblock = cpu_msg->cinfo->ppblock;

	if (msg->msgdata.badblock.planes == 2) {
		nand_io_send_cmd(io_context, CMD_2P_PAGE_PROGRAM_1ST, 1000);
		nand_io_send_addr(io_context, pagesize, pageid, 1000);
		nand_io_send_data(io_context, badblockbuf, eccpos);
		nand_busy_clear(cpu_msg);
		nand_io_send_cmd(io_context, CMD_2P_PAGE_PROGRAM_2ND, 1000);
		ret = wait_nand_busy(cpu_msg, msg);
		if (ret < 0)
			RETURN_ERR(ret, "nand wait rb\n");

		nand_io_send_cmd(io_context, CMD_2P_PAGE_PROGRAM_3RD, 1000);
		nand_io_send_addr(io_context, pagesize, pageid + ppblock, 1000);
		nand_io_send_data(io_context, badblockbuf, eccpos);
		nand_busy_clear(cpu_msg);
		nand_io_send_cmd(io_context, CMD_2P_PAGE_PROGRAM_4TH, 1000);
		ret = wait_nand_busy(cpu_msg, msg);
		if (ret < 0)
			RETURN_ERR(ret, "nand wait rb\n");
	} else {
		nand_io_send_cmd(io_context, CMD_PAGE_PROGRAM_1ST, 1000);
		nand_io_send_addr(io_context, pagesize, pageid, 1000);
		nand_io_send_data(io_context, badblockbuf, eccpos);
		nand_busy_clear(cpu_msg);
		nand_io_send_cmd(io_context, CMD_PAGE_PROGRAM_2ND, 1000);
		ret = wait_nand_busy(cpu_msg, msg);
		if (ret < 0)
			RETURN_ERR(ret, "nand wait rb\n");
	}

	ret = nand_io_send_cmd(io_context, CMD_READ_STATUS_1ST, 1000);
	if (ret & NAND_STATUS_FAIL)
		ret = ENAND;
	else if(!(ret & NAND_STATUS_WP))
		ret = WRITE_PROTECT;
	else
		ret = SUCCESS;

	return ret;
}

static int nand_is_badblock(struct cpu_msg_ops *cpu_msg, struct task_msg *msg)
{
	int i, ret = SUCCESS, count = 0;
	int cs_index = msg->ops.bits.chipsel;
	int ppblock = cpu_msg->cinfo->ppblock;
	int blockid = msg->msgdata.badblock.blockid;

	enable_nand_cs(cpu_msg, cs_index);
	for (i = 0; i < cpu_msg->eccpos; i++) {
		ret = read_badblock(cpu_msg, msg, blockid * ppblock + i);
		if (ret < 0)
			RETURN_ERR(ret, "read bad block flag\n");
		else
			count += ret;
	}
	disable_nand_cs(cpu_msg, cs_index);
	if (count > 64)
		ret = ENAND;
	else
		ret = SUCCESS;

	return ret;
}

static int nand_mark_badblock(struct cpu_msg_ops *cpu_msg, struct task_msg *msg)
{
	int i, ret = SUCCESS;
	int cs_index = msg->ops.bits.chipsel;
	int ppblock = cpu_msg->cinfo->ppblock;
	int blockid = msg->msgdata.badblock.blockid;

	enable_nand_cs(cpu_msg, cs_index);
	for (i = 0; i < cpu_msg->eccpos; i++) {
		ret = write_badblock(cpu_msg, msg, blockid * ppblock + i);
		if (ret < 0)
			RETURN_ERR(ret, "write bad block flag\n");
	}
	disable_nand_cs(cpu_msg, cs_index);
	return ret;
}

static int nand_msg_block(struct cpu_msg_ops *cpu_msg, struct task_msg *msg)
{
	if (msg->ops.bits.model == MCU_ISBADBLOCK)
		return nand_is_badblock(cpu_msg, msg);
	else
		return nand_mark_badblock(cpu_msg, msg);
}

/* ################################################################### *\
 * msg_handler
\* ################################################################### */
static int task_manager(int handle, Nand_Task *nt)
{
	int i, ret = SUCCESS;
	int count, retval = 0, val = 0;
	struct task_msg *msg = nt->msg;
	struct cpu_msg_ops * cpu_msg = (struct cpu_msg_ops *)handle;

	if (msg->ops.bits.type == MSG_MCU_PREPARE) {
		ret = nand_prepare(cpu_msg, msg++);
		if (ret)
			RETURN_ERR(ENAND, "nand prepare error!\n");
	} else
		RETURN_ERR(ENAND, "wrong message type, type = %d\n", msg->ops.bits.type);

	count = cpu_msg->msg_cnt;
	for (i = 0; i < count - 2; i++, msg++) {
		switch (msg->ops.bits.type) {
			case MSG_MCU_CMD:
				ret = nand_msg_cmd(cpu_msg, msg);
				val = set_retVal(cpu_msg, ret);
				break;
			case MSG_MCU_DATA:
				ret = nand_msg_data(cpu_msg, msg);
				val = set_retVal(cpu_msg, ret);
				break;
			case MSG_MCU_BADBLOCK:
				ret = nand_msg_block(cpu_msg, msg);
				break;
		}
		retval |= val;
	}

	if(retval & MSG_RET_FAIL)
		ret = ECC_ERROR;
	else if(retval & MSG_RET_MOVE)
		ret = BLOCK_MOVE;
	else if(retval & MSG_RET_EMPTY)
		ret = ALL_FF;
	else if(retval & MSG_RET_WP)
		ret = ENAND;
	else
		ret = SUCCESS;

	return ret;
}

/* ################################################################### *\
 * init/deinit
\* ################################################################### */
int cpu_msg_handle_init(nand_data *data, Nand_Task *nandtask, int id)
{
	int i;
	struct cpu_msg_ops *cpu_msg = NULL;
	struct msg_handler *handler = NULL;
	chip_info *cinfo = data->cinfo;

	handler = ndd_alloc(sizeof(struct msg_handler));
	if (!handler)
		GOTO_ERR(alloc_handler);

	cpu_msg = ndd_alloc(sizeof(struct cpu_msg_ops));
	if (!cpu_msg)
		GOTO_ERR(alloc_cpu_msg);

	handler->context = (int)cpu_msg;
	handler->handler = task_manager;

	cpu_msg->nddata = data;
	cpu_msg->cinfo = cinfo;
	cpu_msg->csinfo_table = data->csinfo->csinfo_table;
	cpu_msg->current_cs = -1;
	for (i = 0; i < CS_PER_NFI; i++)
		cpu_msg->cs_enabled[i] = 0;
	cpu_msg->par_offset = cinfo->pagesize + cinfo->eccpos;
	cpu_msg->eccpos = cinfo->eccpos;
	cpu_msg->eccsize = data->eccsize;
	cpu_msg->oobsize = cinfo->oobsize;
	cpu_msg->eccblock_cnt = cinfo->pagesize / cpu_msg->eccsize;
	cpu_msg->bitmap = 0LL;
	cpu_msg->bitmap_mask = (1LL << (cinfo->pagesize / NDD_SECTOR_SIZE)) - 1;
	cpu_msg->bch_buf = ndd_alloc(get_parity_size(MAX_BCHSEL));
	if (!cpu_msg->bch_buf)
		GOTO_ERR(alloc_bchbuf);

	cpu_msg->data_buf = ndd_alloc(data->eccsize);
	if (!cpu_msg->data_buf)
		GOTO_ERR(alloc_databuf);

	cpu_msg->aa_buf = ndd_alloc(AA_BUF_SIZE);
	if (!cpu_msg->aa_buf)
		GOTO_ERR(alloc_aabuf);
	ndd_memset(cpu_msg->aa_buf, 0xaa, AA_BUF_SIZE);

	cpu_msg->ff_buf = ndd_alloc(FF_BUF_SIZE);
	if (!cpu_msg->ff_buf)
		GOTO_ERR(alloc_ffbuf);
	ndd_memset(cpu_msg->ff_buf, 0xff, FF_BUF_SIZE);

	cpu_msg->pipe.data = cpu_msg->data_buf;
	cpu_msg->pipe.parity = cpu_msg->bch_buf;
	cpu_msg->ret_addr = nandtask->ret;

	cpu_msg->io_context = nand_io_open(&(data->base->nfi), data->cinfo);
	if(!cpu_msg->io_context)
		GOTO_ERR(open_io);

	cpu_msg->bch_context = nand_bch_open(&(data->base->bch), cpu_msg->eccsize);
	if(!cpu_msg->bch_context)
		GOTO_ERR(open_bch);

	return (int)handler;

ERR_LABLE(open_bch):
	nand_io_close(cpu_msg->io_context);
ERR_LABLE(open_io):
	ndd_free(cpu_msg->ff_buf);
ERR_LABLE(alloc_ffbuf):
	ndd_free(cpu_msg->aa_buf);
ERR_LABLE(alloc_aabuf):
	ndd_free(cpu_msg->data_buf);
ERR_LABLE(alloc_databuf):
	ndd_free(cpu_msg->bch_buf);
ERR_LABLE(alloc_bchbuf):
	ndd_free(cpu_msg);
ERR_LABLE(alloc_cpu_msg):
	ndd_free(handler);
ERR_LABLE(alloc_handler):

	return 0;
}

void cpu_msg_handle_deinit(int handle)
{
	struct msg_handler *handler = (struct msg_handler *)handle;
	struct cpu_msg_ops *cpu_msg = (struct cpu_msg_ops *)handler->context;

	if(!cpu_msg) {
		ndd_print(NDD_ERROR, "ERROR: func:%s line:%d handle is null!\n", __func__, __LINE__);
		return;
	}

	nand_io_close(cpu_msg->io_context);
	nand_bch_close(cpu_msg->bch_context);
	ndd_free(cpu_msg->ff_buf);
	ndd_free(cpu_msg->data_buf);
	ndd_free(cpu_msg->bch_buf);
	ndd_free(cpu_msg);
	ndd_free(handler);
}

int cpu_msg_handle_suspend(int handle)
{
	return 0;
}

int cpu_msg_handle_resume(int handle)
{
	return 0;
}
