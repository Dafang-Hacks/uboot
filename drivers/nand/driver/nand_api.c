/**
 * driver/nand_api.c
 *
 * Ingenic Nand Driver
 *
 **/
#include "nandinterface.h"
#include "vnandinfo.h"
#include "singlelist.h"
#include "blocklist.h"
#include "ppartition.h"
#include "pagelist.h"

#include <os_clib.h>
#include <speed_dug.h>
#include "nand_debug.h"
#include "nddata.h"
#include "nand_api.h"
#include "nand_io.h"
#include "nand_bch.h"
#include "nand_ops.h"
#include "errptinfo.h"

//#define COMPAT_OLD_USB_BURN
#define SECTOR_SIZE 512
#define L4INFOLEN_2K    1136
#define L4INFOLEN_512   372

#define L4INFOLEN(pagesize)     ((pagesize) >= 2048 ? L4INFOLEN_2K : L4INFOLEN_512)
#define VNANDCACHESIZE(pagesize)    (L4INFOLEN(pagesize) / sizeof(unsigned int) * SECTOR_SIZE)
#define REFZONESIZE(pagesize,blocksize)		(((VNANDCACHESIZE(pagesize) + 1) * 4 + (blocksize) -1) / (blocksize) * (blocksize))

/* global */
nand_data *nddata = NULL;
extern int nd_raw_boundary;
extern int g_have_wp;

void (*__clear_rb_state)(rb_item *);
int (*__wait_rb_timeout) (rb_item *, int);
int (*__try_wait_rb) (rb_item *, int);
void (*__wp_enable) (int);
void (*__wp_disable) (int);
int (*__gpio_irq_request)(unsigned short, unsigned short *, void **);
int (*__ndd_gpio_request)(unsigned int, const char *);
int (*nand_auto_adapt_edo)(int , chip_info *, rb_item *);
int (*nand_adjust_to_toggle)(int , chip_info *);
int (*nand_adjust_to_onfi)(int, chip_info *);

extern int os_clib_init(os_clib *clib);

/*========================= NandInterface =========================*/
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
static  unsigned int get_ref_zone_size(chip_info *cinfo, unsigned int totalrbs, unsigned int planenum)
{
	int align_size;
	int zone_size;
	align_size = totalrbs * planenum;
//	ndd_print(NDD_ERROR,"pagesize = %d,blocksize =%d,alsize = %d\n",cinfo->pagesize,cinfo->pagesize * cinfo->ppblock,align_size);
//	ndd_print(NDD_ERROR,"REF-SIZE = %d\n",REFZONESIZE(512,16384));
	zone_size = (REFZONESIZE(cinfo->pagesize,cinfo->pagesize * cinfo->ppblock) + align_size - 1) / align_size;
	return zone_size;

}
static inline int is_blockid_virtual(ndpartition *npt, int blockid)
{
	int totalrbs = nddata->rbinfo->totalrbs;
	int chipprb = nddata->csinfo->totalchips / nddata->rbinfo->totalrbs;
	int column = blockid % totalrbs;
	int row = (npt->startblockid + blockid * npt->blockpvblock) / totalrbs;
	int raw_boundary = nddata->ptinfo->raw_boundary;
	int rbblocks = chipprb * nddata->cinfo->maxvalidblocks;

	return ((column > 0) && (row < raw_boundary)) || ((column == 0) && (row >= rbblocks));
}

static int single_page_read(void *ppartition, int pageid, int offsetbyte, int bytecount, void *data)
{
	int ret;
	PageList ppl_node;
	PPartition *ppt = (PPartition *)ppartition;

	ppl_node.head.next = NULL;
	ppl_node.startPageID = pageid;
	ppl_node.OffsetBytes = offsetbyte;
	ppl_node.Bytes = bytecount;
	ppl_node.pData = data;

	speed_dug_begin(NDD_READ, &ppl_node);
	ret = nandops_read(nddata->ops_context, (ndpartition *)ppt->prData, &ppl_node);
	speed_dug_end(NDD_READ);

	if (ret == ECC_ERROR)
		ndd_dump_uncorrect_err(nddata, ppt, &ppl_node);

	return ret;
}

static int single_page_write(void *ppartition, int pageid, int offsetbyte, int bytecount, void *data)
{
	int ret;
	PageList ppl_node;
	PPartition *ppt = (PPartition *)ppartition;

	ppl_node.head.next = NULL;
	ppl_node.startPageID = pageid;
	ppl_node.OffsetBytes = offsetbyte;
	ppl_node.Bytes = bytecount;
	ppl_node.pData = data;

	__wp_disable(nddata->gpio_wp);
	ndd_dump_rewrite(nddata, ppt, &ppl_node);
	speed_dug_begin(NDD_WRITE, &ppl_node);
	ret = nandops_write(nddata->ops_context, (ndpartition *)ppt->prData, &ppl_node);
	speed_dug_end(NDD_WRITE);
	__wp_enable(nddata->gpio_wp);

	return ret;
}

static int multi_page_read(void *ppartition, PageList *pl)
{
	int ret;
	PPartition *ppt = (PPartition *)ppartition;
	ndpartition *npt = (ndpartition *)ppt->prData;

	speed_dug_begin(NDD_READ, pl);
	ret = nandops_read(nddata->ops_context, npt, pl);
	speed_dug_end(NDD_READ);

	if (ret == ECC_ERROR)
		ndd_dump_uncorrect_err(nddata, ppt, pl);

	return ret;
}

static int multi_page_write(void *ppartition, PageList *pl)
{
	int ret;
	PPartition *ppt = (PPartition *)ppartition;
	ndpartition *npt = (ndpartition *)ppt->prData;

        ndd_dump_write_reread_prepare(pl);

	__wp_disable(nddata->gpio_wp);
	ndd_dump_rewrite(nddata, ppt, pl);
	speed_dug_begin(NDD_WRITE, pl);
	ret = nandops_write(nddata->ops_context, npt, pl);
	speed_dug_end(NDD_WRITE);
	__wp_enable(nddata->gpio_wp);

	if (ret == SUCCESS)
		ndd_dump_write_reread_complete(nddata, ppt, pl);

	return ret;
}

