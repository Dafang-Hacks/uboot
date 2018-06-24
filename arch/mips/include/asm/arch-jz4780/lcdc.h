/*
 * JZ4780 LCDC definitions
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


#ifndef __LCDC_H__
#define __LCDC__H__

#include <asm/arch/base.h>



/*************************************************************************
 * LCDC (LCD Controller)
 *************************************************************************/

#define LCDC_CFG	(LCDC0_BASE + 0x00)
#define LCDC_CTRL	(LCDC0_BASE + 0x30)
#define LCDC_STATE	(LCDC0_BASE + 0x34)
#define LCDC_OSDC	(LCDC0_BASE + 0x100)
#define LCDC_OSDCTRL	(LCDC0_BASE + 0x104)
#define LCDC_OSDS	(LCDC0_BASE + 0x108)
#define LCDC_BGC0	(LCDC0_BASE + 0x10C)
#define LCDC_BGC1	(LCDC0_BASE + 0x2C4)
#define LCDC_KEY0	(LCDC0_BASE + 0x110)
#define LCDC_KEY1	(LCDC0_BASE + 0x114)
#define LCDC_ALPHA	(LCDC0_BASE + 0x118)
#define LCDC_RGBC	(LCDC0_BASE + 0x90)
#define LCDC_VAT	(LCDC0_BASE + 0x0c)
#define LCDC_DAH	(LCDC0_BASE + 0x10)
#define LCDC_DAV	(LCDC0_BASE + 0x14)
#define LCDC_XYP0	(LCDC0_BASE + 0x120)
#define LCDC_XYP1	(LCDC0_BASE + 0x124)
#define LCDC_SIZE0	(LCDC0_BASE + 0x128)
#define LCDC_SIZE1	(LCDC0_BASE + 0x12C)
#define LCDC_VSYNC	(LCDC0_BASE + 0x04)
#define LCDC_HSYNC	(LCDC0_BASE + 0x08)
#define LCDC_PS		(LCDC0_BASE + 0x18)
#define LCDC_CLS	(LCDC0_BASE + 0x1c)
#define LCDC_SPL	(LCDC0_BASE + 0x20)
#define LCDC_REV	(LCDC0_BASE + 0x24)
#define LCDC_IID	(LCDC0_BASE + 0x38)
#define LCDC_DA0	(LCDC0_BASE + 0x40)
#define LCDC_SA0	(LCDC0_BASE + 0x44)
#define LCDC_FID0	(LCDC0_BASE + 0x48)
#define LCDC_CMD0	(LCDC0_BASE + 0x4c)
#define LCDC_DA1	(LCDC0_BASE + 0x50)
#define LCDC_SA1	(LCDC0_BASE + 0x54)
#define LCDC_FID1	(LCDC0_BASE + 0x58)
#define LCDC_CMD1	(LCDC0_BASE + 0x5c)
#define LCDC_OFFS0	(LCDC0_BASE + 0x60)
#define LCDC_PW0	(LCDC0_BASE + 0x64)
#define LCDC_CNUM0	(LCDC0_BASE + 0x68)
#define LCDC_DESSIZE0	(LCDC0_BASE + 0x6C)
#define LCDC_OFFS1	(LCDC0_BASE + 0x70)
#define LCDC_PW1	(LCDC0_BASE + 0x74)
#define LCDC_CNUM1	(LCDC0_BASE + 0x78)
#define LCDC_DESSIZE1	(LCDC0_BASE + 0x7C)
#define LCDC_PCFG	(LCDC0_BASE + 0x2C0)
#define LCDC_CPOS1		(0x78)
#define LCDC_DUAL_CTRL		(0x2c8)
#define LCDC_ENH_CFG		(0x400)
#define LCDC_ENH_CSCCFG		(0x404)
#define LCDC_ENH_LUMACFG	(0x408)
#define LCDC_ENH_CHROCFG0	(0x40c)
#define LCDC_ENH_CHROCFG1	(0x410)
#define LCDC_ENH_DITHERCFG	(0x414)
#define LCDC_ENH_STATUS		(0x418)
#define LCDC_ENH_GAMMA		(0x800)
#define LCDC_ENH_VEE		(0x1000)

/* LCD Configure Register */
#define LCDC_CFG_LCDPIN_BIT	31  /* LCD pins selection */
#define LCDC_CFG_LCDPIN_MASK	(0x1 << LCDC_CFG_LCDPIN_BIT)
#define LCDC_CFG_LCDPIN_LCD	(0x0 << LCDC_CFG_LCDPIN_BIT)
#define LCDC_CFG_LCDPIN_SLCD	(0x1 << LCDC_CFG_LCDPIN_BIT)
#define LCDC_CFG_TVEPEH		(1 << 30) /* TVE PAL enable extra halfline signal */
                                          /* Keep this bit to 0 */
