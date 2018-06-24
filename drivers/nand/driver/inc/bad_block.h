#ifndef __BAD_BLOCK_H
#define __BAD_BLOCK_H

#include "nand_debug.h"
#include "ndmessage.h"
#include "nand_ops.h"
#include "nand_io.h"
#include "nand_info.h"
#include "ndcommand.h"

int is_bad_block(struct nandops_info *ops, Nand_Task *nandtask);
int mark_bad_block(struct nandops_info *ops, Nand_Task *nandtask);
#endif
