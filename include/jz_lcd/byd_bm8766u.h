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

#ifndef __BYD_BM8766U_H__
#define __BYD_BM8766U_H__
/*
 * @gpio_lcd_vsync: vertical sync input
 * @gpio_lcd_hsync: horizontal sync input
 * @gpio_lcd_disp: display on/off
 * @gpio_lcd_de: 1: select DE mode, 0: select SYNC mode
 */
struct byd_bm8766u_data {
	unsigned int gpio_lcd_disp;
	unsigned int gpio_lcd_vsync;
	unsigned int gpio_lcd_hsync;
	unsigned int gpio_lcd_de;
};

extern struct byd_bm8766u_data byd_bm8766u_pdata;

#endif /* __BYD_BM8766U_H__ */
