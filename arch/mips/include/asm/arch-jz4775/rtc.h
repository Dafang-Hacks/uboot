/*
 * JZ4775 rtc definitions
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

#ifndef __RTC_H__
#define __RTC_H__

#include <asm/arch/base.h>

/*************************************************************************
 * RTC (Real Time Clock)
 *************************************************************************/
#define RTC_RTCCR	0x00
#define RTC_RTCSR	0x04
#define RTC_RTCSAR	0x08
#define RTC_RTCGR	0x0C

#define RTC_HCR		0x20
#define RTC_HWFCR	0x24
#define RTC_HRCR	0x28
#define RTC_HWCR	0x2C
#define RTC_HWRSR	0x30
#define RTC_HSPR	0x34
#define RTC_WENR	0x3C
#define RTC_CKPCR	0x40
#define RTC_PWRONCR	0x48

/* RTC Control Register */
#define RTC_RTCCR_WRDY		(1 << 7)
#define RTC_RTCCR_1HZ		(1 << 6)
#define RTC_RTCCR_1HZIE		(1 << 5)
#define RTC_RTCCR_AF		(1 << 4)
#define RTC_RTCCR_AIE		(1 << 3)
#define RTC_RTCCR_AE		(1 << 2)
#define RTC_RTCCR_SELEXC	(1 << 1)
#define RTC_RTCCR_RTCE		(1 << 0)

/* Write Enable Pattern Register */
#define RTC_WENR_WEN		(1 << 31)
#define RTC_WENR_WENPAT_BIT	0
#define RTC_WENR_WENPAT_MASK	(0xffff << RTC_WENR_WENPAT_BIT)

/* HIBERNATE Wakeup Status Register */
#define RTC_HWRSR_APD		(1 << 8)
#define RTC_HWRSR_HR		(1 << 5)
#define RTC_HWRSR_PPR		(1 << 4)
#define RTC_HWRSR_PIN		(1 << 1)
#define RTC_HWRSR_ALM		(1 << 0)

#define RTC_HCR_PD	1

/* HIBERNATE mode Wakeup Filter Counter Register */
#define RTC_HWFCR_HWFCR_BIT	5
#define RTC_HWFCR_HWFCR_MASK	(0x7ff << RTC_HWFCR_HWFCR_BIT)
#define HWFCR_WAIT_TIME(ms)					\
	(((ms) << RTC_HWFCR_HWFCR_BIT) > RTC_HWFCR_HWFCR_MASK ?	\
	RTC_HWFCR_HWFCR_MASK : ((ms) << RTC_HWFCR_HWFCR_BIT))

/* HIBERNATE Reset Counter Register */
#define RTC_HRCR_HRCR_BIT	11
#define RTC_HRCR_HRCR_MASK	(0xf << RTC_HRCR_HRCR_BIT)
#define HRCR_WAIT_TIME(ms)					\
	(ms < 62 ? 0 : (((ms / 62 - 1) << RTC_HRCR_HRCR_BIT) >	\
	RTC_HRCR_HRCR_MASK ? RTC_HRCR_HRCR_MASK :		\
	((ms / 62 - 1) << RTC_HRCR_HRCR_BIT)))

#define RTC_WENR_WENPAT_WRITABLE    (0xa55a)

#endif /* __RTC_H__ */
