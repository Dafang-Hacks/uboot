/*
 * jz4775_nemc.h - Ingenic NEMC Controller driver head file
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 * Written by Feng Gao <fenggao@ingenic.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JZ_NEMC_H__
#define __JZ_NEMC_H__

#define NEMC_SMCR_BASE	0x014 /* Static Memory Control Register 1 */
#define NEMC_SMCR_STEP	0x4
#define NEMC_SACR_BASE	0x034 /* Static Bank Address Configuration Register 1 */
#define NEMC_SACR_STEP	0x4

#define NEMC_SMCR(n) 	(NEMC_SMCR_BASE + NEMC_SMCR_STEP * (n)) /* 0 ~ 5 */
#define NEMC_SACR(n)	(NEMC_SACR_BASE + NEMC_SACR_STEP * (n)) /* 0 ~ 5 */
#define NEMC_NFCSR	0x050 /* NAND Flash Control/Status Register */
#define NEMC_PNCR 	0x100 /* NAND PN Control Register */
#define NEMC_PNDR 	0x104 /* NAND PN Data Register */
#define NEMC_BITCNT	0x108 /* NAND Bit Counter */

/* NEMC for TOGGLE NAND */
#define NEMC_TGCR_BASE	0x110 /* Toggle NAND Control Register 1 */
#define NEMC_TGCR_STEP	0x4

#define NEMC_TGWE	0x10C /* Toggle NAND Data Write Access */
#define NEMC_TGCR(n)	(NEMC_TGCR_BASE + NEMC_TGCR_STEP * (n)) /* 0 ~ 5 */
#define NEMC_TGSR	0x128 /* Toggle NAND RD# to DQS and DQ delay Register */
#define NEMC_TGFL	0x12C /* Toggle NAND ALE Fall to DQS Rise (bank 1/2/3 TGFL) */
#define NEMC_TGFH	0x130 /* Toggle NAND ALE Fall to DQS Rise (bank 4/5/6 TGFH) */
#define NEMC_TGCL	0x134 /* Toggle NAND CLE to RD# Low (bank 1/2/3 TGCL) */
#define NEMC_TGCH	0x138 /* Toggle NAND CLE to RD# low (bank 4/5/6 TGCH) */
#define NEMC_TGPD	0x13C /* Toggle NAND Data Postamble Hold Time Done */
#define NEMC_TGSL	0x140 /* Toggle NAND DQS Setup Time for Data Input Start (bank 1/2/3 TGSL) */
#define NEMC_TGSH	0x144 /* Toggle NAND DQS Setup Time for Data Input Start (bank 4/5/6 TGSH) */
#define NEMC_TGRR	0x148 /* Toggle NAND Timer for Random Data Input and Register Read Out */
#define NEMC_TGDR	0x14C /* Toggle NAND DQS Delay Control Register */

/* NAND Flash Control/Status Register */
#define NEMC_NFCSR_DAEC		(1 << 31)		 /* Toggle NAND Data Access Enabel Clear */
#define NEMC_NFCSR_TNFE(n)	(1 << ((n) + 16))	 /* Toggle NAND Flash CSn Enable, 0 ~ 5 */
#define NEMC_NFCSR_NFCE(n)	(1 << (((n) << 1) + 1))	 /* NAND Flash CSn Enable, 0 ~ 5 */
#define NEMC_NFCSR_NFE(n)	(1 << ((n) << 1))	 /* NAND Flash CSn FCE# Assertion Enable, 0 ~ 5 */
#define NEMC_NFCSR_NFCEC(n)	(0 << (((n) << 1) + 1))	 /* NAND Flash CSn Enable, 0 ~ 5 */
#define NEMC_NFCSR_NFEC(n)	(0 << ((n) << 1))	 /* NAND Flash CSn FCE# Assertion Enable, 0 ~ 5 */


/* Static Memory Control Register(SMCR) */
#define NEMC_SMCR_SMT_BIT   0
#define NEMC_SMCR_BL_BIT    1
#define NEMC_SMCR_BW_BIT    6   /* bus width, 0: 8-bit 1: 16-bit */
#define NEMC_SMCR_TAS_BIT   8
#define NEMC_SMCR_TAH_BIT   12
#define NEMC_SMCR_TBP_BIT   16
#define NEMC_SMCR_TAW_BIT   20
#define NEMC_SMCR_STRV_BIT  24

#define NEMC_SMCR_TAS_MASK  0xf
#define NEMC_SMCR_TAH_MASK  0xf
#define NEMC_SMCR_TBP_MASK  0xf
#define NEMC_SMCR_TAW_MASK  0xf
#define NEMC_SMCR_STRV_MASK 0x3f

