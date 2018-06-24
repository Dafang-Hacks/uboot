#include <os_clib.h>
#include <nand_ops.h>
#include <nand_io.h>
#include <ndcommand.h>
#include <nand_debug.h>

extern void (*__wp_enable) (int);
extern void (*__wp_disable) (int);

void dump_array(int *array, int count)
{
	int i;
	for (i = 0; i < count; i++) {
		if (!(i % 8))
			ndd_print(NDD_DEBUG, "\n\t %d:", i);
		ndd_print(NDD_DEBUG, "\t%d", array[i]);
	}
	ndd_print(NDD_DEBUG, "\n");
}

void ndd_dump_pagelist(PageList *pl)
{
	PageList *pl_node;
	struct singlelist *pos;

	singlelist_for_each(pos, &pl->head) {
		pl_node = singlelist_entry(pos, PageList, head);
		ndd_print(NDD_DEBUG, "PageID = %d, Offset = %d, Bytes = %d, pData = %p, retVal = %d\n",
			  pl_node->startPageID, pl_node->OffsetBytes, pl_node->Bytes, pl_node->pData, pl_node->retVal);
	}
}

void __ndd_dump_badblockinfo(ndpartition *pt, unsigned int plat_ptcount)
{
	int pt_index, i;

	for (pt_index = 0; pt_index < plat_ptcount; pt_index++) {
		ndd_print(NDD_DEBUG, "\t pt[%s]: \n", (pt + pt_index)->name);
		for (i = 0; ; i++) {
			if(i % 8 == 0)
				ndd_print(NDD_DEBUG, "\n\t");
			if((pt + pt_index)->pt_badblock_info[i] != -1)
				ndd_print(NDD_DEBUG, " %d",(pt + pt_index)->pt_badblock_info[i]);
			else
				break;
		}
	}
}

void __ndd_dump_nandflash(nand_flash *ndflash)
{
	ndd_print(NDD_DEBUG, "ndflash->id: 0x%08x\n", ndflash->id);
	ndd_print(NDD_DEBUG, "ndflash->extid: 0x%08x\n", ndflash->extid);
	ndd_print(NDD_DEBUG, "ndflash->pagesize: %d\n", ndflash->pagesize);
	ndd_print(NDD_DEBUG, "ndflash->oobsize: %d\n", ndflash->oobsize);
	ndd_print(NDD_DEBUG, "ndflash->blocksize: %d\n", ndflash->blocksize);
	ndd_print(NDD_DEBUG, "ndflash->totalblocks: %d\n", ndflash->totalblocks);
	ndd_print(NDD_DEBUG, "ndflash->maxvalidblocks: %d\n", ndflash->maxvalidblocks);
	ndd_print(NDD_DEBUG, "ndflash->eccbit: %d\n", ndflash->eccbit);
	ndd_print(NDD_DEBUG, "ndflash->planepdie: %d\n", ndflash->planepdie);
	ndd_print(NDD_DEBUG, "ndflash->diepchip: %d\n", ndflash->diepchip);
	ndd_print(NDD_DEBUG, "ndflash->chips: %d\n", ndflash->chips);
	ndd_print(NDD_DEBUG, "ndflash->buswidth: %d\n", ndflash->buswidth);
	ndd_print(NDD_DEBUG, "ndflash->realplanenum: %d\n", ndflash->realplanenum);
	ndd_print(NDD_DEBUG, "ndflash->badblockpos: %d\n", ndflash->badblockpos);
	ndd_print(NDD_DEBUG, "ndflash->rowcycles: %d\n", ndflash->rowcycles);
	ndd_print(NDD_DEBUG, "ndflash->planeoffset: %d\n", ndflash->planeoffset);
	ndd_print(NDD_DEBUG, "ndflash->options: 0x%08x\n", ndflash->options);
}

void __ndd_dump_rbinfo(rb_info *rbinfo)
{
	int rb_index;
	rb_item *rbitem;

	ndd_print(NDD_DEBUG, "\ndump rbinfo, totalrbs = %d !!!\n", rbinfo->totalrbs);
	for (rb_index = 0; rb_index < rbinfo->totalrbs; rb_index++) {
		rbitem = &(rbinfo->rbinfo_table[rb_index]);
		ndd_print(NDD_DEBUG, "id: [%d]\n", rbitem->id);
		ndd_print(NDD_DEBUG, "\t gpio = [%d]\n", rbitem->gpio);
		ndd_print(NDD_DEBUG, "\t irq = [%d]\n", rbitem->irq);
		ndd_print(NDD_DEBUG, "\t pulldown_strength = [%d]\n", rbitem->pulldown_strength);
		ndd_print(NDD_DEBUG, "\t irq_private = [%p]\n", rbitem->irq_private);
	}
}

void __ndd_dump_plat_partition(plat_ptinfo *plat_ptinfo)
{
	int pt_index;
	plat_ptitem *plat_ptitem;

	ndd_print(NDD_DEBUG, "\ndump plat partiton, ptcount = %d !!!\n", plat_ptinfo->ptcount);
	for (pt_index = 0; pt_index < plat_ptinfo->ptcount; pt_index++) {
		plat_ptitem = &(plat_ptinfo->pt_table[pt_index]);
		ndd_print(NDD_DEBUG, "name: [%s]\n", plat_ptitem->name);
		ndd_print(NDD_DEBUG, "\t offset = [%lld]\n", plat_ptitem->offset);
		ndd_print(NDD_DEBUG, "\t size = [%lld]\n", plat_ptitem->size);
		ndd_print(NDD_DEBUG, "\t ops_mode = [%d]\n", plat_ptitem->ops_mode);
		ndd_print(NDD_DEBUG, "\t nm_mode = [%d]\n", plat_ptitem->nm_mode);
		ndd_print(NDD_DEBUG, "\t flags = [%08x]\n", plat_ptitem->flags);
	}
}

