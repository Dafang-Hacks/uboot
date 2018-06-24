/*
 * JZ4775 OST(TCU) definitions
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

#ifndef __OST_H__
#define __OST_H__

#include <asm/arch/base.h>

#define TCU_TESR			0x14
#define TCU_OSTDR			0xe0
#define TCU_OSTCNTL			0xe4
#define TCU_OSTCNTH			0xe8
#define TCU_OSTCSR			0xec
#define TCU_OSTCNTHBUF			0xfc

#define TER_OSTEN			(1 << 15)

#define OSTCSR_CNT_MD			(1 << 15)
#define OSTCSR_SD			(1 << 9)
#define OSTCSR_PRESCALE_1		(0 << 3)
#define OSTCSR_PRESCALE_4		(1 << 3)
#define OSTCSR_PRESCALE_16		(2 << 3)
#define OSTCSR_PRESCALE_64		(3 << 3)
#define OSTCSR_PRESCALE_256		(4 << 3)
#define OSTCSR_PRESCALE_1024		(5 << 3)

#define OST_DIV				4
#if (OST_DIV == 1)
#define OSTCSR_PRESCALE			OSTCSR_PRESCALE_1
#elif (OST_DIV == 4)
#define OSTCSR_PRESCALE			OSTCSR_PRESCALE_4
#elif (OST_DIV == 16)
#define OSTCSR_PRESCALE			OSTCSR_PRESCALE_16
#elif (OST_DIV == 64)
#define OSTCSR_PRESCALE			OSTCSR_PRESCALE_64
#elif (OST_DIV == 256)
#define OSTCSR_PRESCALE			OSTCSR_PRESCALE_256
#elif (OST_DIV == 1024)
#define OSTCSR_PRESCALE			OSTCSR_PRESCALE_1024
#endif

#define OSTCSR_EXT_EN			(1 << 2)

#endif /* __OST_H__ */
