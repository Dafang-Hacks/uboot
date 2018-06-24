#ifndef __NAND_IO_H__
#define __NAND_IO_H__

#include <nddata.h>
#include "transadaptor.h"
#include "nand_info.h"

#define SMCR_DEFAULT_VAL       0x3fffff00//0x11444400      //slowest

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0			0x00
#define NAND_CMD_READ_OOB_512   0x50
#define NAND_CMD_RNDOUT			0x05
#define NAND_CMD_STATUS			0x70
#define NAND_CMD_READID			0x90
#define NAND_CMD_2P_READ		0x60
#define NAND_CMD_2P_READSTART	0x30
#define NAND_CMD_READSTART      0x30
#define NAND_CMD_RNDOUTSTART    0xE0

#define NAND_CMD_WRITE			0x80
#define NAND_CMD_RNDIN			0x85
#define NAND_CMD_PROGRAM		0x10
#define NAND_CMD_2P_WRITE1  	0x80
#define NAND_CMD_2P_PROGRAM1 	0x11
#define NAND_CMD_2P_WRITE2      0x81

#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_ERASE2		0xD0
#define NAND_CMD_RESET		0xFF

/* Extended commands for large page devices */
#define NAND_CMD_CACHEDPROG     0x15
#define TWOP_FIRST_OOB            0x00
#define TWOP_SECOND_OOB           0x80

/* Status Bits */
#define NAND_STATUS_FAIL	0x01
#define NAND_STATUS_FAIL_N1	0x02
#define NAND_STATUS_TRUE_READY	0x20
#define NAND_STATUS_READY	0x40
#define NAND_STATUS_WP		0x80

#define NAND_CMD_OFFSET        0x00400000      /* command port offset for unshare mode */
#define NAND_ADDR_OFFSET       0x00800000      /* address port offset for unshare mode */

int nand_io_chip_select(int context, int cs);
int nand_io_chip_deselect(int context, int cs);
int nand_io_send_cmd(int context, unsigned char command, unsigned int delay);
void nand_io_send_addr(int context, int offset, int pageid, unsigned int delay);
void nand_io_send_spec_addr(int context, int addr, unsigned int cycle, unsigned int delay);
int nand_io_send_data(int context, unsigned char *src, unsigned int len);
int nand_io_receive_data(int context, unsigned char *dst, unsigned int len);
void nand_io_setup_default_16bit(nfi_base *base);
int nand_io_send_waitcomplete(int context, chip_info *cinfo);

int nand_io_open(nfi_base *base, chip_info *cinfo);
void nand_io_close(int context);
void pn_enable(int context);
void pn_disable(int context);
void nand_io_counter0_enable(int context);
void nand_io_counter1_enable(int context);
unsigned int nand_io_read_counter(int context);
void nand_io_counter_disable(int context);
char* nand_io_get_clk_name(void);
int nand_io_suspend(void);
int nand_io_resume(void);
#endif
