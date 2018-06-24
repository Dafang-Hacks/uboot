#include <os_clib.h>
#include "nand_debug.h"
#include "spl_rw.h"

#define SPL_BAKUP_STEPS 128 /* 128 pages */

struct AlignList {
        int pageid;
        int count;
        PageList *pagelist;
        struct AlignList *next;
};

struct Spl {
        int io_context;
        int bch_context;
        nand_data *nddata;
        chip_info *cinfo;
        int bchsize;
        unsigned char *bchbuf;
};

struct AlignList alist[128];

static void nand_busy_clear(struct Spl *spl)
{
	spl->nddata->clear_rb(0);
}

static int nand_wait_busy(struct Spl *spl)
{
        int ret = SUCCESS;

        ret = spl->nddata->wait_rb(0, 500);

        ndd_ndelay(spl->cinfo->ops_timing.tRR);
        return ret;
}

static int nand_read_status(struct Spl *spl)
{
        int ret = SUCCESS;
        unsigned char status;

        nand_io_send_cmd(spl->io_context, CMD_READ_STATUS_1ST, spl->cinfo->ops_timing.tWHR);
        nand_io_receive_data(spl->io_context, &status, 1);
	ret = (status & 0x1 ? 1 : 0);

        return ret;
}

static void nand_write_start(struct Spl *spl, int pageid, int offset)
{
	/* if pagesize == 512, write 0x00 to reset pointer to 0 before program */
	if (spl->cinfo->pagesize == 512)
		nand_io_send_cmd(spl->io_context, CMD_PAGE_READ_1ST, 0);
        nand_io_send_cmd(spl->io_context, CMD_PAGE_PROGRAM_1ST, 0);
        nand_io_send_addr(spl->io_context, offset, pageid, spl->cinfo->ops_timing.tADL);
}

static void nand_write_random(struct Spl *spl, int offset)
{
        nand_io_send_cmd(spl->io_context, CMD_RANDOM_INPUT_1ST, spl->cinfo->ops_timing.tCWAW);
        nand_io_send_addr(spl->io_context, offset, -1, spl->cinfo->ops_timing.tADL);
}

static void nand_write_data(struct Spl *spl, unsigned char *buf, int size)
{
//        int i;
        nand_io_send_data(spl->io_context, buf, size);
/*        for (i=0; i < size; i++) {
                if (i % 16 == 0)
                        ndd_print(NDD_DEBUG, "\n[0x%08x]",i);
                ndd_print(NDD_DEBUG, "%02x ", buf[i]);
        }
*/
}

static int nand_write_finish(struct Spl *spl)
{
        int ret = SUCCESS;
	nand_busy_clear(spl);
        nand_io_send_cmd(spl->io_context, CMD_PAGE_PROGRAM_2ND, spl->cinfo->ops_timing.tWB);
        ret = nand_wait_busy(spl);
        if (ret < 0)
                return TIMEOUT;
        ret = nand_read_status(spl);

        return ret;
}

static void nand_read_random(struct Spl *spl, int offset)
{
        nand_io_send_cmd(spl->io_context, CMD_RANDOM_OUTPUT_1ST, 0);
        nand_io_send_addr(spl->io_context, offset, -1, 0);
        nand_io_send_cmd(spl->io_context, CMD_RANDOM_OUTPUT_2ND, spl->cinfo->ops_timing.tWHR2);
}

static void nand_read_data(struct Spl *spl, unsigned char *buf, int size)
{
 //       int i;
        nand_io_receive_data(spl->io_context, buf, size);
/*        for (i=0; i < size; i++) {
                if (i % 16 == 0)
                        ndd_print(NDD_DEBUG, "\n[0x%08x]",i);
                ndd_print(NDD_DEBUG, "%02x ", buf[i]);
        }*/
}

static int nand_read_start(struct Spl *spl, int pageid, int offset)
{
        int ret = SUCCESS;
        nand_io_send_cmd(spl->io_context, CMD_PAGE_READ_1ST, 0);
	nand_busy_clear(spl);
        nand_io_send_addr(spl->io_context, offset, pageid, 0);
	if (spl->cinfo->pagesize != 512)
		nand_io_send_cmd(spl->io_context, CMD_PAGE_READ_2ND, spl->cinfo->ops_timing.tWB);

        ret = nand_wait_busy(spl);
        if (ret < 0)
                ret = TIMEOUT;
        return ret;
}
static void dump_alignedlist(void)
{
        struct AlignList *al = alist;
        while (al) {
                ndd_print(NDD_DEBUG, "Alist: pl = %p, count = %d, pageid = %d\n",
			  al->pagelist, al->count, al->pageid);
                al = al->next;
        }
}

