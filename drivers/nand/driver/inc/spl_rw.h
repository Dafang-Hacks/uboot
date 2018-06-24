#ifndef _SPL_RW_H
#define _SPL_RW_H

#include "nand_ops.h"
#include "nddata.h"
#include "ndcommand.h"
#include "pagelist.h"
#include "nand_io.h"
#include "nand_bch.h"

#define SPL_BAK_NUM     1
#define SPL_BCH_SIZE    256
#define SPL_BCH_BIT     64
#define SPL_PAR_SIZE    (SPL_BCH_BIT * 14 / 8)
#define SPL_BCH_ID      0
#define SPL_CS          0

int spl_write(int handle, PageList *pl);
int spl_read(int handle, PageList *pl);
int spl_init(nand_data *data);
void spl_deinit(int handle);

#endif