#define LCDC_CFG_NEWDES		(1 << 28) /* use new descripter. old: 4words, new:8words */
#define LCDC_CFG_PALBP		(1 << 27) /* bypass data format and alpha blending */
#define LCDC_CFG_TVEN		(1 << 26) /* indicate the terminal is lcd or tv */
#define LCDC_CFG_RECOVER		(1 << 25) /* Auto recover when output fifo underrun */
/* Dither function has been move to DITHER CFG Register*/
#define LCDC_CFG_PSM		(1 << 23) /* PS signal mode */
#define LCDC_CFG_CLSM		(1 << 22) /* CLS signal mode */
#define LCDC_CFG_SPLM		(1 << 21) /* SPL signal mode */
#define LCDC_CFG_REVM		(1 << 20) /* REV signal mode */
#define LCDC_CFG_HSYNM		(1 << 19) /* HSYNC signal mode */
#define LCDC_CFG_PCLKM		(1 << 18) /* PCLK signal mode */
#define LCDC_CFG_INVDAT		(1 << 17) /* Inverse output data */
#define LCDC_CFG_SYNDIR_IN	(1 << 16) /* VSYNC&HSYNC direction */
#define LCDC_CFG_PSP		(1 << 15) /* PS pin reset state */
#define LCDC_CFG_CLSP		(1 << 14) /* CLS pin reset state */
#define LCDC_CFG_SPLP		(1 << 13) /* SPL pin reset state */
#define LCDC_CFG_REVP		(1 << 12) /* REV pin reset state */
#define LCDC_CFG_HSP		(1 << 11) /* HSYNC polarity:0-active high,1-active low */
#define LCDC_CFG_PCP		(1 << 10) /* PCLK polarity:0-rising,1-falling */
#define LCDC_CFG_DEP		(1 << 9)  /* DE polarity:0-active high,1-active low */
#define LCDC_CFG_VSP		(1 << 8)  /* VSYNC polarity:0-rising,1-falling */
#define LCDC_CFG_MODE_TFT_18BIT 	(1 << 7)  /* 18bit TFT */
#define LCDC_CFG_MODE_TFT_16BIT 	(0 << 7)  /* 16bit TFT */
#define LCDC_CFG_MODE_TFT_24BIT 	(1 << 6)  /* 24bit TFT */

#define LCDC_CFG_MODE_BIT	0  /* Display Device Mode Select */
#define LCDC_CFG_MODE_MASK	(0x0f << LCDC_CFG_MODE_BIT)
#define LCDC_CFG_MODE_GENERIC_TFT	(0 << LCDC_CFG_MODE_BIT) /* 16,18 bit TFT */
#define LCDC_CFG_MODE_SPECIAL_TFT_1	(1 << LCDC_CFG_MODE_BIT)
#define LCDC_CFG_MODE_SPECIAL_TFT_2	(2 << LCDC_CFG_MODE_BIT)
#define LCDC_CFG_MODE_SPECIAL_TFT_3	(3 << LCDC_CFG_MODE_BIT)
#define LCDC_CFG_MODE_NONINTER_CCIR656	(4 << LCDC_CFG_MODE_BIT)
#define LCDC_CFG_MODE_INTER_CCIR656	(6 << LCDC_CFG_MODE_BIT)
#define LCDC_CFG_MODE_SERIAL_TFT	(12 << LCDC_CFG_MODE_BIT)
#define LCDC_CFG_MODE_LCM  		(13 << LCDC_CFG_MODE_BIT)

/* LCD Control Register */
#define LCDC_CTRL_PINMD		(1 << 31) /* This register set Pin distribution in 16-bit parallel mode
					    0: 16-bit data correspond with LCDC_D[15:0]
					    1: 16-bit data correspond with LCDC_D[17:10], LCDC_D[8:1] */
#define LCDC_CTRL_BST_BIT	28  /* Burst Length Selection */
#define LCDC_CTRL_BST_MASK	(0x7 << LCDC_CTRL_BST_BIT)
#define LCDC_CTRL_BST_4		(0 << LCDC_CTRL_BST_BIT) /* 4-word */
#define LCDC_CTRL_BST_8		(1 << LCDC_CTRL_BST_BIT) /* 8-word */
#define LCDC_CTRL_BST_16	(2 << LCDC_CTRL_BST_BIT) /* 16-word */
#define LCDC_CTRL_BST_32	(3 << LCDC_CTRL_BST_BIT) /* 32-word */
#define LCDC_CTRL_BST_64	(4 << LCDC_CTRL_BST_BIT) /* 64-word */
#define LCDC_CTRL_RGB565		(0 << 27) /* RGB565 mode(foreground 0 in OSD mode) */
#define LCDC_CTRL_RGB555		(1 << 27) /* RGB555 mode(foreground 0 in OSD mode) */
#define LCDC_CTRL_OFUP		(1 << 26) /* Output FIFO underrun protection enable */
#define LCDC_CTRL_PDD_BIT	16  /* Load Palette Delay Counter */
#define LCDC_CTRL_PDD_MASK	(0xff << LCDC_CTRL_PDD_BIT)
                                /* Keep this bit to 0 */