void __ndd_dump_ptinfo(pt_info *ptinfo)
{
	int pt_index, part_index;
	ndpartition *pt = ptinfo->pt;
	unsigned int ptcount = ptinfo->ptcount;

	ndd_print(NDD_DEBUG, "\ndump ndpartiton !!!\n");
	ndd_print(NDD_DEBUG, "\traw_boundary: [%d]\n", ptinfo->raw_boundary);
	for (pt_index = 0; pt_index < ptcount; pt_index++) {
		ndd_print(NDD_DEBUG, "partition: [%s]\n", pt[pt_index].name);
		ndd_print(NDD_DEBUG, "\t pagesize = [%d]\n", pt[pt_index].pagesize);
		ndd_print(NDD_DEBUG, "\t pagepblock = [%d]\n", pt[pt_index].pagepblock);
		ndd_print(NDD_DEBUG, "\t blockpvblock = [%d]\n", pt[pt_index].blockpvblock);
		ndd_print(NDD_DEBUG, "\t vblockpgroup = [%d]\n", pt[pt_index].vblockpgroup);
		ndd_print(NDD_DEBUG, "\t groupspzone = [%d]\n", pt[pt_index].groupspzone);
		ndd_print(NDD_DEBUG, "\t startblockid = [%d]\n", pt[pt_index].startblockid);
		ndd_print(NDD_DEBUG, "\t totalblocks = [%d]\n", pt[pt_index].totalblocks);
		ndd_print(NDD_DEBUG, "\t eccbit = [%d]\n", pt[pt_index].eccbit);
		ndd_print(NDD_DEBUG, "\t ops_mode = [%d]\n", (int)pt[pt_index].ops_mode);
		ndd_print(NDD_DEBUG, "\t planes = [%d]\n", (int)pt[pt_index].planes);
		ndd_print(NDD_DEBUG, "\t nm_mode = [%d]\n", (int)pt[pt_index].nm_mode);
		ndd_print(NDD_DEBUG, "\t copy_mode = [%d]\n", (int)pt[pt_index].copy_mode);
		ndd_print(NDD_DEBUG, "\t flags = [0x%08x]\n", (int)pt[pt_index].flags);
		ndd_print(NDD_DEBUG, "\t ndparts_num = [%d]\n", pt[pt_index].ndparts_num);
		ndd_print(NDD_DEBUG, "\t handler = [%d]\n", pt[pt_index].handler);
		for (part_index = 0; part_index < pt[pt_index].ndparts_num; part_index++) {
			ndd_print(NDD_DEBUG, "\t\t part name = [%s]\n", pt[pt_index].ndparts[part_index].name);
			ndd_print(NDD_DEBUG, "\t\t\t part startblockid = [%d]\n", pt[pt_index].ndparts[part_index].startblockID);
			ndd_print(NDD_DEBUG, "\t\t\t part totalblocks = [%d]\n", pt[pt_index].ndparts[part_index].totalblocks);
		}
	}
}

void __ndd_dump_ppartition(PPartition *pt, unsigned int ptcount)
{
	int pt_index, part_index;

	ndd_print(NDD_DEBUG, "\ndump PPartiton !!!\n");
	for (pt_index = 0; pt_index < ptcount; pt_index++) {
		ndd_print(NDD_DEBUG, "partition: [%s]\n", pt[pt_index].name);
		ndd_print(NDD_DEBUG, "\t byteperpage = [%d]\n", pt[pt_index].byteperpage);
		ndd_print(NDD_DEBUG, "\t pageperblock = [%d]\n", pt[pt_index].pageperblock);
		ndd_print(NDD_DEBUG, "\t startblockID = [%d]\n", pt[pt_index].startblockID);
		ndd_print(NDD_DEBUG, "\t totalblocks = [%d]\n", pt[pt_index].totalblocks);
		ndd_print(NDD_DEBUG, "\t PageCount = [%d]\n", pt[pt_index].PageCount);
		ndd_print(NDD_DEBUG, "\t badblockcount = [%d]\n", pt[pt_index].badblockcount);
		ndd_print(NDD_DEBUG, "\t hwsector = [%d]\n", pt[pt_index].hwsector);
		ndd_print(NDD_DEBUG, "\t startPage = [%d]\n", pt[pt_index].startPage);
		ndd_print(NDD_DEBUG, "\t mode = [%d]\n", pt[pt_index].mode);
		ndd_print(NDD_DEBUG, "\t prData = [%p]\n", pt[pt_index].prData);
		ndd_print(NDD_DEBUG, "\t pagespgroup = [%d]\n", pt[pt_index].pagespergroup);
		ndd_print(NDD_DEBUG, "\t groupspzone = [%d]\n", pt[pt_index].groupperzone);
		ndd_print(NDD_DEBUG, "\t parts_num = [%d]\n", pt[pt_index].parts_num);
		ndd_print(NDD_DEBUG, "\t flags = [0x%08x]\n", pt[pt_index].flags);
		for (part_index = 0; part_index < pt[pt_index].parts_num; part_index++) {
			ndd_print(NDD_DEBUG, "\t\t part name = [%s]\n", pt[pt_index].parts[part_index].name);
			ndd_print(NDD_DEBUG, "\t\t\t part startblockid = [%d]\n", pt[pt_index].parts[part_index].startblockID);
			ndd_print(NDD_DEBUG, "\t\t\t part totalblocks = [%d]\n", pt[pt_index].parts[part_index].totalblocks);
		}

		if (pt[pt_index].pt_badblock_info) {
			ndd_print(NDD_DEBUG, "\t bad block table:");
			dump_array((int *)(pt[pt_index].pt_badblock_info), pt[pt_index].badblockcount);
			ndd_print(NDD_DEBUG, "\t available block table:");
			dump_array((int *)(pt[pt_index].pt_availableblockid), pt[pt_index].totalblocks - pt[pt_index].badblockcount);
			ndd_print(NDD_DEBUG, "\n\n");
		}

	}
}

