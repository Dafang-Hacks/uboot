/*
 * Ingenic dorado lcd code
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

#include <regulator.h>
#include <asm/gpio.h>
#include <jz_lcd/jz_lcd_v1_2.h>
#include <jz_lcd/truly_tft240240_2_e.h>

//#define CONFIG_SLCD_TRULY_18BIT

unsigned long truly_cmd_buf[]= {
    0x2C2C2C2C,
};

void board_set_lcd_power_on(void)
{
	return;
	char *id = "out11";
	struct regulator *lcd_regulator = regulator_get(id);
	regulator_set_voltage(lcd_regulator, 3300000, 1800000);
	regulator_enable(lcd_regulator);
}

struct smart_lcd_data_table truly_tft240240_data_table[] = {
    /* LCD init code */
    {SMART_CONFIG_CMD, 0x01},  //soft reset, 120 ms = 120 000 us
    {SMART_CONFIG_UDELAY, 120000},
    {SMART_CONFIG_CMD, 0x11},
    {SMART_CONFIG_UDELAY, 5000},	  /* sleep out 5 ms  */

    {SMART_CONFIG_CMD, 0x36},
#ifdef	CONFIG_TRULY_240X240_ROTATE_180
    {SMART_CONFIG_DATA, 0xd0}, //40
#else
    {SMART_CONFIG_DATA, 0x00}, //40
#endif

    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0xef},

    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0xef},


    {SMART_CONFIG_CMD, 0x3a},
#if defined(CONFIG_SLCD_TRULY_18BIT)  //if 18bit/pixel unusual. try to use 16bit/pixel
    {SMART_CONFIG_DATA, 0x06}, //6-6-6
#else
    {SMART_CONFIG_DATA, 0x05}, //5-6-5
#endif
    //	{SMART_CONFIG_DATA, 0x55},

    {SMART_CONFIG_CMD, 0xb2},
    {SMART_CONFIG_DATA, 0x7f},
    {SMART_CONFIG_DATA, 0x7f},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_DATA, 0xde},
    {SMART_CONFIG_DATA, 0x33},

    {SMART_CONFIG_CMD, 0xb3},
    {SMART_CONFIG_DATA, 0x10},
    {SMART_CONFIG_DATA, 0x05},
    {SMART_CONFIG_DATA, 0x0f},

    {SMART_CONFIG_CMD, 0xb4},
    {SMART_CONFIG_DATA, 0x0b},

    {SMART_CONFIG_CMD, 0xb7},
    {SMART_CONFIG_DATA, 0x35},

    {SMART_CONFIG_CMD, 0xbb},
    {SMART_CONFIG_DATA, 0x28}, //23

    {SMART_CONFIG_CMD, 0xbc},
    {SMART_CONFIG_DATA, 0xec},

    {SMART_CONFIG_CMD, 0xc0},
    {SMART_CONFIG_DATA, 0x2c},

    {SMART_CONFIG_CMD, 0xc2},
    {SMART_CONFIG_DATA, 0x01},

    {SMART_CONFIG_CMD, 0xc3},
    {SMART_CONFIG_DATA, 0x1e}, //14

    {SMART_CONFIG_CMD, 0xc4},
    {SMART_CONFIG_DATA, 0x20},

    {SMART_CONFIG_CMD, 0xc6},
    {SMART_CONFIG_DATA, 0x14},

    {SMART_CONFIG_CMD, 0xd0},
    {SMART_CONFIG_DATA, 0xa4},
    {SMART_CONFIG_DATA, 0xa1},

    {SMART_CONFIG_CMD, 0xe0},
    {SMART_CONFIG_DATA, 0xd0},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x08},
    {SMART_CONFIG_DATA, 0x07},
    {SMART_CONFIG_DATA, 0x05},
    {SMART_CONFIG_DATA, 0x29},
    {SMART_CONFIG_DATA, 0x54},
    {SMART_CONFIG_DATA, 0x41},
    {SMART_CONFIG_DATA, 0x3c},
    {SMART_CONFIG_DATA, 0x17},
    {SMART_CONFIG_DATA, 0x15},
    {SMART_CONFIG_DATA, 0x1a},
    {SMART_CONFIG_DATA, 0x20},

    {SMART_CONFIG_CMD, 0xe1},
    {SMART_CONFIG_DATA, 0xd0},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x08},
    {SMART_CONFIG_DATA, 0x07},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_DATA, 0x29},
    {SMART_CONFIG_DATA, 0x44},
    {SMART_CONFIG_DATA, 0x42},
    {SMART_CONFIG_DATA, 0x3b},
    {SMART_CONFIG_DATA, 0x16},
    {SMART_CONFIG_DATA, 0x15},
    {SMART_CONFIG_DATA, 0x1b},
    {SMART_CONFIG_DATA, 0x1f},

    {SMART_CONFIG_CMD, 0x35}, // TE on
    {SMART_CONFIG_DATA, 0x00}, // TE mode: 0, mode1; 1, mode2
    //	{SMART_CONFIG_CMD, 0x34}, // TE off

    {SMART_CONFIG_CMD, 0x29}, //Display ON

    /* set window size*/
    //	{SMART_CONFIG_CMD, 0xcd},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0},
    {SMART_CONFIG_DATA, 0},
    {SMART_CONFIG_DATA, (239>> 8) & 0xff},
    {SMART_CONFIG_DATA, 239 & 0xff},
#ifdef	CONFIG_TRULY_240X240_ROTATE_180
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_DATA, ((320-240)>>8)&0xff},
    {SMART_CONFIG_DATA, ((320-240)>>0)&0xff},
    {SMART_CONFIG_DATA, ((320-1)>>8) & 0xff},
    {SMART_CONFIG_DATA, ((320-1)>>0) & 0xff},
#else
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_DATA, 0},
    {SMART_CONFIG_DATA, 0},
    {SMART_CONFIG_DATA, (239>> 8) & 0xff},
    {SMART_CONFIG_DATA, 239 & 0xff},
#endif
};

struct jzfb_config_info jzfb1_init_data = {
	.num_modes = 1,
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_SLCD,
	.bpp    = 18,
	.pinmd  = 0,

	.smart_config.rsply_cmd_high       = 0,
	.smart_config.csply_active_high    = 0,
	/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
	.smart_config.newcfg_fmt_conv =  1,
	.smart_config.write_gram_cmd = truly_cmd_buf,
	.smart_config.length_cmd = ARRAY_SIZE(truly_cmd_buf),
	.smart_config.bus_width = 8,
	.smart_config.length_data_table =  ARRAY_SIZE(truly_tft240240_data_table),
	.smart_config.data_table = truly_tft240240_data_table,
	.dither_enable = 0,
};

struct truly_tft240240_2_e_data truly_tft240240_2_e_pdata = {
	.gpio_lcd_rd  = GPIO_PC(17),
	.gpio_lcd_rst = GPIO_PA(12),
	.gpio_lcd_cs  = GPIO_PC(14),
	.gpio_lcd_bl  = GPIO_PC(18),
};
