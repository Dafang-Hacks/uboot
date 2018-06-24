/*
 * linux/include/asm-mips/mach-jz4780/jz4780nfi.h
 *
 * M200 nfi register definition.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 */
#ifndef __JZNFI_H__
#define __JZNFI_H__

#define NFI_BASE	0xB3410000

#define NFI_CS_COUNT   6   /*THe max count of cs nfi support*/
/*************************************************************************
 * NFI (NAND FLASH INTERFACE)
 *************************************************************************/
#if 0
//#define NAND_NFCM	(NFI_BASE + 0x00) /* nand flash command Register */
//#define NAND_NFAD	(NFI_BASE + 0x04) /* nand flash address Register */
#define NAND_NFCS	(NFI_BASE + 0x08) /* nand flash chip select Register */
#define NAND_NFBC	(NFI_BASE + 0x0c) /* nand flash busy control Register */
#define NAND_NFCR	(NFI_BASE + 0x10) /* nfi enable Register */
#define NAND_PNCR 	(NFI_BASE + 0x14) /* nand PN Control Register */
#define NAND_PNDR 	(NFI_BASE + 0x18) /* nand PN Data Register */
#define NAND_BCNT	(NFI_BASE + 0x1c) /* nand Bit Counter */
#define NAND_NFDL	(NFI_BASE + 0x20) /* nand flash DQS delay line */
#define NAND_NFRT	(NFI_BASE + 0x24) /* nand flash read data timeout */
#define NAND_NFIT0	(NFI_BASE + 0x28) /* nand flash time0 */
#define NAND_NFIT1	(NFI_BASE + 0x2c) /* nand flash time1 */
#define NAND_NFIT2	(NFI_BASE + 0x30) /* nand flash time2 */
#define NAND_NFIT3	(NFI_BASE + 0x34) /* nand flash time3 */
#define NAND_NFIT4	(NFI_BASE + 0x38) /* nand flash time4 */
#define NAND_NFITG0	(NFI_BASE + 0x3c) /* toggle nand flash time0 */
#define NAND_NFITG1	(NFI_BASE + 0x40) /* toggle nand flash time1 */
#define NAND_NFIOF0	(NFI_BASE + 0x44) /* ONFI nand flash time0 */
#define NAND_NFIOF1	(NFI_BASE + 0x48) /* ONFI nand flash time1 */
#define NAND_NFBS0	(NFI_BASE + 0x4c) /* NAND flash busy detector0 */
#define NAND_NFBS1	(NFI_BASE + 0x50) /* NAND flash busy detector1 */

