/*
 * JZ4775 TCU definitions
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
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

#ifndef __TCU_H__
#define __TCU_H__

#include <asm/arch/base.h>

enum pwm_clk{
	EXTAL,
	RTCCLK,
	PCLK,
};

struct pwm {
	int channels;
	int div;
	enum pwm_clk pwm_clock;
	int full_data;
	int half_data;
};

/*struct pwm pwm_backlight;*/

/*************************************************************************
 * TCU (Timer Counter Unit)
 *************************************************************************/
#define TCU_TSR		(TCU_BASE + 0x1C)
#define TCU_TSSR	(TCU_BASE + 0x2C)
#define TCU_TSCR	(TCU_BASE + 0x3C)
#define TCU_TER		(TCU_BASE + 0x10)
#define TCU_TESR	(TCU_BASE + 0x14)
#define TCU_TECR	(TCU_BASE + 0x18)
#define TCU_TFR		(TCU_BASE + 0x20)
#define TCU_TFSR	(TCU_BASE + 0x24)
#define TCU_TFCR	(TCU_BASE + 0x28)
#define TCU_TMR		(TCU_BASE + 0x30)
#define TCU_TMSR	(TCU_BASE + 0x34)
#define TCU_TMCR	(TCU_BASE + 0x38)
#define TCU_TDFR0	(TCU_BASE + 0x40)
#define TCU_TDHR0	(TCU_BASE + 0x44)
#define TCU_TCNT0	(TCU_BASE + 0x48)
#define TCU_TCSR0	(TCU_BASE + 0x4C)
#define TCU_TDFR1	(TCU_BASE + 0x50)
#define TCU_TDHR1	(TCU_BASE + 0x54)
#define TCU_TCNT1	(TCU_BASE + 0x58)
#define TCU_TCSR1	(TCU_BASE + 0x5C)
#define TCU_TDFR2	(TCU_BASE + 0x60)
#define TCU_TDHR2	(TCU_BASE + 0x64)
#define TCU_TCNT2	(TCU_BASE + 0x68)
#define TCU_TCSR2	(TCU_BASE + 0x6C)
#define TCU_TDFR3	(TCU_BASE + 0x70)
#define TCU_TDHR3	(TCU_BASE + 0x74)
#define TCU_TCNT3	(TCU_BASE + 0x78)
#define TCU_TCSR3	(TCU_BASE + 0x7C)
#define TCU_TDFR4	(TCU_BASE + 0x80)
#define TCU_TDHR4	(TCU_BASE + 0x84)
#define TCU_TCNT4	(TCU_BASE + 0x88)
#define TCU_TCSR4	(TCU_BASE + 0x8C)
#define TCU_TDFR5	(TCU_BASE + 0x90)
#define TCU_TDHR5	(TCU_BASE + 0x94)
#define TCU_TCNT5	(TCU_BASE + 0x98)
#define TCU_TCSR5	(TCU_BASE + 0x9C)

#define TCU_TDFR(n)	(TCU_BASE + (0x40 + (n)*0x10))
#define TCU_TDHR(n)	(TCU_BASE + (0x44 + (n)*0x10))
#define TCU_TCNT(n)	(TCU_BASE + (0x48 + (n)*0x10))
#define TCU_TCSR(n)	(TCU_BASE + (0x4C + (n)*0x10))
#define TCU_TCSR_PWM_SD		(1 << 9)
#define TCU_TCSR_PWM_INITL_HIGH	(1 << 8)
#define TCU_TCSR_PWM_EN		(1 << 7)
#define TCU_TCSR_PRESCALE_BIT	3
#define TCU_TCSR_PRESCALE_MASK	(0x7 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE1	(0x0 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE4	(0x1 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE16	(0x2 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE64	(0x3 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE256	(0x4 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE1024	(0x5 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_EXT_EN		(1 << 2)
#define TCU_TCSR_RTC_EN		(1 << 1)
#define TCU_TCSR_PCK_EN		(1 << 0)

#define TCU_TER_TCEN5		(1 << 5)
#define TCU_TER_TCEN4		(1 << 4)
#define TCU_TER_TCEN3		(1 << 3)
#define TCU_TER_TCEN2		(1 << 2)
#define TCU_TER_TCEN1		(1 << 1)
#define TCU_TER_TCEN0		(1 << 0)

#define TCU_TESR_TCST5		(1 << 5)
#define TCU_TESR_TCST4		(1 << 4)
#define TCU_TESR_TCST3		(1 << 3)
#define TCU_TESR_TCST2		(1 << 2)
#define TCU_TESR_TCST1		(1 << 1)
#define TCU_TESR_TCST0		(1 << 0)

#define TCU_TECR_TCCL5		(1 << 5)
#define TCU_TECR_TCCL4		(1 << 4)
#define TCU_TECR_TCCL3		(1 << 3)
#define TCU_TECR_TCCL2		(1 << 2)
#define TCU_TECR_TCCL1		(1 << 1)
#define TCU_TECR_TCCL0		(1 << 0)

#define TCU_TFR_HFLAG5		(1 << 21)
#define TCU_TFR_HFLAG4		(1 << 20)
#define TCU_TFR_HFLAG3		(1 << 19)
#define TCU_TFR_HFLAG2		(1 << 18)
#define TCU_TFR_HFLAG1		(1 << 17)
#define TCU_TFR_HFLAG0		(1 << 16)
#define TCU_TFR_FFLAG5		(1 << 5)
#define TCU_TFR_FFLAG4		(1 << 4)
#define TCU_TFR_FFLAG3		(1 << 3)
#define TCU_TFR_FFLAG2		(1 << 2)
#define TCU_TFR_FFLAG1		(1 << 1)
#define TCU_TFR_FFLAG0		(1 << 0)

