/*
 * JZ4780 ddr definitions
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

#ifndef __DDR_H__
#define __DDR_H__

#include <asm/arch/base.h>

/*************************************************************************
 * DDRC (DDR Controller)
 *************************************************************************/
#define DDR_MEM_PHY_BASE		0x20000000


#define DDRC_STATUS			0x0
#define DDRC_CFG			0x4
#define DDRC_CTRL			0x8
#define DDRC_LMR			0xc
#define DDRC_REFCNT			0x18
#define DDRC_MMAP0			0x24
#define DDRC_MMAP1			0x28
#define DDRC_DLP			0xbc
#define DDRC_STRB			0x34
#define DDRC_AUTOSR_CNT                 0x308
#define DDRC_AUTOSR_EN		        0x304

#define DDRC_CLKSTP_CFG                 (DDR_PHY_OFFSET + 0x1000 + 0x68)

#define DDRC_TIMING(n)			(0x60 + 4 * (n - 1))
#define DDRC_REMAP(n)			(0x9c + 4 * (n - 1))

#define DDRP_PIR	(DDR_PHY_OFFSET + 0x4) /* PHY Initialization Register */
#define DDRP_PGCR	(DDR_PHY_OFFSET + 0x8) /* PHY General Configuration Register*/
#define DDRP_PGSR	(DDR_PHY_OFFSET + 0xc) /* PHY General Status Register*/
#define DDRP_DLLGCR	(DDR_PHY_OFFSET + 0x10) /* DLL General Control Register*/
#define DDRP_ACDLLCR	(DDR_PHY_OFFSET + 0x14) /* AC DLL Control Register*/

#define DDRP_PTR0	(DDR_PHY_OFFSET + 0x18) /* PHY Timing Register 0 */
#define DDRP_PTR1	(DDR_PHY_OFFSET + 0x1c) /* PHY Timing Register 1 */
#define DDRP_PTR2	(DDR_PHY_OFFSET + 0x20) /* PHY Timing Register 2 */

#define DDRP_ACIOCR	(DDR_PHY_OFFSET + 0x24) /* AC I/O Configuration Register */
#define DDRP_DXCCR	(DDR_PHY_OFFSET + 0x28) /* DATX8 Common Configuration Register */
#define DDRP_DSGCR	(DDR_PHY_OFFSET + 0x2c) /* DDR System General Configuration Register */
#define DDRP_DCR	(DDR_PHY_OFFSET + 0x30) /* DRAM Configuration Register*/

#define DDRP_DTPR0	(DDR_PHY_OFFSET + 0x34) /* DRAM Timing Parameters Register 0 */
#define DDRP_DTPR1	(DDR_PHY_OFFSET + 0x38) /* DRAM Timing Parameters Register 1 */
#define DDRP_DTPR2	(DDR_PHY_OFFSET + 0x3c) /* DRAM Timing Parameters Register 2 */
#define DDRP_MR0	(DDR_PHY_OFFSET + 0x40) /* Mode Register 0 */
#define DDRP_MR1	(DDR_PHY_OFFSET + 0x44) /* Mode Register 1 */
#define DDRP_MR2	(DDR_PHY_OFFSET + 0x48) /* Mode Register 2 */
#define DDRP_MR3	(DDR_PHY_OFFSET + 0x4c) /* Mode Register 3 */

#define DDRP_ODTCR	(DDR_PHY_OFFSET + 0x50) /* ODT Configure Register */
#define DDRP_DTAR	(DDR_PHY_OFFSET + 0x54) /* Data Training Address Register */
#define DDRP_DTDR0	(DDR_PHY_OFFSET + 0x58) /* Data Training Data Register 0 */
#define DDRP_DTDR1	(DDR_PHY_OFFSET + 0x5c) /* Data Training Data Register 1 */

#define DDRP_DCUAR	(DDR_PHY_OFFSET + 0xc0) /* DCU Address Register */
#define DDRP_DCUDR	(DDR_PHY_OFFSET + 0xc4) /* DCU Data Register */
#define DDRP_DCURR	(DDR_PHY_OFFSET + 0xc8) /* DCU Run Register */
#define DDRP_DCULR	(DDR_PHY_OFFSET + 0xcc) /* DCU Loop Register */
#define DDRP_DCUGCR	(DDR_PHY_OFFSET + 0xd0) /* DCU Gerneral Configuration Register */
#define DDRP_DCUTPR	(DDR_PHY_OFFSET + 0xd4) /* DCU Timing Parameters Register */
#define DDRP_DCUSR0	(DDR_PHY_OFFSET + 0xd8) /* DCU Status Register 0 */
#define DDRP_DCUSR1	(DDR_PHY_OFFSET + 0xdc) /* DCU Status Register 1 */

#define DDRP_DXGCR(n)	(DDR_PHY_OFFSET + 0x1c0 + n * 0x40) /* DATX8 n General Configuration Register */
#define DDRP_DXGSR0(n)	(DDR_PHY_OFFSET + 0x1c4 + n * 0x40) /* DATX8 n General Status Register */
#define DDRP_DXGSR1(n)	(DDR_PHY_OFFSET + 0x1c8 + n * 0x40) /* DATX8 n General Status Register */
#define DDRP_DXDLLCR(n)	(DDR_PHY_OFFSET + 0x1cc + n * 0x40) /* DATX8 n General Status Register */
#define DDRP_DXDQSTR(n)	(DDR_PHY_OFFSET + 0x1d4 + n * 0x40) /* DATX8 n DQS Timing Register */
#define DDRP_ZQXCR0(n)	(DDR_PHY_OFFSET + 0x180 + n * 0x10) /* ZQ impedance Control Register 0 */
#define DDRP_ZQXCR1(n)	(DDR_PHY_OFFSET + 0x184 + n * 0x10) /* ZQ impedance Control Register 1 */
#define DDRP_ZQXSR0(n)	(DDR_PHY_OFFSET + 0x188 + n * 0x10) /* ZQ impedance Status Register 0 */
#define DDRP_ZQXSR1(n)	(DDR_PHY_OFFSET + 0x18c + n * 0x10) /* ZQ impedance Status Register 1 */