#define LCDC_CTRL_DACTE		(1 << 14) /* DAC loop back test */
#define LCDC_CTRL_EOFM		(1 << 13) /* EOF interrupt mask */
#define LCDC_CTRL_SOFM		(1 << 12) /* SOF interrupt mask */
#define LCDC_CTRL_OFUM		(1 << 11) /* Output FIFO underrun interrupt mask */
#define LCDC_CTRL_IFUM0		(1 << 10) /* Input FIFO 0 underrun interrupt mask */
#define LCDC_CTRL_IFUM1		(1 << 9)  /* Input FIFO 1 underrun interrupt mask */
#define LCDC_CTRL_LDDM		(1 << 8)  /* LCD disable done interrupt mask */
#define LCDC_CTRL_QDM		(1 << 7)  /* LCD quick disable done interrupt mask */
#define LCDC_CTRL_BEDN		(1 << 6)  /* Endian selection */
#define LCDC_CTRL_PEDN		(1 << 5)  /* Endian in byte:0-msb first, 1-lsb first */
#define LCDC_CTRL_DIS		(1 << 4)  /* Disable indicate bit */
#define LCDC_CTRL_ENA		(1 << 3)  /* LCDC enable bit */
#define LCDC_CTRL_BPP_BIT	0  /* Bits Per Pixel */
#define LCDC_CTRL_BPP_MASK	(0x07 << LCDC_CTRL_BPP_BIT)
#define LCDC_CTRL_BPP_1		(0 << LCDC_CTRL_BPP_BIT) /* 1 bpp */
#define LCDC_CTRL_BPP_2		(1 << LCDC_CTRL_BPP_BIT) /* 2 bpp */
#define LCDC_CTRL_BPP_4		(2 << LCDC_CTRL_BPP_BIT) /* 4 bpp */
#define LCDC_CTRL_BPP_8		(3 << LCDC_CTRL_BPP_BIT) /* 8 bpp */
#define LCDC_CTRL_BPP_16	(4 << LCDC_CTRL_BPP_BIT) /* 15/16 bpp */
#define LCDC_CTRL_BPP_18_24	(5 << LCDC_CTRL_BPP_BIT) /* 18/24/32 bpp */
#define LCDC_CTRL_BPP_CMPS_24	(6 << LCDC_CTRL_BPP_BIT) /* 24 compress bpp */
#define LCDC_CTRL_BPP_30	(7 << LCDC_CTRL_BPP_BIT) /* 30 bpp */

/* LCD Status Register */
#define LCDC_STATE_QD		(1 << 7) /* Quick Disable Done */
#define LCDC_STATE_EOF		(1 << 5) /* EOF Flag */
#define LCDC_STATE_SOF		(1 << 4) /* SOF Flag */
#define LCDC_STATE_OFU		(1 << 3) /* Output FIFO Underrun */
#define LCDC_STATE_IFU0		(1 << 2) /* Input FIFO 0 Underrun */
#define LCDC_STATE_IFU1		(1 << 1) /* Input FIFO 1 Underrun */
#define LCDC_STATE_LDD		(1 << 0) /* LCD Disabled */

/* OSD Configure Register */
#define LCDC_OSDC_PREMULTI1		(1 << 23) /*
						   * Premulti enable of foreground 1
						   * 0:data has been premultied and don't need premulti
						   * 1:data should be premultied by lcd
						   */
#define LCDC_OSDC_COEF_SLE1_BIT		21 /* Select coefficient for foreground 1 */
#define LCDC_OSDC_COEF_SLE1_MASK		(0x03 << LCDC_OSDC_COEF_SLE1_BIT)
#define LCDC_OSDC_COEF_SLE1_0		(0 << LCDC_OSDC_COEF_SLE1_BIT) /* 00:0 */
#define LCDC_OSDC_COEF_SLE1_1		(1 << LCDC_OSDC_COEF_SLE1_BIT) /* 01:1 */
#define LCDC_OSDC_COEF_SLE1_2		(2 << LCDC_OSDC_COEF_SLE1_BIT) /* 10:alpha0 */
#define LCDC_OSDC_COEF_SLE1_3		(3 << LCDC_OSDC_COEF_SLE1_BIT) /* 11:1-alpha0 */

#define LCDC_OSDC_PREMULTI0		(1 << 20) /*
						   * Premulti enable of foreground 0
						   * 0:data has been premultied and don't need premulti
						   * 1:data should be premultied by lcd
						   */
#define LCDC_OSDC_COEF_SLE0_BIT		18 /* Select coefficient for foreground 0 */
#define LCDC_OSDC_COEF_SLE0_MASK		(0x03 << LCDC_OSDC_COEF_SLE0_BIT)
#define LCDC_OSDC_COEF_SLE0_0		(0 << LCDC_OSDC_COEF_SLE0_BIT) /* 00:0 */
#define LCDC_OSDC_COEF_SLE0_1		(1 << LCDC_OSDC_COEF_SLE0_BIT) /* 01:1 */
#define LCDC_OSDC_COEF_SLE0_2		(2 << LCDC_OSDC_COEF_SLE0_BIT) /* 10:alpha1 */
#define LCDC_OSDC_COEF_SLE0_3		(3 << LCDC_OSDC_COEF_SLE0_BIT) /* 11:1-alpha1 */
#define LCDC_OSDC_ALPHAMD1		(1 << 17) /* Alpha blending mode for foreground 1 */

