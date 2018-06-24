/*
 * JZ4780 NAND (NEMC/BCH) driver definitions
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
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

#ifndef __NAND_H__
#define __NAND_H__

/* NEMC registers */
#define NEMC_SMCR1  0x014
#define NEMC_SMCR2  0x018
#define NEMC_SMCR3  0x01c
#define NEMC_SMCR4  0x020
#define NEMC_SMCR5  0x024
#define NEMC_SMCR6  0x028
#define NEMC_NFCSR  0x050
#define NEMC_PNCR   0x100

/* NEMC NAND Flash Control & Status Register (NFCSR) */
#define NEMC_NFCSR_FCEn(n) (1 << (((n - 1) * 2) + 1))
#define NEMC_NFCSR_NFEn(n) (1 << (((n - 1) * 2) + 0))

/* NEMC PN Control Register (PNCR) */
#define NEMC_PNCR_PNRST (1 << 1)
#define NEMC_PNCR_PNEN  (1 << 0)

/* BCH registers */
#define BCH_BHCR    0x000
#define BCH_BHCSR   0x004
#define BCH_BHCCR   0x008
#define BCH_BHCNT   0x00c
#define BCH_BHDR    0x010
#define BCH_BHPAR0  0x014
/* ...  BCH_BHPARn */
#define BCH_BHPAR27 0x080
#define BCH_BHERR0  0x084
/* ...  BCH_BHERRn */
#define BCH_BHERR63 0x180
#define BCH_BHINT   0x184
#define BCH_BHINTE  0x190

/* BCH Control Register (BHCR) */
#define BCH_BHCR_TAG_SHIFT  16
#define BCH_BHCR_TAG_MASK   (0xffff << BCH_BHCR_TAG_SHIFT)
#define BCH_BHCR_MZEB_SHIFT 13
#define BCH_BHCR_MZEB_MASK  (0x7 << BCH_BHCR_MZEB_SHIFT)
#define BCH_BHCR_BPS        (1 << 12)
#define BCH_BHCR_BSEL_SHIFT 4
#define BCH_BHCR_BSEL_MASK  (0x7f << BCH_BHCR_BSEL_MASK)
#define BCH_BHCR_ENCE       (1 << 2)
#define BCH_BHCR_INIT       (1 << 1)
#define BCH_BHCR_BCHE       (1 << 0)

/* BCH Count Register (BHCNT) */
#define BCH_BHCNT_PARITYSIZE_SHIFT 16
#define BCH_BHCNT_PARITYSIZE_MASK  (0x7f << BCH_BHCNT_PARITYSIZE_SHIFT)
#define BCH_BHCNT_BLOCKSIZE_SHIFT  0
#define BCH_BHCNT_BLOCKSIZE_MASK   (0x7ff << BCH_BHCNT_BLOCKSIZE_SHIFT)

/* BCH Error Report Register (BCHERRn) */
#define BCH_BHERRn_MASK_SHIFT  16
#define BCH_BHERRn_MASK_MASK   (0xffff << BCH_BHERRn_MASK_SHIFT)
#define BCH_BHERRn_INDEX_SHIFT 0
#define BCH_BHERRn_INDEX_MASK  (0x7ff << BCH_BHERRn_INDEX_SHIFT)

/* BCH Interrupt Status Register (BHINT) */
#define BCH_BHINT_ERRC_SHIFT  24
#define BCH_BHINT_ERRC_MASK   (0x7f << BCH_BHINT_ERRC_SHIFT)
#define BCH_BHINT_BPSO        (1 << 23)
#define BCH_BHINT_TERRC_SHIFT 16
#define BCH_BHINT_TERRC_MASK  (0x7f << BCH_BHINT_TERRC_SHIFT)
#define BCH_BHINT_SDMF        (1 << 5)
#define BCH_BHINT_ALL_F       (1 << 4)
#define BCH_BHINT_DECF        (1 << 3)
#define BCH_BHINT_ENCF        (1 << 2)
#define BCH_BHINT_UNCOR       (1 << 1)
#define BCH_BHINT_ERR         (1 << 0)

extern int jz4780_nand_init(struct nand_chip *nand);
extern void jz4780_nand_set_pn(nand_info_t *nand, int bytes, int size, int skip);

#endif /* __NAND_H__ */