/* DDRC Status Register */
#define DDRC_ST_ENDIAN	(1 << 7) /* 0 Little data endian
					    1 Big data endian */
#define DDRC_DSTATUS_MISS	(1 << 6)
#define DDRC_ST_DPDN		(1 << 5) /* 0 DDR memory is NOT in deep-power-down state
					    1 DDR memory is in deep-power-down state */
#define DDRC_ST_PDN		(1 << 4) /* 0 DDR memory is NOT in power-down state
					    1 DDR memory is in power-down state */
#define DDRC_ST_AREF		(1 << 3) /* 0 DDR memory is NOT in auto-refresh state
					    1 DDR memory is in auto-refresh state */
#define DDRC_ST_SREF		(1 << 2) /* 0 DDR memory is NOT in self-refresh state
					    1 DDR memory is in self-refresh state */
#define DDRC_ST_CKE1		(1 << 1) /* 0 CKE1 Pin is low
					    1 CKE1 Pin is high */
#define DDRC_ST_CKE0		(1 << 0) /* 0 CKE0 Pin is low
					    1 CKE0 Pin is high */

/* DDRC Configure Register */
#define DDRC_CFG_ROW1_BIT	27 /* Row Address width. */
#define DDRC_CFG_ROW1_MASK	(0x7 << DDRC_CFG_ROW1_BIT)
#define DDRC_CFG_COL1_BIT	24 /* Row Address width. */
#define DDRC_CFG_COL1_MASK	(0x7 << DDRC_CFG_COL1_BIT)
#define DDRC_CFG_BA1_BIT	23 /* Bank Address width of DDR memory */
#define DDRC_CFG_IMBA		(1 << 22)
#define DDRC_CFG_BL_BIT		21 /* Burst length for DDR chips */
#define DDRC_CFG_TYPE_BIT	17
//#define DDRC_CFG_TYPE_MASK	(0x7 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_DDR1	(2 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_MDDR	(3 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_DDR2	(4 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_LPDDR2	(5 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_DDR3	(6 << DDRC_CFG_TYPE_BIT)
//#define DDRC_CFG_ODT_EN		(1 << 16)  /* ODT EN */
#define DDRC_CFG_ODT_EN		1  /* ODT EN */
#define DDRC_CFG_ODT_EN_BIT	16  /* ODT EN */
#define DDRC_CFG_MISPE		(1 << 15)  /* mem protect */
#define DDRC_CFG_ROW0_BIT	11 /* Row Address width. */
#define DDRC_CFG_ROW0_MASK	(0x7 << DDRC_CFG_ROW0_BIT)
//#define DDRC_CFG_ROW_12	(0 << DDRC_CFG_ROW_BIT) /* 12-bit row address is used */
//#define DDRC_CFG_ROW_13	(1 << DDRC_CFG_ROW_BIT) /* 13-bit row address is used */
//#define DDRC_CFG_ROW_14	(2 << DDRC_CFG_ROW_BIT) /* 14-bit row address is used */
#define DDRC_CFG_COL0_BIT	8 /* Column Address width.
				     Specify the Column address width of external DDR. */
#define DDRC_CFG_COL0_MASK	(0x7 << DDRC_CFG_COL0_BIT)
//#define DDRC_CFG_COL_8	(0 << DDRC_CFG_COL_BIT) /* 8-bit Column address is used */
//#define DDRC_CFG_COL_9	(1 << DDRC_CFG_COL_BIT) /* 9-bit Column address is used */
//#define DDRC_CFG_COL_10	(2 << DDRC_CFG_COL_BIT) /* 10-bit Column address is used */
//#define DDRC_CFG_COL_11	(3 << DDRC_CFG_COL_BIT) /* 11-bit Column address is used */
#define DDRC_CFG_CS1EN_BIT	7 /* DDR Chip-Select-1 Enable */
#define DDRC_CFG_CS0EN_BIT	6 /* DDR Chip-Select-0 Enable */
#define DDRC_CFG_CS1EN		(1 << 7) /* 0 DDR Pin CS1 un-used
//					    1 There're DDR memory connected to CS1 */
#define DDRC_CFG_CS0EN		(1 << 6) /* 0 DDR Pin CS0 un-used
//					    1 There're DDR memory connected to CS0 */
#define DDRC_CFG_CL_BIT		2 /* CAS Latency */
//#define DDRC_CFG_CL_MASK	(0xf << DDRC_CFG_CL_BIT)
//#define DDRC_CFG_CL_3		(0 << DDRC_CFG_CL_BIT) /* CL = 3 tCK */
//#define DDRC_CFG_CL_4		(1 << DDRC_CFG_CL_BIT) /* CL = 4 tCK */
//#define DDRC_CFG_CL_5		(2 << DDRC_CFG_CL_BIT) /* CL = 5 tCK */
//#define DDRC_CFG_CL_6		(3 << DDRC_CFG_CL_BIT) /* CL = 6 tCK */
#define DDRC_CFG_BA0_BIT	1 /* Bank Address width of DDR memory */
//#define DDRC_CFG_BA0		(1 << 1) /* 0 4 bank device, Pin ba[1:0] valid, ba[2] un-used
//					    1 8 bank device, Pin ba[2:0] valid*/
#define DDRC_CFG_DW_BIT		0 /* External DDR Memory Data Width */
//#define DDRC_CFG_DW		(1 << 0) /*0 External memory data width is 16-bit
//					   1 External memory data width is 32-bit */

/* DDRC Control Register */
#define DDRC_CTRL_DFI_RST	(1 << 23)
#define DDRC_CTRL_DLL_RST	(1 << 22)
#define DDRC_CTRL_CTL_RST	(1 << 21)
#define DDRC_CTRL_CFG_RST	(1 << 20)
#define DDRC_CTRL_ACTPD		(1 << 15) /* 0 Precharge all banks before entering power-down
					     1 Do not precharge banks before entering power-down */