#define LCDC_OSDC_SOFM1		(1 << 15) /* Start of frame interrupt mask for foreground 1 */
#define LCDC_OSDC_EOFM1		(1 << 14) /* End of frame interrupt mask for foreground 1 */
#define LCDC_OSDC_SOFM0		(1 << 11) /* Start of frame interrupt mask for foreground 0 */
#define LCDC_OSDC_EOFM0		(1 << 10) /* End of frame interrupt mask for foreground 0 */
#define LCDC_OSDC_DENDM		(1 << 9) /* Display end interrupt mask */
#define LCDC_OSDC_F1EN		(1 << 4) /* enable foreground 1 */
#define LCDC_OSDC_F0EN		(1 << 3) /* enable foreground 0 */
#define LCDC_OSDC_ALPHAEN	(1 << 2) /* enable alpha blending */
#define LCDC_OSDC_ALPHAMD0	(1 << 1) /* alpha blending mode */
#define LCDC_OSDC_OSDEN		(1 << 0) /* OSD mode enable */

/* OSD Controll Register */
#define LCDC_OSDCTRL_IPU_CLKEN	(1 << 15) /* IPU clock enable: 1: enable; 0:disable */
#define LCDC_OSDCTRL_RGB0_RGB565	(0 << 5) /* foreground 0, 16bpp, 0-RGB565, 1-RGB555 */
#define LCDC_OSDCTRL_RGB0_RGB555	(1 << 5) /* foreground 0, 16bpp, 0-RGB565, 1-RGB555 */
#define LCDC_OSDCTRL_RGB1_RGB565	(0 << 4) /* foreground 1, 16bpp, 0-RGB565, 1-RGB555 */
#define LCDC_OSDCTRL_RGB1_RGB555	(1 << 4) /* foreground 1, 16bpp, 0-RGB565, 1-RGB555 */

#define LCDC_OSDCTRL_BPP_BIT	0 /* Bits Per Pixel of OSD Channel 1 */
#define LCDC_OSDCTRL_BPP_MASK	(0x7<<LCDC_OSDCTRL_BPP_BIT) /* Bits Per Pixel of OSD Channel 1's MASK */
#define LCDC_OSDCTRL_BPP_15_16	(4 << LCDC_OSDCTRL_BPP_BIT) /* RGB 15,16 bit*/
#define LCDC_OSDCTRL_BPP_18_24	(5 << LCDC_OSDCTRL_BPP_BIT) /* RGB 18,24 bit*/
#define LCDC_OSDCTRL_BPP_CMPS_24	(6 << LCDC_OSDCTRL_BPP_BIT) /* RGB compress 24 bit*/
#define LCDC_OSDCTRL_BPP_30		(7 << LCDC_OSDCTRL_BPP_BIT) /* RGB 30 bit*/

/* OSD State Register */
#define LCDC_OSDS_SOF1		(1 << 15) /* Start of frame flag for foreground 1 */
#define LCDC_OSDS_EOF1		(1 << 14) /* End of frame flag for foreground 1 */
#define LCDC_OSDS_SOF0		(1 << 11) /* Start of frame flag for foreground 0 */
#define LCDC_OSDS_EOF0		(1 << 10) /* End of frame flag for foreground 0 */
#define LCDC_OSDS_DEND		(1 << 8) /* Display end */

/* Background 0 or Background 1 Color Register */
#define LCDC_BGC_RED_OFFSET	16  /* Red color offset */
#define LCDC_BGC_RED_MASK	(0xFF << LCDC_BGC_RED_OFFSET)
#define LCDC_BGC_GREEN_OFFSET    8   /* Green color offset */
#define LCDC_BGC_GREEN_MASK	(0xFF << LCDC_BGC_GREEN_OFFSET)
#define LCDC_BGC_BLUE_OFFSET	0   /* Blue color offset */
#define LCDC_BGC_BLUE_MASK	(0xFF << LCDC_BGC_BLUE_OFFSET)

/* Foreground 0 or Foreground 1 Color Key Register */
#define LCDC_KEY_KEYEN		(1 << 31)   /* enable color key */
#define LCDC_KEY_KEYMD		(1 << 30)   /* color key mode */
#define LCDC_KEY_RED_OFFSET	16  /* Red color offset */
#define LCDC_KEY_RED_MASK	(0xFF << LCDC_KEY_RED_OFFSET)
#define LCDC_KEY_GREEN_OFFSET	8   /* Green color offset */
#define LCDC_KEY_GREEN_MASK	(0xFF << LCDC_KEY_GREEN_OFFSET)
#define LCDC_KEY_BLUE_OFFSET	0   /* Blue color offset */
#define LCDC_KEY_BLUE_MASK	(0xFF << LCDC_KEY_BLUE_OFFSET)
#define LCDC_KEY_MASK		(LCDC_KEY_RED_MASK | LCDC_KEY_GREEN_MASK\
/* ALPHA Register */
#define LCDC_ALPHA1_OFFSET	8 /* ALPHA 1 offset */
#define LCDC_ALPHA1_MASK		(0xFF << LCDC_ALPHA1_OFFSET)
#define LCDC_ALPHA0_OFFSET	0 /* ALPHA 0 offset */
#define LCDC_ALPHA0_MASK		(0xFF << LCDC_ALPHA0_OFFSET)