/* NAND PN Control Register */
// PN(bit 0):0-disable, 1-enable
// PN(bit 1):0-no reset, 1-reset
// (bit 2):Reserved
// BITCNT(bit 3):0-disable, 1-enable
// BITCNT(bit 4):0-calculate, 1's number, 1-calculate 0's number
// BITCNT(bit 5):0-no reset, 1-reset bitcnt
#define NEMC_PNCR_BITRST	(1 << 5)
#define NEMC_PNCR_BIT_MASK	(1 << 4)
#define NEMC_PNCR_BIT0		(1 << 4)
#define NEMC_PNCR_BIT1		(0 << 4)
#define NEMC_PNCR_BITEN		(1 << 3)
#define NEMC_PNCR_PNRST		(1 << 1)
#define NEMC_PNCR_PNEN		(1 << 0)

/* Toggle NAND Data Write Access */
#define NEMC_TGWE_DAE		(1 << 31) /* Toggle NAND Data Access Enabel */
#define NEMC_TGWE_WCD		(1 << 16) /* DQS Setup Time for data input start Done */
#define NEMC_TGWE_SDE(n)	(1 << (n)) /* 0 ~ 5 */

/* Toggle NAND RD# to DQS and DQ delay Register */
#define NEMC_TGSR_DQSRE5(n)	((n) << 20) /* Toggle NAND Flash RD# to DQS and DQ delay bank6 */
#define NEMC_TGSR_DQSRE4(n)	((n) << 16)
#define	NEMC_TGSR_DQSRE3(n)	((n) << 12)
#define NEMC_TGSR_DQSRE2(n)	((n) << 8)
#define NEMC_TGSR_DQSRE1(n)	((n) << 4)
#define NEMC_TGSR_DQSRE0(n)	((n) << 0)

/* Toggle NAND ALE Fall to DQS Rise (bank 3/2/1 TGFL) */
#define NEMC_TGFL_FDA2(n)	((n) << 16) /* Toggle NAND Flash ALE Fall to DQS Rise Bank3 */
#define NEMC_TGFL_FDA1(n)	((n) << 8)
#define NEMC_TGFL_FDA0(n)	((n) << 0)
/* Toggle NAND ALE Fall to DQS Rise (bank 4/5/6 TGFH) */
#define NEMC_TGFH_FDA5(n)	((n) << 16) /* Toggle NAND Flash First ALE Fall to DQS Rise Bank6 */
#define NEMC_TGFH_FDA4(n)	((n) << 8)
#define NEMC_TGFH_FDA3(n)	((n) << 0)

/* Toggle NAND CLE to RD# Low (bank 1/2/3 TGCL) */
#define NEMC_TGCL_CLR2(n)	((n) << 16) /* Toggle NAND Flash CLE to RE_n Low Bank3 */
#define NEMC_TGCL_CLR1(n)	((n) << 8)
#define NEMC_TGCL_CLR0(n)	((n) << 0)
/* Toggle NAND CLE to RD# low (bank 4/5/6 TGCH) */
#define NEMC_TGCH_CLR5(n)	((n) << 16) /* Toggle NAND Flash CLE to RE_n Low Bank6 */
#define NEMC_TGCH_CLR4(n)	((n) << 8)
#define NEMC_TGCH_CLR3(n)	((n) << 0)

/* Toggle NAND Data Postamble Hold Time Done */
#define NEMC_TGPD_DPHTD(n)	(1 << (n)) /* 0 ~ 5 */

/* Toggle NAND DQS Setup Time for Data Input Start (bank 1/2/3 TGSL) */
#define NEMC_TGSL_CQDSS2(n)	((n) << 16) /* DQS Setup Time for data input start (bank3) */
#define NEMC_TGSL_CQDSS1(n)	((n) << 8)
#define NEMC_TGSL_CQDSS0(n)	((n) << 0)
/* Toggle NAND DQS Setup Time for Data Input Start (bank 4/5/6 TGSH) */
#define NEMC_TGSH_CQDSS5(n)	((n) << 16) /* DQS Setup Time for data input start (bank6) */
#define NEMC_TGSH_CQDSS4(n)	((n) << 8)
#define NEMC_TGSH_CQDSS3(n)	((n) << 0)

/* Toggle NAND Timer for Random Data Input and Register Read Out */
#define NEMC_TGRR_TD_MASK	(1 << 16) /* Timer Done */
#define NEMC_TGRR_CWAW_MASK	0xFF /* Command Write Cycle to Address Write Cycle Time */

/* Toggle NAND DQS Delay Control */
#define NEMC_TGDR_ERR_MASK	(1 << 29) /* DQS Delay Detect ERROR */
#define NEMC_TGDR_DONE_MASK	(1 << 28) /* Delay Detect Done */
#define NEMC_TGDR_DET		(1 << 23) /* Start Delay Detecting */
#define NEMC_TGDR_AUTO		(1 << 22) /* Hardware Auto-detect & Set Delay Line */
#define NEMC_TGDR_RDQS_BIT	0 /* Number of Delay Elements Used on the Read DQS Delay-line */
#define NEMC_TGDR_RDQS_MASK	(0x1F << NEMC_TGDR_RDQS_BIT)

#define NEMC_CS_COUNT   6   /*THe max count of cs nemc support*/

#define NAND_STATUS_FALL 0x01
#define NAND_IO_CLK_NAME "nemc"
#endif /* __JZ_NEMC_H__ */
