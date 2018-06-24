/*
 * inc/nand_ops.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __NAND_OPS_H__
#define __NAND_OPS_H__
#include "pagelist.h"
#include "blocklist.h"
#include "vnandinfo.h"
#include "nddata.h"
#include "ndmessage.h"
#include "lib_nandopsinterface.h"

struct msg_handler{
	int context;
	int (*handler)(int context, Nand_Task *nt);
};
struct opsunit_handler{
	struct msg_handler *cpu;
	struct msg_handler *dma;
};
/*
*
*/
struct nandops_info{
	unsigned int lib_ops;
	void *mem;
	struct opsunit_handler *handler_unit;
	Nand_Task **nandtask;
	unsigned int unit_cnt;
	struct lib_nandchipinfo *libcinfo;
	struct lib_nandioinfo   *libioinfo;
	nand_data *nanddata;
	PageList *retrytop;
	unsigned int xboot_offset_page;
};
/* get the value of return about command */
int nandops_read(int context, ndpartition *npt, PageList *pl);
int nandops_write(int context, ndpartition *npt, PageList *pl);
int nandops_erase(int context, ndpartition *npt, BlockList *bl);
int nandops_isbadblk(int context, ndpartition *npt, int blkid);
int nandops_markbadblk(int context, ndpartition *npt, int blkid);
int nandops_suspend(int context);
int nandops_resume(int context);
int nandops_init(nand_data *data);
void nandops_deinit(int context);
#endif // __NAND_OPS_H__