static int multi_block_erase(void* ppartition, BlockList *bl)
{
	int ret = 0, cur = 0;
	unsigned int bitmap = 0xffffffff;
	struct singlelist list_head;
	struct singlelist *pos, *top = NULL, *tmp_node = NULL, *virtual_list = NULL, *normal_list = NULL;
	BlockList *bl_node;
	PPartition *ppt = (PPartition *)ppartition;
	ndpartition *npt = (ndpartition *)ppt->prData;

	/* singlelist head can not be deleted, 'list_head' is used to fix this issue */
	list_head.next = &bl->head;
	singlelist_for_each(pos, &bl->head) {
		if (tmp_node) {
			tmp_node->next = NULL;
			if (!virtual_list)
				virtual_list = tmp_node;
			else
				singlelist_add_tail(virtual_list, tmp_node);
			bitmap &= ~(1 << (cur - 1));
			tmp_node = NULL;
		}
		bl_node = singlelist_entry(pos, BlockList, head);
		if (is_blockid_virtual(npt, bl_node->startBlock)) {
			ndd_debug("%s, pt[%s], block [%d] is virtual\n", __func__, npt->name, bl_node->startBlock);
			bl_node->retVal = 0; /* we return ok, but do nothing */
			singlelist_del(&list_head, pos);
			tmp_node = pos;
		}
		cur++;
	}

	if (list_head.next) {
		__wp_disable(nddata->gpio_wp);
		ret = nandops_erase(nddata->ops_context, npt, singlelist_container_of(list_head.next, BlockList, head));
		__wp_enable(nddata->gpio_wp);
		if (!ret)
			ndd_dump_erase(nddata, npt, bl);
	}

	if (virtual_list) {
		normal_list = &bl->head;
		while (normal_list || virtual_list) {
			for (cur = 0; cur < sizeof(unsigned int) * 8; cur++) {
				if ((bitmap & (1 << cur)) == 0) {
					if (!virtual_list)
						RETURN_ERR(ENAND, "virtual list not map bitmap\n");
					tmp_node = virtual_list;
					singlelist_del(virtual_list, tmp_node);
				} else {
					if (!normal_list)
						RETURN_ERR(ENAND,"normal list not map bitmap\n");
					tmp_node = normal_list;
					singlelist_del(normal_list, tmp_node);
				}
				tmp_node->next = NULL;
				if (!top)
					top = tmp_node;
				else
					singlelist_add(top, tmp_node);
			}
		}

		bl = singlelist_entry(top, BlockList, head);
	}

	return ret;
}

static int is_badblock(void *ppartition, int blockid)
{
	int ret;
	PPartition *ppt = (PPartition *)ppartition;
	ndpartition *npt = (ndpartition *)ppt->prData;

	/* if block is virtual, it is bad block */
	if (is_blockid_virtual(npt, blockid)) {
		ndd_debug("%s, pt[%s], block [%d] is virtual\n", __func__, npt->name, blockid);
		return ENAND;
	}

	ret = nandops_isbadblk(nddata->ops_context, npt, blockid);

#ifdef CHECK_USED_BLOCK
	if ((ret == 0) && (blockid >= 0))
		ret = ndd_dump_check_used_block(nddata, ppt, blockid);
#endif

	return ret;
}

static int mark_badblock(void *ppartition, int blockid)
{
	int ret;
	PPartition *ppt = (PPartition *)ppartition;
	ndpartition *npt = (ndpartition *)ppt->prData;

	/* if block is virtual, mark it will do nothing */
	if (is_blockid_virtual(npt, blockid)) {
		ndd_debug("%s, pt[%s], block [%d] is virtual\n", __func__, npt->name, blockid);
		return 0;
	}

	__wp_disable(nddata->gpio_wp);
	ret = nandops_markbadblk(nddata->ops_context, npt, blockid);
	__wp_enable(nddata->gpio_wp);

	return ret;
}

static inline int get_ppt_startblockID(ndpartition *npt);
static inline int get_ppt_totalblocks(ndpartition *npt);
static inline void fill_ptavailable_badblock(PPartition *ppt);
static int ioctl(enum ndd_cmd cmd, int args)
{
	int ret = 0;

	switch (cmd) {
	case NDD_UPDATE_ERRPT: {
		PPartition *pt = (PPartition *)args;
		nand_errpt_init((nand_flash *)(nddata->cinfo->flash));
		ret = nand_update_errpt(nddata,(nand_flash *)(nddata->cinfo->flash), pt);
		nand_errpt_deinit();
		if(ret < 0){
			ndd_print(NDD_ERROR,"%s %d nand_update_errpt error!\n",__func__,__LINE__);
		}
		if(ret >= 0)
			fill_ptavailable_badblock(pt);
		break;
	}
	default:
		ndd_print(NDD_WARNING, "WARNING, unknown ioctl command!\n");
	}

	return ret;
}

static inline const char* get_ppt_name(ndpartition *npt)
{
	return npt->name;
}
static inline int get_ppt_byteperpage(ndpartition *npt)
{
	return npt->pagesize;
}
static inline int get_ppt_pageperblock(ndpartition *npt)
{
	return npt->blockpvblock * npt->pagepblock;
}
static inline int get_ppt_startblockID(ndpartition *npt)
{
	return npt->startblockid;
}
static inline int get_ppt_totalblocks(ndpartition *npt)
{
	return npt->totalblocks / npt->blockpvblock;
}
static inline int get_ppt_PageCount(ndpartition *npt)
{
	return npt->totalblocks * npt->pagepblock;
}
static inline int get_ppt_startPage(ndpartition *npt)
{
	return npt->startblockid * npt->pagepblock;
}
static inline int get_ppt_mode(ndpartition *npt)
{
	return npt->nm_mode;
}
static inline void* get_ppt_prData(ndpartition *npt)
{
	return (void *)npt;
}
static inline int get_ppt_pagespergroup(ndpartition *npt)
{
	return npt->pagepblock * npt->blockpvblock * npt->vblockpgroup;
}
static inline int get_ppt_groupperzone(ndpartition *npt)
{
	return npt->groupspzone;
}
static inline int get_ppt_flags(ndpartition *npt)
{
	return npt->flags;
}

static inline void fill_ptavailable_badblock(PPartition *ppt)
{
	int i, blockid;
	int endflag = 0;
	int totalblocks = ppt->totalblocks;
	int unit_size = ppt->groupperzone;
	unsigned int *badTable = ppt->pt_badblock_info;
	unsigned int *alailableTable = ppt->pt_availableblockid;
	int block_in_badtable = 0;
	int unalign_unit_start = 0, unalign_unit_end = 0;
	int bad_index = 0, bad_count = 0, totalbads = 0;
	int align_index = 0, unalign_index = 0;
	int bad_blockid = -1;

	/* clear alailableTable */
	ndd_memset(alailableTable, 0xff, totalblocks * sizeof(int));

	/* get total badblock count */
	while (badTable[bad_index++] != -1)
		totalbads++;

	/* fill ppt->badblockcount */
	ppt->badblockcount = totalbads;

	bad_index = 0;
	unalign_index = totalblocks - totalbads - 1;
	do {
		bad_blockid = badTable[bad_index];

		if (bad_blockid == -1) {
			/* for the last time */
			bad_blockid = totalblocks;
			endflag = 1;
		}

		unalign_unit_start = bad_blockid / unit_size * unit_size;
		unalign_unit_end = (totalblocks - unalign_unit_start) >= unit_size ?
			(unalign_unit_start + unit_size - 1) : (totalblocks - unalign_unit_start - 1);

		/* fill align blocks */
		for (; (align_index + bad_count) < unalign_unit_start; align_index++)
			alailableTable[align_index] = align_index + bad_count;

		/* fill unalign blocks */
		for (blockid = unalign_unit_start; blockid <= unalign_unit_end; blockid++) {
			block_in_badtable = 0;
			/* check if block in badtable */
			for (i = bad_index; badTable[i] <= unalign_unit_end; i++) {
				if (badTable[i] == blockid) {
					block_in_badtable = 1;
					bad_count++;
					bad_index++;
					break;
				}
			}
			if (!block_in_badtable) {
				alailableTable[unalign_index--] = blockid;
				/* regard unaligned block as badblock in algorithm */
				bad_count++;
			}
		}
		//ppt->unalign_block_start = unalign_index + 1;

		/* get next bad blockid */
		if (endflag) break;
	} while (1);
}

