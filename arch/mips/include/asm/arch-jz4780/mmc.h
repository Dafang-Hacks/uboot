/*
 * Ingenic JZ MMC support
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __MMC_H__
#define __MMC_H__

#include <asm/arch/base.h>

#define MSC_STRPCL		0x000
#define MSC_STAT		0x004
#define MSC_CLKRT		0x008
#define MSC_CMDAT		0x00C
#define MSC_RESTO		0x010
#define MSC_RDTO		0x014
#define MSC_BLKLEN		0x018
#define MSC_NOB			0x01C
#define MSC_SNOB		0x020
#define MSC_IMASK		0x024
#define MSC_IREG		0x028
#define MSC_CMD			0x02C
#define MSC_ARG			0x030
#define MSC_RES			0x034
#define MSC_RXFIFO		0x038
#define MSC_TXFIFO		0x03C
#define MSC_LPM 		0x040
#define MSC_DBG			0x0fc
#define MSC_DMAC		0x044
#define MSC_DMANDA		0x048
#define MSC_DMADA		0x04c
#define MSC_DMALEN		0x050
#define MSC_DMACMD		0x054
#define MSC_CTRL2		0x058
#define MSC_RTCNT		0x05c

/* MSC Clock and Control Register (MSC_STRPCL) */

#define MSC_STRPCL_EXIT_MULTIPLE	(1 << 7)
#define MSC_STRPCL_EXIT_TRANSFER	(1 << 6)
#define MSC_STRPCL_START_READWAIT	(1 << 5)
#define MSC_STRPCL_STOP_READWAIT	(1 << 4)
#define MSC_STRPCL_RESET		(1 << 3)
#define MSC_STRPCL_START_OP		(1 << 2)
#define MSC_STRPCL_CLOCK_CONTROL_BIT	0
#define MSC_STRPCL_CLOCK_CONTROL_MASK	(0x3 << MSC_STRPCL_CLOCK_CONTROL_BIT)
#define MSC_STRPCL_CLOCK_CONTROL_STOP	  (0x1 << MSC_STRPCL_CLOCK_CONTROL_BIT) /* Stop MMC/SD clock */
#define MSC_STRPCL_CLOCK_CONTROL_START  (0x2 << MSC_STRPCL_CLOCK_CONTROL_BIT) /* Start MMC/SD clock */

/* MSC Status Register (MSC_STAT) */

#define MSC_STAT_AUTO_CMD_DONE		(1 << 31)
#define MSC_STAT_IS_RESETTING		(1 << 15)
#define MSC_STAT_SDIO_INT_ACTIVE	(1 << 14)
#define MSC_STAT_PRG_DONE		(1 << 13)
#define MSC_STAT_DATA_TRAN_DONE		(1 << 12)
#define MSC_STAT_END_CMD_RES		(1 << 11)
#define MSC_STAT_DATA_FIFO_AFULL	(1 << 10)
#define MSC_STAT_IS_READWAIT		(1 << 9)
#define MSC_STAT_CLK_EN			(1 << 8)
#define MSC_STAT_DATA_FIFO_FULL		(1 << 7)
#define MSC_STAT_DATA_FIFO_EMPTY	(1 << 6)
#define MSC_STAT_CRC_RES_ERR		(1 << 5)
#define MSC_STAT_CRC_READ_ERROR		(1 << 4)
#define MSC_STAT_CRC_WRITE_ERROR_BIT	2
#define MSC_STAT_CRC_WRITE_ERROR_MASK	(0x3 << MSC_STAT_CRC_WRITE_ERROR_BIT)
#define MSC_STAT_CRC_WRITE_ERROR_NO		(0 << MSC_STAT_CRC_WRITE_ERROR_BIT) /* No error on transmission of data */
#define MSC_STAT_CRC_WRITE_ERROR		(1 << MSC_STAT_CRC_WRITE_ERROR_BIT) /* Card observed erroneous transmission of data */
#define MSC_STAT_CRC_WRITE_ERROR_NOSTS	(2 << MSC_STAT_CRC_WRITE_ERROR_BIT) /* No CRC status is sent back */
#define MSC_STAT_TIME_OUT_RES		(1 << 1)
#define MSC_STAT_TIME_OUT_READ		(1 << 0)

/* MSC Bus Clock Control Register (MSC_CLKRT) */

#define MSC_CLKRT_CLK_RATE_BIT		0
#define MSC_CLKRT_CLK_RATE_MASK		(0x7 << MSC_CLKRT_CLK_RATE_BIT)
#define MSC_CLKRT_CLK_RATE_DIV_1	  (0x0 << MSC_CLKRT_CLK_RATE_BIT) /* CLK_SRC */
#define MSC_CLKRT_CLK_RATE_DIV_2	  (0x1 << MSC_CLKRT_CLK_RATE_BIT) /* 1/2 of CLK_SRC */
#define MSC_CLKRT_CLK_RATE_DIV_4	  (0x2 << MSC_CLKRT_CLK_RATE_BIT) /* 1/4 of CLK_SRC */
#define MSC_CLKRT_CLK_RATE_DIV_8	  (0x3 << MSC_CLKRT_CLK_RATE_BIT) /* 1/8 of CLK_SRC */
#define MSC_CLKRT_CLK_RATE_DIV_16	  (0x4 << MSC_CLKRT_CLK_RATE_BIT) /* 1/16 of CLK_SRC */
#define MSC_CLKRT_CLK_RATE_DIV_32	  (0x5 << MSC_CLKRT_CLK_RATE_BIT) /* 1/32 of CLK_SRC */
#define MSC_CLKRT_CLK_RATE_DIV_64	  (0x6 << MSC_CLKRT_CLK_RATE_BIT) /* 1/64 of CLK_SRC */
#define MSC_CLKRT_CLK_RATE_DIV_128	  (0x7 << MSC_CLKRT_CLK_RATE_BIT) /* 1/128 of CLK_SRC */

