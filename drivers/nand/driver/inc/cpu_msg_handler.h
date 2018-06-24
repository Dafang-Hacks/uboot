#ifndef __CPU_MSG_HANDLER_H
#define __CPU_MSG_HANDLER_H

#include "ndmessage.h"
#include "nddata.h"
#include "nand_io.h"
#include "nand_bch.h"
#include "ndcommand.h"
#include "nand_ops.h"
#include "lib_nandopsinterface.h"
#include "nand_debug.h"
/*
#define SUCCESS         0
#define FAIL            -1
#define WRITE_WP        -3
#define TIMEOUT         -4
#define ECCERR          -5
#define ALL_FF          -6
#define BLOCKMOVE       1
*/
#define NAND_STATUS_FAIL	0x01
#define NAND_STATUS_FAIL_N1	0x02
#define NAND_STATUS_TRUE_READY	0x20
#define NAND_STATUS_READY	0x40
#define NAND_STATUS_WP		0x80

int cpu_msg_handle_suspend(int handle);
int cpu_msg_handle_resume(int handle);
int cpu_msg_handle_init(nand_data *data, Nand_Task *nandtask, int id);
void cpu_msg_handle_deinit(int handle);

#endif
