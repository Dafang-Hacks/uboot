#include <os_clib.h>
#include "errptinfo.h"
#include <nand_debug.h>
#include "nand_io.h"
#include "nand_bch.h"
#include "ppartition.h"
#include "pagelist.h"
#include "ndcommand.h"
#include "nandinterface.h"

#define  ERRPT_BLOCK_CNT 4
#define  MAX_SERIALNUM_SIZE 32
#define PT_HEADINFO_CNT 1
#define PT_INFO_CNT     2
#define ERRPT_BLCOK_CNT 4
#define RB_READY     1
#define RB_NO_READY  0
#define NO_DELAY        0
#define NANDFLASH_OFFSET_IN_ERRPTHEAD	64

typedef struct _errpt_info{
	int io_context;
	int bch_context;
	unsigned char *bchbuf;
}errpt_info;

typedef struct _rb_msg{
	unsigned short gpio;
	unsigned short strength;
}rb_msg;
typedef struct _ppa_head{
	unsigned int magicid;
	nm_version version;
	rb_msg rbmsg[CS_PER_NFI];
	unsigned short gpio_wp;
	unsigned short drv_strength;
	unsigned int ptcount;
	unsigned int raw_boundary;
}ppa_head;

typedef struct __errpt_l2pblock{
	unsigned char *csinrb;
	unsigned short rb_bound;
	unsigned short ol_bound;
	unsigned int cs_bound;
}errpt_l2pblock;

typedef struct _nand_ex_partition{
	long long offset;
	long long size;
	char name[MAX_NAME_SIZE];
}nand_ex_partition;

typedef struct _ppa_info{
	unsigned char ptindex;
	char name[MAX_NAME_SIZE];
	unsigned long long offset;
	unsigned long long size;
	unsigned char ops_mode;
	unsigned char nm_mode;
	unsigned int flags;
	nand_ex_partition ex_partition[MUL_PARTS];
}ppa_info;

extern void (*__wp_enable) (int);
extern void (*__wp_disable) (int);

int nd_raw_boundary;
errpt_info nd_errpt_info;
static void dump_pheadinfo(ppa_head *phead)
{
	int i;
	ndd_debug("dump ept phead info:\n");
	ndd_debug("\t magicid: [0x%08x]\n"
                  "\t NM version:%d.%d.%d\n"
                  "\t gpio_wp:   [%d]\n"
                  "\t drv_strength:  [%d]\n"
                  "\t ptcount: [%d]\n",
		  phead->magicid,phead->version.major,phead->version.minor,phead->version.revision,
		  		phead->gpio_wp,phead->drv_strength,phead->ptcount);
	for(i = 0; i< CS_PER_NFI; i++){
		ndd_debug("\t rb[%d]:gpio = [%d],pulldown = %d\n",i, phead->rbmsg[i].gpio, phead->rbmsg[i].strength);
	}
}
static void dump_ppainfo(ppa_info *ppainfo, int ptcount)
{
	int i, pt_index;
	ndd_debug("dump ept partition info:\n");
	for(pt_index = 0; pt_index < ptcount; pt_index++){
		ndd_debug("\n pt[%s]: \n",(ppainfo + pt_index)->name);
		ndd_debug("\t ptindex: [%d]\n",(ppainfo + pt_index)->ptindex);
		ndd_debug("\t offset:  [%d]\n",(unsigned int)((ppainfo + pt_index)->offset));
		ndd_debug("\t size:    [%d]\n",(unsigned int)((ppainfo + pt_index)->size));
		ndd_debug("\t ops_mode:[%d]\n",(ppainfo + pt_index)->ops_mode);
		ndd_debug("\t nm_mode: [%d]\n",(ppainfo + pt_index)->nm_mode);
		ndd_debug("\t flags:    [%d]\n",(ppainfo + pt_index)->flags);
#if 1
		ndd_debug("\t mulpartsinfo:\n");
		for(i = 0; i < MUL_PARTS; i++){
			ndd_debug("\t\t pt[%s]:\n",((ppainfo + pt_index)->ex_partition + i)->name);
			ndd_debug("\t\t offset: [%d]",(unsigned int)(((ppainfo + pt_index)->ex_partition + i)->offset));
			ndd_debug("\t\t size:   [%d]",(unsigned int)(((ppainfo + pt_index)->ex_partition + i)->size));
		}
#endif
	}
}

static void dump_platptinfo(plat_ptinfo *ptinfo)
{
	int i, pt_index;
	plat_ptitem *ptitem = ptinfo->pt_table;
	ndd_debug("dump plat_ptinfo info:\n");
	ndd_debug("----- ptcount = %d -----\n",ptinfo->ptcount);
	for(pt_index = 0; pt_index < ptinfo->ptcount; pt_index++){
		ndd_debug("\n pt[%s]: \n",(ptitem + pt_index)->name);
		ndd_debug("\t offset:  [%d]\n",(unsigned int)((ptitem + pt_index)->offset));
		ndd_debug("\t size:    [%d]\n",(unsigned int)((ptitem + pt_index)->size));
		ndd_debug("\t ops_mode:[%d]\n",(ptitem + pt_index)->ops_mode);
		ndd_debug("\t nm_mode: [%d]\n",(ptitem + pt_index)->nm_mode);
		ndd_debug("\t flags:    [%d]\n",(ptitem + pt_index)->flags);
		ndd_debug("\t mulpartsinfo:\n");
		for(i = 0; i < MUL_PARTS; i++){
			ndd_debug("\t\t pt[%s]:\n",((ptitem + pt_index)->ex_partition + i)->name);
			ndd_debug("\t\t offset: [%d]",(unsigned int)(((ptitem + pt_index)->ex_partition + i)->offset));
			ndd_debug("\t\t size:   [%d]",(unsigned int)(((ptitem + pt_index)->ex_partition + i)->size));
		}
	}
}
static void nand_io_bch_open(io_base *base, chip_info *cinfo, unsigned int eccsize, unsigned char buswidth)
{
	nd_errpt_info.io_context = nand_io_open(&base->nfi, cinfo);
	if((buswidth == 16) && (!cinfo)){
		nand_io_setup_default_16bit(&base->nfi);
	}
	nd_errpt_info.bch_context = nand_bch_open(&base->bch, eccsize);
}
static void nand_io_bch_close(void)
{
	nand_io_close(nd_errpt_info.io_context);
	nand_bch_close(nd_errpt_info.bch_context);
}
static void nand_busy_clear(nand_data *nddata, int chip_index)
{
	nddata->clear_rb(chip_index);
}
static int nand_wait_ready(nand_data *nddata, int chip_index)
{
	volatile int timeout = 500;
	int ret = SUCCESS;
	ret = nddata->wait_rb(chip_index, timeout);
	if (ret < 0)
		ret = TIMEOUT;

	ndd_ndelay(nddata->cinfo->ops_timing.tRR);

	return ret;
}
static void errpt_nand_io_send_addr(int io_context, nand_flash *ndflash, int offset, int pageid, unsigned int delay)
{
	if(offset >= 0){
		offset /= (ndflash->buswidth / 8);
		if(ndflash->pagesize != 512)
			nand_io_send_spec_addr(io_context, offset, 2, 0);
		else
			nand_io_send_spec_addr(io_context, offset, 1, 0);
	}

	if(pageid >= 0){
		nand_io_send_spec_addr(io_context, pageid, ndflash->rowcycles, delay);
	}
}
static void sent_readcmd_start(nand_data *nddata, nand_flash *ndflash, int offset, int pageid)
{
	nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_READ0, NO_DELAY);
	errpt_nand_io_send_addr(nd_errpt_info.io_context, ndflash, offset, pageid, NO_DELAY);
	if(ndflash->pagesize != 512)
		nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_READSTART, 100);
}

static int check_badblock_buf(unsigned char *buf, int bufsize)
{
	int i, k;
	int bit0_count = 0;

	for (i = 0; i < bufsize; i++) {
		if(buf[i] != 0xff) {
			ndd_debug("%x ",buf[i]);
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
#ifndef NAND_ERRPT_BURNTOOL
static int read_badblock(nand_data *nddata, nand_flash *ndflash,int chip_index, unsigned int pageid, int rb_ready)
{
	int ret = 0;
	unsigned char badblockbuf[4] = {0xff};
	unsigned int badblockpos = (ndflash->pagesize != 512) ? ndflash->pagesize + ndflash->oobsize - 4 : ndflash->oobsize - 4;

	nand_io_chip_select(nd_errpt_info.io_context, chip_index);
	if(ndflash->pagesize != 512)
		nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_READ0, NO_DELAY);
	else
		nand_io_send_cmd(nd_errpt_info.io_context, CMD_OOB_READ_1ST_512, NO_DELAY);
	if (rb_ready)
		nand_busy_clear(nddata,chip_index);
	errpt_nand_io_send_addr(nd_errpt_info.io_context, ndflash, badblockpos, pageid, NO_DELAY);
	if(ndflash->pagesize != 512)
		nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_READSTART, 100);

	if(rb_ready){
		ret = nand_wait_ready(nddata,chip_index);
		if (ret < 0) {
			ndd_print(NDD_ERROR, "func:%s line:%d wait rb timeout ret=%d\n",__func__, __LINE__, ret);
			goto err;
		}
	}else
		ndd_ndelay(5 * 1000 * 1000);
	ret = nand_io_receive_data(nd_errpt_info.io_context,badblockbuf,4);
	if (ret < 0) {
		ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
		goto err;
	}
	ret = check_badblock_buf(badblockbuf, 4);
err:
	nand_io_chip_deselect(nd_errpt_info.io_context, chip_index);
	return ret;
}
#else
static int read_badblock(nand_data *nddata, nand_flash *ndflash,int chip_index, unsigned int pageid, int rb_ready)
{
	cs_item *csinfo_table = nddata->csinfo->csinfo_table + chip_index;
	int ret = 0;
	unsigned char badblockbuf[4] = {0xff};
	unsigned int badblockpos = (ndflash->pagesize != 512) ? ndflash->pagesize + ndflash->oobsize - 4 : ndflash->oobsize - 4;

	nand_io_chip_select(nd_errpt_info.io_context, csinfo_table->id);
	if(ndflash->pagesize != 512)
		nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_READ0, NO_DELAY);
	else
		nand_io_send_cmd(nd_errpt_info.io_context, CMD_OOB_READ_1ST_512, NO_DELAY);
	if(rb_ready)
		nand_busy_clear(nddata,chip_index);
	errpt_nand_io_send_addr(nd_errpt_info.io_context, ndflash, badblockpos, pageid, NO_DELAY);
	if(ndflash->pagesize != 512)
		nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_READSTART, 100);

	if(rb_ready){
		ret = nand_wait_ready(nddata,chip_index);
		if (ret < 0) {
			ndd_print(NDD_ERROR, "func:%s line:%d wait rb timeout ret=%d\n",__func__, __LINE__, ret);
			goto err;
		}
	}else
		ndd_ndelay(5 * 1000 * 1000);
	ret = nand_io_receive_data(nd_errpt_info.io_context,badblockbuf,4);
	if (ret < 0) {
		ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
		goto err;
	}
	ret = check_badblock_buf(badblockbuf, 4);
