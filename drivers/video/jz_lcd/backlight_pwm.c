 /*
 * JZ BACKLIGHT
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
#include <lcd.h>
#include <asm/types.h>
#include <asm/arch/tcu.h>
#include <asm/arch/gpio.h>


/*
 * LCD backlight
 * support pwm and paulse adjuster
 */

#ifndef DEFAULT_BACKLIGHT_LEVEL
#define DEFAULT_BACKLIGHT_LEVEL 80
#endif

#define PWM_BACKLIGHT_CHIP 1/*0: digital pusle; 1: PWM*/

#if PWM_BACKLIGHT_CHIP
void lcd_set_backlight_level(int num)
{
	int _val = num;
	int _half;
	int _period;
	int prescaler = 1;
	if (_val>=CONFIG_SYS_PWM_FULL)
		_val = CONFIG_SYS_PWM_FULL-1;
	if (_val<1)
		_val =1;
	if (CONFIG_SYS_PWM_PERIOD < 200)
		_period = 200;
	else if (CONFIG_SYS_PWM_PERIOD > 1000000000)
		_period = 1000000000;
	else
		_period = CONFIG_SYS_PWM_PERIOD;
	_period = (unsigned long long)CONFIG_SYS_EXTAL * _period / 1000000000;
	while (_period > 0xffff && prescaler < 6) {
		_period >>= 2;
		++prescaler;
	}
	_half =_period * _val / (CONFIG_SYS_PWM_FULL);
	gpio_set_func(GPIO_PORT_E, GPIO_FUNC_0,1 << (CONFIG_GPIO_LCD_PWM % 32));
	struct pwm pwm_backlight = {CONFIG_SYS_PWM_CHN,prescaler,EXTAL,_period,_half};
	pwm_init(&pwm_backlight);
}

void lcd_close_backlight(void)
{
        gpio_direction_output(CONFIG_GPIO_LCD_PWM,-1);
        gpio_direction_output(CONFIG_GPIO_LCD_PWM,0);
}

#else

#define MAX_BRIGHTNESS_STEP	16				/* RT9365 supports 16 brightness step */
#define CONVERT_FACTOR		(256/MAX_BRIGHTNESS_STEP)	/* System support 256 brightness step */

void lcd_init_backlight(int num)
{
	unsigned int tmp = (num)/CONVERT_FACTOR + 1;
	gpio_direction_output(CONFIG_GPIO_LCD_PWM,-1);
	gpio_direction_output(CONFIG_GPIO_LCD_PWM),1;
	udelay(30);
	send_low_pulse(MAX_BRIGHTNESS_STEP-tmp);
}

void send_low_pulse(int num)
{
	unsigned int i;
	for (i = n; i > 0; i--)	{
		gpio_direction_output(CONFIG_GPIO_LCD_PWM,0);
		udelay(1);
		gpio_direction_output(CONFIG_GPIO_LCD_PWM,1);
		udelay(3);
	}
}

void lcd_set_backlight_level(int num)
{
	unsigned int last = lcd_backlight_level / CONVERT_FACTOR + 1;
	unsigned int tmp = (num) / CONVERT_FACTOR + 1;
	if (tmp <= last) {
		send_low_pulse(last-tmp);
	} else {
		send_low_pulse(last + MAX_BRIGHTNESS_STEP - tmp);
	}
	udelay(30);
}

void lcd_close_backlight(void)
{
		gpio_direction_output(CONFIG_GPIO_LCD_PWM,-1);
		gpio_direction_output(CONFIG_GPIO_LCD_PWM,0);
}
#endif
