/* linux/drivers/video/exynos/samsung.c
 *
 * MIPI-DSI based samsung AMOLED lcd 4.65 inch panel driver.
 *
 * Inki Dae, <inki.dae@samsung.com>
 * Donghwa Lee, <dh09.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include <jz_lcd/jz_dsim.h>
#include <jz_lcd/auo_x163.h>
#include "../jz_mipi_dsi/jz_mipi_dsih_hal.h"

vidinfo_t panel_info = { 320, 320, LCD_BPP, };

void auo_x163_regulator_enable(struct dsi_device *dsi)
{
#if 0
	int ret = 0;
	struct lcd_platform_data *pd = NULL;

	pd = lcd->ddi_pd;
	mutex_lock(&lcd->lock);
	if (!lcd->enabled) {
		regulator_enable(lcd->lcd_vcc_reg);
		regulator_enable(lcd->lcd_vcc_reg1);
		if (ret)
			goto out;

		lcd->enabled = true;
	}
	mdelay(pd->power_on_delay);
out:
	mutex_unlock(&lcd->lock);
#endif
}

void auo_x163_regulator_disable(struct dsi_device *dsi)
{
#if 0
	int ret = 0;

	mutex_lock(&lcd->lock);
	if (lcd->enabled) {
		regulator_disable(lcd->lcd_vcc_reg);
		regulator_disable(lcd->lcd_vcc_reg);
		if (ret)
			goto out;

		lcd->enabled = false;
	}
out:
	mutex_unlock(&lcd->lock);
#endif
}

void auo_x163_sleep_in(struct dsi_device *dsi) /* enter sleep */
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x10, 0x00};

	write_command(dsi, data_to_send);
}

void auo_x163_sleep_out(struct dsi_device *dsi) /* exit sleep */
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x11, 0x00};

	write_command(dsi, data_to_send);
}

void auo_x163_display_on(struct dsi_device *dsi) /* display on */
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x29, 0x00};

	write_command(dsi, data_to_send);
}

void auo_x163_display_off(struct dsi_device *dsi) /* display off */
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x28, 0x00};

	write_command(dsi, data_to_send);
}

void auo_x163_set_pixel_off(struct dsi_device *dsi) /* set_pixels_off */
{
	struct dsi_cmd_packet data_to_send = {0x39, 0x02, 0x00, {0x22,0x00}};

	write_command(dsi, data_to_send);
}

void auo_x163_set_pixel_on(struct dsi_device *dsi) /* set_pixels_on */
{
	struct dsi_cmd_packet data_to_send = {0x39, 0x02, 0x00, {0x23,0x00}};

	write_command(dsi, data_to_send);
}

void auo_x163_set_brightness(struct dsi_device *dsi, unsigned int brightness) /* set brightness */
{
	if(brightness >= 255) {
		debug("the max brightness is 255, set it 255\n");
		brightness = 255;
	}
	struct dsi_cmd_packet data_to_send = {0x39, 0x02, 0x00, {0x51, brightness}};
	write_command(dsi, data_to_send);
}

void panel_power_on(void)
{
	debug("--------------------%s\n", __func__);
	gpio_direction_output(auo_x163_pdata.gpio_rst, 1);
	mdelay(300);
	gpio_direction_output(auo_x163_pdata.gpio_rst, 0);  //reset active low
	mdelay(10);
	gpio_direction_output(auo_x163_pdata.gpio_rst, 1);
	mdelay(50);
	serial_puts("auo_x163 panel display on\n");
}

struct dsi_cmd_packet auo_x163_cmd_list1[] = {

	{0x39, 0x06, 0x00, {0xf0, 0x55, 0xaa, 0x52, 0x08, 0x00}},
	{0x39, 0x06, 0x00, {0xbd, 0x03, 0x20, 0x14, 0x4b, 0x00}},
	{0x39, 0x06, 0x00, {0xbe, 0x03, 0x20, 0x14, 0x4b, 0x01}},
	{0x39, 0x06, 0x00, {0xbf, 0x03, 0x20, 0x14, 0x4b, 0x00}},
	{0x39, 0x04, 0x00, {0xbb, 0x07, 0x07, 0x07}},
	{0x39, 0x02, 0x00, {0xc7, 0x40}},
	{0x39, 0x02, 0x00, {0x51, 0x64}},
	{0x39, 0x02, 0x00, {0x53, 0x20}},
	{0x39, 0x06, 0x00, {0xf0, 0x55, 0xaa, 0x52, 0x08, 0x02}},