static struct AlignList *aligned_list(PageList *pl)
{
        struct singlelist *listhead;
        PageList *plnext = NULL;
        struct AlignList *alignlist = alist;
        PageList *pagelist = pl;

        alignlist->pagelist = pl;
        alignlist->count = 0;
        alignlist->pageid = pl->startPageID;
        alignlist->next = NULL;

        singlelist_for_each(listhead, &pl->head){
                plnext = singlelist_entry(listhead, PageList, head);
                if(plnext->startPageID == pagelist->startPageID) {
                        alignlist->count++;
                } else {
                        pagelist = plnext;
                        alignlist->next = alignlist + 1;
                        alignlist = alignlist->next;
                        alignlist->pagelist = pagelist;
                        alignlist->count = 1;
                        alignlist->pageid = pagelist->startPageID;
                        alignlist->next = NULL;
                }
        }
        dump_alignedlist();
        return alist;
}

static int write_data(struct Spl *spl, struct AlignList *al, int pageid)
{
	int i, j, ret = SUCCESS;
	PageList *pagelist = al->pagelist;
	struct singlelist *pos = &pagelist->head;
	unsigned char *data = NULL;

	nand_write_start(spl, pageid, pagelist->OffsetBytes);

	for (i = 0; i < al->count; i++) {
		pagelist = singlelist_entry(pos, PageList, head);
		pos = (pagelist->head).next;

		if(i && (spl->cinfo->pagesize != 512))
			nand_write_random(spl, pagelist->OffsetBytes);
		for(j = 0; j < (pagelist->Bytes / SPL_BCH_SIZE); j++){
			/** the first 256 bytes of spl don't use PN **/
			if(i != 0 || j != 0 || (pageid % SPL_BAKUP_STEPS))
				pn_enable(spl->io_context);
			data = (unsigned char *)(pagelist->pData) + j * SPL_BCH_SIZE;
			nand_write_data(spl, data, SPL_BCH_SIZE);
			nand_io_send_waitcomplete(spl->io_context,spl->cinfo);
			if(i != 0 || j != 0 || (pageid % SPL_BAKUP_STEPS))
				pn_disable(spl->io_context);

		}
	}

	ret = nand_write_finish(spl);

	return ret;
}

static int write_bch(struct Spl *spl, int pageid)
{
        int ret = SUCCESS;

        nand_write_start(spl, pageid, 0);
    	pn_enable(spl->io_context);
        nand_write_data(spl, spl->bchbuf, spl->bchsize);
		nand_io_send_waitcomplete(spl->io_context,spl->cinfo);
    	pn_disable(spl->io_context);
        ret = nand_write_finish(spl);

        return ret;
}

static int write_nand(struct Spl *spl, struct AlignList *al)
{
        int ret = SUCCESS;
        int i, pageid;
	int copy_offset = 0;

        nand_io_chip_select(spl->io_context, SPL_CS);

        pageid = al->pagelist->startPageID;
	copy_offset = spl->cinfo->ppblock > 128 ? spl->cinfo->ppblock : 128;

        for (i = 0; i <= SPL_BAK_NUM; i++) {
                ret = write_data(spl, al, (pageid * 2) + copy_offset * i);
                if (ret < 0)
                        GOTO_ERR(write_data);
                ret = write_bch(spl, (pageid * 2 + 1) + copy_offset * i);
                if (ret < 0)
                        GOTO_ERR(write_bch);
        }

ERR_LABLE(write_bch):
ERR_LABLE(write_data):
        nand_io_chip_deselect(spl->io_context, SPL_CS);
        return ret;
}