/* MSC Command Sequence Control Register (MSC_CMDAT) */

#define MSC_CMDAT_IO_ABORT		(1 << 11)
#define MSC_CMDAT_BUS_WIDTH_BIT		9
#define MSC_CMDAT_BUS_WIDTH_MASK	(0x3 << MSC_CMDAT_BUS_WIDTH_BIT)
#define MSC_CMDAT_BUS_WIDTH_1BIT	  (0x0 << MSC_CMDAT_BUS_WIDTH_BIT) /* 1-bit data bus */
#define MSC_CMDAT_BUS_WIDTH_4BIT	  (0x2 << MSC_CMDAT_BUS_WIDTH_BIT) /* 4-bit data bus */
#define CMDAT_BUS_WIDTH1	  (0x0 << MSC_CMDAT_BUS_WIDTH_BIT)
#define CMDAT_BUS_WIDTH4	  (0x2 << MSC_CMDAT_BUS_WIDTH_BIT)
#define MSC_CMDAT_DMA_EN		(1 << 8)
#define MSC_CMDAT_INIT			(1 << 7)
#define MSC_CMDAT_BUSY			(1 << 6)
#define MSC_CMDAT_STREAM_BLOCK		(1 << 5)
#define MSC_CMDAT_WRITE			(1 << 4)
#define MSC_CMDAT_READ			(0 << 4)
#define MSC_CMDAT_DATA_EN		(1 << 3)
#define MSC_CMDAT_RESPONSE_BIT	0
#define MSC_CMDAT_RESPONSE_MASK	(0x7 << MSC_CMDAT_RESPONSE_BIT)
#define MSC_CMDAT_RESPONSE_NONE  (0x0 << MSC_CMDAT_RESPONSE_BIT) /* No response */
#define MSC_CMDAT_RESPONSE_R1	  (0x1 << MSC_CMDAT_RESPONSE_BIT) /* Format R1 and R1b */
#define MSC_CMDAT_RESPONSE_R2	  (0x2 << MSC_CMDAT_RESPONSE_BIT) /* Format R2 */
#define MSC_CMDAT_RESPONSE_R3	  (0x3 << MSC_CMDAT_RESPONSE_BIT) /* Format R3 */
#define MSC_CMDAT_RESPONSE_R4	  (0x4 << MSC_CMDAT_RESPONSE_BIT) /* Format R4 */
#define MSC_CMDAT_RESPONSE_R5	  (0x5 << MSC_CMDAT_RESPONSE_BIT) /* Format R5 */
#define MSC_CMDAT_RESPONSE_R6	  (0x6 << MSC_CMDAT_RESPONSE_BIT) /* Format R6 */

#define CMDAT_DMA_EN	(1 << 8)
#define CMDAT_INIT	(1 << 7)
#define CMDAT_BUSY	(1 << 6)
#define CMDAT_STREAM	(1 << 5)
#define CMDAT_WRITE	(1 << 4)
#define CMDAT_DATA_EN	(1 << 3)

/* MSC Interrupts Mask Register (MSC_IMASK) */

#define MSC_IMASK_TIME_OUT_RES (1 << 9)
#define MSC_IMASK_TIME_OUT_READ (1 << 8)
#define MSC_IMASK_SDIO			(1 << 7)
#define MSC_IMASK_TXFIFO_WR_REQ		(1 << 6)
#define MSC_IMASK_RXFIFO_RD_REQ		(1 << 5)
#define MSC_IMASK_END_CMD_RES		(1 << 2)
#define MSC_IMASK_PRG_DONE		(1 << 1)
#define MSC_IMASK_DATA_TRAN_DONE	(1 << 0)

/* MSC Interrupts Status Register (MSC_IREG) */

#define MSC_IREG_TIME_OUT_RES (1 << 9)
#define MSC_IREG_TIME_OUT_READ (1 << 8)
#define MSC_IREG_SDIO			(1 << 7)
#define MSC_IREG_TXFIFO_WR_REQ		(1 << 6)
#define MSC_IREG_RXFIFO_RD_REQ		(1 << 5)
#define MSC_IREG_END_CMD_RES		(1 << 2)
#define MSC_IREG_PRG_DONE		(1 << 1)
#define MSC_IREG_DATA_TRAN_DONE		(1 << 0)

#define LPM_DRV_SEL_SHF			30
#define LPM_DRV_SEL_MASK		(0x3 << LPM_DRV_SEL_SHF)
#define LPM_SMP_SEL			(1 << 29)
#define LPM_LPM				(1 << 0)

void jz_mmc_init(void);

#endif /* __MMC_H__ */
