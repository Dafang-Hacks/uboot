/*
 * JZ4780 clock definitions
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

#ifndef __CLK_H__
#define __CLK_H__

#include <asm/arch/base.h>

enum {
	APLL,
	MPLL,
	EPLL,
	VPLL,
};

enum {
	DDR = 1,
	CPU,
	BCH,
	H2CLK,
	MSC0,
	MSC1,
	MSC2,
};

struct cgu {
	unsigned off:8;
	unsigned sel:8;
	unsigned sel_bit:8;
	unsigned en_bit:8;
	unsigned busy_bit:8;
	unsigned div:8;
	unsigned reserved:16;
};

typedef union cpm_cpxpcr {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned PLLEN:1;
		unsigned PLLBP:1;
		unsigned BW_M:1;
		unsigned F_MODE:1;
		unsigned PLL_ON:1;
		unsigned TEST:1;
		unsigned LOCK:1;
		unsigned FBSLIP:1;
		unsigned RFSLIP:1;
		unsigned PLLOD:4;
		unsigned PLLN:6;
		unsigned PLLM:13;
	} b; /* CPXPCR */
} cpm_cpxpcr_t;

#define CGU_MSC_FREQ 24000000
#define CGU_MSC_DIV (CONFIG_SYS_CPU_FREQ / CGU_MSC_FREQ / 2 - 1)
#define SPI_FREQ     60000000
#define SPI_DIV (CONFIG_SYS_CPU_FREQ / SPI_FREQ - 1)
#define CGU_BCH_DIV 7
#define CGU_LCD_DIV (CONFIG_SYS_VPLL_FREQ / CONFIG_SYS_PCLK_FREQ - 1)

unsigned int clk_get_rate(int clk);
void clk_set_rate(int clk, unsigned long rate);
void cgu_clks_init(struct cgu *cgu_sel, int nr_cgu_clks);
void clk_init(void);
void enable_uart_clk(void);
enum otg_mode_t {
	OTG_MODE = 0,
	DEVICE_ONLY_MODE,
	HOST_ONLY_MODE,
};
void otg_phy_init(enum otg_mode_t mode,unsigned extclk);

#endif /* __CLK_H__ */