static inline int fill_ppartition(PPartition *ppt)
{
	int pt_index,i;
	unsigned int ptcount = nddata->ptinfo->ptcount;
	ndpartition *npt = nddata->ptinfo->pt;

	for (pt_index = 0; pt_index < ptcount; pt_index++) {
		/* fill PPartition */
		ppt[pt_index].name = get_ppt_name(npt + pt_index);
		ndd_debug("%s[%d] ppt->name = %s ndparts_num = %d\n",__func__,__LINE__,
					ppt[pt_index].name, npt[pt_index].ndparts_num);
		ppt[pt_index].byteperpage = get_ppt_byteperpage(npt + pt_index);
		ppt[pt_index].pageperblock = get_ppt_pageperblock(npt + pt_index);
		ppt[pt_index].startblockID = get_ppt_startblockID(npt + pt_index);
		ppt[pt_index].totalblocks = get_ppt_totalblocks(npt + pt_index);
		ppt[pt_index].PageCount = get_ppt_PageCount(npt + pt_index);
		ppt[pt_index].badblockcount = 0;
		ppt[pt_index].hwsector = HW_SECTOR;
		ppt[pt_index].startPage = get_ppt_startPage(npt + pt_index);
		ppt[pt_index].mode = get_ppt_mode(npt + pt_index);
		ppt[pt_index].prData = get_ppt_prData(npt + pt_index);
		ppt[pt_index].v2pp = NULL;
		ppt[pt_index].pagespergroup = get_ppt_pagespergroup(npt + pt_index);
		ppt[pt_index].groupperzone = get_ppt_groupperzone(npt + pt_index);
		ppt[pt_index].flags = get_ppt_flags(npt + pt_index);
		ppt[pt_index].pt_badblock_info = (npt + pt_index)->pt_badblock_info;
		ppt[pt_index].parts_num = npt[pt_index].ndparts_num;
		if(pt_index == ptcount -1){
			/* ndvirtual */
			ppt[pt_index].pt_availableblockid = NULL;
			break;
		}
		ppt[pt_index].pt_availableblockid = (unsigned int *)ndd_alloc(ppt[pt_index].totalblocks * sizeof(unsigned int));
		if(!(ppt[pt_index].pt_availableblockid)){
			ndd_print(NDD_ERROR,"alloc pt_availableblockid memory error!\n");
			return -1;
		}
		fill_ptavailable_badblock(&ppt[pt_index]);
		for (i = 0; i < npt[pt_index].ndparts_num; i++) {
			ndd_debug("%s[%d] pt->name = %s totalblocks = %d,blockpvblock = %d\n",__func__,__LINE__,
				npt[pt_index].ndparts[i].name, npt[pt_index].ndparts[i].totalblocks, npt[pt_index].blockpvblock);
			ppt[pt_index].parts[i].name = npt[pt_index].ndparts[i].name;
			ppt[pt_index].parts[i].startblockID = npt[pt_index].ndparts[i].startblockID;
			ppt[pt_index].parts[i].totalblocks = npt[pt_index].ndparts[i].totalblocks /
				npt[pt_index].blockpvblock;
		}
	}
	return 0;
}

static int init_nand(void *vNand)
{
	int ret;
	PPartArray *ppta;
	PPartition *ppt;
	int ptcount = nddata->ptinfo->ptcount;
	int vpt_index = ptcount - 1;
	VNandManager *vnm =(VNandManager *)vNand;

	ppt = ndd_alloc(sizeof(PPartition) * ptcount);
	if (!ppt)
		GOTO_ERR(alloc_ppt);

	ppta = ndd_alloc(sizeof(PPartArray));
	if (!ppta)
		GOTO_ERR(alloc_ppta);

	/* get ppartitions */
	ret = fill_ppartition(ppt);
	if (ret)
		GOTO_ERR(fill_ppt);

	/* PPartArray */
	ppta->ptcount = ptcount - 1;
	ppta->ppt = ppt;

	/* VNandManager -> VNandInfo  ppta->ppt[nddata->ptcount] is 'ndvirtual' */
	vnm->info.startBlockID = ppta->ppt[vpt_index].startblockID;
	vnm->info.PagePerBlock = ppta->ppt[vpt_index].pageperblock;
	vnm->info.BytePerPage = ppta->ppt[vpt_index].byteperpage;
	vnm->info.TotalBlocks = ppta->ppt[vpt_index].totalblocks;
	vnm->info.MaxBadBlockCount = ppta->ppt[vpt_index].badblockcount;
	vnm->info.hwSector = ppta->ppt[vpt_index].hwsector;
	vnm->info.prData = (void *)&ppta->ppt[vpt_index];

	/* VNandManager -> PPartArray */
	vnm->pt = ppta;

	/* dump */
	ndd_dump_ppartition(ppta->ppt, ptcount);

	return 0;

ERR_LABLE(fill_ppt):
	ndd_free(ppta);
ERR_LABLE(alloc_ppta):
	ndd_free(ppt);
ERR_LABLE(alloc_ppt):
	return -1;
}

static int deinit_nand(void *vNand)
{
	VNandManager *vnm =(VNandManager *)vNand;

	if (vnm->pt) {
		if (vnm->pt->ppt)
			ndd_free(vnm->pt->ppt);
		ndd_free(vnm->pt);
	}

	return 0;
}

NandInterface nand_interface = {
	.iPageRead = single_page_read,
	.iPageWrite = single_page_write,
	.iMultiPageRead = multi_page_read,
	.iMultiPageWrite = multi_page_write,
	.iMultiBlockErase = multi_block_erase,
	.iIsBadBlock = is_badblock,
	.iMarkBadBlock = mark_badblock,
	.iIoctl = ioctl,
	.iInitNand = init_nand,
	.iDeInitNand = deinit_nand,
};

