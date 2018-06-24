/*
 * JZ4780 REG_BASE definitions
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

#ifndef __BASE_H__
#define __BASE_H__

/*
 * Define the module base addresses
 */
/* AHB0 BUS Devices Base */
#define HARB0_BASE	0xB3000000
//#define	EMC_BASE	0xB3010000
#define DDRC_BASE	0xB3010000
#define DDR_PHY_OFFSET	0x1000
#define MDMAC_BASE	0xB3420000
#define LCDC0_BASE	0xB3050000
#define LCDC1_BASE	0xB30A0000
#define TVE_BASE	0xB3050000
#define LVDS_BASE	LCDC0_BASE
#define SLCD0_BASE	0xB3050000
#define CIM_BASE	0xB3060000
#define IPU_BASE	0xB3080000

/* AHB1 BUS Devices Base */
#define HARB1_BASE	0xB3200000
#define DMAGP0_BASE	0xB3210000
#define DMAGP1_BASE	0xB3220000
#define DMAGP2_BASE	0xB3230000
#define MC_BASE		0xB3250000
#define ME_BASE		0xB3260000
#define DEBLK_BASE	0xB3270000
#define IDCT_BASE	0xB3280000
#define CABAC_BASE	0xB3290000
#define TCSM0_BASE	0xB32B0000
#define TCSM1_BASE	0xB32C0000
#define SRAM_BASE	0xB32D0000

/* AHB2 BUS Devices Base */
#define HARB2_BASE	0xB3400000
#define NEMC_BASE	0xB3410000
#define EFUSE_BASE	0xb3410000
#define DMAC_BASE	0xB3420000
#define UHC_BASE	0xB3430000
#define UDC_BASE	0xB3440000
#define GPS_BASE	0xB3480000
#define ETHC_BASE	0xB34B0000
#define BCH_BASE	0xB34D0000
#define HDMI_BASE	0xB0180000
#define OTG_BASE	0xb3500000

/* APB BUS Devices Base */
#define CPM_BASE	0xB0000000
#define INTC_BASE	0xB0001000
#define TCU_BASE	0xB0002000
#define OST_BASE	0xB0002000
#define WDT_BASE	0xB0002000
#define RTC_BASE	0xB0003000
#define GPIO_BASE	0xB0010000
#define AIC_BASE	0xB0020000
#define ICDC_BASE	0xB0020000
#ifndef MSC0_BASE
#define MSC0_BASE	0xB3450000
#endif
#define MSC1_BASE	0xB3460000
#define MSC2_BASE	0xB3470000
#define UART0_BASE	0xB0030000
#define UART1_BASE	0xB0031000
#define UART2_BASE	0xB0032000
#define UART3_BASE	0xB0033000
#define SCC_BASE	0xB0040000
#define SSI0_BASE	0xB0043000
#define SSI1_BASE	0xB0044000
#define SSI2_BASE	0xB0045000
#define I2C0_BASE	0xB0050000
#define I2C1_BASE	0xB0051000
#define PS2_BASE	0xB0060000
#define SADC_BASE	0xB0070000
#define OWI_BASE	0xB0072000
#define TSSI_BASE	0xB0073000

/* NAND CHIP Base Address*/
#define NEMC_CS1_BASE	0xbb000000
#define NEMC_CS2_BASE	0xba000000
#define NEMC_CS3_BASE	0xb9000000
#define NEMC_CS4_BASE	0xb8000000
#define NEMC_CS5_BASE	0xb7000000
#define NEMC_CS6_BASE	0xb6000000

#define AUX_BASE	0xb32a0000

#endif /* __BASE_H__ */