void __ndd_dump_chip_info(chip_info *cinfo)
{
	ndd_print(NDD_DEBUG, "chip info:\n");
	ndd_print(NDD_DEBUG, "\t manuf: 0x%02x\n", cinfo->manuf);
	ndd_print(NDD_DEBUG, "\t page size: %d\n", cinfo->pagesize);
	ndd_print(NDD_DEBUG, "\t oob size: %d\n", cinfo->oobsize);
	ndd_print(NDD_DEBUG, "\t pages per block: %d\n", cinfo->ppblock);
	ndd_print(NDD_DEBUG, "\t blocks per chip: %d\n", cinfo->maxvalidblocks);
	ndd_print(NDD_DEBUG, "\t pages per chip: %d\n", cinfo->maxvalidpages);
	ndd_print(NDD_DEBUG, "\t planes per die: %d\n", cinfo->planepdie);
	ndd_print(NDD_DEBUG, "\t die per chip: %d\n", cinfo->totaldies);
	ndd_print(NDD_DEBUG, "\t original bad block position: %d\n", cinfo->origbadblkpos);
	ndd_print(NDD_DEBUG, "\t bad block position: %d\n", cinfo->badblkpos);
	ndd_print(NDD_DEBUG, "\t bus width: %d\n", cinfo->buswidth);
	ndd_print(NDD_DEBUG, "\t row cycles: %d\n", cinfo->rowcycles);
	ndd_print(NDD_DEBUG, "\t ecc bits: %d\n", cinfo->eccbit);
	ndd_print(NDD_DEBUG, "\t planes count: %d\n", cinfo->planenum);
	ndd_print(NDD_DEBUG, "\t options: %08x\n", cinfo->options);

	ndd_print(NDD_DEBUG, "nand flashs timing info:\n");

#define debug_out_cinfo_timing(x) ndd_print(NDD_DEBUG, "\t t"#x": %d",cinfo->ops_timing.t##x)
	ndd_print(NDD_DEBUG, "\t io_timing = %p\n",cinfo->ops_timing.io_timing);
	ndd_print(NDD_DEBUG, "\t io_etiming = %p\n",cinfo->ops_timing.io_etiming);
	debug_out_cinfo_timing(RP);
	debug_out_cinfo_timing(WP);
	debug_out_cinfo_timing(WHR);
	debug_out_cinfo_timing(WHR2);
	debug_out_cinfo_timing(RR);
	debug_out_cinfo_timing(WB);
	debug_out_cinfo_timing(ADL);
	debug_out_cinfo_timing(CWAW);
	debug_out_cinfo_timing(CS);
	debug_out_cinfo_timing(CLH);
	debug_out_cinfo_timing(WC);
#undef debug_out_cinfo_timing
}

void __ndd_dump_csinfo(cs_info *csinfo)
{
	int i;
	ndd_print(NDD_DEBUG, "cs info:\n");
	ndd_print(NDD_DEBUG, "\t totalchips: %d\n", csinfo->totalchips);
	for (i = 0; i < csinfo->totalchips; i++) {
		ndd_print(NDD_DEBUG, "\t cs = %d, rbid = %d, rbgpio = %d, rbirq = %d, rbirq_private = %p, iomem = %p\n",
			  csinfo->csinfo_table[i].id, csinfo->csinfo_table[i].rbitem->id,
			  csinfo->csinfo_table[i].rbitem->gpio, csinfo->csinfo_table[i].rbitem->irq,
			  csinfo->csinfo_table[i].rbitem->irq_private, csinfo->csinfo_table[i].iomem);
	}
}

void __ndd_dump_registers(void)
{
	ndd_print(NDD_DEBUG, "dump registers:\n");
	ndd_print(NDD_DEBUG, "\t REG_CPMPCR = 0x%08x\n", *((volatile unsigned int *)0xB0000014));
	ndd_print(NDD_DEBUG, "\t REG_CLKGR0 = 0x%08x\n", *((volatile unsigned int *)0xB0000020));
	ndd_print(NDD_DEBUG, "\t REG_SPCR1 = 0x%08x\n", *((volatile unsigned int *)0xB00000BC));
	ndd_print(NDD_DEBUG, "\t REG_BCHCDR = 0x%08x\n", *((volatile unsigned int *)0xB00000AC));
	ndd_print(NDD_DEBUG, "\t REG_NFCSR = 0x%08x\n", *((volatile unsigned int *)0xB3410050));
	ndd_print(NDD_DEBUG, "\t REG_PNCR = 0x%08x\n", *((volatile unsigned int *)0xB3410100));
	ndd_print(NDD_DEBUG, "\t REG_PNDR = 0x%08x\n", *((volatile unsigned int *)0xB3410104));
	ndd_print(NDD_DEBUG, "\t REG_BITCNT = 0x%08x\n", *((volatile unsigned int *)0xB3410108));
	ndd_print(NDD_DEBUG, "\t REG_SMCR1 = 0x%08x\n", *((volatile unsigned int *)0xB3410014));
	ndd_print(NDD_DEBUG, "\t REG_SACR1 = 0x%08x\n", *((volatile unsigned int *)0xB3410034));
	ndd_print(NDD_DEBUG, "\t REG_PAINT = 0x%08x\n", *((volatile unsigned int *)0xb0010010));
	ndd_print(NDD_DEBUG, "\t REG_PAMSK = 0x%08x\n", *((volatile unsigned int *)0xb0010020));
	ndd_print(NDD_DEBUG, "\t REG_PAPAT1 = 0x%08x\n", *((volatile unsigned int *)0xb0010030));
	ndd_print(NDD_DEBUG, "\t REG_PAPAT0 = 0x%08x\n", *((volatile unsigned int *)0xb0010040));
	ndd_print(NDD_DEBUG, "\t REG_PAFLG = 0x%08x\n", *((volatile unsigned int *)0xb0010050));
	ndd_print(NDD_DEBUG, "\t REG_PAPEN = 0x%08x\n", *((volatile unsigned int *)0xb0010070));
}

void __ndd_dump_rewrite(nand_data *nddata, PPartition *ppt, PageList *pl)
{
	int ret;
	unsigned int bytes = ppt->byteperpage;
	struct singlelist *pos;
	PageList *pl_node, ppl_node;
	void *buf;

	buf = ndd_alloc(bytes);
	if (buf) {
		ppl_node.head.next = NULL;
		ppl_node.OffsetBytes = 0;
		ppl_node.Bytes = bytes;
		ppl_node.pData = buf;
		singlelist_for_each(pos, &pl->head) {
			pl_node = singlelist_entry(pos, PageList, head);
			ppl_node.startPageID = pl_node->startPageID;
			ret = nandops_read(nddata->ops_context, (ndpartition *)ppt->prData, &ppl_node);
			if (ret != ALL_FF) {
				ndd_print(NDD_ERROR, "ERROR:%s page [%d] has been written yet, do not write again\n",
					  ppt->name, pl_node->startPageID);
				while (1);
			}
		}
		ndd_free(buf);
	}
}