#define TCU_TFSR_HFLAG5		(1 << 21)
#define TCU_TFSR_HFLAG4		(1 << 20)
#define TCU_TFSR_HFLAG3		(1 << 19)
#define TCU_TFSR_HFLAG2		(1 << 18)
#define TCU_TFSR_HFLAG1		(1 << 17)
#define TCU_TFSR_HFLAG0		(1 << 16)
#define TCU_TFSR_FFLAG5		(1 << 5)
#define TCU_TFSR_FFLAG4		(1 << 4)
#define TCU_TFSR_FFLAG3		(1 << 3)
#define TCU_TFSR_FFLAG2		(1 << 2)
#define TCU_TFSR_FFLAG1		(1 << 1)
#define TCU_TFSR_FFLAG0		(1 << 0)

#define TCU_TFCR_HFLAG5		(1 << 21)
#define TCU_TFCR_HFLAG4		(1 << 20)
#define TCU_TFCR_HFLAG3		(1 << 19)
#define TCU_TFCR_HFLAG2		(1 << 18)
#define TCU_TFCR_HFLAG1		(1 << 17)
#define TCU_TFCR_HFLAG0		(1 << 16)
#define TCU_TFCR_FFLAG5		(1 << 5)
#define TCU_TFCR_FFLAG4		(1 << 4)
#define TCU_TFCR_FFLAG3		(1 << 3)
#define TCU_TFCR_FFLAG2		(1 << 2)
#define TCU_TFCR_FFLAG1		(1 << 1)
#define TCU_TFCR_FFLAG0		(1 << 0)

#define TCU_TMR_HMASK5		(1 << 21)
#define TCU_TMR_HMASK4		(1 << 20)
#define TCU_TMR_HMASK3		(1 << 19)
#define TCU_TMR_HMASK2		(1 << 18)
#define TCU_TMR_HMASK1		(1 << 17)
#define TCU_TMR_HMASK0		(1 << 16)
#define TCU_TMR_FMASK5		(1 << 5)
#define TCU_TMR_FMASK4		(1 << 4)
#define TCU_TMR_FMASK3		(1 << 3)
#define TCU_TMR_FMASK2		(1 << 2)
#define TCU_TMR_FMASK1		(1 << 1)
#define TCU_TMR_FMASK0		(1 << 0)

#define TCU_TMSR_HMST5		(1 << 21)
#define TCU_TMSR_HMST4		(1 << 20)
#define TCU_TMSR_HMST3		(1 << 19)
#define TCU_TMSR_HMST2		(1 << 18)
#define TCU_TMSR_HMST1		(1 << 17)
#define TCU_TMSR_HMST0		(1 << 16)
#define TCU_TMSR_FMST5		(1 << 5)
#define TCU_TMSR_FMST4		(1 << 4)
#define TCU_TMSR_FMST3		(1 << 3)
#define TCU_TMSR_FMST2		(1 << 2)
#define TCU_TMSR_FMST1		(1 << 1)
#define TCU_TMSR_FMST0		(1 << 0)

#define TCU_TMCR_HMCL5		(1 << 21)
#define TCU_TMCR_HMCL4		(1 << 20)
#define TCU_TMCR_HMCL3		(1 << 19)
#define TCU_TMCR_HMCL2		(1 << 18)
#define TCU_TMCR_HMCL1		(1 << 17)
#define TCU_TMCR_HMCL0		(1 << 16)
#define TCU_TMCR_FMCL5		(1 << 5)
#define TCU_TMCR_FMCL4		(1 << 4)
#define TCU_TMCR_FMCL3		(1 << 3)
#define TCU_TMCR_FMCL2		(1 << 2)
#define TCU_TMCR_FMCL1		(1 << 1)
#define TCU_TMCR_FMCL0		(1 << 0)

#define TCU_TSR_WDTS		(1 << 16)
#define TCU_TSR_STOP5		(1 << 5)
#define TCU_TSR_STOP4		(1 << 4)
#define TCU_TSR_STOP3		(1 << 3)
#define TCU_TSR_STOP2		(1 << 2)
#define TCU_TSR_STOP1		(1 << 1)
#define TCU_TSR_STOP0		(1 << 0)

#define TCU_TSSR_WDTSS		(1 << 16)
#define TCU_TSSR_STPS5		(1 << 5)
#define TCU_TSSR_STPS4		(1 << 4)
#define TCU_TSSR_STPS3		(1 << 3)
#define TCU_TSSR_STPS2		(1 << 2)
#define TCU_TSSR_STPS1		(1 << 1)
#define TCU_TSSR_STPS0		(1 << 0)

#define TCU_TSSR_WDTSC		(1 << 16)
#define TCU_TSSR_STPC5		(1 << 5)
#define TCU_TSSR_STPC4		(1 << 4)
#define TCU_TSSR_STPC3		(1 << 3)
#define TCU_TSSR_STPC2		(1 << 2)
#define TCU_TSSR_STPC1		(1 << 1)
#define TCU_TSSR_STPC0		(1 << 0)
#define TCU_TCSR_CLRZ	(1 << 10)

int pwm_request(int num);
void pwm_enable(int num);
void pwm_disable(int num);
void pwm_output_value(int num,int value);
void pwm_shutdown(int num,int value);
void pwm_config(int num,int div, enum pwm_clk pwm_clock,int full_data,int half_data);
void pwm_init(struct pwm *pwm_data);

#endif /* __TCU_H__ */