//#define REG_NAND_NFCM	REG32(NAND_NFCM)
//#define REG_NAND_NFAD	REG32(NAND_NFAD)
#define REG_NAND_NFCS	REG32(NAND_NFCS)
#define REG_NAND_NFBC	REG32(NAND_NFBC)
#define REG_NAND_NFCR	REG32(NAND_NFCR)
#define REG_NAND_PNCR	REG32(NAND_PNCR)
#define REG_NAND_PNDR	REG32(NAND_PNDR)
#define REG_NAND_BCNT	REG32(NAND_BCNT)
#define REG_NAND_NFDL	REG32(NAND_NFDL)
#define REG_NAND_NFRT	REG32(NAND_NFRT)
#define REG_NAND_NFIT0	REG32(NAND_NFIT0)
#define REG_NAND_NFIT1	REG32(NAND_NFIT1)
#define REG_NAND_NFIT2	REG32(NAND_NFIT2)
#define REG_NAND_NFIT3	REG32(NAND_NFIT3)
#define REG_NAND_NFIT4	REG32(NAND_NFIT4)
#define REG_NAND_NFITG0	REG32(NAND_NFITG0)
#define REG_NAND_NFITG1	REG32(NAND_NFITG1)
#define REG_NAND_NFIOF0	REG32(NAND_NFIOF0)
#define REG_NAND_NFIOF1	REG32(NAND_NFIOF1)
#define REG_NAND_NFBS0	REG32(NAND_NFBS0)
#define REG_NAND_NFBS1	REG32(NAND_NFBS1)
#else
//#define NAND_NFCM	(0x00) /* nand flash command Register */
//#define NAND_NFAD	(0x04) /* nand flash address Register */
#define NAND_NFCS	(0x08) /* nand flash chip select Register */
#define NAND_NFBC	(0x0c) /* nand flash busy control Register */
#define NAND_NFCR	(0x10) /* nfi enable Register */
#define NAND_PNCR 	(0x14) /* nand PN Control Register */
#define NAND_PNDR 	(0x18) /* nand PN Data Register */
#define NAND_BCNT	(0x1c) /* nand Bit Counter */
#define NAND_NFDL	(0x20) /* nand flash DQS delay line */
#define NAND_NFRT	(0x24) /* nand flash read data timeout */
#define NAND_NFIT0	(0x28) /* nand flash time0 */
#define NAND_NFIT1	(0x2c) /* nand flash time1 */
#define NAND_NFIT2	(0x30) /* nand flash time2 */
#define NAND_NFIT3	(0x34) /* nand flash time3 */
#define NAND_NFIT4	(0x38) /* nand flash time4 */
#define NAND_NFITG0	(0x3c) /* toggle nand flash time0 */
#define NAND_NFITG1	(0x40) /* toggle nand flash time1 */
#define NAND_NFIOF0	(0x44) /* ONFI nand flash time0 */
#define NAND_NFIOF1	(0x48) /* ONFI nand flash time1 */
#define NAND_NFBS0	(0x4c) /* NAND flash busy detector0 */
#define NAND_NFBS1	(0x50) /* NAND flash busy detector1 */
#define NAND_NFRB	(0x54) /* NAND flash busy filter */
#endif
/* NAND Flash Command Register */
#define NAND_NFCM_DLYF_MASK		(0x7ff << 21)	 /* Extra delay time before CLE goes to high */
#define NAND_NFCM_DLYF(n)		((n) << 21)	 /* Extra delay time before CLE goes to high */
#define NAND_NFCM_DLYB_MASK		(0x7ff << 10)	 /* Extra delay time after CLE goes to low */
#define NAND_NFCM_DLYB(n)		((n) << 10)	 /* Extra delay time after CLE goes to low */
#define NAND_NFCM_BUSY			(1 << 8)	 /* wait busy start */
#define NAND_NFCM_CMD_MASK		(0xff << 0)	 /* nand flash command */
#define NAND_NFCM_CMD(n)		((n) << 0)	 /* nand flash command */
/* NAND Flash Address Register */
#define NAND_NFAD_DLYF_MASK		(0x7ff << 21)	 /* Extra delay time before ALE goes to high */
#define NAND_NFAD_DLYF(n)		((n) << 21)	 /* Extra delay time before ALE goes to high */
#define NAND_NFAD_DLYB_MASK		(0x7ff << 10)	 /* Extra delay time after ALE goes to low */
#define NAND_NFAD_DLYB(n)		((n) << 10)	 /* Extra delay time after ALE goes to low */
#define NAND_NFAD_BUSY			(1 << 8)	 /* wait busy start */
#define NAND_NFAD_ADDR_MASK		(0xff << 0)	 /* nand flash address */
#define NAND_NFAD_ADDR(n)		((n) << 0)	 /* nand flash address */
/* NAND Flash Chip select Register */
#define NAND_NFCS_CS(n)			(0x1 << (n))	 /* nand chip select */
/* NFI busy control Register */
#define NAND_NFCR_BUSY_MASK(n)		(0x1 << (22 + (n) -1))	 /* nand flash busy int mask */
#define NAND_NFCR_BUSY(n)		(0x1 << (16 + (n) -1))	 /* nand flash busy detect done */
#define NAND_NFCR_EMPTY			(0x1 << 11)		 /* All command/address/write data/read data process done */
#define NAND_NFCR_TMOERR		(0x1 << 10)		 /* nand flash read data timeout error */
#define NAND_NFCR_TMOEN			(0x1 << 9)		 /* nand flash read data timeout enable */