#define DDRC_CTRL_PDT_BIT	12 /* Power-Down Timer */
#define DDRC_CTRL_PDT_MASK	(0x7 << DDRC_CTRL_PDT_BIT)
#define DDRC_CTRL_PDT_DIS	(0 << DDRC_CTRL_PDT_BIT) /* power-down disabled */
#define DDRC_CTRL_PDT_8	(1 << DDRC_CTRL_PDT_BIT) /* Enter power-down after 8 tCK idle */
#define DDRC_CTRL_PDT_16	(2 << DDRC_CTRL_PDT_BIT) /* Enter power-down after 16 tCK idle */
#define DDRC_CTRL_PDT_32	(3 << DDRC_CTRL_PDT_BIT) /* Enter power-down after 32 tCK idle */
#define DDRC_CTRL_PDT_64	(4 << DDRC_CTRL_PDT_BIT) /* Enter power-down after 64 tCK idle */
#define DDRC_CTRL_PDT_128	(5 << DDRC_CTRL_PDT_BIT) /* Enter power-down after 128 tCK idle */
#define DDRC_CTRL_ACTSTP	(1 << 11)
#define DDRC_CTRL_PRET_BIT	8 /* Precharge Timer */
#define DDRC_CTRL_PRET_MASK	(0x7 << DDRC_CTRL_PRET_BIT) /*  */
#define DDRC_CTRL_PRET_DIS	(0 << DDRC_CTRL_PRET_BIT) /* PRET function Disabled */
#define DDRC_CTRL_PRET_8	(1 << DDRC_CTRL_PRET_BIT) /* Precharge active bank after 8 tCK idle */
#define DDRC_CTRL_PRET_16	(2 << DDRC_CTRL_PRET_BIT) /* Precharge active bank after 16 tCK idle */
#define DDRC_CTRL_PRET_32	(3 << DDRC_CTRL_PRET_BIT) /* Precharge active bank after 32 tCK idle */
#define DDRC_CTRL_PRET_64	(4 << DDRC_CTRL_PRET_BIT) /* Precharge active bank after 64 tCK idle */
#define DDRC_CTRL_PRET_128	(5 << DDRC_CTRL_PRET_BIT) /* Precharge active bank after 128 tCK idle */

#define DDRC_CTRL_DPD		(1 << 6) /* 1 Drive external MDDR device entering Deep-Power-Down mode */

#define DDRC_CTRL_SR		(1 << 5) /* 1 Drive external DDR device entering self-refresh mode
					    0 Drive external DDR device exiting self-refresh mode */
#define DDRC_CTRL_UNALIGN	(1 << 4) /* 0 Disable unaligned transfer on AXI BUS
					    1 Enable unaligned transfer on AXI BUS */
#define DDRC_CTRL_ALH		(1 << 3) /* Advanced Latency Hiding:
					    0 Disable ALH
					    1 Enable ALH */
#define DDRC_CTRL_RDC		(1 << 2) /* 0 dclk clock frequency is lower than 60MHz
					    1 dclk clock frequency is higher than 60MHz */
#define DDRC_CTRL_CKE		(1 << 1) /* 0 Not set CKE Pin High
					    1 Set CKE Pin HIGH */
#define DDRC_CTRL_RESET	(1 << 0) /* 0 End resetting ddrc_controller
					    1 Resetting ddrc_controller */


/* DDRC Load-Mode-Register */
#define DDRC_LMR_DDR_ADDR_BIT	16 /* When performing a DDR command, DDRC_ADDR[13:0]
					      corresponding to external DDR address Pin A[13:0] */
#define DDRC_LMR_DDR_ADDR_MASK	(0x3fff << DDRC_LMR_DDR_ADDR_BIT)

#define DDRC_LMR_BA_BIT		8 /* When performing a DDR command, BA[2:0]
				     corresponding to external DDR address Pin BA[2:0]. */
#define DDRC_LMR_BA_MASK	(0x7 << DDRC_LMR_BA_BIT)
/* For DDR2 */
#define DDRC_LMR_BA_MRS	(0 << DDRC_LMR_BA_BIT) /* Mode Register set */
#define DDRC_LMR_BA_EMRS1	(1 << DDRC_LMR_BA_BIT) /* Extended Mode Register1 set */
#define DDRC_LMR_BA_EMRS2	(2 << DDRC_LMR_BA_BIT) /* Extended Mode Register2 set */
#define DDRC_LMR_BA_EMRS3	(3 << DDRC_LMR_BA_BIT) /* Extended Mode Register3 set */
/* For mobile DDR */
#define DDRC_LMR_BA_M_MRS	(0 << DDRC_LMR_BA_BIT) /* Mode Register set */
#define DDRC_LMR_BA_M_EMRS	(2 << DDRC_LMR_BA_BIT) /* Extended Mode Register set */
#define DDRC_LMR_BA_M_SR	(1 << DDRC_LMR_BA_BIT) /* Status Register set */
/* For Normal DDR1 */
#define DDRC_LMR_BA_N_MRS	(0 << DDRC_LMR_BA_BIT) /* Mode Register set */
#define DDRC_LMR_BA_N_EMRS	(1 << DDRC_LMR_BA_BIT) /* Extended Mode Register set */

#define DDRC_LMR_CMD_BIT	4
#define DDRC_LMR_CMD_MASK	(0x3 << DDRC_LMR_CMD_BIT)
#define DDRC_LMR_CMD_PREC	(0 << DDRC_LMR_CMD_BIT)/* Precharge one bank/All banks */
#define DDRC_LMR_CMD_AUREF	(1 << DDRC_LMR_CMD_BIT)/* Auto-Refresh */
#define DDRC_LMR_CMD_LMR	(2 << DDRC_LMR_CMD_BIT)/* Load Mode Register */

#define DDRC_LMR_START		(1 << 0) /* 0 No command is performed
						    1 On the posedge of START, perform a command
						    defined by CMD field */

