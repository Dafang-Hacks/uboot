/*
 * JZ4780 cpm definitions
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

#ifndef __CPM_H__
#define __CPM_H__

#include <asm/arch/base.h>

/*************************************************************************
 * CPM (Clock reset and Power control Management)
 *************************************************************************/
#define CPM_CPCCR		(0x00) /* Clock control register		*/
#define CPM_CPCSR		(0xd4) /* Clock Status register		*/
#define CPM_CPPCR		(0x0c) /* PLL control register 		*/
#define CPM_CPAPCR		(0x10) /* APLL control Register	*/
#define CPM_CPMPCR		(0x14) /* MPLL control Register	*/
#define CPM_CPEPCR		(0x18) /* EPLL control Register	*/
#define CPM_CPVPCR		(0x1c) /* VPLL control Register	*/
#define CPM_DDRCDR		(0x2c) /* DDR clock divider register	*/
#define CPM_VPUCDR		(0x30) /* VPU clock divider register	*/
#define CPM_CPSPR		(0x34) /* CPM scratch pad register		*/
#define CPM_CPSPPR		(0x38) /* CPM scratch protected register	*/
#define CPM_USBPCR		(0x3c) /* USB parameter control register	*/
#define CPM_USBRDT		(0x40) /* USB reset detect timer register	*/
#define CPM_USBVBFIL		(0x44) /* USB jitter filter register		*/
#define CPM_USBPCR1		(0x48) /* USB parameter control register 1	*/
#define CPM_USBCDR		(0x50) /* USB OTG PHY clock divider register	*/
#define CPM_I2SCDR		(0x60) /* I2S device clock divider register	*/
#define CPM_I2S1CDR		(0xa0) /* I2S1 device clock divider register	*/
#define CPM_LPCDR1		(0x64) /* LCD1 pix clock divider register	*/
#define CPM_LPCDR		(0x54) /* LCD pix clock divider register	*/
#define CPM_MSC0CDR		(0x68) /* MSC clock divider register		*/
#define CPM_MSC1CDR		(0xa4) /* MSC1 clock divider register		*/
#define CPM_MSC2CDR		(0xa8) /* MSC2 clock divider register		*/
#define CPM_UHCCDR		(0x6C) /* UHC 48M clock divider register	*/
#define CPM_SSICDR		(0x74) /* SSI clock divider register		*/
#define CPM_CIMCDR		(0x7c) /* CIM MCLK clock divider register	*/
#define CPM_PCMCDR		(0x84) /* PCM device clock divider register	*/
#define CPM_GPUCDR		(0x88) /* GPU clock divider register		*/
#define CPM_HDMICDR		(0x8C) /* GPU clock divider register		*/
#define CPM_BCHCDR		(0xAC) /* BCH clock divider register		*/

#define CPM_LCR			(0x04)
#define CPM_SPCR0		(0xb8) /* SRAM Power Control Register0 */
#define CPM_SPCR1		(0xbc) /* SRAM Power Control Register1 */
#define CPM_PSWCST(n)		(0x4*(n)+0x90)
#define CPM_CLKGR		(0x20) /* Clock Gate Register0 */
#define CPM_CLKGR0		(0x20) /* Clock Gate Register0 */
#define CPM_CLKGR1		(0x28) /* Clock Gate Register1 */
#define CPM_OPCR		(0x24) /* Oscillator and Power Control Register */

#define CPM_RSR			(0x08)

#define LCR_LPM_MASK		(0x3)
#define LCR_LPM_SLEEP		(0x1)

#define CPM_RSR_HR		(1 << 3)
#define CPM_RSR_P0R		(1 << 2)
#define CPM_RSR_WR		(1 << 1)
#define CPM_RSR_PR		(1 << 0)

#define OPCR_ERCS		(0x1<<2)
#define OPCR_PD			(0x1<<3)
#define OPCR_IDLE		(0x1<<31)

#define CLKGR_VPU              (0x1<<19)