/* IPU Restart Register */
#define LCDC_IPUR_IPUREN		(1 << 31)   /* IPU restart function enable*/
#define LCDC_IPUR_IPURMASK	(0xFFFFFF)   /* IPU restart value mask*/

/* RGB Control Register */
#define LCDC_RGBC_RGBDM		(1 << 15)   /* enable RGB Dummy data */
#define LCDC_RGBC_DMM		(1 << 14)   /* RGB Dummy mode */
#define LCDC_RGBC_422		(1 << 8)    /* Change 444 to 422 */
#define LCDC_RGBC_RGBFMT	(1 << 7)    /* RGB format enable */
#define LCDC_RGBC_ODDRGB_BIT	4	/* odd line serial RGB data arrangement */
#define LCDC_RGBC_ODDRGB_MASK	(0x7 << LCDC_RGBC_ODDRGB_BIT)
#define LCDC_RGBC_ODD_RGB	(0 << LCDC_RGBC_ODDRGB_BIT) /* RGB */
#define LCDC_RGBC_ODD_RBG	(1 << LCDC_RGBC_ODDRGB_BIT) /* RBG */
#define LCDC_RGBC_ODD_GRB	(2 << LCDC_RGBC_ODDRGB_BIT) /* GRB */
#define LCDC_RGBC_ODD_GBR	(3 << LCDC_RGBC_ODDRGB_BIT) /* GBR */
#define LCDC_RGBC_ODD_BRG	(4 << LCDC_RGBC_ODDRGB_BIT) /* BRG */
#define LCDC_RGBC_ODD_BGR	(5 << LCDC_RGBC_ODDRGB_BIT) /* BGR */

#define LCDC_RGBC_EVENRGB_BIT	0	/* even line serial RGB data arrangement */
#define LCDC_RGBC_EVENRGB_MASK	(0x7<<LCDC_RGBC_EVENRGB_BIT)
#define LCDC_RGBC_EVEN_RGB	0 /* RGB */
#define LCDC_RGBC_EVEN_RBG	1 /* RBG */
#define LCDC_RGBC_EVEN_GRB	2 /* GRB */
#define LCDC_RGBC_EVEN_GBR	3 /* GBR */
#define LCDC_RGBC_EVEN_BRG	4 /* BRG */
#define LCDC_RGBC_EVEN_BGR	5 /* BGR */

/* Vertical Synchronize Register */
#define LCDC_VSYNC_VPS_BIT	16  /* VSYNC pulse start in line clock, fixed to 0 */
#define LCDC_VSYNC_VPS_MASK	(0xfff << LCDC_VSYNC_VPS_BIT)
#define LCDC_VSYNC_VPE_BIT	0   /* VSYNC pulse end in line clock */
#define LCDC_VSYNC_VPE_MASK	(0xfff << LCDC_VSYNC_VPE_BIT)

/* Horizontal Synchronize Register */
#define LCDC_HSYNC_HPS_BIT	16  /* HSYNC pulse start position in dot clock */
#define LCDC_HSYNC_HPS_MASK	(0xfff << LCDC_HSYNC_HPS_BIT)
#define LCDC_HSYNC_HPE_BIT	0   /* HSYNC pulse end position in dot clock */
#define LCDC_HSYNC_HPE_MASK	(0xfff << LCDC_HSYNC_HPE_BIT)

/* Virtual Area Setting Register */
#define LCDC_VAT_HT_BIT		16  /* Horizontal Total size in dot clock */
#define LCDC_VAT_HT_MASK		(0xfff << LCDC_VAT_HT_BIT)
#define LCDC_VAT_VT_BIT		0   /* Vertical Total size in dot clock */
#define LCDC_VAT_VT_MASK		(0xfff << LCDC_VAT_VT_BIT)

/* Display Area Horizontal Start/End Point Register */
#define LCDC_DAH_HDS_BIT		16  /* Horizontal display area start in dot clock */
#define LCDC_DAH_HDS_MASK	(0xfff << LCDC_DAH_HDS_BIT)
#define LCDC_DAH_HDE_BIT		0   /* Horizontal display area end in dot clock */
#define LCDC_DAH_HDE_MASK	(0xfff << LCDC_DAH_HDE_BIT)

/* Display Area Vertical Start/End Point Register */
#define LCDC_DAV_VDS_BIT		16  /* Vertical display area start in line clock */
#define LCDC_DAV_VDS_MASK	(0xfff << LCDC_DAV_VDS_BIT)
#define LCDC_DAV_VDE_BIT		0   /* Vertical display area end in line clock */
#define LCDC_DAV_VDE_MASK	(0xfff << LCDC_DAV_VDE_BIT)

