/*
 * JZ4780 WDT definitions
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

#ifndef __WDT_H__
#define __WDT_H__

#include <asm/arch/base.h>

#define TCU_TSCR			0x3c

#define WDT_TCSR			0xc
#define WDT_TCER			0x4
#define WDT_TDR				0x0
#define WDT_TCNT			0x8

#define TSCR_WDTSC			(1 << 16)

#define TCSR_PRESCALE_1			(0 << 3)
#define TCSR_PRESCALE_4			(1 << 3)
#define TCSR_PRESCALE_16		(2 << 3)
#define TCSR_PRESCALE_64		(3 << 3)
#define TCSR_PRESCALE_256		(4 << 3)
#define TCSR_PRESCALE_1024		(5 << 3)

#define TCSR_EXT_EN			(1 << 2)
#define TCSR_RTC_EN			(1 << 1)
#define TCSR_PCK_EN			(1 << 0)

#define TCER_TCEN			(1 << 0)

#define WDT_DIV				64
#if (WDT_DIV == 1)
#define TCSR_PRESCALE			TCSR_PRESCALE_1
#elif (WDT_DIV == 4)
#define TCSR_PRESCALE			TCSR_PRESCALE_4
#elif (WDT_DIV == 16)
#define TCSR_PRESCALE			TCSR_PRESCALE_16
#elif (WDT_DIV == 64)
#define TCSR_PRESCALE			TCSR_PRESCALE_64
#elif (WDT_DIV == 256)
#define TCSR_PRESCALE			TCSR_PRESCALE_256
#elif (WDT_DIV == 1024)
#define TCSR_PRESCALE			TCSR_PRESCALE_1024
#endif

#define RESET_DELAY_MS			4
#define RTC_FREQ			32768

#endif /* __WDT_H__ */