#define CPM_CLKGR0_DDR1		(1 << 31)
#define CPM_CLKGR0_DDR0		(1 << 30)
#define CPM_CLKGR0_IPU		(1 << 29)
#define CPM_CLKGR0_LCD		(1 << 28)
#define CPM_CLKGR0_TVE		(1 << 27)
#define CPM_CLKGR0_CIM		(1 << 26)
#define CPM_CLKGR0_SMB2		(1 << 25)
#define CPM_CLKGR0_UHC		(1 << 24)
#define CPM_CLKGR0_MAC		(1 << 23)
#define CPM_CLKGR0_GPS		(1 << 22)
#define CPM_CLKGR0_PDMA		(1 << 21)
#define CPM_CLKGR0_SSI2		(1 << 20)
#define CPM_CLKGR0_SSI1		(1 << 19)
#define CPM_CLKGR0_UART3	(1 << 18)
#define CPM_CLKGR0_UART2	(1 << 17)
#define CPM_CLKGR0_UART1	(1 << 16)
#define CPM_CLKGR0_UART0	(1 << 15)
#define CPM_CLKGR_SADC		(1 << 14)
#define CPM_CLKGR0_SADC		(1 << 14)
#define CPM_CLKGR0_KBC		(1 << 13)
#define CPM_CLKGR0_MSC2		(1 << 12)
#define CPM_CLKGR0_MSC1		(1 << 11)
#define CPM_CLKGR0_OWI		(1 << 10)
#define CPM_CLKGR0_TSSI0	(1 << 9)
#define CPM_CLKGR0_AIC		(1 << 8)
#define CPM_CLKGR0_SCC		(1 << 7)
#define CPM_CLKGR0_SMB1		(1 << 6)
#define CPM_CLKGR0_SMB0		(1 << 5)
#define CPM_CLKGR0_SSI0		(1 << 4)
#define CPM_CLKGR0_MSC0		(1 << 3)
#define CPM_CLKGR0_OTG0		(1 << 2)
#define CPM_CLKGR0_BCH		(1 << 1)
#define CPM_CLKGR0_NEMC		(1 << 0)

#define CPM_CLKGR1_P1		(1 << 15)
#define CPM_CLKGR1_X2D		(1 << 14)
#define CPM_CLKGR1_DES		(1 << 13)
#define CPM_CLKGR1_SMB4		(1 << 12)
#define CPM_CLKGR1_AHB_MON	(1 << 11)
#define CPM_CLKGR1_UART4	(1 << 10)
#define CPM_CLKGR1_HDMI		(1 << 9)
#define CPM_CLKGR1_OTG1		(1 << 8)
#define CPM_CLKGR1_GPVLC	(1 << 7)
#define CPM_CLKGR1_AIC1		(1 << 6)
#define CPM_CLKGR1_COMPRESS	(1 << 5)
#define CPM_CLKGR1_GPU		(1 << 4)
#define CPM_CLKGR1_PCM		(1 << 3)
#define CPM_CLKGR1_VPU	 	(1 << 2)
#define CPM_CLKGR1_TSSI1	(1 << 1)
#define CPM_CLKGR1_SMB3		(1 << 0)

#define cpm_inl(off)		readl(CPM_BASE + (off))
#define cpm_outl(val,off)	writel(val, CPM_BASE + (off))
#define cpm_test_bit(bit,off)	(cpm_inl(off) & 0x1<<(bit))
#define cpm_set_bit(bit,off)	(cpm_outl((cpm_inl(off) | 0x1<<(bit)),off))
#define cpm_clear_bit(bit,off)	(cpm_outl(cpm_inl(off) & ~(0x1 << bit), off))

/*USBPCR*/
#define USBPCR_USB_MODE_ORG	(1 << 31)
#define USBPCR_VBUSVLDEXT	(1 << 24)
#define USBPCR_VBUSVLDEXTSEL	(1 << 23)
#define USBPCR_POR		(1 << 22)
#define USBPCR_OTG_DISABLE	(1 << 20)

/*USBPCR1*/
#define USBPCR1_USB_SEL		(1 << 28)
#define USBPCR1_REFCLKSEL_BIT	(26)
#define USBPCR1_REFCLKSEL_MSK	(0x3 << USBPCR1_REFCLKSEL_BIT)
#define USBPCR1_REFCLKSEL_CORE	(0x2 << USBPCR1_REFCLKSEL_BIT)
#define USBPCR1_REFCLKSEL_EXT	(0x1 << USBPCR1_REFCLKSEL_BIT)
#define USBPCR1_REFCLKSEL_CSL	(0x0 << USBPCR1_REFCLKSEL_BIT)
#define USBPCR1_REFCLKDIV_BIT	(24)
#define USBPCR1_REFCLKDIV_MSK	(0X3 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_REFCLKDIV_19_2M	(0x3 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_REFCLKDIV_48M	(0x2 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_REFCLKDIV_24M	(0x1 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_REFCLKDIV_12M	(0x0 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_WORD_IF0_16_30	(1 << 19)

/*OPCR*/
#define OPCR_SPENDN0		(1 << 7)

/* CPM scratch pad protected register(CPSPPR) */
#define CPSPPR_CPSPR_WRITABLE   (0x00005a5a)
#define RECOVERY_SIGNATURE      (0x1a1a)        /* means "RECY" */
#define RECOVERY_SIGNATURE_SEC  0x800           /* means "RECY" */
#define FASTBOOT_SIGNATURE      (0x0666)        /* means "FASTBOOT" */

#define cpm_get_scrpad()        readl(CPM_BASE + CPM_CPSPR)
#define cpm_set_scrpad(data)                    \
do {                                            \
	volatile int i = 0x3fff;                \
	writel(0x00005a5a,CPM_BASE+CPM_CPSPPR);		\
	while(i--);				\
	writel(data,CPM_BASE+CPM_CPSPR);		\
	writel(0x0000a5a5,CPM_BASE+CPM_CPSPPR);      	\
} while (0)

#endif /* __CPM_H__ */
