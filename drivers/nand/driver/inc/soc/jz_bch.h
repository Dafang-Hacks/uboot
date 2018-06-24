/*
 * jz4780_bch.h - Ingenic BCH Controller driver head file
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 * Written by Feng Gao <fenggao@ingenic.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JZ_BCH_H__
#define __JZ_BCH_H__

#define BCH_CR			0x000 /* BCH Control register */
#define BCH_CRS			0x004 /* BCH Control Set register */
#define BCH_CRC			0x008 /* BCH Control Clear register */
#define BCH_CNT			0x00C /* BCH ENC/DEC Count register */
#define BCH_DR			0x010 /* BCH Data register */

#define BCH_PAR0		0x014 /* BCH Parity 0 register */
#define BCH_PAR1		0x018 /* BCH Parity 1 register */
#define BCH_PAR2		0x01C /* BCH Parity 2 register */
#define BCH_PAR3		0x020 /* BCH Parity 3 register */
#define BCH_PAR4		0x024 /* BCH Parity 4 register */
#define BCH_PAR5		0x028 /* BCH Parity 5 register */
#define BCH_PAR6		0x02C /* BCH Parity 6 register */
#define BCH_PAR7		0x030 /* BCH Parity 7 register */
#define BCH_PAR8		0x034 /* BCH Parity 8 register */
#define BCH_PAR9		0x038 /* BCH Parity 9 register */
#define BCH_PAR10		0x03C /* BCH Parity 10 register */
#define BCH_PAR11		0x040 /* BCH Parity 11 register */
#define BCH_PAR12		0x044 /* BCH Parity 12 register */
#define BCH_PAR13		0x048 /* BCH Parity 13 register */
#define BCH_PAR14		0x04C /* BCH Parity 14 register */
#define BCH_PAR15		0x050 /* BCH Parity 15 register */
#define BCH_PAR16		0x054 /* BCH Parity 16 register */
#define BCH_PAR17		0x058 /* BCH Parity 17 register */
#define BCH_PAR18		0x05C /* BCH Parity 18 register */
#define BCH_PAR19		0x060 /* BCH Parity 19 register */
#define BCH_PAR20		0x064 /* BCH Parity 20 register */
#define BCH_PAR21		0x068 /* BCH Parity 21 register */
#define BCH_PAR22		0x06C /* BCH Parity 22 register */
#define BCH_PAR23		0x070 /* BCH Parity 23 register */
#define BCH_PAR24		0x074 /* BCH Parity 24 register */
#define BCH_PAR25		0x078 /* BCH Parity 25 register */
#define BCH_PAR26		0x07C /* BCH Parity 26 register */
#define BCH_PAR27		0x080 /* BCH Parity 27 register */
#define BCH_PAR(n)		(BCH_PAR0 + ((n) << 2)) /* BCH Parity n register */