/*========================= NandDriver =========================*/

unsigned int get_nandflash_maxvalidblocks(void)
{
	if(!nddata && !(nddata->cinfo) && !(nddata->cinfo->flash))
		return 0;
	return ((nand_flash *)(nddata->cinfo->flash))->maxvalidblocks;
}

unsigned int get_nandflash_pagesize(void)
{
	if(!nddata && !(nddata->cinfo) && !(nddata->cinfo->flash))
		return 0;
	return ((nand_flash *)(nddata->cinfo->flash))->pagesize;
}

static inline int next_platpt_connected(unsigned int plat_index,
					plat_ptitem *plat_pt, unsigned int plat_ptcount)
{
	return (((plat_index + 1) < plat_ptcount) &&
		((plat_pt[plat_index].offset + plat_pt[plat_index].size) == plat_pt[plat_index + 1].offset));
}

static inline int get_group_vblocks(chip_info *cinfo, unsigned short blockpvlblock, unsigned int groupspzone,unsigned int ref_zone_size)
{
	unsigned int vblocksize = (cinfo->pagesize * cinfo->ppblock) * blockpvlblock;

#ifdef COMPAT_OLD_USB_BURN
	return 4;
#endif
	/* group vblocks may be at 2 ~ 16 */
	if ((ref_zone_size / groupspzone / vblocksize) < 2)
		return 2;
	else if ((ref_zone_size / groupspzone / vblocksize) > 16)
		return 16;
	else
		return ref_zone_size / groupspzone / vblocksize;
}

static inline unsigned int get_raw_boundary(plat_ptinfo *platptinfo, chip_info *cinfo, unsigned short totalrbs)
{
	unsigned int raw_boundary = 0;
	unsigned char nm_mode, last_nm_mode = -1;
	unsigned short pt_index = platptinfo->ptcount;
	plat_ptitem *plat_pt = platptinfo->pt_table;

	while (pt_index--) {
		nm_mode = plat_pt[pt_index].nm_mode;
		if ((nm_mode != ZONE_MANAGER) && (last_nm_mode == ZONE_MANAGER)) {
			unsigned int blocksize = cinfo->pagesize * cinfo->ppblock;
			unsigned short alignunit = cinfo->planenum * totalrbs;
			unsigned int blockid = ndd_div_s64_32((plat_pt[pt_index + 1].offset + blocksize - 1), blocksize);
			raw_boundary = ((blockid + (alignunit - 1)) / alignunit) * alignunit;
			break;
		}
		last_nm_mode = nm_mode;
	}
	nd_raw_boundary = raw_boundary;
	return raw_boundary;
}

static inline int get_startblockid(plat_ptitem *plat_pt, chip_info *cinfo,
				   unsigned short totalrbs, unsigned int raw_boundary)
{
	unsigned int blocksize = cinfo->pagesize * cinfo->ppblock;
	unsigned int blockid = ndd_div_s64_32((plat_pt->offset + blocksize - 1), blocksize);

#if 0
	if (plat_pt->nm_mode == ZONE_MANAGER) {
		unsigned short alignunit = cinfo->planenum * totalrbs;
		return (((blockid + (alignunit - 1)) / alignunit) * alignunit)
			+ (totalrbs > 1 ? (raw_boundary * (totalrbs - 1)) : 0);
	} else
		return blockid * totalrbs;
#else
	return blockid;
#endif
}

static inline int get_endblockid(plat_ptitem *plat_pt, chip_info *cinfo,
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
		blockid += (totalrbs > 1) ? (raw_boundary * totalrbs) : 0;
	} else {
		offset = plat_pt->offset + plat_pt->size;
		blockid = ndd_div_s64_32(offset, blocksize);
	}
#if 0
	if (plat_pt->nm_mode == ZONE_MANAGER) {
		unsigned short alignunit = cinfo->planenum * totalrbs;
		return (blockid / alignunit * alignunit) - 1;
	} else
		return blockid * totalrbs - 1;
#else
	return blockid - 1;
#endif
}

static inline int fill_ex_partition(ndpartition *pt, plat_ex_partition *plat_expta,
				    chip_info *cinfo, unsigned short totalrbs)
{
	int i = 0;
	int blockid, statblockID = 0, endblockID;
	unsigned int blocksize = cinfo->pagesize * cinfo->ppblock;

	while (plat_expta[i].size != 0) {
		pt->ndparts[i].startblockID = statblockID;
		if (plat_expta[i].size == -1)
			endblockID = pt->totalblocks - 1;
		else {
			blockid = ndd_div_s64_32(plat_expta[i].offset + plat_expta[i].size, blocksize);
			if (pt->nm_mode == ZONE_MANAGER) {
				unsigned short alignunit = cinfo->planenum * totalrbs;
				endblockID = (blockid / alignunit * alignunit) - 1;
			} else
				endblockID = blockid - 1;
		}
		pt->ndparts[i].totalblocks = endblockID - statblockID + 1;
		pt->ndparts[i].name = plat_expta[i].name;
		statblockID += pt->ndparts[i].totalblocks;
		i++;
	}
	pt->ndparts_num = i;

	return 0;
}

