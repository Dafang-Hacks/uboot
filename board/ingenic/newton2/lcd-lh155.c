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
#include <asm/gpio.h>
#include <jz_lcd/jz_lcd_v1_2.h>

#include <jz_lcd/jz_dsim.h>

unsigned long lh155_cmd_buf[]= {
    0x2C2C2C2C,
};

void board_set_lcd_power_on(void)
{
    return;
    char *id = "lcd_1.8v";
    struct regulator *lcd_regulator = regulator_get(id);
    regulator_set_voltage(lcd_regulator, 3300000, 1800000);
    regulator_enable(lcd_regulator);
}

struct dsi_config jz_dsi_config={
    .max_lanes = 2,
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
    .modes = &jzfb1_videomode,

    .lcd_type = LCD_TYPE_SLCD,
    .bpp = 18,

    .smart_config.smart_type      = SMART_LCD_TYPE_PARALLEL,
    .smart_config.rsply_cmd_high = 0,
    .smart_config.csply_active_high = 0,
    .smart_config.write_gram_cmd = lh155_cmd_buf,
    .smart_config.length_data_table = ARRAY_SIZE(lh155_cmd_buf),
    .smart_config.bus_width = 8,
    .dither_enable = 1,
    .dither.dither_red = 1,	/* 6bit */
    .dither.dither_red = 1,	/* 6bit */
    .dither.dither_red = 1,	/* 6bit */
};
