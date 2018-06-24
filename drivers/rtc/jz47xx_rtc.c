/*
 * (C) Copyright 2003
 * David Müller ELSOFT AG Switzerland. d.mueller@elsoft.ch
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

/*
 * Date & Time support for the built-in Samsung S3C24X0 RTC
 */
#include <common.h>
#include <command.h>

#include <rtc.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include "jz47xx_rtc.h"
typedef enum {
	RTC_ENABLE,
	RTC_DISABLE
} RTC_ACCESS;
static struct rtc_time default_tm = {
	.tm_year = (2013 - 1900), // year 2012
	.tm_mon = (1 - 1),       // month 06
	.tm_mday = 7,             // day 1
	.tm_hour = 16,
	.tm_min = 0,
	.tm_sec = 0
};


static inline void SetRTC_Access(RTC_ACCESS a)
{
}

/* ------------------------------------------------------------------------- */

int rtc_get(struct rtc_time *tmp)
{

	return 0;
}

static unsigned int jzrtc_readl(int offset)
{
	unsigned int data, timeout = 0x100000;
	do {
		data = readl(RTC_BASE + offset);
	} while (readl(RTC_BASE + offset) != data && timeout--);
	if (timeout <= 0)
		printf("RTC rtc_read_reg timeout!\n");
	return data;
}

static inline void wait_write_ready(void)
{
	int timeout = 0x100000;
	while (!(jzrtc_readl(RTC_RTCCR) & RTCCR_WRDY) && timeout--);
	if (timeout <= 0)
		printf("RTC __wait_write_ready timeout!\n");
}

static void jzrtc_writel(int offset, unsigned int value)
{
	int timeout = 0x100000;

	wait_write_ready();
	writel(WENR_WENPAT_WRITABLE, RTC_BASE + RTC_WENR);
	wait_write_ready();
	while (!(jzrtc_readl(RTC_WENR) & WENR_WEN) && timeout--);
	if (timeout <= 0) {
		while(1) {
			timeout = 0x100000;
			writel(WENR_WENPAT_WRITABLE, RTC_BASE + RTC_WENR);
			wait_write_ready();
			while (!(jzrtc_readl(RTC_WENR) & WENR_WEN) && timeout--);
			if (timeout > 0)
				break;
			else
				printf("RTC __wait_writable 0x%x timeout!\n", (jzrtc_readl(RTC_WENR)));
		}
	}

	writel(value,RTC_BASE + offset);
	wait_write_ready();
}

int rtc_tm_to_time(struct rtc_time *tm, unsigned long *time)
{
	*time = mktime(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour , tm->tm_min, tm->tm_sec);
	return 0;
}

int rtc_read_time(void)
{
	struct rtc_time *tm;
	unsigned int tmp;
	rtc_set(&default_tm);
	tmp = jzrtc_readl(RTC_RTCSR);
	to_tm(tmp, tm);
	printf("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);	
	return 0;
}


int rtc_set(struct rtc_time *tmp)
{
	unsigned long time;
	rtc_tm_to_time(tmp,&time);
	jzrtc_writel(RTC_RTCSR, time);

	return 0;
}

void rtc_reset(void)
{
}

