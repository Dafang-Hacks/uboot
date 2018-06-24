#include <os_clib.h>
#include "nand_ops.h"
#include "spl_rw.h"
#include "bad_block.h"
#include "nand_debug.h"
#include "cpu_msg_handler.h"
#include "dma_msg_handler.h"

static void nandops_retry(nand_data *ndata, struct task_msg *msg, int num,int cycle)
{
	union taskmsghead *ops;
	unsigned int i;
	char cs_array[4] = {-1, -1, -1, -1};
	if(ndata->cinfo->options & NAND_READ_RETRY){
		for(i = 0; i < num; i++){
			ops = &((msg + i)->ops);
			if(cs_array[ops->bits.chipsel] == -1){
				cs_array[ops->bits.chipsel] = ops->bits.chipsel;
				set_retry_feature((int)ndata, ops->bits.chipsel, 1);
			}
		}
	}
}
int nandops_read(int context, ndpartition *pt, PageList *pl)
{
	int flag = 0, ret = 0, i = 0, index = 0;
	struct nandops_info *ops = (struct nandops_info *)context;
	nand_data *data = ops->nanddata;
	struct opsunit_handler *handler = ops->handler_unit;
	unsigned int unit_cnt = ops->unit_cnt;
	struct msg_handler *msghandler = NULL;
	Nand_Task **nandtask = ops->nandtask;
	struct singlelist *pos;
	PageList *pl_node, *probe_node = NULL;
	PageList *tmp_pl, *new_pl = NULL;
	unsigned int retrycnt = 0;
	int ecc_error_cnt = 0;
	int allff_cnt = 0;
	int retry_flag = 0;
	int want_retry = 0;

	/* handle spl partition */
	if(pt - data->ptinfo->pt == 0){
		if(pl->startPageID < ops->xboot_offset_page){
			int spl = spl_init(data);
			ret = spl_read(spl, pl);
			spl_deinit(spl);
			return ret;
		}
	}
	/* handle others */
	if (pt->ops_mode == DMA_OPS) {
		singlelist_for_each(pos, &(pl->head)){
			pl_node = singlelist_entry(pos, PageList, head);
			ndd_dma_cache_inv((unsigned int)pl_node->pData,pl_node->Bytes);
		}
	}

	tmp_pl = pl;
	do{
		ret = lib_nandops_creat_task(ops->lib_ops,pt->planes,pt->pagepblock,pt->startblockid,pt->eccbit,
				(unsigned int)tmp_pl,NANDOPS_READ);
		if(ret)
			RETURN_ERR(ENAND, "lib_nandops_creat_task is failed !");
		for(i = 0; i < unit_cnt; i++){
			if(nandtask[i]->msg_index != 0){
				if(retrycnt != 0)
					nandops_retry(data, nandtask[i]->msg, nandtask[i]->msg_index, retrycnt);
				if (pt->ops_mode == DMA_OPS) {
					ndd_dma_cache_inv((unsigned int)nandtask[i]->ret,nandtask[i]->msg_maxcnt);
					msghandler = handler[i].dma;
				} else
					msghandler = handler[i].cpu;
				/* set the feature of retry */
				flag = msghandler->handler(msghandler->context, nandtask[i]);
				if(flag){
					ret = flag;
					if((ret != ALL_FF) && (ret != BLOCK_MOVE) && (retrycnt >= 8)){
						ndd_print(NDD_ERROR, "ERROR: [%s] nandops read error, ret = %d retrycnt = %d\n",
								pt->name, ret, retrycnt);
						ndd_dump_taskmsg(nandtask[i]->msg,nandtask[i]->msg_index);
					}
				}
				flag = lib_nandops_getret(ops->lib_ops, pt->planes, pt->pagepblock,
						pt->startblockid,pt->eccbit, i,NANDOPS_READ);
			}
		}
	retry_flag = 0;
		if(ret){
			singlelist_for_each(pos, &(tmp_pl->head)){
				pl_node = singlelist_entry(pos, PageList, head);
				if(pl_node->retVal == ALL_FF){
					ndd_memset(pl_node->pData,0xff,pl_node->Bytes);
					ndd_dma_cache_wback((unsigned int)pl_node->pData,pl_node->Bytes);
				}

				if(retrycnt == 0){	/* creat retry pagelist */
					if((pl_node->retVal == ECC_ERROR) || (pl_node->retVal == ALL_FF) || (pl_node->retVal == ND_ECC_TOOLARGE)){
						if(new_pl == NULL){
							ndd_memset(ops->retrytop, 0x0,
								   (sizeof(PageList) *
								    ((VNANDCACHESIZE(data->cinfo->pagesize) + 512) / 512)));
							probe_node = ops->retrytop + index;
							new_pl = probe_node;
							singlelist_init(&(probe_node->head));
						}
						else{
							singlelist_add(&(probe_node->head),&((ops->retrytop + index)->head));
							probe_node = ops->retrytop + index;
						}
						probe_node->startPageID = pl_node->startPageID;
						probe_node->_startPageID = pl_node->_startPageID;
						probe_node->OffsetBytes = pl_node->OffsetBytes;
						probe_node->Bytes = pl_node->Bytes;
						probe_node->pData = pl_node->pData;
						want_retry = 1;
					}
					index++;
				}else{		/* scan retry pagelist */
					if((pl_node->retVal == ECC_ERROR) || (pl_node->retVal == ALL_FF) || (pl_node->retVal == ND_ECC_TOOLARGE)){
						if(new_pl == NULL){
							probe_node = pl_node;
							new_pl = probe_node;
						}else{
							probe_node->head.next = &(pl_node->head);
							probe_node = pl_node;
						}
						want_retry = 1;
					}
				}
			}
			if(new_pl){
				tmp_pl = new_pl;
				new_pl = NULL;
			}
		}
		if(probe_node)
			probe_node->head.next = NULL;
		retrycnt++;
	if(ret == ALL_FF)
		allff_cnt++;
	if((ret == ALL_FF) || (ret == ECC_ERROR) || (ret == ND_ECC_TOOLARGE))
		ecc_error_cnt++;

	if(want_retry == 1 && retrycnt < 9)
		retry_flag = 1;
	if((ret != ECC_ERROR) && retrycnt >= 9 && want_retry != 1)
		retry_flag = 0;
	else if(ret == ECC_ERROR && retrycnt >= 9)
		retry_flag = 1;

	if(allff_cnt >= 2){
		ret = ALL_FF;
		retry_flag = 0;
	}
	if(ecc_error_cnt >= 17){
		ret = ECC_ERROR;
		retry_flag = 0;
	}

	want_retry = 0;

	}while(retry_flag && retrycnt < 18);
	if(retrycnt > 1){
		ndd_print(NDD_INFO, "^^^^^ retrycnt = %d ^^^^^^\n",retrycnt);
		index = 0;
		tmp_pl = pl;
		singlelist_for_each(pos, &(tmp_pl->head)){
			pl_node = singlelist_entry(pos, PageList, head);
			if(ops->retrytop[index].pData)
				pl_node->retVal = ops->retrytop[index].retVal;
			index++;
		}
	}
	return ret; // manager should scan pagelist
}
int nandops_write(int context, ndpartition *pt, PageList *pl)
{
	int flag = 0, ret = 0, i = 0;
	struct nandops_info *ops = (struct nandops_info *)context;
	nand_data *data = ops->nanddata;
	struct opsunit_handler *handler = ops->handler_unit;
	unsigned int unit_cnt = ops->unit_cnt;
	struct msg_handler *msghandler = NULL;
	Nand_Task **nandtask = ops->nandtask;
	struct singlelist *pos;
	PageList *pl_node;

	/* handle spl partition */
	if(pt - data->ptinfo->pt == 0){
		if(pl->startPageID < ops->xboot_offset_page){
			int spl = spl_init(data);
			ret = spl_write(spl, pl);
			spl_deinit(spl);
			return ret;
		}
	}
	/* handle others */
	if (pt->ops_mode == DMA_OPS) {
		singlelist_for_each(pos, &(pl->head)){
			pl_node = singlelist_entry(pos, PageList, head);
			ndd_dma_cache_wback((unsigned int)pl_node->pData,pl_node->Bytes);
		}
	}

	ret = lib_nandops_creat_task(ops->lib_ops,pt->planes,pt->pagepblock,pt->startblockid,pt->eccbit,(unsigned int)pl,NANDOPS_WRITE);
	if(ret)
		RETURN_ERR(ENAND, "lib_nandops_creat_task is failed !");
	for(i = 0; i < unit_cnt; i++){
		if(nandtask[i]->msg_index != 0){
			if (pt->ops_mode == DMA_OPS) {
				ndd_dma_cache_inv((unsigned int)nandtask[i]->ret,nandtask[i]->msg_maxcnt);
				msghandler = handler[i].dma;
			} else
				msghandler = handler[i].cpu;

			flag = msghandler->handler(msghandler->context, nandtask[i]);
			if(flag){
				ret = flag;
				ndd_print(NDD_ERROR, "ERROR: [%s] nandops write error, ret = %d\n", pt->name, ret);
				ndd_dump_taskmsg(nandtask[i]->msg,nandtask[i]->msg_index);
			}
			flag = lib_nandops_getret(ops->lib_ops,pt->planes,pt->pagepblock,pt->startblockid,pt->eccbit, i,NANDOPS_WRITE);
		}
	}
	return ret; // manager should scan pagelist
}
int nandops_erase(int context, ndpartition *pt, BlockList *bl)
{
	int ret = 0, flag = 0, i;
	struct nandops_info *ops = (struct nandops_info *)context;
	struct opsunit_handler *handler = ops->handler_unit;
	unsigned int unit_cnt = ops->unit_cnt;
	struct msg_handler *msghandler = NULL;
	Nand_Task **nandtask = ops->nandtask;

	ret = lib_nandops_creat_task(ops->lib_ops,pt->planes,pt->pagepblock,pt->startblockid,pt->eccbit,(unsigned int)bl,NANDOPS_ERASE);
	if(ret)
		RETURN_ERR(ENAND, "lib_nandops_creat_task is failed !");
	for(i = 0; i < unit_cnt; i++){
		if(nandtask[i]->msg_index != 0){
			if (pt->ops_mode == DMA_OPS) {
				ndd_dma_cache_inv((unsigned int)nandtask[i]->ret,nandtask[i]->msg_maxcnt);
				msghandler = handler[i].dma;
			} else
				msghandler = handler[i].cpu;

			flag = msghandler->handler(msghandler->context, nandtask[i]);
			if(flag){
				ret = flag;
				ndd_print(NDD_ERROR, "ERROR: [%s] nandops erase error, ret = %d\n", pt->name, ret);
				ndd_dump_taskmsg(nandtask[i]->msg, nandtask[i]->msg_index);
			}
			flag = lib_nandops_getret(ops->lib_ops,pt->planes,pt->pagepblock,pt->startblockid,pt->eccbit, i,NANDOPS_ERASE);
		}
	}
	return ret; // manager should scan blocklist
}
int nandops_isbadblk(int context, ndpartition *pt, int blockid)
{
	int ret = 0,i = 0;
	struct nandops_info *ops = (struct nandops_info *)context;
	ret = lib_nandops_creat_task(ops->lib_ops,pt->planes,pt->pagepblock,pt->startblockid,pt->eccbit,blockid,NANDOPS_ISBADBLOCK);
	if(ret)
		RETURN_ERR(ENAND, "lib_nandops_creat_task is failed !");
	for(i = 0; i < ops->unit_cnt; i++){
		if(ops->nandtask[i]->msg_index){
			ret = is_bad_block(ops, ops->nandtask[i]);
		}
	}
	return ret;
}
int nandops_markbadblk(int context, ndpartition *pt, int blockid)
{
	int ret = 0,i = 0;
	struct nandops_info *ops = (struct nandops_info *)context;
	ret = lib_nandops_creat_task(ops->lib_ops,pt->planes,pt->pagepblock,pt->startblockid,pt->eccbit,blockid,NANDOPS_MARKBADBLOCK);
	if(ret)
		RETURN_ERR(ENAND, "lib_nandops_creat_task is failed !");
	for(i = 0; i < ops->unit_cnt; i++){
		if(ops->nandtask[i]->msg_index){
			ret = mark_bad_block(ops, ops->nandtask[i]);
		}
	}
	return ret;
}
int nandops_suspend(int context)
{
	int i, ret = 0;
	struct nandops_info *ops = (struct nandops_info *)context;
	for(i = 0; i < ops->unit_cnt; i++){
		ret = cpu_msg_handle_suspend((int)(ops->handler_unit[i].cpu));
		if (ret)
			RETURN_ERR(ret, "unit[%d] cpu handler suspend error!", i);
		ret = dma_msg_handle_suspend((int)(ops->handler_unit[i].dma));
		if (ret)
			RETURN_ERR(ret, "unit[%d] dma handler suspend error!", i);
	}
	return ret;
}

