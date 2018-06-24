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
#include <asm/arch/tcu.h>
#include <asm/arch/lcdc.h>
#include <asm/arch/gpio.h>
#include <regulator.h>
#include <jz_lcd/byd_8991.h>

extern void Initial_IC(void);
struct byd_8991_data byd_8991_pdata;

vidinfo_t panel_info = { 480, 800, LCD_BPP, };

void panel_pin_init(void)
{
	gpio_direction_output(byd_8991_pdata.gpio_lcd_disp, 1);
	gpio_direction_output(byd_8991_pdata.gpio_spi_cs, 1);
	gpio_direction_output(byd_8991_pdata.gpio_spi_clk, 1);
	gpio_direction_output(byd_8991_pdata.gpio_spi_mosi, 1);
	gpio_direction_input(byd_8991_pdata.gpio_spi_miso);
	serial_puts("8991ftgf panel display pin init\n");
}

void panel_power_on(void)
{
	Initial_IC();
	//mdelay(80); //for decreasing by ykliu
	serial_puts("8991ftgf panel display on\n");
}

void panel_power_off(void)
{
	gpio_direction_output(byd_8991_pdata.gpio_lcd_disp, 0);
	serial_puts("8991ftgf panel display off\n");
}

struct fb_videomode jzfb1_videomode = {
	.name = "480*800",
	.refresh = 55,
	.xres = 480,
	.yres = 800,
	.pixclock = KHZ2PICOS(30000),
	.left_margin = 70,
	.right_margin = 70,
	.upper_margin = 2,
	.lower_margin = 2,
	.hsync_len = 42,
	.vsync_len = 11,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
};
