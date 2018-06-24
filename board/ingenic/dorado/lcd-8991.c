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
#include <jz_pca953x.h>
#include <jz_lcd/jz_lcd_v1_2.h>
#include <jz_lcd/byd_8991.h>
#ifdef CONFIG_PMU_RICOH6x
#define CONFIG_LCD_REGULATOR	"RICOH619_LDO9"
#elif defined CONFIG_PMU_D2041
#define CONFIG_LCD_REGULATOR	""
#endif

void board_set_lcd_power_on(void)
{
	char *id = CONFIG_LCD_REGULATOR;
	struct regulator *lcd_regulator = regulator_get(id);

	regulator_set_voltage(lcd_regulator, 1800000,1800000);
	regulator_enable(lcd_regulator);
}

struct jzfb_config_info jzfb1_init_data = {
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,

	.pixclk_falling_edge = 1,
	.date_enable_active_low = 0,

	.dither_enable = 0,
};

#ifdef CONFIG_DORADO_V20
#define GPIO_LCD_DISP GPIO_PE(10)
#define GPIO_LCD_SPI_CLK	GPIO_PD(28)
#endif
#ifdef CONFIG_DORADO_V21
#define GPIO_LCD_DISP GPIO_PB(0)
#define GPIO_LCD_SPI_CLK	GPIO_PD(28)
#endif
#ifdef CONFIG_DORADO_V22
#define GPIO_LCD_DISP PCA953X_GPIO(9)
#define GPIO_LCD_SPI_CLK	GPIO_PB(0)
#endif
struct byd_8991_data byd_8991_pdata = {
	.gpio_lcd_disp = GPIO_LCD_DISP,
	.gpio_lcd_de = 0,
	.gpio_lcd_vsync = 0,
	.gpio_lcd_hsync = 0,
	.gpio_spi_cs = GPIO_PA(11),
	.gpio_spi_clk = GPIO_LCD_SPI_CLK,
	.gpio_spi_mosi = GPIO_PE(3),
	.gpio_spi_miso = GPIO_PE(0),
};