static int bch_encode(struct Spl *spl, struct AlignList *al)
{
        int i, j, ret = SUCCESS;
        PipeNode pipe;
        PageList *pagelist = al->pagelist;
        struct singlelist *pos = &pagelist->head;

        pipe.parity = spl->bchbuf;
        for (i = 0; i < al->count; i++) {
                pagelist = singlelist_entry(pos, PageList, head);
                pos = (pagelist->head).next;
                pipe.data = pagelist->pData;

                for(j = 0; j < pagelist->Bytes / SPL_BCH_SIZE; j++) {
                        ret = nand_bch_encode_prepare(spl->bch_context, &pipe, SPL_BCH_BIT);
                        if (ret < 0)
                                RETURN_ERR(ret, "bch encode prepare error");
                        ret = nand_bch_encode_complete(spl->bch_context, &pipe);
                        if (ret < 0)
				RETURN_ERR(ret, "bch encode complete error");
                        pipe.data += SPL_BCH_SIZE;
                        pipe.parity += SPL_PAR_SIZE;
                }
        }

        return ret;
}

static int __spl_write(struct Spl *spl, struct AlignList *al)
{
        int ret = SUCCESS;

        while (al) {
                ret = bch_encode(spl, al);
                if (ret)
			RETURN_ERR(ret, "bch encode error");
                ret = write_nand(spl, al);
                if (ret)
			RETURN_ERR(ret, "spl write error");
                al = al->next;
        }

        return ret;
}

static int bch_decode(struct Spl *spl, struct AlignList *al)
{
        int i, j;
        int ret = SUCCESS;
        PipeNode pipe;
        PageList *pagelist = al->pagelist;
        struct singlelist *pos = &pagelist->head;

        for (i = 0; i < al->count; i++) {
                pagelist = singlelist_entry(pos, PageList, head);
                pos = (pagelist->head).next;
                pipe.data = pagelist->pData;
                pipe.parity = spl->bchbuf + pagelist->OffsetBytes / SPL_BCH_SIZE * SPL_PAR_SIZE;

                for(j = 0; j < pagelist->Bytes / SPL_BCH_SIZE; j++) {
                        nand_bch_decode_prepare(spl->bch_context, &pipe, SPL_BCH_BIT);
                        ret = nand_bch_decode_complete(spl->bch_context, &pipe);
                        if (ret < 0)
				RETURN_ERR(ret, "bch decode complete error");
                        pipe.data += SPL_BCH_SIZE;
                        pipe.parity += SPL_PAR_SIZE;
                }
        }
        return ret;
}

static int read_data(struct Spl *spl, struct AlignList *al, int pageid)
{
        int i, j, ret = SUCCESS;
        PageList *pagelist = al->pagelist;
        struct singlelist *pos = &pagelist->head;
	unsigned char *data = NULL;

        ret = nand_read_start(spl, pageid, pagelist->OffsetBytes);
        if (ret < 0)
		RETURN_ERR(ret, "data read start error");

        for (i = 0; i < al->count; i++) {
                pagelist = singlelist_entry(pos, PageList, head);
                pos = (pagelist->head).next;

                if (i && (spl->cinfo->pagesize != 512))
                        nand_read_random(spl, pagelist->OffsetBytes);
		for(j = 0; j < (pagelist->Bytes / SPL_BCH_SIZE); j++){
			/** the first 256 bytes of spl don't use PN **/
			if(i != 0 || j != 0 || (pageid % SPL_BAKUP_STEPS))
				pn_enable(spl->io_context);
			data = (unsigned char *)(pagelist->pData) + j * SPL_BCH_SIZE;
			nand_read_data(spl, data, SPL_BCH_SIZE);
			if(i != 0 || j != 0 || (pageid % SPL_BAKUP_STEPS))
				pn_disable(spl->io_context);
		}
		if (spl->cinfo->pagesize == 512) {
			unsigned char spare_buf[spl->cinfo->oobsize];
			nand_read_data(spl, spare_buf, spl->cinfo->oobsize);
		//	if (nand_wait_busy(spl) < 0)
		//		ndd_print(NDD_WARNING, "wait rb of spl data complete timeout\n");
		}
        }

        return ret;
}

static int read_bch(struct Spl *spl, int pageid)
{
	int ret = SUCCESS;
	ret = nand_read_start(spl, pageid, 0);
	if (ret < 0)
		RETURN_ERR(ret, "bch read start error");
	pn_enable(spl->io_context);
	if (spl->cinfo->pagesize != 512)
		nand_read_data(spl, spl->bchbuf, spl->bchsize);
	else {
		nand_read_data(spl, spl->bchbuf, spl->cinfo->pagesize + spl->cinfo->oobsize);
	//	if (nand_wait_busy(spl) < 0)
	//		ndd_print(NDD_WARNING, "wait rb of spl bch complete timeout\n");
	}
	pn_disable(spl->io_context);

	return ret;
}