#define NAND_NFCR_BUSYMOD_MASK		(0x3 << 7)		 /* nand flash busy mode select */
#define NAND_NFCR_BUSYMOD(n)		((n) << 7)		 /* nand flash busy mode select */
#define BUSYMOD_DEDGE			0x00			 /* negedge and posedge */
#define BUSYMOD_PEDGE			0x01			 /* posedge only */
#define BUSYMOD_NEDGE			0x10			 /* negedge only */

/* NFI enable Register */
#define NAND_NFCR_CSEN_MASK		(0x3f << 15)
#define NAND_NFCR_CSEN(n)		(0x1 << (15 +(n) - 1))	 /* when the nand is common and NAND_NFCR_CSEN is 1, the CSNOD(n)
									should be set 1. */
#define NAND_NFCR_CSMOD			(0x1 << 15)		 /* if the bit is 1, we should be select chip with NFCS when the nand
									is common nand. moveover the bit is 0.*/
#define NAND_NFCR_COMMON_CS_ENABLE	(0x1 << 15)
#define NAND_NFCR_COMMON_CS_DISABLE	(0x0 << 15)

#define NAND_NFCR_EMPTY			(0x1 << 11)		 /* All command/address/write data/read data process done */
#define NAND_NFCR_IDLE			(0x1 << 6)		 /* NFI is idle;when write->read,
									the bit must is 1 before read access */
#define NAND_NFCR_BUSWIDTH_MASK		(0x1 << 5)		 /* the buswidth of nand interface */
#define NAND_NFCR_BUSWIDTH(n)		((n) << 5)		 /* the buswidth of nand interface */
#define NAND_BUSWIDTH_8BIT		0x0
#define NAND_BUSWIDTH_16BIT		0x1

#define NAND_NFCR_SEL_MASK		(0x7 << 2)		 /* nand flash interface select */
#define NAND_NFCR_SEL(n)		((n) << 2)		 /* nand flash interface select */
#define NAND_IF_COMMON			0x0
#define NAND_IF_EDO			0x1
#define NAND_IF_TOGGLE			0x2
#define NAND_IF_ONFI			0x3
//#define NAND_IF_NORMAL			0x4

#define NAND_NFCR_INIT			(0x1 << 1)		 /* inital NFI, the bit is wrote 1 only,when soft reset */
#define NAND_NFCR_EN			(0x1 << 0)		 /* NFI enable */

#define NAND_NFCR_EDO_EN		(0x1 << 2)      /* nfi enable edo mode */
/* NAND PN Control Register */
// PN(bit 0):0-disable, 1-enable
// PN(bit 1):0-no reset, 1-reset
// BITCNT(bit 2):0-disable, 1-enable
// BITCNT(bit 3):0-calculate, 1's number, 1-calculate 0's number
// BITCNT(bit 4):0-no reset, 1-reset bitcnt
#define NAND_PNCR_BITRST	(1 << 4)
#define NAND_PNCR_BIT_MASK	(1 << 3)
#define NAND_PNCR_BIT0		(0 << 3)
#define NAND_PNCR_BIT1		(1 << 3)
#define NAND_PNCR_BITEN		(1 << 2)
#define NAND_PNCR_PNRST		(1 << 1)
#define NAND_PNCR_PNEN		(1 << 0)