unsigned char *write_databuf = NULL;
unsigned char *orign_databuf = NULL;
void __ndd_dump_write_reread_prepare(PageList *pl)
{
#ifdef DEBUG_REREAD_WITHDATA
	int offset;
	struct singlelist *pos;
	PageList *pl_node;

	/* alloc a 256K buf to backup original data */
	if (write_databuf == NULL)
		write_databuf = ndd_alloc(256 * 1024);

	ndd_memset(write_databuf, 0xff, 256 * 1024);
	offset = 0;
	singlelist_for_each(pos, &pl->head) {
		pl_node = singlelist_entry(pos, PageList, head);
		ndd_memcpy(write_databuf + offset, pl_node->pData, pl_node->Bytes);
		offset += pl_node->Bytes;
	}
#endif
}

static inline int calc_diff_bit(unsigned char v1, unsigned char v2)
{
	int i, cnt = 0;

	for (i = 0; i < 8; i++)
	{
		if ((v1 & (1 << i)) != (v2 & (1 << i)))
			cnt++;
	}

	return cnt;
}

void __ndd_dump_write_reread_complete(nand_data *nddata, PPartition *ppt, PageList *pl)
{
	int i, ret, offset, errbit;
	unsigned char v1, v2;
	struct singlelist *pos;
	PageList *pl_node;

	/* alloc a 256K buf to backup original data */
	if (orign_databuf == NULL)
		orign_databuf = ndd_alloc(256 * 1024);

	ndd_memset(orign_databuf, 0xff, 256 * 1024);
	offset = 0;
	singlelist_for_each(pos, &pl->head) {
		pl_node = singlelist_entry(pos, PageList, head);
		ndd_memcpy(orign_databuf + offset, pl_node->pData, pl_node->Bytes);
#ifdef DEBUG_REREAD_WITHDATA
		for (i = 0; i < pl_node->Bytes; i++) {
			if (*(orign_databuf + offset + i) != *(write_databuf + offset + i)) {
				ndd_print(NDD_ERROR, "ERROR: write buf has been changed\n");
				ndd_print(NDD_DEBUG, "PageID = %d, Offset = %d, Bytes = %d, pData = %p, retVal = %d\n",
					  pl_node->startPageID, pl_node->OffsetBytes, pl_node->Bytes,
					  pl_node->pData, pl_node->retVal);

				ndd_print(NDD_DEBUG, "============= orign write data:");
				for (i = 0; i < pl_node->Bytes; i++) {
					v1 = (unsigned char)(*((unsigned char *)write_databuf + offset + i));
					if (!(i % 16))
						ndd_print(NDD_DEBUG, "\n%04d: ", i / 16);
					ndd_print(NDD_DEBUG, " %02x", v1);
				}
				ndd_print(NDD_DEBUG, "\n============= data after write ops:");
				for (i = 0; i < pl_node->Bytes; i++) {
					v2 = (unsigned char)(*((unsigned char *)orign_databuf + offset + i));
					if (!(i % 16))
						ndd_print(NDD_DEBUG, "\n%04d: ", i / 16);
					ndd_print(NDD_DEBUG, " %02x", v2);
				}
				ndd_print(NDD_DEBUG, "\n");

				while (1);
			}
		}
#endif
		offset += pl_node->Bytes;
		pl_node->retVal = 0;
	}

	offset = 0;
	ret = nandops_read(nddata->ops_context, (ndpartition *)ppt->prData, pl);
	if (ret < 0) {
		ndd_print(NDD_ERROR, "ERROR:%s write failed, reread ret = %d\n", ppt->name, ret);
		singlelist_for_each(pos, &pl->head) {
			pl_node = singlelist_entry(pos, PageList, head);
			ndd_print(NDD_DEBUG, "PageID = %d, Offset = %d, Bytes = %d, pData = %p, retVal = %d\n",
				  pl_node->startPageID, pl_node->OffsetBytes, pl_node->Bytes,
				  pl_node->pData, pl_node->retVal);
			if (pl_node->retVal == ECC_ERROR) {
				errbit = 0;
				ndd_print(NDD_DEBUG, "============= orign data:");
				for (i = 0; i < pl_node->Bytes; i++) {
					v1 = (unsigned char)(*((unsigned char *)orign_databuf + offset + i));
					if (!(i % 16))
						ndd_print(NDD_DEBUG, "\n%04d: ", i / 16);
					ndd_print(NDD_DEBUG, " %02x", v1);
				}
				ndd_print(NDD_DEBUG, "\n============= read data:");
				for (i = 0; i < pl_node->Bytes; i++) {
					v1 = (unsigned char)(*((unsigned char *)orign_databuf + offset + i));
					v2 = (unsigned char)(*((unsigned char *)pl_node->pData + i));
					errbit += calc_diff_bit(v1, v2);
					if (!(i % 16))
						ndd_print(NDD_DEBUG, "\n%04d: ", i / 16);
					ndd_print(NDD_DEBUG, " %02x", v2);
				}
				ndd_print(NDD_DEBUG, "\nERROR_BIT = %d\n", errbit);
			}
			offset += pl_node->Bytes;
		}
		while (1);
	} else {
#ifdef DEBUG_REREAD_WITHDATA
		singlelist_for_each(pos, &pl->head) {
			errbit = 0;
			pl_node = singlelist_entry(pos, PageList, head);
			for (i = 0; i < pl_node->Bytes; i++) {
				v1 = (unsigned char)(*((unsigned char *)orign_databuf + offset + i));
				v2 = (unsigned char)(*((unsigned char *)pl_node->pData + i));
				errbit += calc_diff_bit(v1, v2);
			}
			if (errbit) {
				ndd_print(NDD_ERROR, "error data has not corrected, ERROR_BIT = %d\n", errbit);
				ndd_print(NDD_DEBUG, "PageID = %d, Offset = %d, Bytes = %d, pData = %p, retVal = %d\n",
					  pl_node->startPageID, pl_node->OffsetBytes, pl_node->Bytes,
					  pl_node->pData, pl_node->retVal);
				ndd_print(NDD_DEBUG, "============= orign data:");
				for (i = 0; i < pl_node->Bytes; i++) {
					v1 = (unsigned char)(*((unsigned char *)orign_databuf + offset + i));
					if (!(i % 16))
						ndd_print(NDD_DEBUG, "\n%04d: ", i / 16);
					ndd_print(NDD_DEBUG, " %02x", v1);
				}
				ndd_print(NDD_DEBUG, "\n============= read data:");
				for (i = 0; i < pl_node->Bytes; i++) {
					v2 = (unsigned char)(*((unsigned char *)pl_node->pData + i));
					if (!(i % 16))
						ndd_print(NDD_DEBUG, "\n%04d: ", i / 16);
					ndd_print(NDD_DEBUG, " %02x", v2);
				}
				ndd_print(NDD_DEBUG, "\n");
				while (1);
			}
			offset += pl_node->Bytes;
		}
#endif
	}

	singlelist_for_each(pos, &pl->head) {
		pl_node = singlelist_entry(pos, PageList, head);
		pl_node->retVal = 0;
	}
}

