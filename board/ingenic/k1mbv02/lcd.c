/*
 * Ingenic mensa lcd code
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

#include <regulator.h>
#include <asm/gpio.h>
#include <jz_lcd/jz_lcd_v1_2.h>
#ifdef CONFIG_PMU_RICOH6x
#define CONFIG_LCD_REGULATOR	"RICOH619_LDO9"
#define CONFIG_LCD_REGULATOR	"RICOH619_DC4"
#elif defined CONFIG_PMU_D2041
#define CONFIG_LCD_REGULATOR	""
#endif

void board_set_lcd_power_on(void)
{
	char *id = CONFIG_LCD_REGULATOR;

	struct regulator *lcd_regulator = regulator_get(id);
	regulator_set_voltage(lcd_regulator, 3300000, 3300000);
	regulator_enable(lcd_regulator);
}

struct jzfb_config_info jzfb1_init_data = {
#if defined(CONFIG_VIDEO_ILI_6122)
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

//	.lvds = 0,
	.dither_enable = 0,
#else
#error "Please add the board data!!!"
#endif
};

#ifdef CONFIG_VIDEO_ILI_6122
#include <jz_lcd/ili_6122.h>
struct ili_6122_data ili_6122_pdata = {
	.gpio_lcd_disp = GPIO_PE(0),
	.gpio_lcd_de = GPIO_PC(9),   /* chose sync mode */
	.gpio_lcd_vsync = 0,//GPIO_PC(19),
	.gpio_lcd_hsync = 0,//GPIO_PC(18),
};
#endif /* CONFIG_LCD_ILI_6122 */