/* NAND flash DQS Delay line */
#define NAND_NFDL_ERR_MASK	(1 << 29) /* DQS Delay Detect ERROR */
#define NAND_NFDL_DONE_MASK	(1 << 28) /* Delay Detect Done */
#define NAND_NFDL_DET		(1 << 23) /* Start Delay Detecting; the bit is wrote 1 only */
#define NAND_NFDL_AUTO		(1 << 22) /* Hardware Auto-detect & Set Delay Line */
#define NAND_NFDL_ADQS_BIT	16 /* Number of Delay Elements detect by hardware */
#define NAND_NFDL_ADQS_MASK	(0x1F << NAND_NFDL_ADQS_BIT)
#define NAND_NFDL_IDQS_BIT	8 /* Inital number of Delay Elements Used on auto detect */
#define NAND_NFDL_IDQS_MASK	(0x1F << NAND_NFDL_IDQS_BIT)
#define NAND_NFDL_IDQS(n)	((n) << NAND_NFDL_IDQS_BIT)
#define NAND_NFDL_RDQS_BIT	0 /* Number of Delay Elements Used on the Read DQS Delay-line */
#define NAND_NFDL_RDQS_MASK	(0x1F << NAND_NFDL_RDQS_BIT)
#define NAND_NFDL_RDQS(n)	((n) << NAND_NFDL_RDQS_BIT)

/* NAND	flash timing0 */
#define NAND_NFIT0_SWE_BIT	16 /* Number of clock cycles+1(HCLK) to set up the WE; min ~ max -> 0x0 ~ 0xfffe */
#define NAND_NFIT0_SWE_MASK	(0xFFFF << NAND_NFIT0_SWE_BIT)
#define NAND_NFIT0_SWE(n)	((n) << NAND_NFIT0_SWE_BIT)
#define NAND_NFIT0_WWE_BIT	0 /* Define the minimum number of HCLK clock cycles to assert WE;min~max -> 0x1~0xfffe */
#define NAND_NFIT0_WWE_MASK	(0xFFFF << NAND_NFIT0_WWE_BIT)
#define NAND_NFIT0_WWE(n)	((n) << NAND_NFIT0_WWE_BIT)

/* NAND	flash timing1 */
#define NAND_NFIT1_HWE_BIT	16 /* Number of clock cycles+1(HCLK) to hold the WE; min ~ max -> 0x0 ~ 0xfffe */
#define NAND_NFIT1_HWE_MASK	(0xFFFF << NAND_NFIT1_HWE_BIT)
#define NAND_NFIT1_HWE(n)	((n) << NAND_NFIT1_HWE_BIT)
#define NAND_NFIT1_SRE_BIT	0 /* Number of clock cycles+1(HCLK) to set up the RE; min~max -> 0x0~0xfffe */
#define NAND_NFIT1_SRE_MASK	(0xFFFF << NAND_NFIT1_SRE_BIT)
#define NAND_NFIT1_SRE(n)	((n) << NAND_NFIT1_SRE_BIT)

/* NAND	flash timing2 */
#define NAND_NFIT2_WRE_BIT	16 /* Define the minimum number of HCLK clock cycles to assert RE;min~max -> 0x1~0xfffe */
#define NAND_NFIT2_WRE_MASK	(0xFFFF << NAND_NFIT2_WRE_BIT)
#define NAND_NFIT2_WRE(n)	((n) << NAND_NFIT2_WRE_BIT)
#define NAND_NFIT2_HRE_BIT	0 /* Number of clock cycles+1(HCLK) to hold the RE; min ~ max -> 0x0 ~ 0xfffe */
#define NAND_NFIT2_HRE_MASK	(0xFFFF << NAND_NFIT2_HRE_BIT)
#define NAND_NFIT2_HRE(n)	((n) << NAND_NFIT2_HRE_BIT)

/* NAND	flash timing3 */
#define NAND_NFIT3_SCS_BIT	16 /* Nand chip select setup time; min~max -> 0x1~0xfffe */
#define NAND_NFIT3_SCS_MASK	(0xFFFF << NAND_NFIT3_SCS_BIT)
#define NAND_NFIT3_SCS(n)	((n) << NAND_NFIT3_SCS_BIT)
#define NAND_NFIT3_WCS_BIT	0 /* Nand chip select wait time; min~max -> 0x1~0xfffe */
#define NAND_NFIT3_WCS_MASK	(0xFFFF << NAND_NFIT3_WCS_BIT)
#define NAND_NFIT3_WCS(n)	((n) << NAND_NFIT3_WCS_BIT)


