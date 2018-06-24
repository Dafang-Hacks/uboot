/*
 * inc/lib_nandopsinterface.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __LIB_NANDOPSINTERFACE_H__
#define __LIB_NANDOPSINTERFACE_H__
#include "pagelist.h"
#include "blocklist.h"
#include "vnandinfo.h"
#include <nand_ops_timing.h>

enum nandops_opsmodel{
	NANDOPS_READ,
	NANDOPS_WRITE,
	NANDOPS_ERASE,
	NANDOPS_ISBADBLOCK,
	NANDOPS_MARKBADBLOCK,
};
struct nand_task{
	struct task_msg *msg;
	unsigned int msg_phyaddr;
	unsigned char *ret;
	unsigned int ret_phyaddr;
	unsigned int msg_index;
	unsigned int pipeid;
	unsigned int msg_maxcnt;
};
typedef struct nand_task Nand_Task;

/* lib_nandchipinfo->options bitmap */
#define NAND_CACHE_READ         (1 << 0)	// support cache read operation
#define NAND_CACHE_PROGRAM      (1 << 1)	// support page cache program operation
#define NAND_MULTI_READ         (1 << 2)	// support multi-plane page read operation
#define NAND_MULTI_PROGRAM      (1 << 3)	// support multi-plane page program operation
#define NAND_READ_RETRY	        (1 << 8)	// support READ RETRY
#define NAND_MICRON_NORMAL      (1 << 9)	// the command of two-planes read is 00-32-00-30
#define NAND_MICRON_PARTICULAR  (1 << 10)	// the command of two-planes read is 00-00-30


struct lib_nandchipinfo{
	unsigned int manuf;
	unsigned int options;
	unsigned int totalblocks;
	unsigned int totalpages;
	unsigned int pagesize;
	nand_ops_timing *ops_timing;
};

struct lib_nandioinfo{
	unsigned short rb_cnt;
	unsigned short chipprb;
};
/* get the value of return about command */
char set_ret_value(unsigned char *array, unsigned int index, unsigned char value);
char get_ret_value(unsigned char *array, unsigned int index);
int lib_nandops_creat_task(int context,unsigned int planes,unsigned int pagepblock,unsigned int startblockid,
						unsigned int eccbit, unsigned int para,unsigned int model);
int lib_nandops_getret(int context,unsigned int planes,unsigned int pagepblock,unsigned int startblockid,
						unsigned int eccbit, unsigned int para,unsigned int model);
int lib_nandops_init(unsigned int eccsize, unsigned int iocount,Nand_Task **nandtask,void *heap,int heapsize,void *cinfo,void *ioinfo);
void lib_nandops_deinit(int context);
#endif // __NAND_OPS_H__
