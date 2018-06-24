/*
 * Copyright (C) 2013 Ingenic Semiconductor Inc.
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

#include <asm/arch/tcu.h>
#include <config.h>
#include <asm/io.h>
#include <common.h>

char pwm_flag = 0;

/* Request a channel:
 * return 1:this channel is using
 * return 0:this channel is free*/
int pwm_request(int num){
	if (pwm_flag & (1 << num))
		return 1;
	else
		return 0;
}

void pwm_enable(int num)
{
	if (!pwm_request(num)){
		if (num >= 0 && num <= 3){
			writew(readw(TCU_TCSR(num)) | (TCU_TCSR_PWM_EN), TCU_TCSR(num));
			writew(readw(TCU_TESR) | (1 << num), TCU_TESR);
			pwm_flag |= 1 << num;
		} else {
			printf("the channel is not support pwm!\n");
		}
	} else {
		printf("the channel is using!\n");
		return ;
	}
}

void pwm_disable(int num)
{
	if (pwm_flag & (1 << num)){
		writew(readw(TCU_TCSR(num)) & (~TCU_TCSR_PWM_EN), TCU_TCSR(num));
		writew(readw(TCU_TECR) | (1 << num), TCU_TECR);
	}
	pwm_flag &= (~(1 << num));
}

void pwm_output_value(int num, int value)
{
	if (!!value == 1)
		writew(readw(TCU_TCSR(num)) | (TCU_TCSR_PWM_INITL_HIGH), TCU_TCSR(num));
	else
		writew(readw(TCU_TCSR(num)) & (~TCU_TCSR_PWM_INITL_HIGH), TCU_TCSR(num));
}

/* The value is the mode to shutdown
 * 1:shutdown graceful
 * 0:shutdown abrupt
 */
void pwm_shutdown(int num, int value)
{
	if (value == 1)
		writew(readw(TCU_TCSR(num)) & (~TCU_TCSR_PWM_SD), TCU_TCSR(num));
	else if (value == 0)
		writew(readw(TCU_TCSR(num)) | (TCU_TCSR_PWM_SD), TCU_TCSR(num));
	else
		printf("do not support this mode!\n");
}

void pwm_config(int num, int div, enum pwm_clk pwm_clock, int full_data, int half_data)
{
	switch (div){
	case 2:
		writew((readw(TCU_TCSR(num))
			& (~TCU_TCSR_PRESCALE_MASK))
			| (TCU_TCSR_PRESCALE4), TCU_TCSR(num));
		break;
	case 3:
		writew((readw(TCU_TCSR(num))
			& (~TCU_TCSR_PRESCALE_MASK))
			| (TCU_TCSR_PRESCALE16), TCU_TCSR(num));
		break;
	case 4:
		writew((readw(TCU_TCSR(num))
			& (~TCU_TCSR_PRESCALE_MASK))
			| (TCU_TCSR_PRESCALE64), TCU_TCSR(num));
		break;
	case 5:
		writew((readw(TCU_TCSR(num))
			& (~TCU_TCSR_PRESCALE_MASK))
			| (TCU_TCSR_PRESCALE256), TCU_TCSR(num));
		break;
	case 6:
		writew((readw(TCU_TCSR(num))
			& (~TCU_TCSR_PRESCALE_MASK))
			| (TCU_TCSR_PRESCALE1024), TCU_TCSR(num));
		break;
	case 1:
	default:
		writew((readw(TCU_TCSR(num))
			& (~TCU_TCSR_PRESCALE_MASK))
			| (TCU_TCSR_PRESCALE1), TCU_TCSR(num));
		break;
	}
	writel(1 << (num), TCU_TMSR);
	writel(1 << ((num) + 16), TCU_TMSR);
	writew(readw(TCU_TCSR(num)) | (TCU_TCSR_CLRZ), TCU_TCSR(num));
	writew(full_data, TCU_TDFR(num));
	writew(half_data, TCU_TDHR(num));
	switch (pwm_clock) {
	case EXTAL:
		writew((readw(TCU_TCSR(num))
			& (~(TCU_TCSR_EXT_EN
			| TCU_TCSR_RTC_EN
			| TCU_TCSR_PCK_EN)))
			| TCU_TCSR_EXT_EN, TCU_TCSR(num));
		break;
	case RTCCLK:
		writew((readw(TCU_TCSR(num))
			& (~(TCU_TCSR_EXT_EN
			| TCU_TCSR_RTC_EN
			| TCU_TCSR_PCK_EN)))
			| TCU_TCSR_RTC_EN, TCU_TCSR(num));
		break;
	case PCLK:
		writew((readw(TCU_TCSR(num))
			& (~(TCU_TCSR_EXT_EN
			| TCU_TCSR_RTC_EN
			| TCU_TCSR_PCK_EN)))
			| TCU_TCSR_PCK_EN, TCU_TCSR(num));

		break;
	default:
		printf("tcu do not support this clock!\n");
	}
}

void pwm_init(struct pwm *pwm_data)
{
	if (!pwm_request(pwm_data->channels)){
		pwm_disable(pwm_data->channels);
		pwm_output_value(pwm_data->channels, 1);
		pwm_config(pwm_data->channels,
			pwm_data->div,
			pwm_data->pwm_clock,
			pwm_data->full_data,
			pwm_data->half_data);
		pwm_enable(pwm_data->channels);
	} else {
		printf("the channel is using!\n");
	}
}