/* DDRC Timing Config Register 1 */
#define DDRC_TIMING1_tRTP_BIT	24 /* READ to PRECHARGE command period. */
#define DDRC_TIMING1_tRTP_MASK	(0x3f << DDRC_TIMING1_tRTP_BIT)
#define DDRC_TIMING1_tWTR_BIT	16 /* WRITE to READ command delay. */
#define DDRC_TIMING1_tWTR_MASK	(0x3f << DDRC_TIMING1_tWTR_BIT)
#define DDRC_TIMING1_tWTR_1		(0 << DDRC_TIMING1_tWTR_BIT)
#define DDRC_TIMING1_tWTR_2		(1 << DDRC_TIMING1_tWTR_BIT)
#define DDRC_TIMING1_tWTR_3		(2 << DDRC_TIMING1_tWTR_BIT)
#define DDRC_TIMING1_tWTR_4		(3 << DDRC_TIMING1_tWTR_BIT)
#define DDRC_TIMING1_tWR_BIT 	8 /* WRITE Recovery Time defined by register MR of DDR2 DDR3 memory */
#define DDRC_TIMING1_tWR_MASK	(0x3f << DDRC_TIMING1_tWR_BIT)
#define DDRC_TIMING1_tWR_1		(0 << DDRC_TIMING1_tWR_BIT)
#define DDRC_TIMING1_tWR_2		(1 << DDRC_TIMING1_tWR_BIT)
#define DDRC_TIMING1_tWR_3		(2 << DDRC_TIMING1_tWR_BIT)
#define DDRC_TIMING1_tWR_4		(3 << DDRC_TIMING1_tWR_BIT)
#define DDRC_TIMING1_tWR_5		(4 << DDRC_TIMING1_tWR_BIT)
#define DDRC_TIMING1_tWR_6		(5 << DDRC_TIMING1_tWR_BIT)
#define DDRC_TIMING1_tWL_BIT 	0 /* Write latency = RL - 1 */
#define DDRC_TIMING1_tWL_MASK	(0x3f << DDRC_TIMING1_tWL_BIT)

/* DDRC Timing Config Register 2 */
#define DDRC_TIMING2_tCCD_BIT 	24 /* CAS# to CAS# command delay */
#define DDRC_TIMING2_tCCD_MASK 	(0x3f << DDRC_TIMING2_tCCD_BIT)
#define DDRC_TIMING2_tRAS_BIT 	16 /* ACTIVE to PRECHARGE command period (2 * tRAS + 1) */
#define DDRC_TIMING2_tRAS_MASK 	(0x3f << DDRC_TIMING2_tRAS_BIT)
#define DDRC_TIMING2_tRCD_BIT	8  /* ACTIVE to READ or WRITE command period. */
#define DDRC_TIMING2_tRCD_MASK	(0x3f << DDRC_TIMING2_tRCD_BIT)
#define DDRC_TIMING2_tRL_BIT	0  /* Read latency = AL + CL*/
#define DDRC_TIMING2_tRL_MASK	(0x3f << DDRC_TIMING2_tRL_BIT)

/* DDRC Timing Config Register 3 */
#define DDRC_TIMING3_ONUM_BIT  27
#define DDRC_TIMING3_tCKSRE_BIT		24 /* Valid clock after enter self-refresh. */
#define DDRC_TIMING3_tCKSRE_MASK 	(0x3f << DDRC_TIMING3_tCKSRE_BIT)
#define DDRC_TIMING3_tRP_BIT	16 /* PRECHARGE command period. */
#define DDRC_TIMING3_tRP_MASK 	(0x3f << DDRC_TIMING3_tRP_BIT)
#define DDRC_TIMING3_tRRD_BIT	8 /* ACTIVE bank A to ACTIVE bank B command period. */
#define DDRC_TIMING3_tRRD_MASK	(0x3f << DDRC_TIMING3_tRRD_BIT)
#define DDRC_TIMING3_tRRD_DISABLE	(0 << DDRC_TIMING3_tRRD_BIT)
#define DDRC_TIMING3_tRRD_2		(1 << DDRC_TIMING3_tRRD_BIT)
#define DDRC_TIMING3_tRRD_3		(2 << DDRC_TIMING3_tRRD_BIT)
#define DDRC_TIMING3_tRRD_4		(3 << DDRC_TIMING3_tRRD_BIT)
#define DDRC_TIMING3_tRC_BIT 	0 /* ACTIVE to ACTIVE command period. */
#define DDRC_TIMING3_tRC_MASK 	(0x3f << DDRC_TIMING3_tRC_BIT)

/* DDRC Timing Config Register 4 */
#define DDRC_TIMING4_tRFC_BIT         24 /* AUTO-REFRESH command period. */
#define DDRC_TIMING4_tRFC_MASK        (0x3f << DDRC_TIMING4_tRFC_BIT)
#define DDRC_TIMING4_tEXTRW_BIT	      21 /* ??? */
#define DDRC_TIMING4_tEXTRW_MASK      (0x7 << DDRC_TIMING4_tEXTRW_BIT)
#define DDRC_TIMING4_tRWCOV_BIT	      19 /* ??? */
#define DDRC_TIMING4_tRWCOV_MASK      (0x3 << DDRC_TIMING4_tRWCOV_BIT)
#define DDRC_TIMING4_tCKE_BIT	      16 /* ??? */
#define DDRC_TIMING4_tCKE_MASK        (0x7 << DDRC_TIMING4_tCKE_BIT)
#define DDRC_TIMING4_tMINSR_BIT       8  /* Minimum Self-Refresh / Deep-Power-Down time */
#define DDRC_TIMING4_tMINSR_MASK      (0xf << DDRC_TIMING4_tMINSR_BIT)
#define DDRC_TIMING4_tXP_BIT          4  /* EXIT-POWER-DOWN to next valid command period. */
#define DDRC_TIMING4_tXP_MASK         (0x7 << DDRC_TIMING4_tXP_BIT)
#define DDRC_TIMING4_tMRD_BIT         0  /* Load-Mode-Register to next valid command period. */
#define DDRC_TIMING4_tMRD_MASK        (0x3 << DDRC_TIMING4_tMRD_BIT)