void __ndd_dump_uncorrect_err(nand_data *nddata, PPartition *ppt, PageList *pl)
{
	int i;
	unsigned int bytes = pl->Bytes;
	struct singlelist *pos;
	PageList *pl_node, ppl_node;
	void *buf;

	ndd_print(NDD_DEBUG, "DEBUG: dump ecc uncorrect error!\n");
	singlelist_for_each(pos, &pl->head) {
		pl_node = singlelist_entry(pos, PageList, head);
		ndd_print(NDD_DEBUG, "startPageID = %d, OffsetBytes = %d, Bytes = %d, pData = %p, retVal = %d\n",
			  pl_node->startPageID, pl_node->OffsetBytes, pl_node->Bytes, pl_node->pData, pl_node->retVal);
	}

	buf = ndd_alloc(bytes);
	if (buf) {
		ppl_node.head.next = NULL;
		ppl_node.OffsetBytes = 0;
		ppl_node.Bytes = bytes;
		ppl_node.pData = buf;
		ppl_node.retVal = 0;
		singlelist_for_each(pos, &pl->head) {
			pl_node = singlelist_entry(pos, PageList, head);
			if (pl_node->retVal == ECC_ERROR) {
				for (i = 0; i <= 16; i++) {
					ppl_node.startPageID = pl_node->startPageID + 8 - i;
					if ((int)ppl_node.startPageID < 0)
						break;
					nandops_read(nddata->ops_context, (ndpartition *)ppt->prData, &ppl_node);
					ndd_print(NDD_DEBUG, "additional read. startPageID = %d, retVal = %d\n",
						  ppl_node.startPageID, ppl_node.retVal);
				}
			}
		}
		ndd_free(buf);
	}
}

int __ndd_dump_check_used_block(nand_data *nddata, PPartition *ppt, int blockid)
{
	int i, ret = 0;
	void *buf;
	PageList ppl_node;
	unsigned int bytes = ppt->byteperpage;
	unsigned int pages = ppt->pageperblock;
	unsigned int startpage = blockid * ppt->pageperblock;

	buf = ndd_alloc(bytes);
	if (buf) {
		ppl_node.head.next = NULL;
		ppl_node.startPageID = startpage;
		ppl_node.OffsetBytes = 0;
		ppl_node.Bytes = bytes;
		ppl_node.pData = buf;
		ppl_node.retVal = 0;

		ret = nandops_read(nddata->ops_context, (ndpartition *)ppt->prData, &ppl_node);
		if (ret == ALL_FF) {
		for (i = 0; i < pages; i++) {
			ppl_node.startPageID = startpage + i;
			__wp_disable(nddata->gpio_wp);
			ret = nandops_write(nddata->ops_context, (ndpartition *)ppt->prData, &ppl_node);
			__wp_enable(nddata->gpio_wp);
			if (ret)
				break;
			ret = nandops_read(nddata->ops_context, (ndpartition *)ppt->prData, &ppl_node);
			if (ret)
				break;
		}
		if (ret < 0)
			ndd_print(NDD_DEBUG, "%s, blockid id check error, pageid = %d, ret = %d\n",
				  ppt->name, ppl_node.startPageID, ret);
		} else
			ret = 0;

		ndd_free(buf);
	}

	return ret;
}

static int erase_ndpartition(nand_data *nddata, ndpartition *npt)
{
	int i, ret;
	int blocks = npt->totalblocks;
	BlockList bl_node = {
		.head = {
			.next = NULL,
		},
		.startBlock = 0,
		.BlockCount = 1,
	};

	for (i = 0; i < blocks; i++) {
		bl_node.startBlock = i;
		__wp_disable(nddata->gpio_wp);
		ret = nandops_erase(nddata->ops_context, npt, &bl_node);
		__wp_enable(nddata->gpio_wp);
		if (ret)
			RETURN_ERR(ret, "erase block [%d] of [%s] error!\n", i, npt->name);
	}

	return 0;
}

static int fill_ndpartition(nand_data *nddata, ndpartition *npt, unsigned char buf_value)
{
	int i, j, ret;
	int pagesize = npt->pagesize;
	int pages = npt->pagepblock;
	int blocks = npt->totalblocks;
	char *buf = ndd_alloc(pagesize);
	PageList pl_node = {
		.head = {
			.next = NULL,
		},
		.startPageID = 0,
		.OffsetBytes = 0,
		.Bytes = npt->pagesize,
		.pData = buf,
		.retVal = 0,
	};

	ndd_memset(buf, buf_value, pagesize);
	for (i = 0; i < blocks; i++) {
		for (j = 0; j < pages; j++) {
			pl_node.startPageID = i * pages + j;
			__wp_disable(nddata->gpio_wp);
			ret = nandops_write(nddata->ops_context, npt, &pl_node);
			__wp_enable(nddata->gpio_wp);
			if (ret) {
				ndd_free(buf);
				RETURN_ERR(ret, "write page [%d] in block [%d] of [%s] error!\n",
					   pl_node.startPageID, i, npt->name);
			}
		}
	}

	ndd_free(buf);

	return 0;
}