static pt_info* get_ptinfo(chip_info *cinfo, unsigned short totalchips,
			   unsigned short totalrbs, plat_ptinfo *platptinfo)
{
	ndpartition *pt = NULL;
	pt_info *ptinfo = NULL;
	unsigned int endblockid;
	unsigned int ref_zone_size;
	unsigned short plat_ptcount = platptinfo->ptcount;
	unsigned short ptcount = plat_ptcount + REDUN_PT_NUM;
	plat_ptitem *plat_pt = platptinfo->pt_table;
	unsigned short pt_index = plat_ptcount;
	unsigned short redunpt_start = plat_ptcount;

	ndd_dump_plat_partition(platptinfo);

	ptinfo = ndd_alloc(sizeof(pt_info) + sizeof(ndpartition) * ptcount);
	if (!ptinfo)
		RETURN_ERR(NULL, "alloc memory for ptinfo error");
	else {
		ndd_memset(ptinfo, 0, sizeof(pt_info) + sizeof(ndpartition) * ptcount);
		ptinfo->ptcount = ptcount;
		ptinfo->pt = (ndpartition *)((unsigned char *)ptinfo + sizeof(pt_info));
	}

//	ptinfo->raw_boundary = get_raw_boundary(platptinfo, cinfo, totalrbs);
	ptinfo->raw_boundary = nd_raw_boundary;
	pt = ptinfo->pt;

	while (pt_index--) {
		/* init ndpartition */
		pt[pt_index].name = plat_pt[pt_index].name;
		pt[pt_index].pagesize = cinfo->pagesize;
		pt[pt_index].pagepblock = cinfo->ppblock;
		pt[pt_index].startblockid = get_startblockid(&plat_pt[pt_index], cinfo,
							     totalrbs, ptinfo->raw_boundary);
		if (next_platpt_connected(pt_index, plat_pt, plat_ptcount))
			endblockid = pt[pt_index + 1].startblockid - 1;
		else
			endblockid = get_endblockid(&plat_pt[pt_index], cinfo, totalchips,
						    totalrbs, ptinfo->raw_boundary);
		pt[pt_index].totalblocks = endblockid - pt[pt_index].startblockid + 1;

		ndd_debug("%s[%d] pt->name = %s\n", __func__,__LINE__,pt[pt_index].name);
		if (plat_pt[pt_index].nm_mode == ZONE_MANAGER) {
			pt[pt_index].groupspzone = totalrbs;
#ifdef COMPAT_OLD_USB_BURN
			if (!ndd_strcmp("ndsystem", pt[pt_index].name))
				pt[pt_index].blockpvblock = 1;
			else
				pt[pt_index].blockpvblock = cinfo->planenum;
#else
			pt[pt_index].blockpvblock = cinfo->planenum;
#endif
		//	ndd_print(NDD_ERROR,"ref_zone_size = %d\n",ref_zone_size);
			ref_zone_size = get_ref_zone_size(cinfo, totalrbs, cinfo->planenum);
		//	ndd_print(NDD_ERROR,"ref_zone_size = %d\n",ref_zone_size);
			pt[pt_index].vblockpgroup = get_group_vblocks(cinfo, cinfo->planenum, totalrbs,ref_zone_size);
#ifdef COMPAT_OLD_USB_BURN
			if (!ndd_strcmp("ndsystem", pt[pt_index].name))
				pt[pt_index].planes = 1;
			else
				pt[pt_index].planes = cinfo->planenum;
#else
			pt[pt_index].planes = cinfo->planenum;
#endif
			while (1) {
				/* check if pt has too few zones, we need to adjust blocks per zone */
				unsigned int zoneblocks = pt[pt_index].blockpvblock *
					pt[pt_index].vblockpgroup * pt[pt_index].groupspzone;
				unsigned int zonecount = pt[pt_index].totalblocks / zoneblocks;
				if (zonecount < ZONE_COUNT_LIMIT) {
					if (pt[pt_index].vblockpgroup > 1){
						pt[pt_index].vblockpgroup /= 2;
					}
					else if (pt[pt_index].blockpvblock > 1) {
						pt[pt_index].blockpvblock /= 2;
						pt[pt_index].planes /=2;
					} else {
						ndd_print(NDD_WARNING, "WARNING: pt[%s] has too few blocks"
							  " as ZONE_MANAGER partition, totalblocks = %d\n",
							  pt[pt_index].name, pt[pt_index].totalblocks);
						break;
					}
				} else
					break;
			}
		} else {
			pt[pt_index].groupspzone = totalrbs;
			pt[pt_index].blockpvblock = 1;
			pt[pt_index].vblockpgroup = 1;
			pt[pt_index].planes = 1;
		}
		pt[pt_index].eccbit = cinfo->eccbit;
		pt[pt_index].ops_mode = plat_pt[pt_index].ops_mode;
		pt[pt_index].nm_mode = plat_pt[pt_index].nm_mode;
		pt[pt_index].copy_mode = DEFAULT_COPY_MODE;
		pt[pt_index].flags = plat_pt[pt_index].flags;
		pt[pt_index].pt_badblock_info = plat_pt[pt_index].pt_badblock_info;
		fill_ex_partition(&pt[pt_index], plat_pt[pt_index].ex_partition, cinfo, totalrbs);
		pt[pt_index].handler = 0;
	}

	/* ndvirtual */
	pt[redunpt_start].name = "ndvirtual";
	pt[redunpt_start].pagesize = cinfo->pagesize;
	pt[redunpt_start].pagepblock = cinfo->ppblock;
	pt[redunpt_start].startblockid = 0;
	pt[redunpt_start].totalblocks = cinfo->maxvalidblocks * totalchips;
	if (totalrbs > 1)
		pt[redunpt_start].totalblocks += totalrbs * ptinfo->raw_boundary;
	pt[redunpt_start].blockpvblock = 1;
	pt[redunpt_start].groupspzone = totalrbs;
	pt[redunpt_start].vblockpgroup = get_group_vblocks(cinfo, cinfo->planenum, totalrbs,ref_zone_size);
	pt[redunpt_start].eccbit = cinfo->eccbit;
	pt[redunpt_start].planes = ONE_PLANE;
	pt[redunpt_start].ops_mode = DEFAULT_OPS_MODE;
	pt[redunpt_start].nm_mode = ZONE_MANAGER;
	pt[redunpt_start].copy_mode = DEFAULT_COPY_MODE;
	pt[redunpt_start].flags = 0;
	pt[redunpt_start].handler = 0;

	ndd_dump_ptinfo(ptinfo);

	return ptinfo;
}

static void free_ptinfo(pt_info *ptinfo)
{
	ndd_free(ptinfo);
}

static cs_info* get_csinfo(nfi_base *base, rb_info *rbinfo)
{
	int cs_index;
	cs_info *csinfo = NULL;
	rb_item *rbitem = NULL;

	csinfo = ndd_alloc(sizeof(cs_info) + (sizeof(cs_item) * CS_PER_NFI));
	if (!csinfo)
		RETURN_ERR(NULL, "alloc memory for cs info error");
	else {
		csinfo->totalchips = 0;
		csinfo->csinfo_table = (cs_item *)((unsigned char *)csinfo + sizeof(cs_info));
	}

	for (cs_index = 0; cs_index < CS_PER_NFI; cs_index++) {
		rbitem = get_rbitem(base, cs_index, rbinfo);
		if (rbitem) {
			ndd_debug("%s[%d] csinfo->totalchips = %d cs_index = %d [%d] [%d]\n", __func__,__LINE__,
						csinfo->totalchips, cs_index, (unsigned int)csinfo,
									(unsigned int)(csinfo->csinfo_table));
			csinfo->csinfo_table[csinfo->totalchips].id = cs_index;
			csinfo->csinfo_table[csinfo->totalchips].rbitem = rbitem;
			csinfo->csinfo_table[csinfo->totalchips].iomem = base->cs_iomem[cs_index];
			csinfo->totalchips++;
		}
	}

	if (csinfo->totalchips < rbinfo->totalrbs) {
		ndd_free(csinfo);
		RETURN_ERR(NULL, "scand totalchips [%d] is less than totalrbs [%d]",
			   csinfo->totalchips, rbinfo->totalrbs);
	}

	ndd_dump_csinfo(csinfo);

	return csinfo;
}

static void free_csinfo(cs_info **csinfo)
{
	ndd_free(*csinfo);
	*csinfo = NULL;
}