/* DDRC Timing Config Register 5 */
#define DDRC_TIMING5_tCTLUPD_BIT	24 /* ??? */
#define DDRC_TIMING4_tCTLUPD_MASK	(0x3f << DDRC_TIMING5_tCTLUDP_BIT)
#define DDRC_TIMING5_tRTW_BIT		16 /* ??? */
#define DDRC_TIMING5_tRTW_MASK		(0x3f << DDRC_TIMING5_tRTW_BIT)
#define DDRC_TIMING5_tRDLAT_BIT		8 /* RL - 2 */
#define DDRC_TIMING5_tRDLAT_MASK	(0x3f << DDRC_TIMING5_tRDLAT_BIT)
#define DDRC_TIMING5_tWDLAT_BIT		0 /* WL - 1 */
#define DDRC_TIMING5_tWDLAT_MASK	(0x3f << DDRC_TIMING5_tWDLAT_BIT)

/* DDRC Timing Config Register 6 */
#define DDRC_TIMING6_tXSRD_BIT		24 /* exit power-down to READ delay */
#define DDRC_TIMING6_tXSRD_MASK		(0x3f << DDRC_TIMING6_tXSRD_BIT)
#define DDRC_TIMING6_tFAW_BIT		16 /* 4-active command window */
#define DDRC_TIMING6_tFAW_MASK		(0x3f << DDRC_TIMING6_tFAW_BIT)
#define DDRC_TIMING6_tCFGW_BIT		8 /* Write PHY configure registers to other commands delay */
#define DDRC_TIMING6_tCFGW_MASK		(0x3f << DDRC_TIMING6_tCFGW_BIT)
#define DDRC_TIMING6_tCFGR_BIT		0 /* Ready PHY configure registers to other commands delay */
#define DDRC_TIMING6_tCFGR_MASK		(0x3f << DDRC_TIMING6_tCFGR_BIT)

/* DDRC  Auto-Refresh Counter */
#define DDRC_REFCNT_CON_BIT           16 /* Constant value used to compare with CNT value. */
#define DDRC_REFCNT_CON_MASK          (0xff << DDRC_REFCNT_CON_BIT)
#define DDRC_REFCNT_CNT_BIT           8  /* 8-bit counter */
#define DDRC_REFCNT_CNT_MASK          (0xff << DDRC_REFCNT_CNT_BIT)
#define DDRC_REFCNT_CLK_DIV_BIT        1  /* Clock Divider for auto-refresh counter. */
#define DDRC_REFCNT_CLK_DIV_MASK       (0x7 << DDRC_REFCNT_CLKDIV_BIT)
#define DDRC_REFCNT_REF_EN            (1 << 0) /* Enable Refresh Counter */

/* DDRC DQS Delay Control Register */
#define DDRC_DQS_ERROR                (1 << 29) /* ahb_clk Delay Detect ERROR, read-only. */
#define DDRC_DQS_READY                (1 << 28) /* ahb_clk Delay Detect READY, read-only. */
#define DDRC_DQS_AUTO                 (1 << 23) /* Hardware auto-detect & set delay line */
#define DDRC_DQS_DET                  (1 << 24) /* Start delay detecting. */
#define DDRC_DQS_SRDET                (1 << 25) /* Hardware auto-redetect & set delay line. */
#define DDRC_DQS_CLKD_BIT             16 /* CLKD is reference value for setting WDQS and RDQS.*/
#define DDRC_DQS_CLKD_MASK            (0x3f << DDRC_DQS_CLKD_BIT)
#define DDRC_DQS_WDQS_BIT             8  /* Set delay element number to write DQS delay-line. */
#define DDRC_DQS_WDQS_MASK            (0x3f << DDRC_DQS_WDQS_BIT)
#define DDRC_DQS_RDQS_BIT             0  /* Set delay element number to read DQS delay-line. */
#define DDRC_DQS_RDQS_MASK            (0x3f << DDRC_DQS_RDQS_BIT)

/* DDRC DQS Delay Adjust Register */
#define DDRC_DQS_ADJWDQS_BIT          8 /* The adjust value for WRITE DQS delay */
#define DDRC_DQS_ADJWDQS_MASK         (0x1f << DDRC_DQS_ADJWDQS_BIT)
#define DDRC_DQS_ADJRDQS_BIT          0 /* The adjust value for READ DQS delay */
#define DDRC_DQS_ADJRDQS_MASK         (0x1f << DDRC_DQS_ADJRDQS_BIT)

/* DDRC Memory Map Config Register */
#define DDRC_MMAP_BASE_BIT            8 /* base address */
#define DDRC_MMAP_BASE_MASK           (0xff << DDRC_MMAP_BASE_BIT)
#define DDRC_MMAP_MASK_BIT            0 /* address mask */
#define DDRC_MMAP_MASK_MASK           (0xff << DDRC_MMAP_MASK_BIT)

#define DDRC_MMAP0_BASE		     (0x20 << DDRC_MMAP_BASE_BIT)
#define DDRC_MMAP1_BASE_64M	(0x24 << DDRC_MMAP_BASE_BIT) /*when bank0 is 128M*/
#define DDRC_MMAP1_BASE_128M	(0x28 << DDRC_MMAP_BASE_BIT) /*when bank0 is 128M*/
#define DDRC_MMAP1_BASE_256M	(0x30 << DDRC_MMAP_BASE_BIT) /*when bank0 is 128M*/

#define DDRC_MMAP_MASK_64_64	(0xfc << DDRC_MMAP_MASK_BIT)  /*mask for two 128M SDRAM*/
#define DDRC_MMAP_MASK_128_128	(0xf8 << DDRC_MMAP_MASK_BIT)  /*mask for two 128M SDRAM*/
#define DDRC_MMAP_MASK_256_256	(0xf0 << DDRC_MMAP_MASK_BIT)  /*mask for two 128M SDRAM*/