static int read_ndpartition_with_errbit(nand_data *nddata, ndpartition *npt, unsigned char buf_value)
{
	int i, j, k;
	unsigned int errbit = 0;
	int pagesize = npt->pagesize;
	int pages = npt->pagepblock;
	int blocks = npt->totalblocks;
	char *buf = ndd_alloc(pagesize);
	PageList pl_node = {
		.head = {
			.next = NULL,
		},
		.startPageID = 0,
		.OffsetBytes = 0,
		.Bytes = npt->pagesize,
		.pData = buf,
		.retVal = 0,
	};

	ndd_memset(buf, buf_value, pagesize);
	for (i = 0; i < blocks; i++) {
		for (j = 0; j < pages; j++) {
			pl_node.startPageID = i * pages + j;
			nandops_read(nddata->ops_context, npt, &pl_node);
			for (k = 0; k < pagesize; k++)
				errbit += calc_diff_bit(buf[k], buf_value);
		}
	}

	ndd_free(buf);

	return errbit;
}

/**
 * here we tested used ndpartition 'nderror'
 **/
extern int __set_features(int io_context, rb_info *rbinfo, nand_ops_timing *timing,
		   unsigned char addr, unsigned char *data, int len);
int __ndd_dump_diff_mode_errbit(nand_data *nddata)
{
	int i, j, mode, ret = 0;
	unsigned char buf_value[8] = {0x00, 0xff, 0xaa, 0x55,
				      0x55, 0xaa, 0xff, 0x00};
	pt_info *ptinfo = nddata->ptinfo;
	ndpartition *npt = &ptinfo->pt[ptinfo->ptcount - 2];
	unsigned int sum_eccbit = 0;
	int io_context;

	nand_ops_timing *timing = &nddata->cinfo->ops_timing;
	unsigned char data[4] = {0x00, 0x00, 0x00, 0x00};
	/* MT29F64GCBABA, mode 4/3, mode 2, mode 1, mode 0, default */
	//unsigned int smcr_value[5] = {
	//	0x15330100, 0x15441200, 0x15551400, 0x28AA3900, 0x3fffff00,
	//};
	unsigned char arg_table[20][2] = {
		{0x03, 0x04}, {0x03, 0x03}, {0x03, 0x02}, {0x03, 0x01}, {0x03, 0x00},
		{0x02, 0x04}, {0x02, 0x03}, {0x02, 0x02}, {0x02, 0x01}, {0x02, 0x00},
		{0x01, 0x04}, {0x01, 0x03}, {0x01, 0x02}, {0x01, 0x01}, {0x01, 0x00},
		{0x00, 0x04}, {0x00, 0x03}, {0x00, 0x02}, {0x00, 0x01}, {0x00, 0x00},
	};

	io_context = nand_io_open(&(nddata->base->nfi), nddata->cinfo);
	for (mode = 0; mode < 20/* * 5*/; mode++) {
		nand_io_chip_select(io_context, 0);
		/* set driver strength */
		data[0] = arg_table[mode/* / 5*/][0];
		__set_features(io_context, nddata->rbinfo,
			       timing, 0x10, data, 4);
		/* set timing mode */
		data[0] = arg_table[mode/* / 5*/][1];
		__set_features(io_context, nddata->rbinfo,
			       timing, 0x01, data, 4);
		nand_io_chip_deselect(io_context, 0);
		ndd_ndelay(10000);
		/* set timing */
		//*((volatile unsigned int *)0xB3410014) = smcr_value[mode % 5];

		ndd_print(NDD_DEBUG, "DEBUG: driver strength = 0x%02x, timimg mode = 0x%02x, smcr = 0x%08x\n",
			  arg_table[mode/* / 5*/][0], arg_table[mode/* / 5*/][1], *((volatile unsigned int *)0xB3410014));

		for (i = 0; i < 8; i++) {
			/* erase partition */
			ret = erase_ndpartition(nddata, npt);
			if (ret)
				break;

			/* fill partition */
			ret = fill_ndpartition(nddata, npt, buf_value[i]);
			if (ret)
				break;

			/* check block */
			for (j = 0; j < 5; j++)
				sum_eccbit += read_ndpartition_with_errbit(nddata, npt, buf_value[i]);

			ndd_print(NDD_DEBUG, "DEBUG: blocks = %d, buf_value = 0x%02x, errbit = %d\n",
				  npt->totalblocks, buf_value[i], sum_eccbit);

			sum_eccbit = 0;
		}
	}

	nand_io_close(io_context);
	erase_ndpartition(nddata, npt);

	return 0;
}

static void dump_taskmsg_ops(union taskmsghead *ops)
{
    ndd_print(NDD_DEBUG, "/****************************/\n");
    ndd_print(NDD_DEBUG, "type = ");
    switch(ops->bits.type){
        case MSG_MCU_INIT:
            ndd_print(NDD_DEBUG, "MSG_MCU_INIT\n");
            break;
        case MSG_MCU_PREPARE:
            ndd_print(NDD_DEBUG, "MSG_MCU_PREPARE\n");
            break;
        case MSG_MCU_CMD:
            ndd_print(NDD_DEBUG, "MSG_MCU_CMD\n");
            break;
        case MSG_MCU_BADBLOCK:
            ndd_print(NDD_DEBUG, "MSG_MCU_BADBLOCK\n");
            break;
        case MSG_MCU_DATA:
            ndd_print(NDD_DEBUG, "MSG_MCU_DATA\n");
            break;
        case MSG_MCU_RET:
            ndd_print(NDD_DEBUG, "MSG_MCU_RET\n");
            break;
        case MSG_MCU_PARITY:
            ndd_print(NDD_DEBUG, "MSG_MCU_PARITY\n");
            break;
        default:
            ndd_print(NDD_DEBUG, "ERROR!!!!\n");
            break;
    }
    ndd_print(NDD_DEBUG, "model = ");
    switch(ops->bits.model){
        case MCU_NO_RB:
            ndd_print(NDD_DEBUG, "MCU_NO_RB\n");
            break;
        case MCU_WITH_RB:
            ndd_print(NDD_DEBUG, "MCU_WITH_RB\n");
            break;
        case MCU_READ_DATA:
            ndd_print(NDD_DEBUG, "MCU_READ_DATA\n");
            break;
        case MCU_WRITE_DATA_WAIT:
            ndd_print(NDD_DEBUG, "MCU_WRITE_DATA_WAIT\n");
            break;
        case MCU_WRITE_DATA:
            ndd_print(NDD_DEBUG, "MCU_WRITE_DATA\n");
            break;
        case MCU_ISBADBLOCK:
            ndd_print(NDD_DEBUG, "MCU_ISBADBLOCK\n");
            break;
        case MCU_MARKBADBLOCK:
            ndd_print(NDD_DEBUG, "MCU_MARKBADBLOCK\n");
            break;
        default:
            ndd_print(NDD_DEBUG, "ERROR!!!!\n");
            break;
    }
    ndd_print(NDD_DEBUG, "state = ");
    switch(ops->bits.state){
        case CHAN1_DATA:
            ndd_print(NDD_DEBUG, "CHAN1_DATA\n");
            break;
        case CHAN1_PARITY:
            ndd_print(NDD_DEBUG, "CHAN1_PARITY\n");
            break;
        case BCH_PREPARE:
            ndd_print(NDD_DEBUG, "BCH_PREPARE\n");
            break;
        case BCH_FINISH:
            ndd_print(NDD_DEBUG, "BCH_FINISH\n");
            break;
        case CHAN2_DATA:
            ndd_print(NDD_DEBUG, "CHAN2_DATA\n");
            break;
        case CHAN2_FINISH:
            ndd_print(NDD_DEBUG, "CHAN2_FINISH\n");
            break;
        default:
            ndd_print(NDD_DEBUG, "ERROR!!!!\n");
            break;
    }
    ndd_print(NDD_DEBUG, "chipsel = %d\n",ops->bits.chipsel);
    ndd_print(NDD_DEBUG, "resource = 0x%x\n",ops->bits.resource);
}