extern void nand_controll_adpt(chip_info *cinfo, const nand_flash *ndflash);//from nand_io.c
static int fill_cinfo(nand_data *nddata, const nand_flash *ndflash)
{
	nfi_base *base = &(nddata->base->nfi);
	chip_info *cinfo = nddata->cinfo;
	/* init nddata->cinfo */
	cinfo->manuf = ndflash->id >> 8;
	cinfo->pagesize = ndflash->pagesize;
	cinfo->oobsize = ndflash->oobsize;
	cinfo->ppblock = ndflash->blocksize / ndflash->pagesize;
	cinfo->maxvalidblocks = ndflash->maxvalidblocks / ndflash->chips;
	cinfo->maxvalidpages = cinfo->maxvalidblocks * cinfo->ppblock;
	cinfo->planepdie = ndflash->planepdie;
	cinfo->totaldies = ndflash->diepchip;
	cinfo->origbadblkpos = ndflash->badblockpos;
	cinfo->badblkpos = ndflash->oobsize - 4;
	cinfo->eccpos = 0;
	cinfo->buswidth = ndflash->buswidth;
	cinfo->rowcycles = ndflash->rowcycles;
	cinfo->eccbit = ndflash->eccbit;
	cinfo->planenum = ndflash->realplanenum > 2 ? 2 : ndflash->realplanenum;
	cinfo->planeoffset = ndflash->planeoffset;
	cinfo->options = ndflash->options;

	/* if support read retry, get retry parm table */
	if (SUPPROT_READ_RETRY(cinfo)) {
		cinfo->retryparms = ndd_alloc(sizeof(retry_parms));
		if (!cinfo->retryparms)
			RETURN_ERR(ENAND, "can not alloc memory for retryparms");

		cinfo->retryparms->mode = READ_RETRY_MODE(cinfo);
		if (get_retry_parms(base,cinfo,0, nddata->rbinfo, cinfo->retryparms)) {
			ndd_free(cinfo->retryparms);
			RETURN_ERR(ENAND, "get retry data error");
		}
	} else
		cinfo->retryparms = NULL;

	//cinfo->timing = &(ndflash->timing);
	nand_controll_adpt(cinfo,ndflash);
	cinfo->flash = (void *)ndflash;

	return 0;
}

static io_base* get_base(struct nand_api_osdependent *osdep)
{
	return osdep->base;
}

static int nandflash_setup(nfi_base *base, cs_info *csinfo, rb_info *rbinfo, chip_info *cinfo)
{
	int cs_index, cs_id, ret = 0;
	unsigned short totalchips = csinfo->totalchips;
	cs_item *csinfo_table = csinfo->csinfo_table;

	for (cs_index = 0; cs_index < totalchips; cs_index++) {
		cs_id = csinfo_table[cs_index].id;
		ret = nand_set_features(base, cs_id, csinfo_table[cs_index].rbitem, cinfo);
		if (ret)
			break;
	}

	return ret;
}
int clear_rb_state(int cs_index)
{
	cs_item *csitem = &(nddata->csinfo->csinfo_table[cs_index]);

	__clear_rb_state(csitem->rbitem);

	return 0;
}
int wait_rb_timeout(int cs_index, int timeout)
{
	int ret = SUCCESS;

	cs_item *csitem = &(nddata->csinfo->csinfo_table[cs_index]);

	ret = __wait_rb_timeout(csitem->rbitem, timeout);
	if (ret < 0)
		ret = TIMEOUT;
	else
		ret = SUCCESS;

	return ret;
}

static inline int init_platptitem_memory(plat_ptinfo *platptinfo)
{
	platptinfo->pt_table = (plat_ptitem *)ndd_alloc(sizeof(plat_ptitem) * platptinfo->ptcount);
	if(!platptinfo->pt_table){
		ndd_print(NDD_ERROR,"ERROR: %s %d alloc private plat_ptinfo error!\n",__func__,__LINE__);
		return -1;
	}
	return 0;
}
static inline void deinit_platptitem_memory(plat_ptinfo *platptinfo)
{
	ndd_free(platptinfo->pt_table);
	platptinfo->pt_table = NULL;
}

static int get_nand_sharing_params(nand_flash *ndflash)
{
	struct nand_basic_info *src_parms = &(share_parms.nandinfo);

	if (share_parms.magic != NAND_MAGIC)
		RETURN_ERR(ENAND, "get nand sharing params error, magic = [%x]", share_parms.magic);

	ndflash->id = src_parms->id;
	ndflash->extid = src_parms->extid;
	ndflash->pagesize = src_parms->pagesize;
	ndflash->oobsize = src_parms->oobsize;
	ndflash->blocksize = src_parms->blocksize;
	ndflash->totalblocks = src_parms->totalblocks;
	ndflash->maxvalidblocks = src_parms->maxvalidblocks;
	ndflash->eccbit = src_parms->eccbit;
	ndflash->planepdie = src_parms->planepdie;
	ndflash->diepchip = src_parms->diepchip;
	ndflash->chips = src_parms->chips;
	ndflash->buswidth = src_parms->buswidth;
	ndflash->realplanenum = src_parms->realplanenum;
	ndflash->badblockpos = src_parms->badblockpos;
	ndflash->rowcycles = src_parms->rowcycles;
	ndflash->planeoffset = src_parms->planeoffset;
	ndflash->options = src_parms->options;

	return 0;
}

static int get_platinfo_from_errpt(nand_data *nddata, struct nand_api_platdependent *platdep)
{
	int ret = 0;
	nand_flash *ndflash = platdep->nandflash;

	/* get sharing parms of nand */
	ret = get_nand_sharing_params(ndflash);
	if(ret < 0)
		RETURN_ERR(ret, "get sharing params from memory error!\n");

	/* init eccsize */
	if(ndflash->pagesize >= DEFAULT_ECCSIZE)
		nddata->eccsize = DEFAULT_ECCSIZE;
	else
		nddata->eccsize = 512;
	nddata->spl_eccsize = SPL_ECCSIZE;

	/* init operations of errpt */
	nand_errpt_init(ndflash);

	/* get rbinfo and nandflash parameters */
	ret = get_errpt_head(nddata, platdep);
	if (ret < 0)
		ret = ENAND;

	/* deinit operations of errpt */
	nand_errpt_deinit();

	return ret;
}

static int get_ppainfo_from_errpt(nand_data *nddata, struct nand_api_platdependent *platdep)
{
	int ret = 0;
	nand_flash *ndflash = platdep->nandflash;

	/* init operations of errpt */
	nand_errpt_init(ndflash);

	/* get information of paritions */
	ret = get_errpt_ppainfo(nddata, platdep);
	if(ret < 0)
		RETURN_ERR(ret, "get ppainfo from errpt error!\n");

	/* deinit operations of errpt */
	nand_errpt_deinit();

	return 0;
}

