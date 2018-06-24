/*
 * T15 GPIO definitions
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Sonil <ztyan@ingenic.cn>
 * Based on: newxboot/modules/gpio/jz4775_gpio.c|jz4780_gpio.c
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

static struct jz_gpio_func_def uart_gpio_func[] = {
	[0] = { .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x9},
	[1] = { .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x3 << 4},
	[2] = { .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x3 << 6},
	[3] = { .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x9},
};

static struct jz_gpio_func_def gpio_func[] = {
#if defined(CONFIG_JZ_MMC_MSC1_PE)
	{ .port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x30f00000},
#endif
#if defined(CONFIG_JZ_MMC_MSC2_PB)
	{ .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x0000003f},
#endif
#if defined(CONFIG_JZ_SSI0_PA)
	{ .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = (0x3<<18) | (0xf<<1)},
#endif
#if defined(CONFIG_JZ_SFC_PA)
	{ .port = GPIO_PORT_A, .func = GPIO_FUNC_3, .pins = (0x3<<18) | (0xf<<1)},
#endif
};