/* Foreground 0 or Foreground 1 XY Position Register */
#define LCDC_XYP_YPOS_BIT	16  /* Y position bit of foreground 0 or 1 */
#define LCDC_XYP_YPOS_MASK	(0xfff << LCDC_XYP_YPOS_BIT)
#define LCDC_XYP_XPOS_BIT	0   /* X position bit of foreground 0 or 1 */
#define LCDC_XYP_XPOS_MASK	(0xfff << LCDC_XYP_XPOS_BIT)

/* Foreground 0 or Foreground 1 Size Register */
#define LCDC_SIZE_HEIGHT_BIT	16 /* The height of foreground 0 or 1 */
#define LCDC_SIZE_HEIGHT_MASK	(0xfff << LCDC_SIZE_HEIGHT_BIT) /* The height of foreground 0 or 1 */
#define LCDC_SIZE_WIDTH_BIT	0 /* The width of foreground 0 or 1 */
#define LCDC_SIZE_WIDTH_MASK	(0xfff << LCDC_SIZE_WIDTH_BIT) /* The width of foreground 0 or 1 */

/* PS Signal Setting */
#define LCDC_PS_PSS_BIT		16  /* PS signal start position in dot clock */
#define LCDC_PS_PSS_MASK		(0xfff << LCDC_PS_PSS_BIT)
#define LCDC_PS_PSE_BIT		0   /* PS signal end position in dot clock */
#define LCDC_PS_PSE_MASK		(0xfff << LCDC_PS_PSE_BIT)

/* CLS Signal Setting */
#define LCDC_CLS_CLSS_BIT	16  /* CLS signal start position in dot clock */
#define LCDC_CLS_CLSS_MASK	(0xfff << LCDC_CLS_CLSS_BIT)
#define LCDC_CLS_CLSE_BIT	0   /* CLS signal end position in dot clock */
#define LCDC_CLS_CLSE_MASK	(0xfff << LCDC_CLS_CLSE_BIT)

/* SPL Signal Setting */
#define LCDC_SPL_SPLS_BIT	16  /* SPL signal start position in dot clock */
#define LCDC_SPL_SPLS_MASK	(0xfff << LCDC_SPL_SPLS_BIT)
#define LCDC_SPL_SPLE_BIT	0   /* SPL signal end position in dot clock */
#define LCDC_SPL_SPLE_MASK	(0xfff << LCDC_SPL_SPLE_BIT)

/* REV Signal Setting */
#define LCDC_REV_REVS_BIT	16  /* REV signal start position in dot clock */
#define LCDC_REV_REVS_MASK	(0xfff << LCDC_REV_REVS_BIT)

/* DMA Command 0 or 1 Register */
#define LCDC_CMD_SOFINT		(1 << 31) /* Enable start of frame interrupt */
#define LCDC_CMD_EOFINT		(1 << 30) /* Enable end of frame interrupt */
#define LCDC_CMD_CMD		(1 << 29) /* indicate command in slcd mode */
#define LCDC_CMD_PAL		(1 << 28) /* The descriptor contains a palette buffer */
#define LCDC_CMD_COMPEN		(1 << 27) /*
					   * It indicate this frame is 16/24bpp compressed or not
					   * 0:not compressed
					   * 1:compressed
					   */
#define LCDC_CMD_FRM_EN		(1 << 26) /* Indicate this frame is enable */
#define LCDC_CMD_FIELD_SEL	(1 << 25) /* Field select for interlace
					   * 0:odd field or no interlace
					   * 1:even field
					   */
#define LCDC_CMD_16X16BLOCK	(1 << 24) /* Fetch data by 16x16 block */
#define LCDC_CMD_LEN_BIT		0 /* The buffer length value (in word) */
#define LCDC_CMD_LEN_MASK	(0xffffff << LCDC_CMD_LEN_BIT)

/* DMA Offsize Register 0,1 */
#define LCDC_OFFS_BIT		0 /* OFFSIZE value for DMA 0,1(in word) */
#define LCDC_OFFS_OFFSIZE_MASK	(0xffffff << LCDC_OFFS_BIT)

/* DMA Page Width Register 0,1 */
#define LCDC_PW_BIT		0 /* Page width for DMA 0,1(in word) */
#define LCDC_PW_PAGEWIDTH_MASK	(0xffffff << LCDC_PW_BIT)

/* DMA Command Counter Register 0,1 */
#define LCDC_CNUM_BIT		 0 /* Commands' number in this frame transfer by DMA */
#define LCDC_CNUM_CNUM_MASK	(0xff << LCDC_CNUM_BIT) /* Only use in Smart LCD mode */