static int nand_api_later_init(nand_data *nddata, plat_ptinfo *platptinfo) {
	ndd_debug("\nget ndpartition:\n");
	nddata->ptinfo = get_ptinfo(nddata->cinfo, nddata->csinfo->totalchips,
				    nddata->rbinfo->totalrbs, platptinfo);
	if (!nddata->ptinfo)
		GOTO_ERR(get_ptinfo);

	ndd_debug("\nnand ops init:\n");
	nddata->ops_context = nandops_init(nddata);
	if (!nddata->ops_context)
		GOTO_ERR(ops_init);

	ndd_debug("\nregister NandManger:\n");
	Register_NandDriver(&nand_interface);

	ndd_print(NDD_DEBUG, "nand driver later init ok!\n");
	return 0;

ERR_LABLE(ops_init):
	free_ptinfo(nddata->ptinfo);
ERR_LABLE(get_ptinfo):
	return ENAND;
}

static int early_nand_init(nand_data *nddata) {
	int ret = 0, cs_index;

	for (cs_index = 0; cs_index < CS_PER_NFI; cs_index++) {
		ret = early_nand_prepare(&(nddata->base->nfi), cs_index);
		if(ret != SUCCESS)
			ndd_print(NDD_WARNING, "WARNING: early nand prepare faild, cs_index = %d\n", cs_index);
	}

	return 0;
}

static struct nand_api_platdependent* alloc_platdep_memory(void) {
	struct nand_api_platdependent *platdep;

	platdep = ndd_alloc(sizeof(struct nand_api_platdependent) + sizeof(nand_flash) +
			    sizeof(rb_info) + sizeof(plat_ptinfo));
	if (platdep) {
		platdep->nandflash = (nand_flash *)((unsigned char *)platdep + sizeof(struct nand_api_platdependent));
		platdep->rbinfo = (rb_info *)((unsigned char *)platdep->nandflash + sizeof(nand_flash));
		platdep->platptinfo = (plat_ptinfo *)((unsigned char *)platdep->rbinfo + sizeof(rb_info));
	}

	return platdep;
}

static void free_platdep_memory(struct nand_api_platdependent **platdep) {
	ndd_free(*platdep);
	*platdep = NULL;
}

static int  nand_func_auto_adjust(nand_data *nddata)
{
	int context;
	nfi_base *base = &(nddata->base->nfi);
	chip_info *cinfo = nddata->cinfo;
	unsigned short totalchips = nddata->csinfo->totalchips;
	cs_item *csinfo_table = nddata->csinfo->csinfo_table;
	rb_item *rbitem;
	int cs_index;
	int cs_id;
	int ret;

	for(cs_index = 0; cs_index < totalchips; cs_index++){
		cs_id = csinfo_table[cs_index].id;
		rbitem = csinfo_table[cs_index].rbitem;

		context = nand_io_open(base,cinfo);
		if (!context)
			RETURN_ERR(ENAND, "nand io open error");

		ret = nand_io_chip_select(context, cs_id);
		if (ret)
			RETURN_ERR(ENAND, "nand io chip select error, cs = [%d]", cs_id);

		if(nand_auto_adapt_edo)
			nand_auto_adapt_edo(context,cinfo,rbitem);

		if(nand_adjust_to_toggle)
			nand_adjust_to_toggle(context,cinfo);

		if(nand_adjust_to_onfi)
			nand_adjust_to_onfi(context,cinfo);

		nand_io_chip_deselect(context, cs_id);
		nand_io_close(context);
	}

	return 0;
}
int nand_api_init(struct nand_api_osdependent *osdep)
{
	int ret;

	os_clib_init(&(osdep->clib));
	ndd_dump_status();

	__clear_rb_state = osdep->clear_rb_state;
	__wait_rb_timeout = osdep->wait_rb_timeout;
	__try_wait_rb = osdep->try_wait_rb;
	__wp_enable = osdep->wp_enable;
	__wp_disable = osdep->wp_disable;
	__gpio_irq_request = osdep->gpio_irq_request;
	__ndd_gpio_request = osdep->ndd_gpio_request;

	nddata = ndd_alloc(sizeof(nand_data));
	if (!nddata)
		RETURN_ERR(ENAND, "alloc memory for nddata error");
	nddata->clear_rb = clear_rb_state;
	nddata->wait_rb = wait_rb_timeout;

	nddata->cinfo = ndd_alloc(sizeof(chip_info));
	if(!nddata->cinfo)
		GOTO_ERR(alloc_cinfo);

	nddata->base = get_base(osdep);
	if (!nddata->base)
		GOTO_ERR(get_base);

	ret = early_nand_init(nddata);
	if (ret < 0)
		GOTO_ERR(early_nand_init);

	ndd_print(NDD_DEBUG, "nand driver init ok!\n");
	if (share_parms.magic == NAND_MAGIC) {
		int rb_index, totalrbs;
		rb_item *rbinfo_table;

		ndd_debug("\nget memory for platdependent:\n");
		nddata->platdep = alloc_platdep_memory();
		if (!nddata->platdep)
			GOTO_ERR(alloc_platdep_memory);
		nddata->rbinfo = nddata->platdep->rbinfo;

		ndd_debug("\nget platinfo from errpt:\n");
		ret = get_platinfo_from_errpt(nddata, nddata->platdep);
		if (ret < 0)
			GOTO_ERR(get_platinfo);
		nddata->gpio_wp = nddata->platdep->gpio_wp;
		nddata->cinfo->drv_strength = nddata->platdep->drv_strength;

		ndd_debug("\ninit wp gpio:\n");
		ret = __ndd_gpio_request(nddata->gpio_wp, "nand_wp");
		if(ret < 0){
			g_have_wp = 0;
			ndd_print(NDD_DEBUG,"\n%s [line:%d] nand_wp request error, can't use write protect !!!\n",__func__,__LINE__);
		}

		ndd_debug("\ninit irq of rb:\n");
		totalrbs = nddata->rbinfo->totalrbs;
		rbinfo_table = nddata->rbinfo->rbinfo_table;
		for (rb_index = 0; rb_index < totalrbs; rb_index++) {
			ret = __gpio_irq_request(rbinfo_table[rb_index].gpio,
						 &(rbinfo_table[rb_index].irq),
						 &(rbinfo_table[rb_index].irq_private));
			if(ret < 0)
				GOTO_ERR(request_rb_irq);
		}

		ndd_debug("\nfill cinfo:\n");
		ret = fill_cinfo(nddata, nddata->platdep->nandflash);
		if (ret < 0)
			GOTO_ERR(fill_cinfo);
		ndd_dump_chip_info(nddata->cinfo);

		ndd_debug("\nget csinfo:\n");
		nddata->csinfo = get_csinfo(&(nddata->base->nfi), nddata->rbinfo);
		if (!nddata->csinfo)
			GOTO_ERR(get_csinfo);

		ndd_debug("\nnand flash setup:\n");
		ret = nandflash_setup(&(nddata->base->nfi), nddata->csinfo, nddata->rbinfo, nddata->cinfo);
		if (ret)
			ndd_print(NDD_ERROR,"WARNING:nand set feature failed!\n");

		nand_func_auto_adjust(nddata);
		ndd_debug("\nget ppainfo from errpt:\n");
		ret = get_ppainfo_from_errpt(nddata, nddata->platdep);
		if (ret < 0)
			GOTO_ERR(get_ppainfo);

		ndd_debug("\nnand api later init:\n");
		ret = nand_api_later_init(nddata, nddata->platdep->platptinfo);
		if (ret < 0)
			GOTO_ERR(nand_api_later_init);
	} else
		ndd_print(NDD_DEBUG, "no share_parms, please call the nand_api_reinit()!\n");

	return 0;

ERR_LABLE(nand_api_later_init):
ERR_LABLE(get_ppainfo):
	free_csinfo(&(nddata->csinfo));
ERR_LABLE(get_csinfo):
ERR_LABLE(fill_cinfo):
ERR_LABLE(request_rb_irq):
ERR_LABLE(get_platinfo):
	free_platdep_memory(&(nddata->platdep));
ERR_LABLE(alloc_platdep_memory):
ERR_LABLE(early_nand_init):
ERR_LABLE(get_base):
	ndd_free(nddata->cinfo);
ERR_LABLE(alloc_cinfo):
	ndd_free(nddata);
	return ENAND;
}