err:
	nand_io_chip_deselect(nd_errpt_info.io_context, csinfo_table->id);
	return ret;
}
#endif
static int write_badblock(nand_data *nddata, nand_flash *ndflash, int chip_index, int pageid)
{
	int ret = 0;
	unsigned char badblockbuf[4] = {0x00};
	unsigned int badblockpos = (ndflash->pagesize != 512) ? ndflash->pagesize + ndflash->oobsize - 4 : ndflash->oobsize - 4;

	/* if pagesize == 512, write 0x50 to reset pointer to 512 before program */
	if (ndflash->pagesize == 512)
		nand_io_send_cmd(nd_errpt_info.io_context, CMD_OOB_READ_1ST_512, NO_DELAY);
	nand_io_send_cmd(nd_errpt_info.io_context, CMD_PAGE_PROGRAM_1ST, NO_DELAY);
	errpt_nand_io_send_addr(nd_errpt_info.io_context, ndflash, badblockpos, pageid, nddata->cinfo->ops_timing.tADL);
	nand_io_send_data(nd_errpt_info.io_context, badblockbuf, 4);
	nand_busy_clear(nddata,chip_index);
	nand_io_send_cmd(nd_errpt_info.io_context, CMD_PAGE_PROGRAM_2ND, nddata->cinfo->ops_timing.tWB);
	ret = nand_wait_ready(nddata,chip_index);
	if (ret < 0) {
		ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
		goto err;
	}

	ret = nand_io_send_cmd(nd_errpt_info.io_context, CMD_READ_STATUS_1ST, nddata->cinfo->ops_timing.tWHR);
	if (ret & NAND_STATUS_FAIL)
		ret = -1;
	else if(!(ret & NAND_STATUS_WP))
		ret = -2;
	else
		ret = 0;

err:
	return ret;
}

static int nand_erase_block(nand_data *nddata, nand_flash *ndflash, int chip_index, unsigned int blockid)
{
	cs_item *csitem = nddata->csinfo->csinfo_table + chip_index;
	int pageid = blockid * nddata->cinfo->ppblock;
	int ret = 0;
//	ndd_debug("%s blockid[%d] chip_index[%d], csitem->id = %d\n",__func__,blockid,chip_index,csitem->id);
	__wp_disable(nddata->gpio_wp);
	nand_io_chip_select(nd_errpt_info.io_context, csitem->id);
	nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_ERASE1, 0);
	errpt_nand_io_send_addr(nd_errpt_info.io_context, ndflash, -1, pageid, 0);
	nand_busy_clear(nddata,chip_index);
	nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_ERASE2, 100);
	ret = nand_wait_ready(nddata, chip_index);
	if(ret < 0)
		ndd_print(NDD_ERROR,"%s %d wait rb timeout! chipid=%d gpio=%d\n",__func__,__LINE__,csitem->id,csitem->rbitem->gpio);
	ret = nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_STATUS, 0);
	if (ret & NAND_STATUS_FAIL)
		ret = ENAND;
	else if(!(ret & NAND_STATUS_WP))
		ret = WRITE_PROTECT;
	else
		ret = SUCCESS;
	nand_io_chip_deselect(nd_errpt_info.io_context, csitem->id);
	__wp_enable(nddata->gpio_wp);
	return ret;
}

/*
  rb_ready: 1 --- nand_api_init had filled rbinfo.
            0 --- hadn't filled rbinfo, need to read from errpt.
  return 0: it is badblock
  return -1:it isn't badblock
*/
static int is_bad_block(nand_data *nddata, nand_flash *ndflash, int chip_index, unsigned int blockid, unsigned int planes, int rb_ready)
{
	int i,ret = 0;
	int count = 0;
	int pageid = blockid * (ndflash->blocksize / ndflash->pagesize);
	unsigned int ppblock = ndflash->blocksize / ndflash->pagesize;
	ndd_debug("%s blockid[%d] chip_index[%d] rb_ready[%d]\n",__func__,blockid,chip_index,rb_ready);
	while (planes--) {
		for (i = 0; i < 4; i++) {
			ret = read_badblock(nddata, ndflash, chip_index, pageid + i,rb_ready);
			if (ret < 0) {
				ndd_print(NDD_ERROR, "func:%s line:%d ret=%d\n",__func__, __LINE__, ret);
				return ret;
			} else
				count += ret;
		}
		if (count > 64){
			ndd_debug("%s ret=-1\n",__func__);
			return -1;
		}
		count = 0;
		if(planes){
			pageid += ppblock;
		}
	}

	ndd_debug("%s ret=0\n",__func__);
	return 0;
}

static int mark_bad_block(nand_data *nddata, nand_flash *ndflash, int chip_index, unsigned int blockid)
{
	cs_item *csitem = nddata->csinfo->csinfo_table + chip_index;
	int pageid = blockid * (ndflash->blocksize / ndflash->pagesize);
	int i,ret = 0;
	__wp_disable(nddata->gpio_wp);
	nand_io_chip_select(nd_errpt_info.io_context, csitem->id);
	for(i = 0; i < 4; i++){
		ret = write_badblock(nddata, ndflash, chip_index, pageid + i);
		if (ret < 0) {
			ndd_print(NDD_ERROR, "func:%s blockid[%d] line:%d ret=%d\n",__func__,blockid,__LINE__, ret);
			return ret;
		}
	}
	nand_io_chip_deselect(nd_errpt_info.io_context, csitem->id);
	__wp_enable(nddata->gpio_wp);
	ndd_debug("%s [%d]\n",__func__,blockid);
	return ret;
}
/*
* return 0 : it isn't inherent badblock
* return -1 : it is inherent badblock
*/
static int is_inherent_badblock(nand_data *nddata, nand_flash *ndflash, unsigned int cs, unsigned int blockid, int rb_ready)
{
	return 0;
}

static int nand_force_eraseall(nand_data *nddata, nand_flash *ndflash,plat_ptinfo *ptinfo)
{
	cs_info *csinfo = nddata->csinfo;
	unsigned int blockpchip = ndflash->totalblocks / ndflash->chips;
	unsigned int chip_index, blockid;
	plat_ptitem *ptitem = ptinfo->pt_table;
	int pt_index;
	int reserve_flag = 0;
	int ret = 0;
	for(chip_index = 0; chip_index < csinfo->totalchips; chip_index++){
		for(blockid = 0; blockid < blockpchip; blockid++){
			reserve_flag = 0;
			{
				for(pt_index=0;pt_index < ptinfo->ptcount;pt_index++)
				{
					if(PARTITION_NEED_RESERVE((ptitem + pt_index)->attribute)){
						unsigned int blk_start = ndd_div_s64_32((ptitem + pt_index)->offset, ndflash->blocksize);
						unsigned int blk_end = ndd_div_s64_32(((ptitem + pt_index)->offset + (ptitem + pt_index)->size),ndflash->blocksize);
						if(blockid >= blk_start && blockid < blk_end)
						{
							reserve_flag = 1;
							break;
						}
					}
				}
			}
			if(reserve_flag == 1)
				continue;
			ret = nand_erase_block(nddata, ndflash, chip_index, blockid);
			if(ret != SUCCESS){
				if(ret == WRITE_PROTECT){
					ndd_print(NDD_ERROR,"%s %d the nand's state is write_protect !!\n",__func__,__LINE__);
					goto err;
				}
				ret = mark_bad_block(nddata, ndflash, chip_index, blockid);
				if(ret < 0){
					if (is_bad_block(nddata, ndflash, chip_index, blockid, 1, RB_READY) != 0){
						ndd_debug("%s erase fail and is bad block,skip mark[%d]\n",__func__,blockid);
						ret = 0;
						continue;
					} else {
						ndd_print(NDD_ERROR,"%s %d mark_bad_block[%d] error!\n",
							  __func__,__LINE__,blockid);
						goto err;
					}
				}
			}
		}
	}
err:
	return ret;
}