static void dump_data(struct msgdata_data *data)
{
    ndd_print(NDD_DEBUG, "data->offset = %d\n",data->offset);
    ndd_print(NDD_DEBUG, "data->bytes = %d\n",data->bytes);
    ndd_print(NDD_DEBUG, "data->pdata = 0x%x\n",data->pdata);
    ndd_print(NDD_DEBUG, "/****************************/\n");
}

static void dump_cmd(struct msgdata_cmd *cmd)
{
    ndd_print(NDD_DEBUG, "cmd->command = %x\n",cmd->command);
    ndd_print(NDD_DEBUG, "cmd->cmddelay = %d\n",cmd->cmddelay);
    ndd_print(NDD_DEBUG, "cmd->addrdelay = %d\n",cmd->addrdelay);
    ndd_print(NDD_DEBUG, "cmd->offset = %d\n",cmd->offset);
    ndd_print(NDD_DEBUG, "cmd->pageid = %d\n",cmd->pageid);
    ndd_print(NDD_DEBUG, "/****************************/\n");
}

static void dump_prepare(struct msgdata_prepare *prepare)
{
    ndd_print(NDD_DEBUG, "prepare->unit = %d\n",prepare->unit);
    ndd_print(NDD_DEBUG, "prepare->eccbit = %d\n",prepare->eccbit);
    ndd_print(NDD_DEBUG, "prepare->totaltasknum = %d\n",prepare->totaltasknum);
    ndd_print(NDD_DEBUG, "prepare->retnum = %d\n",prepare->retnum);
    ndd_print(NDD_DEBUG, "/****************************/\n");
}

static void dump_ret(struct msgdata_ret *ret)
{
    ndd_print(NDD_DEBUG, "ret->bytes = %d\n",ret->bytes);
    ndd_print(NDD_DEBUG, "ret->retaddr = %x\n",ret->retaddr);
    ndd_print(NDD_DEBUG, "/****************************/\n");
}

static void dump_parity(struct msgdata_parity *parity)
{
    ndd_print(NDD_DEBUG, "parity->offset = %d\n",parity->offset);
    ndd_print(NDD_DEBUG, "parity->bytes = %d\n",parity->bytes);
    ndd_print(NDD_DEBUG, "parity->parityaddr = %x\n",parity->parityaddr);
    ndd_print(NDD_DEBUG, "/****************************/\n");
}

static void dump_badblock(struct msgdata_badblock *badblock)
{
    ndd_print(NDD_DEBUG, "badblock->planes = %d\n",badblock->planes);
    ndd_print(NDD_DEBUG, "badblock->blockid = %d\n",badblock->blockid);
    ndd_print(NDD_DEBUG, "/****************************/\n");
}

void __ndd_dump_taskmsg(struct task_msg *msg, int num)
{
    union taskmsghead *ops;
    union taskmsgdata *msgdata;
    int i;
    ndd_print(NDD_DEBUG, "**** the total count of taskmsg is %d ****\n",num);
    for(i = 0; i < num; i++){
        ops = &((msg + i)->ops);
        msgdata = &((msg + i)->msgdata);
        dump_taskmsg_ops(ops);
        switch(ops->bits.type){
            case MSG_MCU_INIT:
            case MSG_MCU_PREPARE:
                    dump_prepare(&(msgdata->prepare));
                    break;
            case MSG_MCU_CMD:
                    dump_cmd(&(msgdata->cmd));
                    break;
            case MSG_MCU_BADBLOCK:
                    dump_badblock(&(msgdata->badblock));
                    break;
            case MSG_MCU_DATA:
                    dump_data(&(msgdata->data));
                    break;
            case MSG_MCU_RET:
                    dump_ret(&(msgdata->ret));
                    break;
            case MSG_MCU_PARITY:
                    dump_parity(&(msgdata->parity));
                    break;
            default:
                ndd_print(NDD_DEBUG, "ERROR!!!!\n");
                break;
        }
    }
}