/**
 * nand_api_reinit():
 * WARNING: this function can only used in usbburntool or card burntool.
 * NOTE1: if (platdep->erasemode == 0) && (platdep->update_errpt == 0), it
 * will just run the nandmanager with platdep, errpt will be ignored.
 * NOTE2: if use in burntool and (platdep->erasemode == 0), platdep->update_errpt
 * must be set true.
 **/
int nand_api_reinit(struct nand_api_platdependent *platdep)
{
	int ret;
	int rb_index, totalrbs;
	rb_item *rbinfo_table;
	nand_flash *ndflash = platdep->nandflash;

	if (ndflash->pagesize >= DEFAULT_ECCSIZE)
		nddata->eccsize = DEFAULT_ECCSIZE;
	else
		nddata->eccsize = 512;;
	nddata->spl_eccsize = SPL_ECCSIZE;
	nddata->cinfo->drv_strength = platdep->drv_strength;
	nddata->gpio_wp = platdep->gpio_wp;
	nddata->rbinfo = platdep->rbinfo;

	ndd_debug("\ninit wp gpio:\n");
	ret = __ndd_gpio_request(nddata->gpio_wp, "nand_wp");
	if(ret < 0)
		GOTO_ERR(request_wp);

	ndd_debug("\ninit irq of rb:\n");
	totalrbs = nddata->rbinfo->totalrbs;
	rbinfo_table = nddata->rbinfo->rbinfo_table;
	for (rb_index = 0; rb_index < totalrbs; rb_index++) {
		ret = __gpio_irq_request(rbinfo_table[rb_index].gpio,
					 &(rbinfo_table[rb_index].irq),
					 &(rbinfo_table[rb_index].irq_private));
		if(ret < 0)
			GOTO_ERR(request_rb_irq);
	}

	ndd_debug("\nfill cinfo:\n");
	ret = fill_cinfo(nddata, ndflash);
	if (ret)
		GOTO_ERR(fill_cinfo);
	ndd_dump_chip_info(nddata->cinfo);

	ndd_debug("\nget csinfo:\n");
	nddata->csinfo = get_csinfo(&(nddata->base->nfi), nddata->rbinfo);
	if (!nddata->csinfo)
		GOTO_ERR(get_csinfo);

	ndd_debug("\nnand flash setup:\n");
	ret = nandflash_setup(&(nddata->base->nfi), nddata->csinfo, nddata->rbinfo, nddata->cinfo);
	if (ret)
		ndd_print(NDD_ERROR,"WARNING:nand set feature failed!\n");

	nand_func_auto_adjust(nddata);

	if (platdep->erasemode) {
		/* init operations of errpt */
		nand_errpt_init(ndflash);
		ndd_debug("\nwrite errpt:\n");
		ret = nand_write_errpt(nddata, platdep->platptinfo, ndflash, platdep->erasemode);
		/* deinit operations of errpt */
		nand_errpt_deinit();
		if (ret < 0)
			GOTO_ERR(write_errpt);
	} else {
		/* init operations of errpt */
		nand_errpt_init(ndflash);
		ndd_debug("\nupdate errpt:\n");
		ret = get_errpt_head(nddata, platdep);
		if (ret >= 0) {
			nddata->cinfo->maxvalidblocks = ndflash->maxvalidblocks / ndflash->chips;
			nddata->cinfo->maxvalidpages = nddata->cinfo->maxvalidblocks * nddata->cinfo->ppblock;
			ret = get_errpt_ppainfo(nddata, platdep);
		}
		/* deinit operations of errpt */
		nand_errpt_deinit();
		if (ret < 0)
			GOTO_ERR(update_errpt);
	}

	ndd_debug("\nnand api later init:\n");
	nddata->platdep = platdep;
	ret = nand_api_later_init(nddata, nddata->platdep->platptinfo);
	if (ret < 0)
		GOTO_ERR(nand_api_later_init);

	return 0;

ERR_LABLE(nand_api_later_init):
ERR_LABLE(update_errpt):
ERR_LABLE(write_errpt):
ERR_LABLE(get_csinfo):
ERR_LABLE(fill_cinfo):
ERR_LABLE(request_rb_irq):
ERR_LABLE(request_wp):
	return ENAND;
}

int nand_api_get_nandflash_id(nand_flash_id *id)
{
	return get_nand_id(&(nddata->base->nfi), id, 0);
}

int nand_api_suspend(void)
{
	int ret;

	ret = nandops_suspend(nddata->ops_context);
	if (ret)
		RETURN_ERR(ret, "nand ops suspend error!");

	ret = nand_io_suspend();
	if (ret)
		RETURN_ERR(ret, "nand io suspend error!");

	ret = nand_bch_suspend();
	if (ret)
		RETURN_ERR(ret, "nand bch suspend error!");

	return 0;
}

int nand_api_resume(void)
{
	int ret;

	ret = nand_io_resume();
	if (ret)
		RETURN_ERR(ret, "nand io resume error!");

	ret = nand_bch_resume();
	if (ret)
		RETURN_ERR(ret, "nand bch resume error!");

	ret = nandops_resume(nddata->ops_context);
	if (ret)
		RETURN_ERR(ret, "nand ops resume error!");

	return 0;
}