static int nand_normal_eraseall(nand_data *nddata, nand_flash *ndflash)
{
	cs_info *csinfo = nddata->csinfo;
	unsigned int blockpchip = ndflash->totalblocks / ndflash->chips;
	unsigned int chip_index, blockid;
	int ret = 0;
	for(chip_index = 0; chip_index < csinfo->totalchips; chip_index++){
		for(blockid = 0; blockid < blockpchip; blockid++){
			if(is_bad_block(nddata, ndflash, chip_index, blockid, 1, RB_READY) < 0)
				continue;
			ret = nand_erase_block(nddata, ndflash, chip_index, blockid);
			if(ret != SUCCESS){
				if(ret == WRITE_PROTECT){
					ndd_print(NDD_ERROR,"%s %d the nand's state is write_protect !!\n",__func__,__LINE__);
					goto err;
				}
				ret = mark_bad_block(nddata, ndflash, chip_index, blockid);
				if(ret < 0){
					if (is_bad_block(nddata, ndflash, chip_index, blockid, 1, RB_READY) != 0){
						ndd_debug("%s erase fail and is bad block,skip mark[%d]\n",__func__,blockid);
						ret = 0;
						continue;
					} else {
						ndd_print(NDD_ERROR,"%s %d mark_bad_block[%d] error!\n",
							  __func__,__LINE__,blockid);
						goto err;
					}
				}
			}
		}
	}
err:
	return ret;
}

static int nand_factory_eraseall(nand_data *nddata, nand_flash *ndflash)
{
	cs_info *csinfo = nddata->csinfo;
	unsigned int blockpchip = ndflash->totalblocks / ndflash->chips;
	unsigned int chip_index, blockid;
	int ret = 0, inherent = 0;
	for(chip_index = 0; chip_index < csinfo->totalchips; chip_index++){
		for(blockid = 0; blockid < blockpchip; blockid++){
			if(is_inherent_badblock(nddata, ndflash, chip_index, blockid, RB_READY) < 0){
				inherent = 1;
			}
			ret = nand_erase_block(nddata, ndflash, chip_index, blockid);
			if(ret != SUCCESS || inherent != 0){
				if(ret == WRITE_PROTECT){
					ndd_print(NDD_ERROR,"%s %d the nand's state is write_protect !!\n",__func__,__LINE__);
					goto err;
				}
				inherent = 0;
				ret = mark_bad_block(nddata, ndflash, chip_index, blockid);
				if(ret < 0){
					if (is_bad_block(nddata, ndflash, chip_index, blockid, 1, RB_READY) != 0){
						ndd_debug("%s erase fail and is bad block,skip mark[%d]\n",__func__,blockid);
						ret = 0;
						continue;
					} else {
						ndd_print(NDD_ERROR,"%s %d mark_bad_block[%d] error!\n",
							  __func__,__LINE__,blockid);
						goto err;
					}
				}
			}
		}
	}
err:
	return ret;
}

static int nand_write_pagedata(nand_data *nddata, nand_flash *ndflash, int chip_index, unsigned int pageid, unsigned char *wbuf)
{
	cs_item *csitem = nddata->csinfo->csinfo_table + chip_index;
	int i,ret = 0, len = 0;
	int eccsize = nddata->eccsize;
	int oobsize = ndflash->oobsize;
	int eccsteps = ndflash->pagesize / eccsize;
	int eccbytes = 14 * ndflash->eccbit / 8;

	__wp_disable(nddata->gpio_wp);
	nand_io_chip_select(nd_errpt_info.io_context, csitem->id);
	if (ndflash->pagesize == 512)
		nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_READ0, NO_DELAY);
	nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_WRITE, 0);
	errpt_nand_io_send_addr(nd_errpt_info.io_context, ndflash, 0, pageid, 100);
	for(i = 0; i < eccsteps; i++)
	{
		pn_enable(nd_errpt_info.io_context);
		nand_io_send_data(nd_errpt_info.io_context, wbuf + i * eccsize, eccsize);
		nand_io_send_data(nd_errpt_info.io_context, nd_errpt_info.bchbuf + i * eccbytes, eccbytes);
		nand_io_send_waitcomplete(nd_errpt_info.io_context, nddata->cinfo);
		pn_disable(nd_errpt_info.io_context);
	}
	len = oobsize - i * eccbytes;
	if(len > 0)
		nand_io_send_data(nd_errpt_info.io_context, nd_errpt_info.bchbuf + i * eccbytes, len);
	nand_busy_clear(nddata,chip_index);
	nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_PROGRAM, 0);
	ret = nand_wait_ready(nddata,chip_index);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d wait rb timeout! chipid=%d gpio=%d\n",__func__,__LINE__,csitem->id,csitem->rbitem->gpio);
		goto err;
	}
	ret = nand_io_send_cmd(nd_errpt_info.io_context, NAND_CMD_STATUS, 0);
	if (ret & NAND_STATUS_FAIL)
		ret = ENAND;
	else if(!(ret & NAND_STATUS_WP))
		ret = -2;
	else
		ret = SUCCESS;

	nand_io_chip_deselect(nd_errpt_info.io_context, csitem->id);
	__wp_enable(nddata->gpio_wp);
	ndd_debug("%s ret=%d\n",__func__,ret);
err:
	return ret;
}
/*
* the first page of errpt will comply with the rule of spl, when we read and write it.
* the first 256bytes is data, the second 256bytes is ecc parity.
*/
static int nand_encode_errpt_headpage(nand_data *nddata, nand_flash *ndflash, unsigned char *wbuf)
{
	int ret = 0;
	PipeNode pipenode;
	int bch_context = nand_bch_open(&(nddata->base->bch), 256);

	pipenode.data = wbuf;
	pipenode.parity = wbuf + 256;
	ndd_memset(nd_errpt_info.bchbuf,0xff,ndflash->oobsize);
	ret = nand_bch_encode_prepare(bch_context, &pipenode, 64);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d bch encode prepare error, ret = %d\n",ret);
		goto err;
	}
	ret = nand_bch_encode_complete(bch_context, &pipenode);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d bch encode complet error, ret = %d\n",ret);
		goto err;
	}
