/*
 * JZ4775 common routines
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
 * Based on: xboot/boot/lcd/jz4775_android_lcd.h
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

#ifndef __BYD_8991_H__
#define __BYD_8991_H__
/*
 * @gpio_lcd_vsync: vertical sync input
 * @gpio_lcd_hsync: horizontal sync input
 * @gpio_lcd_disp: display on/off
 * @gpio_lcd_de: 1: select DE mode, 0: select SYNC mode
 * @gpio_spi_cs: spi chip_select 1:selected 0:not selected
 * @gpio_spi_clk: spi clock
 * @gpio_spi_mosi: spi master out, slave in
 * @gpio_spi_miso: spi master in, slave out
 */
struct byd_8991_data {
	unsigned int gpio_lcd_disp;
	unsigned int gpio_lcd_vsync;
	unsigned int gpio_lcd_hsync;
	unsigned int gpio_lcd_de;
	unsigned int gpio_spi_cs;
	unsigned int gpio_spi_clk;
	unsigned int gpio_spi_mosi;
	unsigned int gpio_spi_miso;
};

extern struct byd_8991_data byd_8991_pdata;

#endif /* __BYD_8991_H__ */
