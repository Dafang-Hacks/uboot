/* linux/drivers/video/exynos/lh155.c
 *
 * MIPI-DSI based lh155 AMOLED lcd 4.65 inch panel driver.
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
#include "../jz_mipi_dsi/jz_mipi_dsih_hal.h"

#define GPIO_LCD_BLK        GPIO_PC(17)
#define GPIO_LCD_RST        GPIO_PA(12)

vidinfo_t panel_info = {240, 240, LCD_BPP, };

struct dsi_cmd_packet lh155_cmd_list[] = {
    {0x39, 0x03, 0x00,{0xf1, 0x5a, 0x5a}},
    {0x39, 0x12, 0x0, {0xf2, 0x00, 0xd7, 0x03, 0x22, 0x23, 0x00, 0x01, 0x01, 0x12, 0x01, 0x08, 0x57, 0x00, 0x00, 0xd7, 0x22, 0x23}},
    {0x39, 0xf , 0x0, {0xf4, 0x07, 0x00, 0x00, 0x00, 0x21, 0x4f, 0x01, 0x02, 0x2a, 0x66, 0x02, 0x2a, 0x00, 0x02}},
    {0x39, 0xb , 0x0, {0xf5, 0x00, 0x38, 0x5c, 0x00, 0x00, 0x19, 0x00, 0x00, 0x04, 0x04}},
    {0x39, 0xa , 0x0, {0xf6, 0x03, 0x01, 0x06, 0x00, 0x0a, 0x04, 0x0a, 0xf4, 0x06}},
    {0x39, 0x2 , 0x0, {0xf7, 0x40}},
    {0x39, 0x3 , 0x0, {0xf8, 0x33, 0x00}},
    {0x39, 0x2 , 0x0, {0x0, 0xf9, 0x00}},
    {0x39, 0x1d, 0x0, {0xfa, 0x00, 0x04, 0x0e, 0x03, 0x0e, 0x20, 0x2a, 0x30, 0x38, 0x3a,
                          0x34, 0x37, 0x44, 0x3b, 0x00, 0x0b, 0x00, 0x07, 0x0b, 0x17, 0x1b,
                          0x1a, 0x22, 0x2b, 0x34, 0x40, 0x50, 0x3f}},
    {0x39, 0x1d, 0x0, {0xfa, 0x00, 0x1b, 0x11, 0x03, 0x08, 0x16, 0x1d, 0x20, 0x29, 0x2c,
                          0x29, 0x2d, 0x3d, 0x3c, 0x00, 0x0b, 0x12, 0x0e, 0x11, 0x1e, 0x22,
                          0x25, 0x2e, 0x39, 0x40, 0x48, 0x40, 0x3f}},
    {0x39, 0xf, 0x0, {0xfa, 0x00, 0x27, 0x12, 0x02, 0x04, 0x0f, 0x15, 0x17, 0x21, 0x27,
                         0x24, 0x29, 0x31, 0x3c}},
    {0x39, 0x10, 0x0, {0xfa, 0x00, 0x0c, 0x1e, 0x11, 0x17, 0x21, 0x27, 0x2b, 0x37, 0x43,
                          0x49, 0x4f, 0x59, 0x3e, 0x00}},
    //{0x39, 0x1, 0x0, {0x11}},
    {0x39, 0x2, 0x0, {0x36, 0xd8}},
    {0x39, 0x2, 0x0, {0x3a, 0x07}},
    //{0x39, 0x1, 0x0, {0x29}},
    {0x39, 0x5, 0x0, {0x2a, 0x00, 0x00, (239 & 0xff00) >> 8, 239 & 0xff}},
    {0x39, 0x5, 0x0, {0x0, 0x2b, 0x00, 0x00, (239 & 0xff00) >> 8, (239 & 0xff)}},
    {0x39, 0x1, 0x0, {0x2c}},
};

static void lh155_sleep_in(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x10, 0x00};

	write_command(dsi, data_to_send);
}

static void lh155_sleep_out(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x11, 0x00};

	write_command(dsi, data_to_send);
}

static void lh155_display_on(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x29, 0x00};
	write_command(dsi, data_to_send);
}

static void lh155_display_off(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x28, 0x00};
	write_command(dsi, data_to_send);
}

static void lh155_panel_init(struct dsi_device *dsi)
{
	int  i;
	for(i = 0; i < ARRAY_SIZE(lh155_cmd_list); i++) {
		write_command(dsi,  lh155_cmd_list[i]);
	}
}

static int lh155_set_power(struct lcd_device *ld, int power)
{
	int ret = 0;
	return ret;
}

void panel_pin_init(void)
{
	debug("--------------------%s\n", __func__);
	int ret= 0;
	ret = gpio_request(GPIO_LCD_RST, "lcd mipi panel rst");

	ret = gpio_request(GPIO_LCD_BLK, "lcd mipi panel avcc");
	serial_puts("lh155 panel display pin init\n");
}

void panel_init_set_sequence(struct dsi_device *dsi)
{
	debug("--------------------%s\n", __func__);
	lh155_panel_init(dsi);
	lh155_sleep_out(dsi);
	udelay(120*1000);
	lh155_display_on(dsi);
	udelay(10*1000);
}
void panel_display_on(struct dsi_device *dsi)
{
}


void panel_power_on(void)
{
    debug("--------------------%s\n", __func__);
    gpio_direction_output(GPIO_LCD_BLK, 1);
    mdelay(30);
    gpio_direction_output(GPIO_LCD_RST, 1);
    mdelay(300);
    gpio_direction_output(GPIO_LCD_RST, 0);  //reset active low
    mdelay(10);
    gpio_direction_output(GPIO_LCD_RST, 1);
    mdelay(5);
    serial_puts("lh155 panel display on\n");
}

void panel_power_off(void)
{
	gpio_direction_output(GPIO_LCD_BLK, 0);
	serial_puts("lh155 panel display off\n");
}

struct fb_videomode jzfb1_videomode = {
    .name = "lh155-lcd",
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
    .sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
    .vmode = FB_VMODE_NONINTERLACED,
    .flag = 0,
};
