/********************** BEGIN LICENSE BLOCK ************************************
 *
 * INGENIC CONFIDENTIAL--NOT FOR DISTRIBUTION IN SOURCE CODE FORM
 * Copyright (c) Ingenic Semiconductor Co. Ltd 2005. All rights reserved.
 *
 * This file, and the files included with this file, is distributed and made
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 *
 * http://www.ingenic.cn
 *
 ********************** END LICENSE BLOCK **************************************/



/*
 * Include file for Ingenic Semiconductor's JZ4760 CPU.
 */

#ifndef __NAND_COMMAND_H__
#define __NAND_COMMAND_H__
#include "ndmessage.h"
#include "lib_nandopsinterface.h"


/* NAND Flash Manufacturer ID Codes */
#define LIB_NAND_MFR_TOSHIBA	0x98	// Toshiba
#define LIB_NAND_MFR_SAMSUNG	0xec	// Samsung
#define LIB_NAND_MFR_FUJITSU	0x04	// Fujitsu
#define LIB_NAND_MFR_NATIONAL	0x8f	// National
#define LIB_NAND_MFR_RENESAS	0x07	// Renesas
#define LIB_NAND_MFR_STMICRO	0x20	// ST Micro
#define LIB_NAND_MFR_HYNIX		0xad	// Hynix
#define LIB_NAND_MFR_MICRON		0x2c	// Micron
#define LIB_NAND_MFR_AMD		0x01	// AMD/Spansion
#define LIB_NAND_MFR_MACRONIX	0xc2	// Macronix
#define LIB_NAND_MFR_EON		0x92	// Eon


#define CMD_READ_ID_1ST				0x90        //  ReadID
#define CMD_OOB_READ_1ST_512				0x50        //  Read oob of 512B pagesize nandflash
#define CMD_PAGE_READ_1ST				0x00        //  Read0
#define CMD_PAGE_READ_2ND				0x30		//	Read confirm
#define CMD_RANDOM_OUTPUT_1ST			0x05
#define CMD_RANDOM_OUTPUT_2ND			0xE0
#define CMD_READ_STATUS_1ST				0x70        //  Read Status
#define CMD_2P_PAGE_READ_1ST		        0x60   //Two-plane Read phase before addr 1
#define CMD_2P_PAGE_READ_2ND          		0x60   //Two-plane Read phase before addr 2
#define CMD_2P_PAGE_READ_3RD          		0x30
#define CMD_2P_RANDOM_OUTPUT          		0x00
//#define CMD_2P_RANDOM_OUTPUT_2ND			0x05
//#define CMD_2P_RANDOM_OUTPUT_3RD			0xE0

#define CMD_PAGE_PROGRAM_1ST			0x80        //  Write phase 1
#define	CMD_PAGE_PROGRAM_2ND			0x10		//  Write phase 2
#define CMD_RANDOM_INPUT_1ST			0x85
#define CMD_2P_PAGE_PROGRAM_1ST       		0x80	//  Two-plane Write phase before addr 1
#define CMD_2P_PAGE_PROGRAM_2ND     		0x11
#define CMD_2P_PAGE_PROGRAM_3RD       		0x81	//  Two-plane Write phase before addr 2
#define CMD_2P_PAGE_PROGRAM_4TH     		0x10

#define CMD_BLOCK_ERASE_1ST				0x60        //  Erase phase 1
#define CMD_BLOCK_ERASE_2ND				0xD0        //  Erase phase 2,Erase confirm
#define CMD_2P_BLOCK_ERASE_1ST			0x60	//  Two-plane erase phase before addr 1
#define CMD_2P_BLOCK_ERASE_2ND			0x60	//  Two-plane erase phase before addr 2
#define CMD_2P_BLOCK_ERASE_3RD			0xD0	//	Two-plane erase confirm

#define CMD_RESET_1ST				0xFF        //  Reset

#define	CMD_SET_FEATURES				0xEF
#define	CMD_GET_FEATURES				0xEE

typedef struct __nand_basic_command {
	/* common command */
	unsigned char read_id;
	unsigned char read_status;
	unsigned char reset_1st;
	unsigned char set_features;
	unsigned char get_features;
	unsigned char readpage_1st;
	unsigned char readpage_2nd;
	unsigned char random_out_1st;
	unsigned char random_out_2nd;
	unsigned char propage_1st;
	unsigned char propage_2nd;
	unsigned char random_in_1st;
	unsigned char erase_1st;
	unsigned char erase_2nd;
} NandBasicCommand;
typedef struct __nand_extend_command {
	/* two-plane command */
	unsigned char tp_readpage_1st;
	unsigned char tp_readpage_2nd;
	unsigned char tp_readpage_3rd;
	unsigned char tp_readpage_4th;
	unsigned char tp_changeplane_1st;
	unsigned char tp_changeplane_2nd;
	unsigned char tp_propage_1st;
	unsigned char tp_propage_2nd;
	unsigned char tp_propage_3rd;
	unsigned char tp_propage_4th;
	unsigned char tp_erase_1st;
	unsigned char tp_erase_2nd;
	unsigned char tp_erase_3rd;
	unsigned char tp_erase_4th;
} NandExtendCommand;

