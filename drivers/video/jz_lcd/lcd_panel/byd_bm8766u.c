/*
 * JZ LCD PANEL DATA
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
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

#include <config.h>
#include <serial.h>
#include <common.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <lcd.h>
#include <asm/types.h>
#include <asm/arch/tcu.h>
#include <asm/arch/lcdc.h>
#include <asm/arch/gpio.h>
#include <regulator.h>
#include <jz_lcd/byd_bm8766u.h>

struct byd_bm8766u_data byd_bm8766u_pdata;

vidinfo_t panel_info = { 800, 480, LCD_BPP, };

void panel_pin_init(void)
{
	gpio_direction_output(byd_bm8766u_pdata.gpio_lcd_disp, -1);
	serial_puts("8766ftgu panel display pin init\n");
}

void panel_power_on(void)
{
	udelay(50);
	gpio_direction_output(byd_bm8766u_pdata.gpio_lcd_disp, 0);
	udelay(100);
	gpio_direction_output(byd_bm8766u_pdata.gpio_lcd_disp, 1);

	mdelay(80);
	serial_puts("8766ftgu panel display on\n");
}

void panel_power_off(void)
{
	gpio_direction_output(byd_bm8766u_pdata.gpio_lcd_disp, 0);
	serial_puts("8766ftgu panel display off\n");
}

struct fb_videomode jzfb1_videomode = {
	.name = "800x480",
	.refresh = 55,
	.xres = 800,
	.yres = 480,
	.pixclock = KHZ2PICOS(33260),
	.left_margin = 88,
	.right_margin = 40,
	.upper_margin = 8,
	.lower_margin = 35,
	.hsync_len = 128,
	.vsync_len = 2,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
};