#define BCH_ERR0		0x084 /* BCH Error Report 0 register */
#define BCH_ERR1		0x088 /* BCH Error Report 1 register */
#define BCH_ERR2		0x08C /* BCH Error Report 2 register */
#define BCH_ERR3		0x090 /* BCH Error Report 3 register */
#define BCH_ERR4		0x094 /* BCH Error Report 4 register */
#define BCH_ERR5		0x098 /* BCH Error Report 5 register */
#define BCH_ERR6		0x09C /* BCH Error Report 6 register */
#define BCH_ERR7		0x0A0 /* BCH Error Report 7 register */
#define BCH_ERR8		0x0A4 /* BCH Error Report 8 register */
#define BCH_ERR9		0x0A8 /* BCH Error Report 9 register */
#define BCH_ERR10		0x0AC /* BCH Error Report 10 register */
#define BCH_ERR11		0x0B0 /* BCH Error Report 11 register */
#define BCH_ERR12		0x0B4 /* BCH Error Report 12 register */
#define BCH_ERR13		0x0B8 /* BCH Error Report 13 register */
#define BCH_ERR14		0x0BC /* BCH Error Report 14 register */
#define BCH_ERR15		0x0C0 /* BCH Error Report 15 register */
#define BCH_ERR16		0x0C4 /* BCH Error Report 16 register */
#define BCH_ERR17		0x0C8 /* BCH Error Report 17 register */
#define BCH_ERR18		0x0CC /* BCH Error Report 18 register */
#define BCH_ERR19		0x0D0 /* BCH Error Report 19 register */
#define BCH_ERR20		0x0D4 /* BCH Error Report 20 register */
#define BCH_ERR21		0x0D8 /* BCH Error Report 21 register */
#define BCH_ERR22		0x0DC /* BCH Error Report 22 register */
#define BCH_ERR23		0x0E0 /* BCH Error Report 23 register */
#define BCH_ERR24		0x0E4 /* BCH Error Report 24 register */
#define BCH_ERR25		0x0E8 /* BCH Error Report 25 register */
#define BCH_ERR26		0x0EC /* BCH Error Report 26 register */
#define BCH_ERR27		0x0F0 /* BCH Error Report 27 register */
#define BCH_ERR28		0x0F4 /* BCH Error Report 28 register */
#define BCH_ERR29		0x0F8 /* BCH Error Report 29 register */
#define BCH_ERR30		0x0FC /* BCH Error Report 30 register */
#define BCH_ERR31		0x100 /* BCH Error Report 31 register */
#define BCH_ERR32		0x104 /* BCH Error Report 32 register */
#define BCH_ERR33		0x108 /* BCH Error Report 33 register */
#define BCH_ERR34		0x10C /* BCH Error Report 34 register */
#define BCH_ERR35		0x110 /* BCH Error Report 35 register */
#define BCH_ERR36		0x114 /* BCH Error Report 36 register */
#define BCH_ERR37		0x118 /* BCH Error Report 37 register */
#define BCH_ERR38		0x11C /* BCH Error Report 38 register */
#define BCH_ERR39		0x120 /* BCH Error Report 39 register */
#define BCH_ERR40		0x124 /* BCH Error Report 40 register */
#define BCH_ERR41		0x128 /* BCH Error Report 41 register */
#define BCH_ERR42		0x12C /* BCH Error Report 42 register */
#define BCH_ERR43		0x130 /* BCH Error Report 43 register */
#define BCH_ERR44		0x134 /* BCH Error Report 44 register */
#define BCH_ERR45		0x138 /* BCH Error Report 45 register */
#define BCH_ERR46		0x13C /* BCH Error Report 46 register */
#define BCH_ERR47		0x140 /* BCH Error Report 47 register */
#define BCH_ERR48		0x144 /* BCH Error Report 48 register */
#define BCH_ERR49		0x148 /* BCH Error Report 49 register */
#define BCH_ERR50		0x14C /* BCH Error Report 50 register */
#define BCH_ERR51		0x150 /* BCH Error Report 51 register */
#define BCH_ERR52		0x154 /* BCH Error Report 52 register */
#define BCH_ERR53		0x158 /* BCH Error Report 53 register */
#define BCH_ERR54		0x15C /* BCH Error Report 54 register */
#define BCH_ERR55		0x160 /* BCH Error Report 55 register */
#define BCH_ERR56		0x164 /* BCH Error Report 56 register */
#define BCH_ERR57		0x168 /* BCH Error Report 57 register */
#define BCH_ERR58		0x16C /* BCH Error Report 58 register */
#define BCH_ERR59		0x170 /* BCH Error Report 59 register */
#define BCH_ERR60		0x174 /* BCH Error Report 60 register */
#define BCH_ERR61		0x178 /* BCH Error Report 61 register */
#define BCH_ERR62		0x17C /* BCH Error Report 62 register */
#define BCH_ERR63		0x180 /* BCH Error Report 63 register */
#define BCH_ERR(n)		(BCH_ERR0 + ((n) << 2)) /* BCH Parity n register */

#define BCH_INTS		0x184 /* BCH Interrupt Status register */
#define BCH_INTES		0x188 /* BCH Interrupt Set register */
#define BCH_INTEC		0x18C /* BCH Interrupt Clear register */
#define BCH_INTE		0x190 /* BCH Interrupt Enable register */
#define BCH_TO			0x194 /* BCH User Tag Output register */