err:
	nand_bch_close(bch_context);
	return ret;
}
static int nand_write_page(nand_data *nddata, nand_flash *ndflash, int chip_index, unsigned int pageid, unsigned char *wbuf)
{
	int i,ret = 0;
	int eccsize = nddata->eccsize;
	int oobsize = ndflash->oobsize;
	int eccsteps = ndflash->pagesize / eccsize;
	int eccbytes = 14 * ndflash->eccbit / 8;
	PipeNode pipenode;

	ndd_debug("%s pageid[%d] chip_index[%d]\n",__func__,pageid,chip_index);
	ndd_memset(nd_errpt_info.bchbuf,0xff,oobsize);
	for(i = 0; i < eccsteps; i++)
	{
		pipenode.data = wbuf + i * eccsize;
		pipenode.parity = nd_errpt_info.bchbuf + i * eccbytes;
		ret = nand_bch_encode_prepare(nd_errpt_info.bch_context, &pipenode, ndflash->eccbit);
		if(ret < 0){
			ndd_print(NDD_ERROR,"%s %d bch encode prepare error, ret = %d i=%d\n",ret,i);
			goto err;
		}
		ret = nand_bch_encode_complete(nd_errpt_info.bch_context, &pipenode);
		if(ret < 0){
			ndd_print(NDD_ERROR,"%s %d bch encode complet error, ret = %d i=%d\n",ret,i);
			goto err;
		}
	}
	ret = nand_write_pagedata(nddata, ndflash, chip_index, pageid, wbuf);
err:
	return ret;
}
#ifndef NAND_ERRPT_BURNTOOL
static int nand_read_errpt_headpage(nand_data *nddata, nand_flash *ndflash, int chip_index, unsigned int pageid,
								unsigned char *rbuf, int rb_ready)
{
	PipeNode pipenode;
	int bitcount = 0;
	int ret = 0;
	int bch_context = nand_bch_open(&(nddata->base->bch), 256);

	ndd_debug("%s pageid[%d] chip_index[%d] rb_ready[%d]\n",__func__,pageid,chip_index,rb_ready);
	nand_io_chip_select(nd_errpt_info.io_context, chip_index);
	if (rb_ready)
		nand_busy_clear(nddata,chip_index);
	sent_readcmd_start(nddata, ndflash, 0, pageid);
	if(rb_ready){
		ret = nand_wait_ready(nddata, chip_index);
		if(ret < 0){
			ndd_print(NDD_ERROR,"%s %d wait rb timeout! chipid=%d\n",__func__,__LINE__, chip_index);
			goto err;
		}
	}else
		ndd_ndelay(5 * 1000 * 1000);

	/* read first and second 256bytes in headpage of errpt */
	pipenode.data = rbuf;
	pipenode.parity = rbuf + 256;
	nand_io_counter0_enable(nd_errpt_info.io_context);
	pn_enable(nd_errpt_info.io_context);
	nand_io_receive_data(nd_errpt_info.io_context, pipenode.data, nddata->eccsize);
	pn_disable(nd_errpt_info.io_context);
	bitcount = nand_io_read_counter(nd_errpt_info.io_context);
	nand_io_counter_disable(nd_errpt_info.io_context);
	if(bitcount < ndflash->eccbit){
		ndd_debug("allff----->bitcount[%d] eccbit[%d]\n",bitcount,ndflash->eccbit);
		ret = ALL_FF;
		goto err;
	}
	/* decode headpage */
	ret = nand_bch_decode_prepare(bch_context, &pipenode, 64);
	if (ret < 0) {
		ndd_debug("%s %d bch decode prepare ret = %d\n",__func__,__LINE__,ret);
		goto err;
	}
	ret = nand_bch_decode_complete(bch_context, &pipenode);
err:
	nand_bch_close(bch_context);
	nand_io_chip_deselect(nd_errpt_info.io_context, chip_index);
	ndd_debug("%s ret= %d\n",__func__,ret);
	return ret;
}
#else
static int nand_read_errpt_headpage(nand_data *nddata, nand_flash *ndflash, int chip_index, unsigned int pageid,
								unsigned char *rbuf, int rb_ready)
{
	int chipid = (nddata->csinfo->csinfo_table + chip_index)->id;
	PipeNode pipenode;
	int bitcount = 0;
	int ret = 0;
	int bch_context = nand_bch_open(&(nddata->base->bch), 256);

	ndd_debug("%s pageid[%d] chip_index[%d] rb_ready[%d]\n",__func__,pageid,chip_index,rb_ready);
	nand_io_chip_select(nd_errpt_info.io_context, chipid);
	if (rb_ready)
		nand_busy_clear(nddata,chip_index);
	sent_readcmd_start(nddata, ndflash, 0, pageid);
	if(rb_ready){
		ret = nand_wait_ready(nddata, chip_index);
		if(ret < 0){
			ndd_print(NDD_ERROR,"%s %d wait rb timeout! chipid=%d gpio=%d\n",__func__,__LINE__,
					chipid,(nddata->csinfo->csinfo_table + chip_index)->rbitem->gpio);
			goto err;
		}
	}else
		ndd_ndelay(5 * 1000 * 1000);

	/* read first and second 256bytes in headpage of errpt */
	pipenode.data = rbuf;
	pipenode.parity = rbuf + 256;
	nand_io_counter0_enable(nd_errpt_info.io_context);
	pn_enable(nd_errpt_info.io_context);
	nand_io_receive_data(nd_errpt_info.io_context, pipenode.data, nddata->eccsize);
	pn_disable(nd_errpt_info.io_context);
	bitcount = nand_io_read_counter(nd_errpt_info.io_context);
	nand_io_counter_disable(nd_errpt_info.io_context);
	if(bitcount < ndflash->eccbit){
		ndd_debug("allff----->bitcount[%d] eccbit[%d]\n",bitcount,ndflash->eccbit);
		ret = ALL_FF;
		goto err;
	}
	/* decode headpage */
	ret = nand_bch_decode_prepare(bch_context, &pipenode, 64);
	if (ret < 0) {
		ndd_debug("%s %d bch decode prepare ret = %d\n",__func__,__LINE__,ret);
		goto err;
	}
	ret = nand_bch_decode_complete(bch_context, &pipenode);
err:
	nand_bch_close(bch_context);
	nand_io_chip_deselect(nd_errpt_info.io_context, chipid);
	ndd_debug("%s ret= %d\n",__func__,ret);
	return ret;
}
#endif
static int nand_read_page(nand_data *nddata, nand_flash *ndflash, int chip_index, unsigned int pageid,
								unsigned char *rbuf, int rb_ready)
{
	int pagesize = ndflash->pagesize;
	int eccsize = nddata->eccsize;
	int eccsteps = pagesize / eccsize;
	int eccbytes = 14 * ndflash->eccbit / 8;
	int chipid = (nddata->csinfo->csinfo_table + chip_index)->id;
	PipeNode pipenode;
	int bitcount = 0;
	int i,ret = 0;
	int retry_flag = SUPPROT_READ_RETRY(nddata->cinfo);
	int retry_cnt = 0;

	ndd_debug("%s pageid[%d] chip_index[%d] rb_ready[%d]\n",__func__,pageid,chip_index,rb_ready);
read_retry:
	nand_io_chip_select(nd_errpt_info.io_context, chipid);
	if (rb_ready)
		nand_busy_clear(nddata,chip_index);
	sent_readcmd_start(nddata, ndflash, 0, pageid);
	if(rb_ready){
		ret = nand_wait_ready(nddata, chip_index);
		if(ret < 0){
			ndd_print(NDD_ERROR,"%s %d wait rb timeout! chipid=%d gpio=%d\n",__func__,__LINE__,
					chipid,(nddata->csinfo->csinfo_table + chip_index)->rbitem->gpio);
			goto read_page_finish;
		}
	}else
		ndd_ndelay(5 * 1000 * 1000);

	for(i = 0; i < eccsteps; i++){
		pipenode.data = rbuf + i*eccsize;
		pipenode.parity = nd_errpt_info.bchbuf + i*eccbytes;
		nand_io_counter0_enable(nd_errpt_info.io_context);
		pn_enable(nd_errpt_info.io_context);

		nand_io_receive_data(nd_errpt_info.io_context, pipenode.data, eccsize);
		nand_io_receive_data(nd_errpt_info.io_context, pipenode.parity, eccbytes);

		pn_disable(nd_errpt_info.io_context);
		bitcount = nand_io_read_counter(nd_errpt_info.io_context);
		nand_io_counter_disable(nd_errpt_info.io_context);
		if(bitcount < ndflash->eccbit){
			ndd_debug("allff----->offset[%d] bitcount[%d] eccbit[%d]\n",i*eccsize,bitcount,ndflash->eccbit);
			ret = ALL_FF;
			continue;
		}
		ret = nand_bch_decode_prepare(nd_errpt_info.bch_context, &pipenode, ndflash->eccbit);
		if (ret < 0) {
			ndd_debug("%s %d bch decode prepare ret = %d\n",__func__,__LINE__,ret);
			break;
		}
		ret = nand_bch_decode_complete(nd_errpt_info.bch_context, &pipenode);
		if (ret < 0) {
			ndd_debug("%s %d bch decode prepare ret = %d\n",__func__,__LINE__,ret);
			break;
		}
	}
	if((ret == ECC_ERROR) && (retry_cnt < 9)){
		if (retry_flag)
			set_retry_feature((int)nddata, chipid, 1);
		retry_cnt++;
		goto read_retry;
	}
read_page_finish:
	nand_io_chip_deselect(nd_errpt_info.io_context, chipid);
	if (retry_cnt)
		ndd_print(NDD_WARNING, "WARNING: %s, pageid [%d] read retry count = [%d]\n", __func__, pageid, retry_cnt);
	return ret;
}

static int creat_errpt_range(nand_data *nddata, nand_flash *ndflash)
{
	int /*cs_index = 0,*/ blkid;
	int blk_cnt = 0,badblockcnt = 0;
	int ret = 0;
	for(blkid = ndflash->totalblocks / ndflash->chips - 1; blkid > 0; blkid--){
		if(is_bad_block(nddata, ndflash, 0, blkid, 1, RB_READY) < 0){
			ndd_debug("---------- %s %d ----------\n",__func__,__LINE__);
			badblockcnt++;
			continue;
		}
		if(++blk_cnt == ERRPT_BLCOK_CNT){
			break;
		}
	}
	ndd_print(NDD_INFO,"%s %d dump errpt badblocks[%d],sumblocks[%d].\n",__func__,__LINE__,
							badblockcnt,badblockcnt + ERRPT_BLCOK_CNT);
	ndflash->maxvalidblocks = ndflash->totalblocks - (badblockcnt + ERRPT_BLCOK_CNT) * ndflash->chips;
	nddata->cinfo->maxvalidblocks = ndflash->maxvalidblocks / ndflash->chips;
	nddata->cinfo->maxvalidpages = nddata->cinfo->maxvalidblocks * nddata->cinfo->ppblock;
	return ret;
}

static inline int init_badblockbuf(nand_data *nddata, plat_ptinfo *ptinfo)
{
	int ptcount = ptinfo->ptcount;
	int pt_index;
	int i = 0;

	for(pt_index = 0; pt_index < ptcount; pt_index++){
		(ptinfo->pt_table + pt_index)->pt_badblock_info = (unsigned int *)ndd_alloc(nddata->cinfo->pagesize);
		if(!(ptinfo->pt_table + pt_index)->pt_badblock_info){
			ndd_print(NDD_ERROR,"%s %d alloc badblock_info error!\n",__func__,__LINE__);
			goto err;
		}
		ndd_memset((ptinfo->pt_table + pt_index)->pt_badblock_info, 0xff, nddata->cinfo->pagesize);
	}
	return 0;
err:
	for(i = 0; i < pt_index; i++)
		ndd_free((ptinfo->pt_table + i)->pt_badblock_info);
	return -1;
}