/* DMA Command Counter Register */
#define LCDC_CPOS_ALPHAMD1	(1 << 31) /* Alpha blending mode for foreground 0,1 */
#define LCDC_CPOS_RGB_RGB565	(0 << 30) /* foreground 0 or 1, 16bpp, 0-RGB565, 1-RGB555 */
#define LCDC_CPOS_RGB_RGB555	(1 << 30) /* foreground 0 or 1, 16bpp, 0-RGB565, 1-RGB555 */

#define LCDC_CPOS_BPP_BIT	27  /* Bits Per Pixel of OSD channel 1 (cannot use palette) */
#define LCDC_CPOS_BPP_MASK	(0x07 << LCDC_CPOS_BPP_BIT)
#define LCDC_CPOS_BPP_16	(4 << LCDC_CPOS_BPP_BIT) /* 15/16 bpp */
#define LCDC_CPOS_BPP_18_24	(5 << LCDC_CPOS_BPP_BIT) /* 18/24/32 bpp */
#define LCDC_CPOS_BPP_CMPS_24	(6 << LCDC_CPOS_BPP_BIT) /* 24 compress bpp */
#define LCDC_CPOS_BPP_30	(7 << LCDC_CPOS_BPP_BIT) /* 30 bpp */

#define LCDC_CPOS_PREMULTI	(1 << 26) /* Premulti enable of foreground 0,1 */
#define LCDC_CPOS_COEF_SLE_BIT	24 /* Select coefficient for foreground 0,1 */
#define LCDC_CPOS_COEF_SLE_MASK	(0x3 << LCDC_CPOS_COEF_SLE_BIT)
#define LCDC_CPOS_COEF_SLE_0	(0 << LCDC_CPOS_COEF_SLE_BIT) /* 00:0 */
#define LCDC_CPOS_COEF_SLE_1	(1 << LCDC_CPOS_COEF_SLE_BIT) /* 01:1 */
#define LCDC_CPOS_COEF_SLE_2	(2 << LCDC_CPOS_COEF_SLE_BIT) /* 10:alpha1 */
#define LCDC_CPOS_COEF_SLE_3	(3 << LCDC_CPOS_COEF_SLE_BIT) /* 11:1-alpha1 */

#define LCDC_CPOS_YPOS_BIT	12 /* The Y position of top-left part for foreground 0,1 */
#define LCDC_CPOS_YPOS_MASK	(0xfff << LCDC_CPOS_YPOS_BIT)
#define LCDC_CPOS_XPOS_BIT	0 /* The Y position of top-left part for foreground 0,1 */
#define LCDC_CPOS_XPOS_MASK	(0xfff << LCDC_CPOS_XPOS_BIT)

/* Foreground 0,1 Size Register */
#define LCDC_DESSIZE_ALPHA_BIT	24  /*  The global alpha value of foreground 0,1 */
#define LCDC_DESSIZE_ALPHA_MASK	(0xff << LCDC_DESSIZE_ALPHA_BIT)
#define LCDC_DESSIZE_HEIGHT_BIT	12  /* height of foreground 1 */
#define LCDC_DESSIZE_HEIGHT_MASK	(0xfff << LCDC_DESSIZE_HEIGHT_BIT)
#define LCDC_DESSIZE_WIDTH_BIT	0  /* width of foreground 1 */
#define LCDC_DESSIZE_WIDTH_MASK	(0xfff << LCDC_DESSIZE_WIDTH_BIT)

/* Priority level threshold configure Register */
#define LCDC_PCFG_LCDC_PRI_MD	(1 << 31)

#define LCDC_PCFG_HP_BST_BIT	28
#define LCDC_PCFG_HP_BST_MASK	(0x7 << LCDC_PCFG_HP_BST_BIT)
#define LCDC_PCFG_HP_BST_4	(0 << LCDC_PCFG_HP_BST_BIT) /* 000:4 word */
#define LCDC_PCFG_HP_BST_8	(1 << LCDC_PCFG_HP_BST_BIT) /* 001:8 word */
#define LCDC_PCFG_HP_BST_16	(2 << LCDC_PCFG_HP_BST_BIT) /* 010:16 word */
#define LCDC_PCFG_HP_BST_32	(3 << LCDC_PCFG_HP_BST_BIT) /* 011:32 word */
#define LCDC_PCFG_HP_BST_C16	(5 << LCDC_PCFG_HP_BST_BIT) /* 101:Continue 16 */
#define LCDC_PCFG_HP_BST_64	(4 << LCDC_PCFG_HP_BST_BIT) /* 100:64 word */
#define LCDC_PCFG_HP_BST_DIS	(7 << LCDC_PCFG_HP_BST_BIT) /* 111:disable */

#define LCDC_PCFG_PCFG2_BIT	18
#define LCDC_PCFG_PCFG2_MASK	(0x1ff << LCDC_PCFG_PCFG2_BIT)
#define LCDC_PCFG_PCFG1_BIT	9
#define LCDC_PCFG_PCFG1_MASK	(0x1ff << LCDC_PCFG_PCFG1_BIT)
#define LCDC_PCFG_PCFG0_BIT	0
#define LCDC_PCFG_PCFG0_MASK	(0x1ff << LCDC_PCFG_PCFG0_BIT)