	{0x15, 0xeb, 0x02},
	{0x39, 0x03, 0x00, {0xfe, 0x08, 0x50}},
	{0x39, 0x04, 0x00, {0xe9, 0x00, 0x36, 0x38}},
	{0x39, 0x04, 0x00, {0xc3, 0xf2, 0x95, 0x04}},
	{0x39, 0x02, 0x00, {0xca, 0x04}},
	{0x39, 0x06, 0x00, {0xf0, 0x55, 0xaa, 0x52, 0x08, 0x01}},

	{0x39, 0x04, 0x00, {0xb0, 0x03, 0x03, 0x03}},

	{0x39, 0x05, 0x00, {0x2a, 0x00, 0x00, (319 & 0xff00) >> 8, 479 & 0xff}},
	{0x39, 0x05, 0x00, {0x2b, 0x00, 0x00, (559 & 0xff00) >> 8, 559 & 0xff}},

	{0x39, 0x04, 0x00, {0xb1, 0x05, 0x05, 0x05}},
	{0x39, 0x04, 0x00, {0xb2, 0x01, 0x01, 0x01}},
	{0x39, 0x04, 0x00, {0xb4, 0x07, 0x07, 0x07}},
	{0x39, 0x04, 0x00, {0xb5, 0x03, 0x03, 0x03}},

	{0x39, 0x04, 0x00, {0xb6, 0x55, 0x55, 0x55}},
	{0x39, 0x04, 0x00, {0xb7, 0x36, 0x36, 0x36}},

	{0x39, 0x04, 0x00, {0xb8, 0x23, 0x23, 0x23}},
	{0x39, 0x04, 0x00, {0xb9, 0x03, 0x03, 0x03}},
	{0x39, 0x04, 0x00, {0xba, 0x03, 0x03, 0x03}},
	{0x39, 0x04, 0x00, {0xbe, 0x32, 0x30, 0x70}},

	{0x39, 0x08, 0x00, {0xcf, 0xff, 0xd4, 0x95, 0xe8, 0x4f, 0x00, 0x04}},
	{0x39, 0x02, 0x00, {0x35, 0x00}},
	{0x15, 0x02, 0x00, {0x36, 0x00}},
	{0x15, 0x02, 0x00, {0xc0, 0x20}},

	{0x39, 0x07, 0x00, {0xc2, 0x17, 0x17, 0x17, 0x17, 0x17, 0x0b}},
	{0x39, 0x01, 0x00, 0x2c},
};

static void auo_x163_panel_condition_setting(struct dsi_device *dsi)
{
    int  i;
	for(i = 0; i < ARRAY_SIZE(auo_x163_cmd_list1); i++) {
		write_command(dsi,  auo_x163_cmd_list1[i]);
	}
}
void panel_init_set_sequence(struct dsi_device *dsi)
{
	auo_x163_panel_condition_setting(dsi);
	auo_x163_sleep_out(dsi);
	mdelay(350);
//	auo_x163_memory_access(dsi);
//	auo_x163_display_on(dsi);
//	auo_x163_memory_access(dsi);
	mdelay(10);
}
void panel_display_on(struct dsi_device *dsi)
{
	auo_x163_display_on(dsi);
}

void panel_pin_init(void)
{
	debug("--------------------%s\n", __func__);
	int ret= 0;
	ret = gpio_request(auo_x163_pdata.gpio_rst, "lcd mipi panel rst");

	serial_puts("auo_x163 panel display pin init\n");
}

struct fb_videomode jzfb1_videomode = {
	.name = "auo_x163-lcd",
	.refresh = 60,
	.xres = 320,
	.yres = 320,
	.pixclock = KHZ2PICOS(5760),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};