/**
 * if block is 'X', it is virtual
 *	     rb0   rb1   ...   rb(totalrbs)
 *     row ========================= (real start)
 *	   | raw |  X  | ... |  X  |
 *	   | pts |  X  | ... |  X  |
 *	   ------------------------- raw_boundary
 *	   |    zone               |
 *	   |    pts                |
 *	   ========================= rbblocks (real end)
 *	   :  X  : map : ... : map |
 *	   :  X  : ed  : ... : ed  |
 *	   ------------------------- (virtual end)
 *	   column
**/
static inline int is_blockid_virtual(unsigned int column, unsigned int row, unsigned int raw_boundary, unsigned int rbblocks)
{
	return ((column > 0) && (row < raw_boundary)) || ((column == 0) && (row >= rbblocks));
}
static inline int init_errpt_l2pblock(nand_data *nddata, errpt_l2pblock *l2p)
{
	unsigned char *cinr = l2p->csinrb;
	cs_item *csinfo_table = nddata->csinfo->csinfo_table;
	int totalrbs = nddata->rbinfo->totalrbs;
	int chipprb = nddata->csinfo->totalchips / totalrbs;
	int chip_index, rbid;
	int i = 0;

	l2p->rb_bound = 1;
	l2p->ol_bound = totalrbs;
	l2p->cs_bound = nddata->cinfo->maxvalidblocks;

	for(chip_index = 0; chip_index < nddata->csinfo->totalchips; chip_index++){
		rbid = (csinfo_table + chip_index)->rbitem->id;
		for(i = 0; i < chipprb; i++){
			if((cinr + rbid * chipprb)[i] == 0xff){
				(cinr + rbid * chipprb)[i] = chip_index;
				break;
			}
		}
	}
	return 0;
}
static int scan_pt_badblocks(nand_data *nddata, nand_flash *ndflash, plat_ptitem *ptitem, errpt_l2pblock *l2p, int raw_boundary)
{
	chip_info *cinfo = nddata->cinfo;
	int totalrbs = nddata->rbinfo->totalrbs;
	int chipprb = nddata->csinfo->totalchips / totalrbs;
	int chip_index, rb_id, ol_id, cs_id;
	unsigned int rbblocks = chipprb * cinfo->maxvalidblocks;
	unsigned int startblk,totalblocks;
	unsigned int vblockid = 0, pblockid = 0, blockid;
	unsigned int planes = 0;
	unsigned int *badblockbuf = ptitem->pt_badblock_info;
	int i = 0;

	if(ptitem->nm_mode == ZONE_MANAGER){
		planes = cinfo->planenum; /* the mode depends on the planenum in chip_info */
	}else{
		planes = 1;	/* the mode is one plane */
	}
	startblk = ndd_div_s64_32((ptitem->offset + ndflash->blocksize - 1), ndflash->blocksize);
	totalblocks = ndd_div_s64_32((ptitem->size + ndflash->blocksize - 1), ndflash->blocksize * planes);

	ndd_debug("\n%s[%d] size = %d, blocksize * planes = %d \n",__func__, __LINE__,
									(unsigned int)(ptitem->size), ndflash->blocksize * planes);
	ndd_debug("%s pt[%s] startblk[%d] totalblocks[%d]\n",__func__,ptitem->name,startblk,totalblocks);

	for(blockid = 0; blockid < totalblocks; blockid++){
		ol_id = blockid / l2p->ol_bound;
		vblockid = blockid % l2p->ol_bound;
		rb_id = vblockid / l2p->rb_bound;
		vblockid = ol_id * planes + startblk / l2p->ol_bound;
		if(is_blockid_virtual(rb_id, vblockid, raw_boundary, rbblocks)){
			badblockbuf[i++] = blockid;
			continue;
		}
		vblockid = (vblockid >= rbblocks) ? vblockid - rbblocks : vblockid;
		cs_id = vblockid / l2p->cs_bound;
		pblockid = vblockid % l2p->cs_bound;
		chip_index = (l2p->csinrb + rb_id * chipprb)[cs_id];
		if(is_bad_block(nddata, ndflash, chip_index, pblockid, planes, RB_READY) < 0){
			badblockbuf[i++] = blockid;
		}
	}
	return 0;
}

static inline void fill_errpt_info(nand_data *nddata, plat_ptinfo *ptinfo, ppa_head *phead, ppa_info *ppainfo)
{
	int i, rb_index, pt_index;
	rb_info *rbinfo = nddata->rbinfo;
	plat_ptitem *pt_table = ptinfo->pt_table;
	/*fill errpt head*/
	phead->magicid = 0x646e616e;
	Get_NandManagerVersion(&(phead->version));
	phead->drv_strength = nddata->cinfo->drv_strength;
	phead->gpio_wp = nddata->gpio_wp;
	phead->ptcount = ptinfo->ptcount;
	phead->raw_boundary = nd_raw_boundary;
	for(rb_index = 0; rb_index < rbinfo->totalrbs; rb_index++){
		phead->rbmsg[rb_index].gpio = (rbinfo->rbinfo_table + rb_index)->gpio;
		phead->rbmsg[rb_index].strength = (rbinfo->rbinfo_table + rb_index)->pulldown_strength;
	}

	/*fill partition and badblock info*/
	for(pt_index = 0; pt_index < ptinfo->ptcount; pt_index++){
		ndd_memcpy((ppainfo + pt_index)->name, (pt_table + pt_index)->name, MAX_NAME_SIZE);
		(ppainfo + pt_index)->ptindex = pt_index;
		(ppainfo + pt_index)->offset = (pt_table + pt_index)->offset;
		(ppainfo + pt_index)->size = (pt_table + pt_index)->size;
		(ppainfo + pt_index)->ops_mode = (pt_table + pt_index)->ops_mode;
		(ppainfo + pt_index)->nm_mode = (pt_table + pt_index)->nm_mode;
		(ppainfo + pt_index)->flags = (pt_table + pt_index)->flags;
		for(i = 0; i < MUL_PARTS; i++){
			ndd_memcpy(((ppainfo + pt_index)->ex_partition + i)->name,
				   ((pt_table + pt_index)->ex_partition + i)->name, MAX_NAME_SIZE);
			((ppainfo + pt_index)->ex_partition + i)->offset = ((pt_table + pt_index)->ex_partition + i)->offset;
			((ppainfo + pt_index)->ex_partition + i)->size = ((pt_table + pt_index)->ex_partition + i)->size;
		}
	}
}
static inline void trans_ppainfo_to_ptitem(plat_ptitem *ptitem, ppa_info *ppainfo, unsigned int *badblockbuf)
{
	int i = 0;
	ndd_memcpy(ptitem->name, ppainfo->name, MAX_NAME_SIZE);
	ptitem->offset = ppainfo->offset;
	ptitem->size = ppainfo->size;
	ptitem->ops_mode = ppainfo->ops_mode;
	ptitem->nm_mode = ppainfo->nm_mode;
	ptitem->flags = ppainfo->flags;
	ptitem->pt_badblock_info = badblockbuf;
	for(i = 0; i < MUL_PARTS; i++){
		ndd_memcpy((ptitem->ex_partition + i)->name, (ppainfo->ex_partition + i)->name, MAX_NAME_SIZE);
		(ptitem->ex_partition + i)->offset = (ppainfo->ex_partition + i)->offset;
		(ptitem->ex_partition + i)->size = (ppainfo->ex_partition + i)->size;
	}
}
static int write_pt_badblock_info(nand_data *nddata, nand_flash *ndflash, int chip_index, unsigned int *badblockbuf,
							int pageid, unsigned char *databuf)
{
	int ret = 0;
	int chipid = (nddata->csinfo->csinfo_table + chip_index)->id;
	/*write and reread partition info*/
	ret = nand_write_page(nddata, ndflash, chip_index, pageid, databuf);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d write chip[%d] pageid[%d] error!\n",__func__,__LINE__,chipid,pageid);
		goto err;
	}
	ret = nand_read_page(nddata, ndflash, chip_index, pageid, databuf, RB_READY);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d read chip[%d] pageid[%d] error!\n",__func__,__LINE__,chipid,pageid);
		goto err;
	}
        /*write and reread badblockinfo*/
	ret = nand_write_page(nddata, ndflash, chip_index, pageid+1, (unsigned char *)badblockbuf);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d write chip[%d] pageid[%d] error!\n",__func__,__LINE__,chipid,pageid+1);
		goto err;
	}
	ret = nand_read_page(nddata, ndflash, chip_index, pageid+1, databuf, RB_READY);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d read chip[%d] pageid[%d] error!\n",__func__,__LINE__,chipid,pageid+1);
		goto err;
	}
err:
	return ret;
}

static inline unsigned int get_raw_boundary(plat_ptinfo *plat_ptinfo, chip_info *cinfo, unsigned short totalrbs)
{
	unsigned int raw_boundary = 0;
	unsigned char nm_mode, last_nm_mode = -1;
	unsigned short pt_index = plat_ptinfo->ptcount;
	plat_ptitem *plat_pt = plat_ptinfo->pt_table;

	while (pt_index--) {
		nm_mode = plat_pt[pt_index].nm_mode;
		if ((nm_mode != ZONE_MANAGER) && (last_nm_mode == ZONE_MANAGER)) {
			unsigned int blocksize = cinfo->pagesize * cinfo->ppblock;
			unsigned short alignunit = cinfo->planenum * totalrbs;
			unsigned int blockid = ndd_div_s64_32((plat_pt[pt_index + 1].offset + blocksize - 1), blocksize);
			ndd_debug("%s pt[%s] blockid[%d] alignunit[%d] totalrbs[%d]",
				  __func__,plat_pt[pt_index].name,blockid,alignunit,totalrbs);
			raw_boundary = ((blockid + (alignunit - 1)) / alignunit) * alignunit;
			ndd_debug(" raw_boundary[%d]\n",raw_boundary);
			break;
		}
		last_nm_mode = nm_mode;
	}
	nd_raw_boundary = raw_boundary;
	return raw_boundary;
}
static inline unsigned int get_startblockid(plat_ptitem *plat_pt, chip_info *cinfo,
				   unsigned short totalrbs, unsigned int raw_boundary)
{
	unsigned int blocksize = cinfo->pagesize * cinfo->ppblock;
	unsigned int blockid = ndd_div_s64_32((plat_pt->offset + blocksize - 1), blocksize);

	if (plat_pt->nm_mode == ZONE_MANAGER) {
		unsigned short alignunit = cinfo->planenum * totalrbs;
		return (((blockid + (alignunit - 1)) / alignunit) * alignunit)
			+ (totalrbs > 1 ? raw_boundary * (totalrbs - 1) : 0);
	} else
		return blockid * totalrbs;
}
static inline unsigned int get_endblockid(plat_ptitem *plat_pt, chip_info *cinfo,
				 unsigned short totalchips, unsigned short totalrbs,
				 unsigned int raw_boundary)
{
	unsigned int blockid;
	unsigned long long offset;
	unsigned int blocksize = cinfo->pagesize * cinfo->ppblock;

	if (plat_pt->size == -1) {
		blockid = cinfo->maxvalidblocks * totalchips;
		/* update last plat partition totalblocks
		   rb    rb
		   ------------- nand start
		   | raw | X X |
		   | pts | X X | here mark bad blocks, and mirror to 'vtl'
		   -------------
		   |    zone   |
		   |    pts    |
		   ------------- nand end
		   | X X | vtl |
		   | X X |     | here add to last zone pt
		   -------------
		*/
		blockid += (totalrbs > 1) ? raw_boundary * totalrbs : 0;
	} else {
		offset = plat_pt->offset + plat_pt->size;
		blockid = ndd_div_s64_32(offset, blocksize);
	}

	if (plat_pt->nm_mode == ZONE_MANAGER) {
		unsigned short alignunit = cinfo->planenum * totalrbs;
		return (blockid / alignunit * alignunit) - 1;
	} else
		return blockid * totalrbs - 1;
}

