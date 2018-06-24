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
#include <lcd.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <asm/types.h>
#include <asm/arch-m200/tcu.h>
#include <asm/arch-m200/lcdc.h>
#include <asm/arch-m200/gpio.h>
#include <regulator.h>
#include <jz_lcd/ili_6122.h>

struct ili_6122_data ili_6122_pdata;

vidinfo_t panel_info = { 800, 480, LCD_BPP, };

void panel_pin_init(void)
{
	int ret = 0;

	gpio_direction_output(ili_6122_pdata.gpio_lcd_disp, -1);
//	ret = gpio_request(ili_6122_pdata.gpio_lcd_disp,"display on");
//	if (ret < 0)
//		printf("can not requset lcd disp\n");
	 serial_puts("ili6122 panel display pin init\n");
}

void panel_power_on(void)
{
	udelay(50);
	gpio_direction_output(ili_6122_pdata.gpio_lcd_disp, 0);
	udelay(100);
	gpio_direction_output(ili_6122_pdata.gpio_lcd_disp, 1);

	mdelay(80);

	serial_puts("ili6122 panel display on\n");
}

void panel_power_off(void)
{
	gpio_direction_output(ili_6122_pdata.gpio_lcd_disp, 0);
	mdelay (2);
	serial_puts("ili6122 panel display off\n");
}

struct fb_videomode jzfb1_videomode = {
	.name = "800x480",
	.refresh = 60,
	.xres = 800,
	.yres = 480,
	.pixclock = KHZ2PICOS(30000),
	.left_margin = 46,
	.right_margin = 210,
	.upper_margin = 23,
	.lower_margin = 22,
	.hsync_len = 12,
	.vsync_len = 2,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
};