/* Dual LCDC Channel Control register */
/*
 * Select which IPU is able to write back, this field is just
 * available in lcdc1. 0:ipu1; 1:ipu0
 */
#define LCDC_DUAL_CTRL_IPU_WR_SEL	(1 << 8)
/*
 * Select which controller output to the tft/slcd panel, this field is just
 * available in lcdc1. 0:lcdc1; 1:lcdc0
 */
#define LCDC_DUAL_CTRL_TFT_SEL		(1 << 6)
/*
 * 1: fix the priority of ipu0/1 in lcd internal arbiter;
 * 0: use priority of ipu0/1 generated by lcd in lcd internal arbiter
 */
#define LCDC_DUAL_CTRL_PRI_IPU_EN	(1 << 5)
#define LCDC_DUAL_CTRL_PRI_IPU_BIT	3
#define LCDC_DUAL_CTRL_PRI_IPU_MASK	(0x3 << LCDC_DUAL_CTRL_PRI_IPU_BIT)
/*
 * 1: fix the priority of lcd0/1 in lcd internal arbiter;
 * 0: use priority of lcd0/1 generated by lcd in lcd internal arbiter
 */
#define LCDC_DUAL_CTRL_PRI_LCD_EN	(1 << 2)
#define LCDC_DUAL_CTRL_PRI_LCD_BIT	0
#define LCDC_DUAL_CTRL_PRI_LCD_MASK	(0x3 << LCDC_DUAL_CTRL_PRI_LCD_BIT)

/*************************************************************************
 * SLCD (Smart LCD Controller)
 *************************************************************************/

#define SLCDC_CFG       (LCDC0_BASE + 0xA0)  /* SLCDC Configure Register */
#define SLCDC_CTRL      (LCDC0_BASE + 0xA4)  /* SLCDC Control Register */
#define SLCDC_STATE     (LCDC0_BASE + 0xA8)  /* SLCDC Status Register */
#define SLCDC_DATA      (LCDC0_BASE + 0xAC)  /* SLCDC Data Register */

/* SLCDC Configure Register */
#define SLCDC_CFG_DWIDTH_BIT	10
#define SLCDC_CFG_DWIDTH_MASK	(0x7 << SLCDC_CFG_DWIDTH_BIT)
#define SLCDC_CFG_DWIDTH_18BIT	(0 << SLCDC_CFG_DWIDTH_BIT)
#define SLCDC_CFG_DWIDTH_16BIT	(1 << SLCDC_CFG_DWIDTH_BIT)
#define SLCDC_CFG_DWIDTH_8BIT_x3	(2 << SLCDC_CFG_DWIDTH_BIT)
#define SLCDC_CFG_DWIDTH_8BIT_x2	(3 << SLCDC_CFG_DWIDTH_BIT)
#define SLCDC_CFG_DWIDTH_8BIT_x1	(4 << SLCDC_CFG_DWIDTH_BIT)
#define SLCDC_CFG_DWIDTH_24BIT	(5 << SLCDC_CFG_DWIDTH_BIT)
#define SLCDC_CFG_DWIDTH_9BIT_x2	(7 << SLCDC_CFG_DWIDTH_BIT)
#define SLCDC_CFG_CWIDTH_BIT	(8)
#define SLCDC_CFG_CWIDTH_MASK	(0x3 << SLCDC_CFG_CWIDTH_BIT)
#define SLCDC_CFG_CWIDTH_16BIT	(0 << SLCDC_CFG_CWIDTH_BIT)
#define SLCDC_CFG_CWIDTH_8BIT	(1 << SLCDC_CFG_CWIDTH_BIT)
#define SLCDC_CFG_CWIDTH_18BIT	(2 << SLCDC_CFG_CWIDTH_BIT)
#define SLCDC_CFG_CWIDTH_24BIT	(3 << SLCDC_CFG_CWIDTH_BIT)
#define SLCDC_CFG_CS_ACTIVE_LOW	(0 << 4)
#define SLCDC_CFG_CS_ACTIVE_HIGH	(1 << 4)
#define SLCDC_CFG_RS_CMD_LOW	(0 << 3)
#define SLCDC_CFG_RS_CMD_HIGH	(1 << 3)
#define SLCDC_CFG_CLK_ACTIVE_FALLING	(0 << 1)
#define SLCDC_CFG_CLK_ACTIVE_RISING	(1 << 1)
#define SLCDC_CFG_TYPE_PARALLEL	(0 << 0)
#define SLCDC_CFG_TYPE_SERIAL	(1 << 0)

#define SLCDC_CTRL_DMA_MODE     (1 << 2)
#define SLCDC_CTRL_DMA_START    (1 << 1)
#define SLCDC_CTRL_DMA_EN       (1 << 0)
#define SLCDC_STATE_BUSY                (1 << 0)
#define SLCDC_DATA_RS_DATA      (0 << 31)
#define SLCDC_DATA_RS_COMMAND   (1 << 31)
#endif	/* __LCDC__H__ */