static inline int next_platpt_connected(unsigned int plat_index,
					plat_ptitem *plat_pt, unsigned int plat_ptcount)
{
	return (((plat_index + 1) < plat_ptcount) &&
		((plat_pt[plat_index].offset + plat_pt[plat_index].size) == plat_pt[plat_index + 1].offset));
}

static inline void adjust_plat_ptinfo(nand_data *nddata, plat_ptinfo *ptinfo, int raw_boundary)
{
	plat_ptitem *pt_table = ptinfo->pt_table;
	int pt_index = ptinfo->ptcount;
	int totalrbs = nddata->rbinfo->totalrbs;
	int blksize = nddata->cinfo->pagesize * nddata->cinfo->ppblock;
	unsigned int pt_startblk, pt_endblk;
//	int i = 0;

	while(pt_index--){
		pt_startblk = get_startblockid(&pt_table[pt_index], nddata->cinfo, totalrbs, raw_boundary);
		pt_table[pt_index].offset = (unsigned long long)pt_startblk * (unsigned long long)blksize;

		if (next_platpt_connected(pt_index, pt_table, ptinfo->ptcount))
			pt_endblk = ndd_div_s64_32(pt_table[pt_index + 1].offset, blksize) - 1;
		else
			pt_endblk = get_endblockid(&pt_table[pt_index], nddata->cinfo, nddata->csinfo->totalchips,
						    totalrbs, raw_boundary);

		pt_table[pt_index].size = (unsigned long long)(pt_endblk - pt_startblk + 1) * (unsigned long long)blksize;
#if 0
		for(i = 0; i < MUL_PARTS; i++){
			union long_int{
				unsigned long long l;
				unsigned int a[2];
			}tt;
			ndd_debug("----- %s[%d] -----\n", __func__,__LINE__);
			tt.l = pt_table[pt_index].ex_partition[i].offset;
			ndd_debug("----- offset.h = %d offset.l = %d -----\n",tt.a[1],tt.a[0]);
			tt.l = pt_table[pt_index].ex_partition[i].size;
			ndd_debug("----- size.h = %d size.l = %d -----\n",tt.a[1],tt.a[0]);

			if(pt_table[pt_index].ex_partition[i].size != 0){
				pt_table[pt_index].ex_partition[i].offset *= (unsigned long long)blksize; // n Mbytes -> m bytes
				if(pt_table[pt_index].ex_partition[i].size == -1){
					pt_table[pt_index].ex_partition[i].size = pt_table[pt_index].size
									- pt_table[pt_index].ex_partition[i].offset;
				}else{
					pt_table[pt_index].ex_partition[i].size *= (unsigned long long)blksize; // n Mbytes -> m bytes
				}
			}
			ndd_debug("----- %s[%d] -----\n", __func__,__LINE__);
			tt.l = pt_table[pt_index].ex_partition[i].offset;
			ndd_debug("----- offset.h = %d offset.l = %d -----\n",tt.a[1],tt.a[0]);
			tt.l = pt_table[pt_index].ex_partition[i].size;
			ndd_debug("----- size.h = %d size.l = %d -----\n",tt.a[1],tt.a[0]);

		}
#endif
	}
}

static inline int read_partition_info(nand_data *nddata,nand_flash *ndflash, int chip_index, unsigned int *badblockbuf,
						int pageid, unsigned char *databuf)
{
	int chipid = (nddata->csinfo->csinfo_table + chip_index)->id;
	int ret = -1;
	if(!badblockbuf){
		ndd_print(NDD_ERROR,"%s %d alloc ptitem->pt_badblock_info error!\n",__func__,__LINE__);
		goto err;
	}
	ret = nand_read_page(nddata, ndflash, chip_index, pageid, databuf, RB_READY);
	if(ret < 0){
		ndd_print(NDD_INFO,"%s %d read errpt chipid[%d] pagid[%d] error!\n",__func__,__LINE__,chipid,pageid);
		goto err;
	}
	ret = nand_read_page(nddata, ndflash, chip_index, pageid + 1, (unsigned char *)badblockbuf, RB_READY);
	if(ret < 0){
		ndd_print(NDD_INFO,"%s %d read errpt chipid[%d] pagid[%d] error!\n",__func__,__LINE__,chipid,pageid);
		goto err;
	}
err:
	return ret;
}
static inline int fill_partition_info(plat_ptinfo *ptinfo, unsigned char *databuf, int pt_index)
{
	ppa_info *ppainfo = (ppa_info *)databuf;
	int i;

	ndd_memcpy((ptinfo->pt_table + pt_index)->name, ppainfo->name,MAX_NAME_SIZE);
	(ptinfo->pt_table + pt_index)->offset = ppainfo->offset;
	(ptinfo->pt_table + pt_index)->size = ppainfo->size;
	(ptinfo->pt_table + pt_index)->ops_mode = ppainfo->ops_mode;
	(ptinfo->pt_table + pt_index)->nm_mode = ppainfo->nm_mode;
	(ptinfo->pt_table + pt_index)->flags = ppainfo->flags;
	for(i = 0; i< MUL_PARTS; i++){
		ndd_memcpy(((ptinfo->pt_table + pt_index)->ex_partition + i)->name,
			(ppainfo->ex_partition + i)->name,MAX_NAME_SIZE);
		((ptinfo->pt_table + pt_index)->ex_partition + i)->offset = (ppainfo->ex_partition + i)->offset;
		((ptinfo->pt_table + pt_index)->ex_partition + i)->size = (ppainfo->ex_partition + i)->size;
	}
	return 0;
}


static inline int update_badblockbuf(nand_data *nddata, nand_flash *ndflash, ppa_info *ppainfo, PPartition *pt,
						errpt_l2pblock *l2p, unsigned int *badblockbuf)
{
	plat_ptitem ptitem;

	if(!ndd_strcmp(pt->name,ppainfo->name)){
		trans_ppainfo_to_ptitem(&ptitem, ppainfo, pt->pt_badblock_info);
		scan_pt_badblocks(nddata, ndflash, &ptitem, l2p, nddata->ptinfo->raw_boundary);
		ndd_memcpy(badblockbuf, pt->pt_badblock_info, nddata->cinfo->pagesize);
	}
	return 0;
}

static int prepare_pt_info(nand_data *nddata, nand_flash *ndflash, PPartition *pt, unsigned char *databuf)
{
	unsigned char *readbuf;
	errpt_l2pblock l2pblock;
	unsigned int *badblockbuf;
	ppa_info *ppainfo;
	int chip_index, blk_index, pt_index;
	int ept_startblk = ndflash->totalblocks / ndflash->chips - 1;
	int ept_endblk = ndflash->maxvalidblocks / ndflash->chips - 1;
	int ptcount = nddata->ptinfo->ptcount - REDUN_PT_NUM;
	int totalrbs = nddata->rbinfo->totalrbs;
	int chipprb = nddata->csinfo->totalchips / totalrbs;
	int pageid, startpageid;
	int ret = 0;

	l2pblock.csinrb = ndd_alloc(totalrbs * chipprb);
	if(!(l2pblock.csinrb)){
		ndd_print(NDD_ERROR,"%s %d alloc cstorb error!\n",__func__,__LINE__);
		ret = -1;
		goto err;
	}
	ndd_memset(l2pblock.csinrb, 0xff, totalrbs * chipprb);
	init_errpt_l2pblock(nddata, &l2pblock);


	for(chip_index = 0; chip_index < nddata->csinfo->totalchips; chip_index++){
		for(blk_index = ept_startblk; blk_index > ept_endblk; blk_index--){
			if(is_bad_block(nddata, ndflash, chip_index, blk_index, 1, RB_READY) < 0){
				continue;
			}
			startpageid = blk_index * nddata->cinfo->ppblock;
			for(pt_index = 0; pt_index < ptcount; pt_index++){
				if(pt_index == 0){
					ret = nand_read_errpt_headpage(nddata, ndflash, chip_index, startpageid, databuf, RB_READY);
					if(ret < 0){
						ndd_print(NDD_INFO,"%s %d read errpt blk[%d] pagid[%d] error!\n",
							  __func__,__LINE__,blk_index,startpageid);
						break;
					}
				}
				pageid = startpageid + PT_HEADINFO_CNT + pt_index * PT_INFO_CNT;
				readbuf = databuf + (PT_HEADINFO_CNT + pt_index * PT_INFO_CNT) * nddata->cinfo->pagesize;
				badblockbuf = (unsigned int *)(readbuf + nddata->cinfo->pagesize);
				ret = read_partition_info(nddata, ndflash, chip_index, badblockbuf, pageid, readbuf);
				if(ret < 0){
					ndd_print(NDD_INFO,"%s %d read_partition_info pagid[%d]failed! try again!\n",
						  __func__,__LINE__,pageid);
					break;
				}
				ppainfo = (ppa_info *)readbuf;
				update_badblockbuf(nddata, ndflash, ppainfo, pt, &l2pblock, badblockbuf);
			}
			if(ret >= 0 && pt_index == ptcount)
				goto ending;
		}
	}
ending:
	ndd_free(l2pblock.csinrb);
err:
	return ret;
}

