/*
 * inc/ndmessage.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __NAND_MESSAGE_H__
#define __NAND_MESSAGE_H__

/*
* pagesize : physical page size
* oobsize  : physical oob size
* rowcycle :
* ecclevel : eccbit per step
* eccsize  : data bytes per ECC step
* eccbytes : ecc parity per step
* buswidth : the buswidth of nand chip
* cs[]     : chipselect id
* rb1     : it is number of gpio,which is used as rb1
* rb2     : it is number of gpio,which is used as rb2
* taskmsgaddr : the address of message of task in ddr memory
*/
struct __mcu_nand_info {
	unsigned short pagesize;
	unsigned short oobsize;
	unsigned short eccsize;
	unsigned short eccbytes;
	unsigned short ecclevel;
	unsigned short twhr2;
	unsigned short tcwaw;
	unsigned short tadl;
	unsigned short tcs;
	unsigned short tclh;
	unsigned short tsync;
	unsigned char buswidth;
	unsigned char  rowcycle;
	char cs[4];
	unsigned char rb0;
	unsigned char rb1;
	unsigned int taskmsgaddr;
	unsigned int fcycle;
};
typedef struct __mcu_nand_info McuNandInfo;

struct msgdata_data{
	unsigned int offset :16;
	unsigned int bytes  :16;
	unsigned int pdata;
};

struct msgdata_cmd{
	unsigned int command   :8;
	unsigned int cmddelay  :9;
	unsigned int addrdelay :9;
	unsigned int offset    :6; // the unit is 512 bytes
	unsigned int pageid;
};

struct msgdata_prepare{
	unsigned int unit      :4;
	unsigned int eccbit    :8;
	unsigned short totaltasknum;
	unsigned short retnum;
};

struct msgdata_ret{
	unsigned int		:16;
	unsigned int bytes	:16;
	unsigned int retaddr;
};

struct msgdata_parity{
	unsigned int offset	:16;
	unsigned int bytes	:16;
	unsigned int parityaddr;
};
struct msgdata_badblock{
	unsigned int planes     :4;
	unsigned int blockid;
};
union taskmsgdata{
	struct msgdata_data data;
	struct msgdata_cmd  cmd;
	struct msgdata_prepare prepare;
	struct msgdata_ret  ret;
	struct msgdata_parity parity;
	struct msgdata_badblock badblock;
};

/* the definition of task_msg.ops */
enum nand_opsstate{
	CHAN1_DATA,
	CHAN1_PARITY,
	BCH_PREPARE,
	BCH_FINISH,
	CHAN2_DATA,
	CHAN2_FINISH,
};
enum nand_opsmodel{
	MCU_NO_RB,
	MCU_WITH_RB,
	MCU_READ_DATA,
	MCU_WRITE_DATA_WAIT,
	MCU_WRITE_DATA,
	MCU_ISBADBLOCK,
	MCU_MARKBADBLOCK,
};
enum nand_opstype{
	MSG_MCU_INIT = 1,
	MSG_MCU_PREPARE,
	MSG_MCU_CMD,
	MSG_MCU_BADBLOCK,
	MSG_MCU_DATA,
	MSG_MCU_RET,
	MSG_MCU_PARITY,
};

struct taskmsghead_bits{
	unsigned int type     :3;
	unsigned int model    :3;
	unsigned int state    :3;
	unsigned int chipsel  :4; // the chip's number,while should be selected.
	unsigned int          :3;
	unsigned int resource :16;
};

union taskmsghead{
	struct taskmsghead_bits bits;
	int flag;
};

struct task_msg{
	union taskmsghead ops;
	union taskmsgdata msgdata;
};

struct taskmsg_init {
	union taskmsghead ops;
	McuNandInfo	 info;
};

/* the return value of every task */
#define MSG_RET_SUCCESS 0
#define MSG_RET_WP (0x01<<0)
#define MSG_RET_EMPTY (0x01<<1)
#define MSG_RET_MOVE (0x01<<2)
#define MSG_RET_FAIL (0x01<<3)
/* PDMA MailBox msg */
#define MB_MCU_DONE           (0x1 << 4)
#define MB_MCU_ERROR          (0x5 << 5)

struct __resource{
	unsigned int rb0: 1;
	unsigned int rb1: 1;
	unsigned int nemc: 1;
	unsigned int chan0: 1;
	unsigned int chan1: 1;
	unsigned int chan2: 1;
	unsigned int pipe0: 1;
	unsigned int pipe1: 1;
	unsigned int msgflag: 2;
	unsigned int setrb0: 1;
	unsigned int setrb1: 1;
};
union ops_resource{
	struct __resource bits;
	unsigned int flag;
};
typedef union ops_resource OpsResource;

enum __resource_state{
	REDUCE = -1,
	UNAVAILABLE,
	AVAILABLE,
	INCREASE = 1,
	GOTRB = 0,
	WAITRB = 1,
};
typedef enum __resource_state Rstate;

enum __resource_bit{
	R_RB0,
	R_RB1,
	R_NEMC,
	R_CHAN0,
	R_CHAN1,
	R_CHAN2,
	R_PIPE0,
	R_PIPE1,
	R_MSGFLAG,
	R_SETRB0,
	R_SETRB1,
	R_ALLBITS,
};
typedef enum __resource_bit Rbit;

#endif // __NAND_MESSAGE_H__