int nandops_resume(int context)
{
	int i, ret = 0;
	struct nandops_info *ops = (struct nandops_info *)context;

	for(i = 0; i < ops->unit_cnt; i++){
		ret = cpu_msg_handle_resume((int)(ops->handler_unit[i].cpu));
		if (ret)
			RETURN_ERR(ret, "unit[%d] cpu handler resume error!", i);
		ret = dma_msg_handle_resume((int)(ops->handler_unit[i].dma),ops->nandtask[i],i);
		if (ret)
			RETURN_ERR(ret, "unit[%d] dma handler resume error!", i);
	}
	return ret;
}

int nandops_init(nand_data *data)
{
	int i = 0, iocount = 1;
	struct nandops_info *ops;
	struct lib_nandchipinfo libcinfo;
	struct lib_nandioinfo *libioinfo;
	chip_info *cinfo;

	if(!data)
		GOTO_ERR(data);

	cinfo = data->cinfo;

	ops = (struct nandops_info *)ndd_alloc(sizeof(struct nandops_info));
	if(!ops)
		GOTO_ERR(alloc_info);
	ops->nandtask = (Nand_Task **)ndd_alloc(sizeof(Nand_Task *) * iocount);
	if(!ops->nandtask)
		GOTO_ERR(alloc_nandtask);

#if 1
	ops->mem = (void *)ndd_alloc(512 * 1024);
	if(!ops->mem)
		GOTO_ERR(alloc_mem);
#else
	ops->mem = NULL;
#endif
	/* init struct lib_nandchipinfo */
	libcinfo.manuf = cinfo->manuf;
	libcinfo.options = cinfo->options;
	libcinfo.totalblocks = cinfo->maxvalidblocks;
	libcinfo.totalpages = cinfo->maxvalidpages;
	libcinfo.pagesize = cinfo->pagesize;

	libcinfo.ops_timing = &cinfo->ops_timing;


/* init struct lib_nandioinfo */
	libioinfo = (struct lib_nandioinfo *)ndd_alloc(sizeof(struct lib_nandioinfo) * iocount);
	if(!libioinfo)
		GOTO_ERR(alloc_libioinfo);
	for(i = 0; i < iocount; i++){
		libioinfo[i].rb_cnt = data->rbinfo->totalrbs;
		libioinfo[i].chipprb = data->csinfo->totalchips / data->rbinfo->totalrbs;
	}
	/* init lib_ops */
	ops->lib_ops = lib_nandops_init(data->eccsize, iocount , ops->nandtask, ops->mem, 512 * 1024, &libcinfo, libioinfo);
	if(!ops->lib_ops)
		GOTO_ERR(lib_ops_init);
	/*** init retry pagelist ***/
	ops->retrytop = (PageList *)ndd_alloc(sizeof(PageList) *
					      ((VNANDCACHESIZE(data->cinfo->pagesize) + 512) / 512));
	if(!ops->retrytop)
		GOTO_ERR(retrytop_init);
	/*** init handler ***/
	ops->unit_cnt = iocount;
	ops->handler_unit = (struct opsunit_handler *)ndd_alloc(sizeof(struct opsunit_handler) * ops->unit_cnt);
	if(!ops->handler_unit)
		GOTO_ERR(alloc_unit);
	for(i = 0; i < ops->unit_cnt; i++){
		ops->handler_unit[i].cpu = (struct msg_handler *)cpu_msg_handle_init(data,ops->nandtask[i],i);
		ops->handler_unit[i].dma = (struct msg_handler *)dma_msg_handle_init(data,ops->nandtask[i],i);
		if(ops->handler_unit[i].cpu == NULL || ops->handler_unit[i].dma == NULL)
			GOTO_ERR(handler_init);
	}

	ops->xboot_offset_page = (cinfo->ppblock < 128 ? ((128 / cinfo->ppblock) + 1) : 2) * cinfo->ppblock;

	ops->nanddata = data;

	ndd_free(libioinfo);
	return (int)ops;

ERR_LABLE(handler_init):
ERR_LABLE(alloc_unit):
	ndd_free(ops->retrytop);
ERR_LABLE(retrytop_init):
	lib_nandops_deinit(ops->lib_ops);
ERR_LABLE(lib_ops_init):
	ndd_free(libioinfo);
ERR_LABLE(alloc_libioinfo):
#if 1
	ndd_free(ops->mem);
ERR_LABLE(alloc_mem):
#endif
	ndd_free(ops->nandtask);
ERR_LABLE(alloc_nandtask):
	ndd_free(ops);
ERR_LABLE(alloc_info):
ERR_LABLE(data):

	return 0;
}
void nandops_deinit(int context)
{
	struct nandops_info *ops = (struct nandops_info *)context;
	int i = 0;
	/*** deinit handler ***/
	for(i = 0; i < ops->unit_cnt; i++){
		cpu_msg_handle_deinit((int)(ops->handler_unit[i].cpu));
		dma_msg_handle_deinit((int)(ops->handler_unit[i].dma));
	}
	lib_nandops_deinit(ops->lib_ops);
	ndd_free(ops->handler_unit);
	ndd_free(ops);
}