static int epterase_writebadblockinfo(nand_data *nddata, nand_flash *ndflash, unsigned char *databuf)
{
	int chip_index, blkid, pageid;
	int ept_startblk = ndflash->totalblocks / ndflash->chips - 1;
	int ept_endblk = ndflash->maxvalidblocks / ndflash->chips - 1;
	int sumpage =  nddata->ptinfo->ptcount * PT_INFO_CNT + PT_HEADINFO_CNT;
	int pagesize = nddata->cinfo->pagesize;
	int i = 0, ret = 0;

	for(chip_index = 0; chip_index < nddata->csinfo->totalchips; chip_index++){
		for(blkid = ept_startblk; blkid > ept_endblk; blkid--){
			if(is_bad_block(nddata, ndflash, chip_index, blkid, 1, RB_READY) < 0)
				continue;
			ret = nand_erase_block(nddata, ndflash, chip_index, blkid);
			if(ret < 0){
				ret = mark_bad_block(nddata, ndflash, chip_index, blkid);
				if(ret < 0){
					if (is_bad_block(nddata, ndflash, chip_index, blkid, 1, RB_READY) != 0){
						ndd_debug("%s erase fail and is bad block, skip mark[%d]\n",
							  __func__, blkid);
						ret = 0;
						continue;
					} else {
						ndd_print(NDD_ERROR,"%s %d mark_bad_block[%d] error!\n",
							  __func__,__LINE__,blkid);
						goto err;
					}
				}
			}
			for(i = 0; i < sumpage; i++){
				pageid = blkid * nddata->cinfo->ppblock + i;
				if(i == 0){
					nand_encode_errpt_headpage(nddata, ndflash, databuf);
					nand_write_pagedata(nddata, ndflash, chip_index, pageid, databuf);
					continue;
				}
				ret = nand_write_page(nddata, ndflash, chip_index, pageid, databuf + i*pagesize);
				if(ret < 0){
					nand_erase_block(nddata, ndflash, chip_index, blkid);
					ret = mark_bad_block(nddata, ndflash, chip_index, blkid);
					if(ret < 0){
						if (is_bad_block(nddata, ndflash, chip_index, blkid, 1, RB_READY) != 0){
							ndd_debug("%s erase fail and is bad block, skip mark[%d]\n",
								  __func__, blkid);
							ret = 0;
							continue;
						} else {
							ndd_print(NDD_ERROR,"%s %d mark_bad_block[%d] error!\n",
								  __func__,__LINE__,blkid);
							goto err;
						}
					}
					break;
				}
			}
		}
	}
err:
	return ret;
}

/******************************** OPERATE ERRPT INTERFACE *********************************/
int get_errpt_head(nand_data *nddata, struct nand_api_platdependent *platdep)
{
	int ret = ENAND;
	unsigned char *databuf;
	ppa_head *ppahead = NULL;
	int rb_ready = 0;
	int blockid, cs_index = 0, rb_index;
	nm_version version;
	nand_flash *ndflash = platdep->nandflash;
	int ppblock = ndflash->blocksize / ndflash->pagesize;
	int errpt_startblk = ndflash->totalblocks / ndflash->chips -1;
	int errpt_endblk = ndflash->maxvalidblocks / ndflash->chips- 1;
	rb_info *rbinfo = platdep->rbinfo;

	nand_io_bch_open(nddata->base, NULL, nddata->eccsize, ndflash->buswidth);

	databuf = ndd_alloc(ndflash->pagesize);
	if (!databuf)
		GOTO_ERR(alloc_databuf);

	//for (cs_index = 0; (cs_index < CS_PER_NFI) && (ppahead == NULL); cs_index++)
	for (blockid = errpt_startblk; blockid > errpt_endblk; blockid--) {
#ifndef NAND_ERRPT_BURNTOOL
		rb_ready = RB_NO_READY;
#else
		rb_ready = RB_READY;
#endif
		if (is_bad_block(nddata, ndflash, cs_index, blockid, 1, rb_ready))
			continue;
		ret = nand_read_errpt_headpage(nddata, ndflash, cs_index, blockid * ppblock, databuf, rb_ready);
		if (ret < 0) {
			ndd_print(NDD_WARNING, "read errpt blk[%d] pagid[%d] error!\n", blockid, blockid * ppblock);
			continue;
		}

		/* get ppahead and check head info magicid */
		ppahead = (ppa_head *)databuf;
		if (ppahead->magicid == 0x646e616e)
			break;
		else
			ppahead = NULL;
	}

	if (!ppahead)
		GOTO_ERR(get_errpt_head);

	/* compared versions of nandmanager, one is from libnm, other one is from errpt */
	Get_NandManagerVersion(&version);
	if(version.major != ppahead->version.major || version.minor != ppahead->version.minor){
		ndd_print(NDD_ERROR,"ERROR: nandmanager version mismatching!\n",
			  "libnm.version:[%d.%d.%d]; errpt.version:[%d.%d.%d]\n",
			  version.major, version.minor, version.revision, ppahead->version.major,
			  ppahead->version.minor, ppahead->version.revision);
		GOTO_ERR(mismatch_version);
	}

	/**************** fill nandflash *****************/
	ndd_memcpy(ndflash, (databuf + NANDFLASH_OFFSET_IN_ERRPTHEAD), sizeof(nand_flash));

	/****************** fill rbinfo ******************/
	/* get rb count */
	rbinfo->totalrbs = 0;
	for (rb_index = 0; rb_index < MAX_RB_COUNT; rb_index++) {
		if (ppahead->rbmsg[rb_index].gpio == 0xffff)
			break;
		rbinfo->totalrbs++;
	}

	/* alloc rbinfo_table memory,
	 * NOTE: here may caused memory not be freed when errors
	 * at fllowed functions, but we do't care here */
	rbinfo->rbinfo_table = ndd_alloc(sizeof(rb_item) * rbinfo->totalrbs);
	if (!rbinfo->rbinfo_table)
		GOTO_ERR(alloc_rbinfo_table);

	/* fill rb table */
	for (rb_index = 0; rb_index < rbinfo->totalrbs; rb_index++) {
		(rbinfo->rbinfo_table + rb_index)->id = rb_index;
		(rbinfo->rbinfo_table + rb_index)->gpio = ppahead->rbmsg[rb_index].gpio;
		(rbinfo->rbinfo_table + rb_index)->pulldown_strength = ppahead->rbmsg[rb_index].strength;
	}

	/**************** fill nandflash *****************/
	platdep->platptinfo->ptcount = ppahead->ptcount;
	platdep->gpio_wp = ppahead->gpio_wp;
	platdep->drv_strength = ppahead->drv_strength;
	platdep->erasemode = 0;

	/* get raw boundary */
	nd_raw_boundary = ppahead->raw_boundary;

	/* dump */
	ndd_dump_nandflash(ndflash);
	dump_pheadinfo(ppahead);
	ret = 0;

ERR_LABLE(alloc_rbinfo_table):
ERR_LABLE(mismatch_version):
ERR_LABLE(get_errpt_head):
	ndd_free(databuf);
ERR_LABLE(alloc_databuf):
	nand_io_bch_close();

	return ret;
}

