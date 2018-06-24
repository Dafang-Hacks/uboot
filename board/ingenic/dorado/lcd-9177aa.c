/*
 * Copyright (c) 2012 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 *  dorado board lcd setup routines.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <regulator.h>
#include <asm/gpio.h>
#include <jz_lcd/jz_lcd_v1_2.h>
#include <jz_lcd/jz_dsim.h>

#ifdef CONFIG_PMU_RICOH6x
#define CONFIG_LCD_REGULATOR	"RICOH619_LDO9"
#endif

void board_set_lcd_power_on(void)
{
	char *id = CONFIG_LCD_REGULATOR;
	struct regulator *lcd_regulator = regulator_get(id);

	regulator_set_voltage(lcd_regulator, 1800000,1800000);
	regulator_enable(lcd_regulator);
}

struct dsi_config jz_dsi_config={
	.max_lanes = 4,
	.max_hs_to_lp_cycles = 100,
	.max_lp_to_hs_cycles = 40,
	.max_bta_cycles = 4095,
	.color_mode_polarity = 1,
	.shut_down_polarity = 1,
};

struct video_config jz_dsi_video_config={
	.no_of_lanes = 2,
	.virtual_channel = 0,
	.color_coding = COLOR_CODE_24BIT,
	.receive_ack_packets = 0,	/* enable receiving of ack packets */
	.is_18_loosely = 0,
	.data_en_polarity = 1,
};

struct dsi_device jz_dsi = {
	.dsi_config = &jz_dsi_config,
	.video_config = &jz_dsi_video_config,
};

struct jzfb_config_info jzfb1_init_data = {
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,
};