void __ndd_dump_basic_rw(nand_data *nddata, unsigned int pageid, unsigned int pagesize) {
	extern void (*__wp_enable) (int);
	extern void (*__wp_disable) (int);
	extern int wait_rb_timeout(int cs_index, int timeout);
#define NAND_DATAPORT	0xBB000000
#define NAND_ADDRPORT   0xBB800000
#define NAND_COMMPORT   0xBB400000
#define REG_NEMC_NFCSR	0xB3410050
#define __nand_cmd(n)		(*((volatile unsigned char *)(NAND_COMMPORT)) = (n))
#define __nand_addr(n)		(*((volatile unsigned char *)(NAND_ADDRPORT)) = (n))
#define __nand_write_data8(n)	(*((volatile unsigned char *)(NAND_DATAPORT)) = (n))
#define __nand_read_data8()	(*(volatile unsigned char *)(NAND_DATAPORT))
#define __nand_enable()		(*((volatile unsigned int *)(REG_NEMC_NFCSR)) = 0x3)
#define __nand_disable()	(*((volatile unsigned int *)(REG_NEMC_NFCSR)) = 0x0)
	int i, ret;
	unsigned char *wbuf = ndd_alloc(pagesize);
	unsigned char *rbuf = ndd_alloc(pagesize);

	/*######################## write ######################*/
	__wp_disable(nddata->gpio_wp);
	ndd_ndelay(100);

	for (i = 0; i < pagesize; i++)
		wbuf[i] = i;

	ndd_print(NDD_DEBUG, "WRITE DATA:\n");
	for (i = 0; i < pagesize; i++) {
		if (!(i % 16))
			ndd_print(NDD_DEBUG, "\n%d: ", i / 16);
		ndd_print(NDD_DEBUG, " %02x", ((unsigned char *)wbuf)[i]);
	}
	ndd_print(NDD_DEBUG, "\n");

	__nand_enable();
	ndd_ndelay(100);

	if (pagesize != 512) {
		__nand_cmd(0x00);
		ndd_ndelay(100);
	}

	__nand_cmd(0x80);
	ndd_ndelay(100);

	if (pagesize != 512)
		__nand_addr(0);
	__nand_addr(0);
	for (i = 0; i < 3; i++)
		__nand_addr((pageid >> (i * 8)) & 0xff);
	ndd_ndelay(100);

	for (i = 0; i < pagesize; i++)
		__nand_write_data8(((unsigned char *)wbuf)[i]);
	ndd_ndelay(2000);

	__nand_cmd(0x10);
	ndd_ndelay(100);

	ret = wait_rb_timeout(0, 500);
	if (ret < 0)
		ndd_print(NDD_DEBUG, "program wait rb timeout!\n");

	__nand_cmd(0x70);
	ndd_ndelay(100);

	ndd_print(NDD_DEBUG, "write status = %x\n", __nand_read_data8());

	__nand_disable();

	__wp_enable(nddata->gpio_wp);

	/*######################## read ######################*/
	ndd_memset(rbuf, 0xff, pagesize);

	__nand_enable();
	ndd_ndelay(100);

	__nand_cmd(0x00);
	ndd_ndelay(100);

	if (pagesize != 512)
		__nand_addr(0);
	__nand_addr(0);
	for (i = 0; i < 3; i++)
		__nand_addr((pageid >> (i * 8)) & 0xff);
	ndd_ndelay(100);

	if (pagesize != 512) {
		__nand_cmd(0x30);
		ndd_ndelay(100);
	}

	ret = wait_rb_timeout(0, 500);
	if (ret < 0)
		ndd_print(NDD_DEBUG, "read wait rb timeout!\n");
	ndd_ndelay(100);

	for (i = 0; i < pagesize; i++)
		rbuf[i] = __nand_read_data8();
	ndd_ndelay(100);

	__nand_disable();

	ndd_print(NDD_DEBUG, "READ DATA:\n");
	for (i = 0; i < pagesize; i++) {
		if (!(i % 16))
			ndd_print(NDD_DEBUG, "\n%d: ", i / 16);
		ndd_print(NDD_DEBUG, " %02x", ((unsigned char *)rbuf)[i]);
	}
	ndd_print(NDD_DEBUG, "\n");

	while(1);
}

void __ndd_dump_erase(nand_data *nddata, ndpartition *npt, BlockList *bl)
{
	int i;
	PageList ppl_node;
	static unsigned char *buf = NULL;

	if (npt->nm_mode == SPL_MANAGER)
		return;

	if (buf == NULL)
		buf = ndd_alloc(npt->pagesize);

	ppl_node.head.next = NULL;
	ppl_node.OffsetBytes = 0;
	ppl_node.Bytes = npt->pagesize;
	ppl_node.pData = buf;
	ppl_node.retVal = 0;

	for (i = 0; i < npt->pagepblock * bl->BlockCount; i++) {
		ppl_node.startPageID = (bl->startBlock * npt->pagepblock) + i;
		nandops_read(nddata->ops_context, npt, &ppl_node);
		if (ppl_node.retVal != -6) {
			ndd_print(NDD_ERROR, "ERROR: pt[%s] erase blocklist error!"
				  " startblock = %d, blocks = %d, check page %d return %d!\n",
				  npt->name, bl->startBlock, bl->BlockCount, ppl_node.startPageID, ppl_node.retVal);
			while (1);
		}
	}
}

int ndd_dump_status(void)
{
	ndd_print(NDD_DEBUG, "\nNand Driver print level is [%d]\n", PRINT_LEVEL);
#ifdef DEBUG_INIT_INFO
	ndd_print(NDD_DEBUG, "\t Nand Driver init info debug on!\n");
#endif
#ifdef DEBUG_SPEED
	ndd_print(NDD_DEBUG, "\t Nand Driver speed debug on!\n");
#endif
#ifdef DEBUG_ERR_TSKMSG
	ndd_print(NDD_DEBUG, "\t Nand Driver error tasklist debug on!\n");
#endif
#ifdef DEBUG_REWRITE
	ndd_print(NDD_DEBUG, "\t Nand Driver rewrite debug on!\n");
#endif
#ifdef DEBUG_WRITE_REREAD
	ndd_print(NDD_DEBUG, "\t Nand Driver write reread debug on!\n");
#endif
#ifdef DEBUG_ECCERROR
	ndd_print(NDD_DEBUG, "\t Nand Driver ecc error debug on!\n");
#endif
#ifdef CHECK_USED_BLOCK
	ndd_print(NDD_DEBUG, "\t Nand Driver check used block debug on!\n");
#endif
#ifdef DEBUG_WRITE_PAGE_FULL
	ndd_print(NDD_DEBUG, "\t Nand Driver write page full debug on!\n");
#endif
#ifdef TEST_FULL_PAGE_READ
	ndd_print(NDD_DEBUG, "\t Nand Driver whole page read debug on!\n");
#endif
#ifdef DEBUG_ERR_TSKMSG
	ndd_print(NDD_DEBUG, "\t Nand Driver task msg list debug on!\n");
#endif
	return 0;
}