int get_errpt_ppainfo(nand_data *nddata, struct nand_api_platdependent *platdep)
{
	int ret = ENAND;
	unsigned char *databuf;
	unsigned int *badblockbuf;
	int startpageid, pageid;
	int blockid, chip_index, pt_index;
	plat_ptinfo *ptinfo = platdep->platptinfo;
	nand_flash *ndflash = platdep->nandflash;
	int errpt_startblk = ndflash->totalblocks / ndflash->chips -1;
	int errpt_endblk = nddata->cinfo->maxvalidblocks - 1;

	nand_io_bch_open(nddata->base, nddata->cinfo, nddata->eccsize, ndflash->buswidth);

	databuf = ndd_alloc(nddata->cinfo->pagesize);
	if (!databuf)
		GOTO_ERR(alloc_databuf);

	/* alloc plat pt table memory and badblockbuf of plat pt table
	 * NOTE: here may caused memory not be freed when errors
	 * at fllowed functions, but we do't care here */
	ptinfo->pt_table = ndd_alloc(ptinfo->ptcount * sizeof(plat_ptitem));
	if (!ptinfo->pt_table)
		GOTO_ERR(init_platptinfo);
	ret = init_badblockbuf(nddata, ptinfo);
	if(ret < 0)
		GOTO_ERR(init_badblockbuf);

        /*init pt_badblock_info*/
	for (chip_index = 0; chip_index < nddata->csinfo->totalchips; chip_index++) {
		for (blockid = errpt_startblk; blockid > errpt_endblk; blockid--) {
			if (is_bad_block(nddata, ndflash, chip_index, blockid, 1, RB_READY))
				continue;
			startpageid = blockid * nddata->cinfo->ppblock + PT_HEADINFO_CNT;
			for (pt_index = 0; pt_index < ptinfo->ptcount; pt_index++) {
				badblockbuf = (ptinfo->pt_table + pt_index)->pt_badblock_info;
				pageid = startpageid + pt_index * PT_INFO_CNT;
				ret = read_partition_info(nddata, ndflash, chip_index, badblockbuf, pageid, databuf);
				if(ret < 0){
					ndd_print(NDD_WARNING,"%s %d read_partition_info pagid[%d]failed! try again!\n",
						  __func__,__LINE__,pageid);
					//ndd_memset(ptinfo, 0xff, sizeof(plat_ptinfo));
					break;
				}
				fill_partition_info(ptinfo, databuf, pt_index);
			}
			if(ret >= 0 && pt_index == ptinfo->ptcount)
				goto exit;
		}
	}
	dump_platptinfo(ptinfo);

	ret = 0;

ERR_LABLE(init_badblockbuf):
	ndd_free(ptinfo->pt_table);
ERR_LABLE(init_platptinfo):
exit:
	ndd_free(databuf);
ERR_LABLE(alloc_databuf):
	nand_io_bch_close();
	return ret;
}

int nand_write_errpt(nand_data *nddata, plat_ptinfo *ptinfo, nand_flash *ndflash, int erasemode)
{
	cs_info *csinfo = nddata->csinfo;
	errpt_l2pblock l2pblock;
	unsigned int *badblockbuf;
	ppa_head phead;
	ppa_info *ppainfo;
	unsigned char *wbuf;
	unsigned int pageid, raw_boundary;
	unsigned int chip_index,blk_index,pt_index;
	int ept_startblk;
	int ept_endblk;
	int totalrbs = nddata->rbinfo->totalrbs;
	int chipprb = nddata->csinfo->totalchips / totalrbs;
	int ret = -1;

	nand_io_bch_open(nddata->base, nddata->cinfo, nddata->eccsize, ndflash->buswidth);

	switch(erasemode){
		case NAND_NORMAL_ERASE_MODE:
			ret = nand_normal_eraseall(nddata, ndflash);
			break;
		case NAND_FORCE_ERASE_MODE:
			ret = nand_force_eraseall(nddata, ndflash,ptinfo);
			break;
		case NAND_FACTORY_ERASE_MODE:
			ret = nand_factory_eraseall(nddata, ndflash);
			break;
		default:
			ndd_print(NDD_ERROR,"%s %d erasemode is wrong!\n",__func__,__LINE__);
			ret = -1;
			break;
	}
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d nand eraseall failed!\n",__func__,__LINE__);
		goto err0;
	}

	ppainfo = ndd_alloc(sizeof(ppa_info) * ptinfo->ptcount);
	if(!ppainfo){
		ndd_print(NDD_ERROR,"%s %d alloc ppainfo error!\n",__func__,__LINE__);
		goto err0;
	}
	wbuf = ndd_alloc(nddata->cinfo->pagesize);
	if(!wbuf){
		ndd_print(NDD_ERROR,"%s %d alloc wbuf error!\n",__func__,__LINE__);
		goto err1;
	}
	ret = creat_errpt_range(nddata,ndflash);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d creat_errpt_range failed!\n",__func__,__LINE__);
		goto err1;
	}
	raw_boundary = get_raw_boundary(ptinfo, nddata->cinfo, totalrbs);

	l2pblock.csinrb = ndd_alloc(totalrbs * chipprb);
	if(!(l2pblock.csinrb)){
		ndd_print(NDD_ERROR,"%s %d alloc cstorb error!\n",__func__,__LINE__);
		goto err2;
	}
	ndd_memset(l2pblock.csinrb, 0xff, totalrbs * chipprb);
	init_errpt_l2pblock(nddata, &l2pblock);

	adjust_plat_ptinfo(nddata, ptinfo, raw_boundary);

	ret = init_badblockbuf(nddata, ptinfo);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d init_badblockbuf error!\n",__func__,__LINE__);
		ret = -1;
		GOTO_ERR(initbuf);
	}
	ndd_memset(&phead, 0xff, sizeof(ppa_head));
	fill_errpt_info(nddata, ptinfo, &phead, ppainfo);
	dump_pheadinfo(&phead);
	dump_ppainfo(ppainfo, ptinfo->ptcount);
	/* scan badblock in every ppt */
	for(pt_index = 0; pt_index < ptinfo->ptcount; pt_index++){
		scan_pt_badblocks(nddata, ndflash, &ptinfo->pt_table[pt_index], &l2pblock, raw_boundary);
	}
	ept_startblk = ndflash->totalblocks / ndflash->chips - 1;
	ept_endblk = ndflash->maxvalidblocks / ndflash->chips - 1;
//	ndd_debug("\n----- %s[%d] startblk = %d endblk = %d totalchips = %d -----\n",__func__, __LINE__,
//				ept_startblk, ept_endblk, csinfo->totalchips);
	for(chip_index = 0; chip_index < csinfo->totalchips; chip_index++){
		for(pt_index = 0; pt_index < ptinfo->ptcount; pt_index++){
			for(blk_index = ept_startblk; blk_index > ept_endblk; blk_index--){
				if(is_bad_block(nddata, ndflash, chip_index, blk_index, 1, RB_READY) < 0)
					continue;
				ret = 0;
				if(pt_index == 0){
					/*write errpt head*/
					ndd_memset(wbuf, 0xff, nddata->cinfo->pagesize);
					ndd_memcpy(wbuf, &phead, sizeof(ppa_head));
					ndd_memcpy(wbuf + NANDFLASH_OFFSET_IN_ERRPTHEAD, ndflash, sizeof(nand_flash));
					nand_encode_errpt_headpage(nddata, ndflash, wbuf);
					pageid = blk_index * nddata->cinfo->ppblock;
					ret = nand_write_pagedata(nddata, ndflash, chip_index, pageid, wbuf);
				}
				if (ret == 0){
					pageid = blk_index * nddata->cinfo->ppblock + PT_HEADINFO_CNT + pt_index * PT_INFO_CNT;
					ndd_memset(wbuf, 0xff, nddata->cinfo->pagesize);
					ndd_memcpy(wbuf, ppainfo + pt_index, sizeof(ppa_info));
					badblockbuf = (ptinfo->pt_table + pt_index)->pt_badblock_info;
					ret = write_pt_badblock_info(nddata, ndflash, chip_index, badblockbuf, pageid, wbuf);
				}
				if (ret < 0){
					nand_erase_block(nddata, ndflash, chip_index, blk_index);
					if(mark_bad_block(nddata, ndflash, chip_index, blk_index) < 0){
						if (is_bad_block(nddata, ndflash, chip_index, blk_index, 1, RB_READY) != 0){
							ndd_debug("%s erase fail and is bad block, skip mark[%d]\n",
									__func__, blk_index);
							ret = 0;
							continue;
						} else {
							ndd_print(NDD_ERROR,"%s %d mark_bad_block[%d] error!\n",
									__func__,__LINE__, blk_index);
							goto err3;
						}
					}
				}
			}
		}
	}
err3:
ERR_LABLE(initbuf):
	ndd_free(l2pblock.csinrb);
err2:
	ndd_free(wbuf);
err1:
	ndd_free(ppainfo);
err0:
	nand_io_bch_close();
	return ret;
}

int nand_update_errpt(nand_data *nddata, nand_flash *ndflash, PPartition *pt)
{
	unsigned char *databuf;
	int ptcount = nddata->ptinfo->ptcount;
	int ret = 0;

	nand_io_bch_open(nddata->base, nddata->cinfo, nddata->eccsize, ndflash->buswidth);
	databuf = ndd_alloc(nddata->cinfo->pagesize * (ptcount * PT_INFO_CNT + PT_HEADINFO_CNT + 1));
	if(!databuf){
		ndd_print(NDD_ERROR,"%s %d alloc databuf error!\n",__func__,__LINE__);
		ret = -1;
		goto err0;
	}
	ndd_memset(databuf, 0xff, nddata->cinfo->pagesize * (ptcount * PT_INFO_CNT + PT_HEADINFO_CNT + 1));
	ret = prepare_pt_info(nddata, ndflash, pt, databuf);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d error!\n",__func__,__LINE__);
		ret = -1;
		goto err1;
	}
	dump_pheadinfo((ppa_head *)(databuf));
	ret = epterase_writebadblockinfo(nddata, ndflash, databuf);
	if(ret < 0){
		ndd_print(NDD_ERROR,"%s %d error!\n",__func__,__LINE__);
		ret = -1;
		goto err1;
	}
err1:
	ndd_free(databuf);
err0:
	nand_io_bch_close();
	return ret;
}

int nand_errpt_init(nand_flash *ndflash)
{
	if (!ndflash)
		RETURN_ERR(ENAND, "arguments error, ndflash = NULL");

	nd_errpt_info.bchbuf = ndd_alloc(ndflash->oobsize);
	if (!nd_errpt_info.bchbuf)
		RETURN_ERR(ENAND, "alloc bchbuf error");

	return 0;
}

void nand_errpt_deinit(void)
{
	ndd_free(nd_errpt_info.bchbuf);
}