typedef struct __nand_extend_ops {
	/* 2p_read */
	int (* cmd_2p_read)(struct task_msg *, unsigned int, nand_ops_timing *, unsigned int, unsigned int);
	int (* getret_cmd_2p_read)(unsigned char *, unsigned int, unsigned char *);
	int (* cmd_2p_changeplane)(struct task_msg *, unsigned int, nand_ops_timing *, unsigned int);
	int (* getret_cmd_2p_changeplane)(unsigned char *, unsigned int, unsigned char *);
	/* 2p_write */
	int (* cmd_2p_write_page0_start)(struct task_msg *, unsigned int, nand_ops_timing *, unsigned short , unsigned int);
	int (* getret_cmd_2p_write_page0_start)(unsigned char *, unsigned int, unsigned char *);
	int (* cmd_2p_write_page0_confirm)(struct task_msg *, unsigned int,nand_ops_timing *);
	int (* getret_cmd_2p_write_page0_confirm)(unsigned char *, unsigned int, unsigned char *);
	int (* cmd_2p_write_page1_start)(struct task_msg *, unsigned int, nand_ops_timing *, unsigned short, unsigned int);
	int (* getret_cmd_2p_write_page1_start)(unsigned char *, unsigned int, unsigned char *);
	int (* cmd_2p_write_page1_confirm)(struct task_msg *, unsigned int, nand_ops_timing *);
	int (* getret_cmd_2p_write_page1_confirm)(unsigned char *, unsigned int, unsigned char *);
	/* 2p_erase */
	int (* cmd_2p_erase)(struct task_msg *, unsigned int, nand_ops_timing *, unsigned int,unsigned int);
	int (* getret_cmd_2p_erase)(unsigned char *, unsigned int, unsigned char *);

} NandExtendOps;


/* Option constants for bizarre disfunctionality and real
 * features
 */
/* Chip can not auto increment pages */
#define NAND_NO_AUTOINCR        0x00000001
/* Buswitdh is 16 bit */
#define NAND_BUSWIDTH_16        0x00000002
/* Device supports partial programming without padding */
#define NAND_NO_PADDING         0x00000004
/* Chip has cache program function */
#define NAND_CACHEPRG           0x00000008
/* Chip has copy back function */
#define NAND_COPYBACK           0x00000010
/* AND Chip which has 4 banks and a confusing page / block
 * assignment. See Renesas datasheet for further information */
#define NAND_IS_AND             0x00000020
/* Chip has a array of 4 pages which can be read without
 * additional ready /busy waits */
#define NAND_4PAGE_ARRAY        0x00000040
/* Chip requires that BBT is periodically rewritten to prevent
 * bits from adjacent blocks from 'leaking' in altering data.
 * This happens with the Renesas AG-AND chips, possibly others.  */
#define BBT_AUTO_REFRESH        0x00000080
/* Chip does not require ready check on read. True
 * for all large page devices, as they do not support
 * autoincrement.*/
#define NAND_NO_READRDY         0x00000100
/* Chip does not allow subpage writes */
#define NAND_NO_SUBPAGE_WRITE   0x00000200
/* Options valid for Samsung large page devices */
#define NAND_SAMSUNG_LP_OPTIONS \
        (NAND_NO_PADDING | NAND_CACHEPRG | NAND_COPYBACK)

/* 1p_read*/
int command_1p_read(struct task_msg *msg, unsigned int flag, nand_ops_timing *time, unsigned short offset, unsigned int pageid);
int getret_cmd_1p_read(unsigned char *ret, unsigned int index, unsigned char *value);
/* random output */
int command_random_data_output(struct task_msg *msg, unsigned int flag, nand_ops_timing *time, unsigned short offset);
int getret_cmd_random_data_output(unsigned char *ret, unsigned int index, unsigned char *value);
/* 1p_write */
int command_1p_write_start(struct task_msg *msg, unsigned int flag, nand_ops_timing *time, unsigned short, unsigned int);
int getret_cmd_1p_write_start(unsigned char *ret, unsigned int index, unsigned char *value);
int command_1p_write_confirm(struct task_msg *msg, unsigned int flag, nand_ops_timing *time);
int getret_cmd_1p_write_confirm(unsigned char *ret, unsigned int index, unsigned char *value);
/* random input */
int command_random_data_input(struct task_msg *msg, unsigned int flag, nand_ops_timing *time, unsigned short offset);
int getret_cmd_random_data_input(unsigned char *ret, unsigned int index, unsigned char *value);
/* read_status */
int command_read_status(struct task_msg *msg, unsigned int flag, nand_ops_timing *time);
int getret_cmd_read_status(unsigned char *ret, unsigned int index, unsigned char *value);
/*  1p_erase */
int command_1p_erase(struct task_msg *msg, unsigned int flag, nand_ops_timing *time, unsigned int pageid);
int getret_cmd_1p_erase(unsigned char *ret, unsigned int index, unsigned char *value);

int micron_extend_cmd_init(NandExtendOps *ops);
int micron_2_extend_cmd_init(NandExtendOps *ops);
int samsung_extend_cmd_init(NandExtendOps *ops);
int hynix_extend_cmd_init(NandExtendOps *ops);
void nand_extendcommand_init(int context);
void nand_extendcommand_deinit(int context);

#endif // __NAND_COMMAND_H__