/* NAND	flash timing4 */
#define NAND_NFIT4_BUSY_BIT	16 /* Ready to RE high time; min~max -> 0x0~0xfffe */
#define NAND_NFIT4_BUSY_MASK	(0xFFFF << NAND_NFIT4_BUSY_BIT)
#define NAND_NFIT4_BUSY(n)	((n) << NAND_NFIT4_BUSY_BIT)
#define NAND_NFIT4_EDO_BIT	0 /* Nand flash EDO mode delay for reading data; min~max -> 0x1~0xfffe */
#define NAND_NFIT4_EDO_MASK	(0xFFFF << NAND_NFIT4_EDO_BIT)
#define NAND_NFIT4_EDO(n)	((n) << NAND_NFIT4_EDO_BIT)

/* TOGGLE NAND flash timing0 */
#define NAND_NFITG0_FRE_BIT	24 /* First toggle nand data read delay */
#define NAND_NFITG0_FRE_MASK	(0xFF << NAND_NFITG0_FRE_BIT)
#define NAND_NFITG0_FRE(n)	((n) << NAND_NFITG0_FRE_BIT)
#define NAND_NFITG0_FDQS_BIT	16 /* First toggle nand data write delay */
#define NAND_NFITG0_FDQS_MASK	(0xFF << NAND_NFITG0_FDQS_BIT)
#define NAND_NFITG0_FDQS(n)	((n) << NAND_NFITG0_FDQS_BIT)
#define NAND_NFITG0_SDQS_BIT	8 /* Number of clock cycles(+1) HCLK to set up the DQS */
#define NAND_NFITG0_SDQS_MASK	(0xFF << NAND_NFITG0_SDQS_BIT)
#define NAND_NFITG0_SDQS(n)	((n) << NAND_NFITG0_SDQS_BIT)
#define NAND_NFITG0_HDQS_BIT	0 /* Defines the number of HCLK clock cycles(+1) to hold the DQS */
#define NAND_NFITG0_HDQS_MASK	(0xFF << NAND_NFITG0_HDQS_BIT)
#define NAND_NFITG0_HDQS(n)	((n) << NAND_NFITG0_HDQS_BIT)

/* TOGGLE NAND flash timing1 */
#define NAND_NFITG1_DQS2IDLE_BIT	8 /* RE & DQS_0 go to high when CS clear */
#define NAND_NFITG1_DQS2IDLE_MASK	(0xFF << NAND_NFITG1_DQS2IDLE_BIT)
#define NAND_NFITG1_DQS2IDLE(n)		((n) << NAND_NFITG1_DQS2IDLE_BIT)
#define NAND_NFITG1_DQSRE_BIT	0 /* Number of clock cycles(+1) HCLK to set up the DQS_1_MASK */
#define NAND_NFITG1_DQSRE_MASK	(0xFF << NAND_NFITG1_DQSRE_BIT)
#define NAND_NFITG1_DQSRE(n)	((n) << NAND_NFITG1_DQSRE_BIT)