/* DDRP PHY Initialization Register */
#define DDRP_PIR_INIT		(1 << 0)
#define DDRP_PIR_DLLSRST	(1 << 1)
#define DDRP_PIR_DLLLOCK	(1 << 2)
#define DDRP_PIR_ZCAL   	(1 << 3)
#define DDRP_PIR_ITMSRST   	(1 << 4)
#define DDRP_PIR_DRAMRST   	(1 << 5)
#define DDRP_PIR_DRAMINT   	(1 << 6)
#define DDRP_PIR_QSTRN   	(1 << 7)
#define DDRP_PIR_EYETRN   	(1 << 8)
#define DDRP_PIR_DLLBYP   	(1 << 17)
#define DDRP_PIR_LOCKBYP        (1 << 29)
/* DDRP PHY General Configurate Register */
#define DDRP_PGCR_ITMDMD	(1 << 0)
#define DDRP_PGCR_DQSCFG	(1 << 1)
#define DDRP_PGCR_DFTCMP	(1 << 2)
#define DDRP_PGCR_DFTLMT_BIT	3
#define DDRP_PGCR_DTOSEL_BIT	5
#define DDRP_PGCR_CKEN_BIT	9
#define DDRP_PGCR_CKDV_BIT	12
#define DDRP_PGCR_CKINV		(1 << 14)
#define DDRP_PGCR_RANKEN_BIT	18
#define DDRP_PGCR_ZCKSEL_32	(2 << 22)
#define DDRP_PGCR_ZCKSEL_64	(3 << 22)
#define DDRP_PGCR_PDDISDX	(1 << 24)
/* DDRP PHY General Status Register */
#define DDRP_PGSR_IDONE		(1 << 0)
#define DDRP_PGSR_DLDONE	(1 << 1)
#define DDRP_PGSR_ZCDONE	(1 << 2)
#define DDRP_PGSR_DIDONE	(1 << 3)
#define DDRP_PGSR_DTDONE	(1 << 4)
#define DDRP_PGSR_DTERR 	(1 << 5)
#define DDRP_PGSR_DTIERR 	(1 << 6)
#define DDRP_PGSR_DFTEERR 	(1 << 7)
/* DDRP DRAM Configuration Register */
#define DDRP_DCR_TYPE_BIT	0
#define DDRP_DCR_TYPE_MASK	(0x7 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_MDDR	(0 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_DDR	(1 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_DDR2	(2 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_DDR3	(3 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_LPDDR2	(4 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_DDR8BNK_BIT	3
#define DDRP_DCR_DDR8BNK_MASK	(1 << DDRP_DCR_DDR8BNK_BIT)
#define DDRP_DCR_DDR8BNK	(1 << DDRP_DCR_DDR8BNK_BIT)
#define DDRP_DCR_DDR8BNK_DIS	(0 << DDRP_DCR_DDR8BNK_BIT)
/* DDRP PHY Timing Register 0 */
/* DDRP PHY Timing Register 1 */
/* DDRP PHY Timing Register 2 */
#define DDRP_PTR0_tDLLSRST  	50		// 50ns
#define DDRP_PTR0_tDLLSRST_BIT	0
#define DDRP_PTR0_tDLLLOCK 	5120 		// 5.12us
#define DDRP_PTR0_tDLLLOCK_BIT	6
#define DDRP_PTR0_tITMSRST_8 	8		// 8tck
#define DDRP_PTR0_tITMSRST_BIT	18
#define DDRP_PTR1_tDINIT1_BIT	19
#define DDRP_PTR1_tDINIT0_DDR3	500 * 1000 	// 500us
#define DDRP_PTR1_tDINIT0_BIT	0
#define DDRP_PTR2_tDINIT2_BIT	0
#define DDRP_PTR2_tDINIT2_DDR3 	200 * 1000	// 200us
#define DDRP_PTR2_tDINIT3_BIT	17
#define DDRP_PTR2_tDINIT3_DDR3	512 		// 512 tck

/* DDRP DRAM Timing Parameters Register 0 */
#define DDRP_DTPR0_tCCD_BIT	31
#define DDRP_DTPR0_tRC_BIT	25
#define DDRP_DTPR0_tRRD_BIT	21
#define DDRP_DTPR0_tRAS_BIT	16
#define DDRP_DTPR0_tRCD_BIT	12
#define DDRP_DTPR0_tRP_BIT	8
#define DDRP_DTPR0_tWTR_BIT	5
#define DDRP_DTPR0_tRTP_BIT	2
#define DDRP_DTPR0_tMRD_BIT	0
/* DDRP DRAM Timing Parameters Register 1 */
#define DRP_DTRP1_RTODT  (1 << 11)    /* ODT may not be turned on until one clock after the read post-amble */
#define DDRP_DTPR1_tRFC_BIT	16
#define DDRP_DTPR1_tRTODT_BIT	11
#define DDRP_DTPR1_tMOD_BIT	9
#define DDRP_DTPR1_tFAW_BIT	3
/* DDRP DRAM Timing Parameters Register 2 */
#define DDRP_DTPR2_tDLLK_BIT	19
#define DDRP_DTPR2_tCKE_BIT	15
#define DDRP_DTPR2_tXP_BIT	10
#define DDRP_DTPR2_tXS_BIT	0
/* DDRP DATX8 n Gen#define eral Configuration Register */
#define DDRP_DXGCR_DXEN  (1 << 0)    /* Data Byte Enable */

#define DDRP_ZQXCR_ZDEN_BIT		28
#define DDRP_ZQXCR_ZDEN			(1 << DDRP_ZQXCR_ZDEN_BIT)
#define DDRP_ZQXCR_PULLUP_IMPED_BIT	5
#define DDRP_ZQXCR_PULLDOWN_IMPED_BIT	0

