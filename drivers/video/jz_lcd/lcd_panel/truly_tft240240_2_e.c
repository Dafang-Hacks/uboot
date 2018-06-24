/*
 * JZ LCD PANEL DATA
 *
 * Copyright (c) 2014 Ingenic Semiconductor Co.,Ltd
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
#include <jz_lcd/truly_tft240240_2_e.h>

struct truly_tft240240_2_e_data truly_tft240240_2_e_pdata;

vidinfo_t panel_info = { 240, 240, LCD_BPP, };

void panel_pin_init(void)
{
	int ret = 0;
	ret = gpio_request(truly_tft240240_2_e_pdata.gpio_lcd_cs, "lcd_cs");
	if(ret){
		printf("canot request gpio lcd_cs\n");
	}

	ret = gpio_request(truly_tft240240_2_e_pdata.gpio_lcd_rd, "lcd_rd");
	if(ret){
		printf("canot request gpio lcd_rd\n");
	}

        ret = gpio_request(truly_tft240240_2_e_pdata.gpio_lcd_rst, "lcd_rst");
	if(ret){
		printf("canot request gpio lcd_rst\n");
	}

	ret = gpio_request(truly_tft240240_2_e_pdata.gpio_lcd_bl, "lcd_bl");
	if(ret){
		printf("canot request gpio lcd_bl\n");
	}
	serial_puts("truly_tft240240_2_e panel display pin init\n");
}

void panel_power_on(void)
{
	gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_cs, 1);
	gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_rd, 1);

	/*power reset*/
        gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_rst, 0);
        mdelay(20);
	gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_rst, 1);
	mdelay(10);

	gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_cs, 0);
	gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_bl, 1);

	serial_puts("truly_tft240240_2_e panel display on\n");
}

void panel_power_off(void)
{
	gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_cs, 0);
	gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_rd, 0);
	gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_rst, 0);
	gpio_direction_output(truly_tft240240_2_e_pdata.gpio_lcd_bl, 0);
	serial_puts("truly_tft240240_2_e panel display off\n");
}

struct fb_videomode jzfb1_videomode = {
	.name = "240x240",
	.refresh = 60,
	.xres = 240,
	.yres = 240,
	.pixclock = KHZ2PICOS(30000),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};