static int read_nand(struct Spl *spl, struct AlignList *al, int pageid)
{
        int ret = SUCCESS;

        nand_io_chip_select(spl->io_context, SPL_CS);

        ret = read_data(spl, al, pageid);
        if (ret < 0)
		RETURN_ERR(ret, "read data error");
        ret = read_bch(spl, pageid + 1);
        if (ret < 0)
		RETURN_ERR(ret, "read bch error");

        nand_io_chip_deselect(spl->io_context, SPL_CS);

        return ret;
}

static int __spl_read(struct Spl *spl, struct AlignList *al)
{
        int ret = SUCCESS, i;
        int pageid, times = 0;
	struct AlignList *read_al = NULL;
	int copy_offset = 0;
	int retry = (spl->cinfo->retryparms != NULL) ? spl->cinfo->retryparms->cycle + 1 : 9;
	copy_offset = spl->cinfo->ppblock > 128 ? spl->cinfo->ppblock : 128;
backup:
	read_al = al;
        while (read_al) {
		i = 0;
readretry:
                pageid = read_al->pagelist->startPageID;
                ret = read_nand(spl, read_al, (pageid * 2) + times * copy_offset);
                if (ret < 0) {
			times++;
                        if (times > SPL_BAK_NUM)
				RETURN_ERR(ENAND, "spl read error");
                        goto backup;
                }
                ret = bch_decode(spl, read_al);
                if ((spl->cinfo->options & NAND_READ_RETRY) && ret < 0 && i < retry) {
			ret = set_retry_feature((int)(spl->nddata), 0, 1);
			i++;
			goto readretry;
		}
		if(ret < 0){
			times++;
			if (times > SPL_BAK_NUM)
				RETURN_ERR(ENAND, "spl read error");
			goto backup;
		}
                read_al = read_al->next;
        }

        return ret;
}

int spl_read(int handle, PageList *pl)
{
        struct Spl *spl = (struct Spl *)handle;
        struct AlignList *al = NULL;
        int ret = SUCCESS;

        al = aligned_list(pl);
        ret = __spl_read(spl, al);

        return ret;
}

int spl_write(int handle, PageList *pl)
{
        struct Spl *spl = (struct Spl *)handle;
        struct AlignList *al = NULL;
        int ret = SUCCESS;

        al = aligned_list(pl);
        ret = __spl_write(spl, al);

        return ret;
}

int spl_init(nand_data *data)
{
        struct Spl *spl_handle;

        spl_handle = (struct Spl *)ndd_alloc(sizeof(struct Spl));
        if (spl_handle == NULL)
		GOTO_ERR(err1);

        spl_handle->nddata = data;
        spl_handle->cinfo = data->cinfo;
        spl_handle->bchsize = data->cinfo->pagesize / SPL_BCH_SIZE * SPL_PAR_SIZE;

	if (data->cinfo->pagesize == 512)
		spl_handle->bchbuf = ndd_alloc(data->cinfo->pagesize + data->cinfo->oobsize);
	else
		spl_handle->bchbuf = ndd_alloc(spl_handle->bchsize);
        if (spl_handle->bchbuf == NULL)
		GOTO_ERR(err2);

        spl_handle->io_context = nand_io_open(&(data->base->nfi), data->cinfo);
        if(!spl_handle->io_context)
		GOTO_ERR(err3);

        spl_handle->bch_context = nand_bch_open(&(data->base->bch), data->spl_eccsize);
        if(!spl_handle->bch_context)
		GOTO_ERR(err4);

        return (int)spl_handle;

ERR_LABLE(err4):
        nand_io_close(spl_handle->io_context);
ERR_LABLE(err3):
        ndd_free(spl_handle->bchbuf);
ERR_LABLE(err2):
        ndd_free(spl_handle);
ERR_LABLE(err1):
        return -1;
}

void spl_deinit(int handle)
{
        struct Spl *spl = (struct Spl *)handle;

        if(!spl)
                return;
        nand_io_close(spl->io_context);
        nand_bch_close(spl->bch_context);
        ndd_free(spl->bchbuf);
        ndd_free(spl);
}