/* ONFI NAND flash timing0 */
#define NAND_NFIOF0_WR2CLE_BIT	24 /* WR -> CLE goes to high */
#define NAND_NFIOF0_WR2CLE_MASK	(0xFF << NAND_NFIOF0_WR2CLE_BIT)
#define NAND_NFIOF0_WR2CLE(n)	((n) << NAND_NFIOF0_WR2CLE_BIT)
#define NAND_NFIOF0_DQSS_BIT	16 /* delay for DQS_0 & DQ_0 */
#define NAND_NFIOF0_DQSS_MASK	(0xFF << NAND_NFIOF0_DQSS_BIT)
#define NAND_NFIOF0_DQSS(n)	((n) << NAND_NFIOF0_DQSS_BIT)
#define NAND_NFIOF0_SCLK_BIT	8 /* Number of clock cycles(+1) HCLK to set up CLK */
#define NAND_NFIOF0_SCLK_MASK	(0xFF << NAND_NFIOF0_SCLK_BIT)
#define NAND_NFIOF0_SCLK(n)	((n) << NAND_NFIOF0_SCLK_BIT)
#define NAND_NFIOF0_HCLK_BIT	0 /* Defines the number of HCLK clock cycles(+1) to hold CLK */
#define NAND_NFIOF0_HCLK_MASK	(0xFF << NAND_NFIOF0_HCLK_BIT)
#define NAND_NFIOF0_HCLK(n)	((n) << NAND_NFIOF0_HCLK_BIT)

/* ONFI NAND flash timing1 */
#define NAND_NFIOF1_DQS2IDLE_BIT	8 /* RE & DQS_0 go to input state when CS clear; the bit can't be 0 */
#define NAND_NFIOF1_DQS2IDLE_MASK	(0xFF << NAND_NFITG1_DQS2IDLE_BIT)
#define NAND_NFIOF1_DQS2IDLE(n)		((n) << NAND_NFITG1_DQS2IDLE_BIT)
#define NAND_NFIOF1_R2IDLE_BIT		0 /* When R2IDLE cycles done,W/E goes to HI_Z; the bit can't be 0 */
#define NAND_NFIOF1_R2IDLE_MASK		(0xFF << NAND_NFIOF1_R2IDLE_BIT)
#define NAND_NFIOF1_R2IDLE(n)		((n) << NAND_NFIOF1_R2IDLE_BIT)

/* NAND flash busy detector0 */
#define NAND_NFIBS0_CB4_BIT	24 /* FBR detector4 select */
#define NAND_NFIBS0_CB4_MASK	(0x3F << NAND_NFIBS0_CB4_BIT)
#define NAND_NFIBS0_CB4(n)	((n) << NAND_NFIBS0_CB4_BIT)
#define NAND_NFIBS0_CB3_BIT	16 /* FBR detector3 select */
#define NAND_NFIBS0_CB3_MASK	(0x3F << NAND_NFIBS0_CB3_BIT)
#define NAND_NFIBS0_CB3(n)	((n) << NAND_NFIBS0_CB3_BIT)
#define NAND_NFIBS0_CB2_BIT	8 /* FBR detector2 select */
#define NAND_NFIBS0_CB2_MASK	(0x3F << NAND_NFIBS0_CB2_BIT)
#define NAND_NFIBS0_CB2(n)	((n) << NAND_NFIBS0_CB2_BIT)
#define NAND_NFIBS0_CB1_BIT	0 /* FBR detector1 select */
#define NAND_NFIBS0_CB1_MASK	(0x3F << NAND_NFIBS0_CB1_BIT)
#define NAND_NFIBS0_CB1(n)	((n) << NAND_NFIBS0_CB1_BIT)

/* NAND flash busy detector1 */
#define NAND_NFIBS1_CB6_BIT	8 /* FBR detector6 select */
#define NAND_NFIBS1_CB6_MASK	(0x3F << NAND_NFIBS1_CB6_BIT)
#define NAND_NFIBS1_CB6(n)	((n) << NAND_NFIBS1_CB6_BIT)
#define NAND_NFIBS1_CB5_BIT	0 /* FBR detector5 select */
#define NAND_NFIBS1_CB5_MASK	(0x3F << NAND_NFIBS1_CB5_BIT)
#define NAND_NFIBS1_CB5(n)	((n) << NAND_NFIBS1_CB5_BIT)

#define NAND_IO_CLK_NAME "nfi"
#endif /* __JZNFI_H__ */
