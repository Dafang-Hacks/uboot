/*
 * Copyright (c) 2014 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 *  dorado board lcd setup routines.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <regulator.h>
//#include <include/regulator.h>
#include <asm/gpio.h>
#include <jz_lcd/jz_lcd_v1_2.h>

#include <jz_lcd/jz_dsim.h>
#include <jz_lcd/auo_x163.h>

#ifdef CONFIG_PMU_RICOH6x
#define CONFIG_LCD_REGULATOR_9    "RICOH619_LDO9"
#define CONFIG_LCD_REGULATOR_10    "RICOH619_LDO10"
#elif defined CONFIG_PMU_D2041
#define CONFIG_LCD_REGULATOR    ""
#endif

#ifdef CONFIG_ACRAB
#define GPIO_LCD_BLK_EN GPIO_PC(9)
#endif

#ifdef CONFIG_ACRAB
#define MIPI_RST_N GPIO_PC(16)
#else
#define MIPI_RST_N GPIO_PC(19)
#endif

void board_set_lcd_power_on(void)
{
	char *id_9  = CONFIG_LCD_REGULATOR_9;
	char *id_10 = CONFIG_LCD_REGULATOR_10;
	struct regulator *lcd_regulator_9 = regulator_get(id_9);
	struct regulator *lcd_regulator_10 = regulator_get(id_10);

	regulator_set_voltage(lcd_regulator_9, 1800000, 1800000);
	regulator_set_voltage(lcd_regulator_10, 3300000, 3300000);
	regulator_enable(lcd_regulator_9);
	regulator_enable(lcd_regulator_10);

#ifdef CONFIG_ACRAB
	gpio_direction_output(GPIO_LCD_BLK_EN, 1);
#endif
}

struct auo_x163_platform_data auo_x163_pdata = {
	.gpio_rst = MIPI_RST_N,
};

struct dsi_config jz_dsi_config={
	.max_lanes = 1,
	.max_hs_to_lp_cycles = 100,
	.max_lp_to_hs_cycles = 40,
	.max_bta_cycles = 4095,
	.color_mode_polarity = 1,
	.shut_down_polarity = 1,
};

struct video_config jz_dsi_video_config={
	.no_of_lanes = 1,
	.virtual_channel = 0,
	.color_coding = COLOR_CODE_24BIT,
	//.color_coding = COLOR_CODE_18BIT_CONFIG1,
	//.byte_clock = ( CONFIG_DATALANE_BPS_MHZ * 1000) / 8,
	.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
	.receive_ack_packets = 0,	/* enable receiving of ack packets */
	.is_18_loosely = 0, /*loosely: R0R1R2R3R4R5__G0G1G2G3G4G5G6__B0B1B2B3B4B5B6,
			      not loosely: R0R1R2R3R4R5G0G1G2G3G4G5B0B1B2B3B4B5*/
	.data_en_polarity = 1,
};

struct dsi_device jz_dsi = {
	.dsi_config = &jz_dsi_config,
	.video_config = &jz_dsi_video_config,
};

struct jzfb_config_info jzfb1_init_data = {
	//.num_modes = 1,
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_SLCD,
	.bpp = 24,
	.smart_config.smart_type      = SMART_LCD_TYPE_PARALLEL,
	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high = 0,
	.smart_config.csply_active_high = 0,
	.smart_config.bus_width = 8,
	.dither_enable = 1,
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
};