/* BCH Control Register*/
#define BCH_CR_TAG_BIT		16
#define BCH_CR_TAG_MASK		(0xffff << BCH_CR_TAG_BIT)
#define BCH_CR_TAG(n)		((n) << BCH_CR_TAG_BIT)  /* BCH User-provided TAG */
#define BCH_CR_MZSB_BIT		13
#define BCH_CR_MZSB_MASK	(0x7 << BCH_CR_MZSB_BIT)
#define BCH_CR_MZEB(n)		((n) << BCH_CR_MZSB_BIT)  /* BCH MAX Zero Bit in Erased Block */
#define BCH_CR_BPS		(1 << 12)  /* BCH Decoder Bypass */
#define BCH_CR_BSEL_BIT		4
#define BCH_CR_BSEL_MASK	(0x7f << BCH_CR_BSEL_BIT)
#define BCH_CR_BSEL(n)		((n) << BCH_CR_BSEL_BIT)  /* n Bit BCH Select */
#define BCH_CR_BSEL_4		(0x4 << BCH_CR_BSEL_BIT)  /* 4 Bit BCH Select */
#define BCH_CR_BSEL_8		(0x8 << BCH_CR_BSEL_BIT)  /* 8 Bit BCH Select */
#define BCH_CR_BSEL_24		(0x18 << BCH_CR_BSEL_BIT) /* 24 Bit BCH Select */
#define BCH_CR_BSEL_40		(0x28 << BCH_CR_BSEL_BIT) /* 40 Bit BCH Select */
#define BCH_CR_BSEL_64		(0x40 << BCH_CR_BSEL_BIT) /* 64 Bit BCH Select */
#define	BCH_CR_ENCE		(1 << 2)  /* BCH Encoding Select */
#define	BCH_CR_DECE		(0 << 2)  /* BCH Decoding Select */
#define	BCH_CR_INIT		(1 << 1)  /* BCH Reset */
#define	BCH_CR_BCHE		(1 << 0)  /* BCH Enable */

/* BCH ENC/DEC Count Register */
#define BCH_CNT_PARITY_BIT	16
#define BCH_CNT_PARITY_MASK	(0x7f << BCH_CNT_PARITY_BIT)
#define BCH_CNT_BLOCK_BIT	0
#define BCH_CNT_BLOCK_MASK	(0x7ff << BCH_CNT_BLOCK_BIT)

/* BCH Error Report Register */
#define BCH_ERR_MASK_BIT	16
#define BCH_ERR_MASK_MASK	(0xffff << BCH_ERR_MASK_BIT)
#define BCH_ERR_INDEX_BIT	0
#define BCH_ERR_INDEX_MASK	(0x7ff << BCH_ERR_INDEX_BIT)

/* BCH Interrupt Status Register */
#define	BCH_INTS_ERRC_BIT	24
#define	BCH_INTS_ERRC_MASK	(0x7f << BCH_INTS_ERRC_BIT)
#define BCH_INTS_BPSO		(1 << 23)
#define BCH_INTS_TERRC_BIT	16
#define BCH_INTS_TERRC_MASK	(0x7f << BCH_INTS_TERRC_BIT)
#define	BCH_INTS_SDMF		(1 << 5)
#define	BCH_INTS_ALLf		(1 << 4)
#define	BCH_INTS_DECF		(1 << 3)
#define	BCH_INTS_ENCF		(1 << 2)
#define	BCH_INTS_UNCOR		(1 << 1)
#define	BCH_INTS_ERR		(1 << 0)

/* BCH Interrupt Enable Register */
#define BCH_INTE_SDMFE		(1 << 5)
#define BCH_INTE_ALL_FE		(1 << 4)
#define BCH_INTE_DECFE		(1 << 3)
#define BCH_INTE_ENCFE		(1 << 2)
#define BCH_INTE_UNCORE		(1 << 1)
#define BCH_INTE_ERRE		(1 << 0)

/* BCH Interrupt Enable Set Register (BHINTES) */
#define BCH_INTES_SDMFES	(1 << 5)
#define BCH_INTES_ALL_FES	(1 << 4)
#define BCH_INTES_DECFES	(1 << 3)
#define BCH_INTES_ENCFES	(1 << 2)
#define BCH_INTES_UNCORES	(1 << 1)
#define BCH_INTES_ERRES		(1 << 0)

/* BCH Interrupt Enable Clear Register (BHINTEC) */
#define BCH_INTEC_SDMFES	(1 << 5)
#define BCH_INTEC_ALL_FEC	(1 << 4)
#define BCH_INTEC_DECFEC	(1 << 3)
#define BCH_INTEC_ENCFEC	(1 << 2)
#define BCH_INTEC_UNCOREC	(1 << 1)
#define BCH_INTEC_ERREC		(1 << 0)

/* BCH User TAG OUTPUT Register */
#define BCH_BHTO_TAGO_BIT	0
#define BCH_BHTO_TAGO_MASK	(0xffff << BCH_BHTO_TAGO_BIT)


#define BCH_ENCODE      1
#define BCH_DECODE      0
#define __bch_cale_eccbytes(eccbit)  (14*(eccbit)>>3)
#define BCH_ENABLE_MAXBITS 64

#define BCH_CLK_NAME    "bch"
#define BCH_CGU_CLK_NAME    "cgu_bch"
#endif /* __JZ_BCH_H__ */