/* DDR Mode Register Set*/
#define DDR1_MRS_OM_BIT		7        /* Operating Mode */
#define DDR1_MRS_OM_MASK	(0x3f << DDR1_MRS_OM_BIT)
#define DDR1_MRS_OM_NORMAL	(0 << DDR1_MRS_OM_BIT)
#define DDR1_MRS_OM_TEST	(1 << DDR1_MRS_OM_BIT)
#define DDR1_MRS_OM_DLLRST	(2 << DDR1_MRS_OM_BIT)
#define DDR_MRS_CAS_BIT		4        /* CAS Latency */
#define DDR_MRS_CAS_MASK	(7 << DDR_MRS_CAS_BIT)
#define DDR_MRS_BT_BIT		3        /* Burst Type */
#define DDR_MRS_BT_MASK		(1 << DDR_MRS_BT_BIT)
#define DDR_MRS_BT_SEQ	(0 << DDR_MRS_BT_BIT) /* Sequential */
#define DDR_MRS_BT_INT	(1 << DDR_MRS_BT_BIT) /* Interleave */
#define DDR_MRS_BL_BIT		0        /* Burst Length */
#define DDR_MRS_BL_MASK		(7 << DDR_MRS_BL_BIT)
#define DDR_MRS_BL_4		(2 << DDR_MRS_BL_BIT)
#define DDR_MRS_BL_8		(3 << DDR_MRS_BL_BIT)
/* DDR1 Extended Mode Register */
#define DDR1_EMRS_OM_BIT	2	/* Partial Array Self Refresh */
#define DDR1_EMRS_OM_MASK	(0x3ff << DDR1_EMRS_OM_BIT)
#define DDR1_EMRS_OM_NORMAL	(0 << DDR1_EMRS_OM_BIT) /*All Banks*/

#define DDR1_EMRS_DS_BIT	1	/* Driver strength */
#define DDR1_EMRS_DS_MASK	(1 << DDR1_EMRS_DS_BIT)
#define DDR1_EMRS_DS_FULL	(0 << DDR1_EMRS_DS_BIT)	/*Full*/
#define DDR1_EMRS_DS_HALF	(1 << DDR1_EMRS_DS_BIT)	/*1/2 Strength*/

#define DDR1_EMRS_DLL_BIT	0	/* Driver strength */
#define DDR1_EMRS_DLL_MASK	(1 << DDR1_EMRS_DLL_BIT)
#define DDR1_EMRS_DLL_EN	(0 << DDR1_EMRS_DLL_BIT)	/*Full*/
#define DDR1_EMRS_DLL_DIS	(1 << DDR1_EMRS_DLL_BIT)	/*1/2 Strength*/

/* MDDR Mode Register Set*/
/* Mobile SDRAM Extended Mode Register */
#define DDR_EMRS_DS_BIT		5	/* Driver strength */
#define DDR_EMRS_DS_MASK	(3 << DDR_EMRS_DS_BIT)
#define DDR_EMRS_DS_FULL	(0 << DDR_EMRS_DS_BIT)	/*Full*/
#define DDR_EMRS_DS_HALF	(1 << DDR_EMRS_DS_BIT)	/*1/2 Strength*/
#define DDR_EMRS_DS_QUTR	(2 << DDR_EMRS_DS_BIT)	/*1/4 Strength*/

#define DDR_EMRS_PRSR_BIT	0	/* Partial Array Self Refresh */
#define DDR_EMRS_PRSR_MASK	(7 << DDR_EMRS_PRSR_BIT)
#define DDR_EMRS_PRSR_ALL	(0 << DDR_EMRS_PRSR_BIT) /*All Banks*/
#define DDR_EMRS_PRSR_HALF_TL	(1 << DDR_EMRS_PRSR_BIT) /*Half of Total Bank*/
#define DDR_EMRS_PRSR_QUTR_TL	(2 << DDR_EMRS_PRSR_BIT) /*Quarter of Total Bank*/
#define DDR_EMRS_PRSR_HALF_B0	(5 << DDR_EMRS_PRSR_BIT) /*Half of Bank0*/
#define DDR_EMRS_PRSR_QUTR_B0	(6 << DDR_EMRS_PRSR_BIT) /*Quarter of Bank0*/

/* DDR2 Mode Register Set*/
#define DDR2_MRS_PD_BIT		10 /* Active power down exit time */
#define DDR2_MRS_PD_MASK	(1 << DDR_MRS_PD_BIT)
#define DDR2_MRS_PD_FAST_EXIT	(0 << 10)
#define DDR2_MRS_PD_SLOW_EXIT	(1 << 10)
#define DDR2_MRS_WR_BIT		9 /* Write Recovery for autoprecharge */
#define DDR2_MRS_WR_MASK	(7 << DDR_MRS_WR_BIT)
#define DDR2_MRS_DLL_RST	(1 << 8) /* DLL Reset */
#define DDR2_MRS_TM_BIT		7        /* Operating Mode */
#define DDR2_MRS_TM_MASK	(1 << DDR_MRS_TM_BIT)
#define DDR2_MRS_TM_NORMAL	(0 << DDR_MRS_TM_BIT)
#define DDR2_MRS_TM_TEST	(1 << DDR_MRS_TM_BIT)
/* DDR2 Extended Mode Register1 Set */
#define DDR_EMRS1_QOFF		(1<<12) /* 0 Output buffer enabled
					   1 Output buffer disabled */
#define DDR_EMRS1_RDQS_EN	(1<<11) /* 0 Disable
					   1 Enable */
#define DDR_EMRS1_DQS_DIS	(1<<10) /* 0 Enable
					   1 Disable */
#define DDR_EMRS1_OCD_BIT	7 /* Additive Latency 0 -> 6 */
#define DDR_EMRS1_OCD_MASK	(0x7 << DDR_EMRS1_OCD_BIT)
#define DDR_EMRS1_OCD_EXIT		(0 << DDR_EMRS1_OCD_BIT)
#define DDR_EMRS1_OCD_D0		(1 << DDR_EMRS1_OCD_BIT)
#define DDR_EMRS1_OCD_D1		(2 << DDR_EMRS1_OCD_BIT)
#define DDR_EMRS1_OCD_ADJ		(4 << DDR_EMRS1_OCD_BIT)
#define DDR_EMRS1_OCD_DFLT		(7 << DDR_EMRS1_OCD_BIT)
#define DDR_EMRS1_AL_BIT	3 /* Additive Latency 0 -> 6 */
#define DDR_EMRS1_AL_MASK	(7 << DDR_EMRS1_AL_BIT)
#define DDR_EMRS1_RTT_BIT	2 /* On Die Termination */
#define DDR_EMRS1_RTT_MASK	(0x11 << DDR_EMRS1_RTT_BIT) /* Bit 6 and  Bit 2 */
#define DDR_EMRS1_RTT_DIS	(0x00 << DDR_EMRS1_RTT_BIT)
#define DDR_EMRS1_RTT_75	(0x01 << DDR_EMRS1_RTT_BIT)
#define DDR_EMRS1_RTT_150	(0x10 << DDR_EMRS1_RTT_BIT)
#define DDR_EMRS1_RTT_50	(0x11 << DDR_EMRS1_RTT_BIT)
#define DDR_EMRS1_DIC_BIT	1        /* Output Driver Impedence Control */
#define DDR_EMRS1_DIC_MASK	(1 << DDR_EMRS1_DIC_BIT) /* 100% */
#define DDR_EMRS1_DIC_NORMAL	(0 << DDR_EMRS1_DIC_BIT) /* 60% */
#define DDR_EMRS1_DIC_HALF	(1 << DDR_EMRS1_DIC_BIT)
#define DDR_EMRS1_DLL_BIT	0        /* DLL Enable  */
#define DDR_EMRS1_DLL_MASK	(1 << DDR_EMRS1_DLL_BIT)
#define DDR_EMRS1_DLL_EN	(0 << DDR_EMRS1_DLL_BIT)
#define DDR_EMRS1_DLL_DIS	(1 << DDR_EMRS1_DLL_BIT)

/* LPDDR2 Mode2 Register Set*/
#define LPDDR2_MRS2_BL_BIT		0        /* Burst Length */
#define LPDDR2_MRS2_BL_MASK		(7 << DDR_MRS_BL_BIT)
#define LPDDR2_MRS2_BL_4		(2 << DDR_MRS_BL_BIT)
#define LPDDR2_MRS2_BL_8		(3 << DDR_MRS_BL_BIT)
#define LPDDR2_MRS2_BL_16		(4 << DDR_MRS_BL_BIT)

/* DDR3 Mode Register Set*/
#define DDR3_MR0_BL_BIT		0
#define DDR3_MR0_BL_MASK	(3 << DDR3_MR0_BL_BIT)
#define DDR3_MR0_BL_8		(0 << DDR3_MR0_BL_BIT)
#define DDR3_MR0_BL_fly	(1 << DDR3_MR0_BL_BIT)
#define DDR3_MR0_BL_4		(2 << DDR3_MR0_BL_BIT)
#define DDR3_MR0_BT_BIT		3
#define DDR3_MR0_BT_MASK	(1 << DDR3_MR0_BT_BIT)
#define DDR3_MR0_BT_SEQ 	(0 << DDR3_MR0_BT_BIT)
#define DDR3_MR0_BT_INTER 	(1 << DDR3_MR0_BT_BIT)
#define DDR3_MR0_CL2_BIT	2
#define DDR3_MR0_CL4_BIT	4
#define DDR3_MR0_WR_BIT		9

#define DDR3_MR1_DLL_DISABLE	1
#define DDR3_MR1_DIC_6 		(0 << 5 | 0 << 1)
#define DDR3_MR1_DIC_7 		(0 << 5 | 1 << 1)
#define DDR3_MR1_RTT_DIS	(0 << 9 | 0 << 6 | 0 << 2)
#define DDR3_MR1_RTT_4 		(0 << 9 | 0 << 6 | 1 << 2)
#define DDR3_MR1_RTT_2 		(0 << 9 | 1 << 6 | 0 << 2)
#define DDR3_MR1_RTT_6 		(0 << 9 | 1 << 6 | 1 << 2)
#define DDR3_MR1_RTT_12 	(1 << 9 | 0 << 6 | 0 << 2)
#define DDR3_MR1_RTT_8 		(1 << 9 | 0 << 6 | 1 << 2)

#define DDR3_MR2_CWL_BIT	3


#define DDRC_MDELAY_MAUTO_BIT (6)
#define DDRC_MDELAY_MAUTO  (1 << DDRC_MDELAY_MAUTO_BIT)

#if 0
#define DDR_GET_VALUE(x, y) (((x * 1000)% y == 0) ?	\
		((x * 1000)/ y) : ((x * 1000) / y + 1))
#endif

//#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ddr_writel(value, reg)	writel((value), DDRC_BASE + reg)
#define ddr_readl(reg)		readl(DDRC_BASE + reg)

extern unsigned int __ps_per_tck;

#define DDR_SELECT_MAX__tCK_ps(tck, ps)						\
({										\
	unsigned int value;							\
	value = (tck * __ps_per_tck > ps) ? (tck * __ps_per_tck) : ps;   	\
	value;									\
})

#define DDR__ns(ns)   (ns * 1000)
#define DDR__ps(ps)   (ps)
#define DDR__tck(tck) (tck * __ps_per_tck)

#if 0

#define DDR_GET_VALUE(x, y)					\
({								\
	unsigned long value, temp;		                \
	 temp = x * 1000;					\
	 value = (temp % y == 0) ? (temp / y) : (temp / y + 1); \
	 value;							\
 })
#endif

struct jzsoc_ddr_hook
{
	void (*prev_ddr_init)(int bypass,enum ddr_type type);
	void (*post_ddr_init)(int bypass,enum ddr_type type);
};
void register_ddr_hook(struct jzsoc_ddr_hook * hook);
#endif /* __DDR_H__ */
