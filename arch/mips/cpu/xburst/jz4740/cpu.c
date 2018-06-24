/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 * (C) Copyright 2011
 * Xiangfu Liu <xiangfu@openmobilefree.net>
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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/jz4740.h>

void _machine_restart(void)
{
	struct jz4740_wdt *wdt = (struct jz4740_wdt *)JZ4740_WDT_BASE;
	struct jz4740_tcu *tcu = (struct jz4740_tcu *)JZ4740_TCU_BASE;
	u16 tmp;

	/* wdt_select_extalclk() */
	tmp = readw(&wdt->tcsr);
	tmp &= ~(WDT_TCSR_EXT_EN | WDT_TCSR_RTC_EN | WDT_TCSR_PCK_EN);
	tmp |= WDT_TCSR_EXT_EN;
	writew(tmp, &wdt->tcsr);

	/* wdt_select_clk_div64() */
	tmp = readw(&wdt->tcsr);
	tmp &= ~WDT_TCSR_PRESCALE_MASK;
	tmp |= WDT_TCSR_PRESCALE64,
	writew(tmp, &wdt->tcsr);

	writew(100, &wdt->tdr); /* wdt_set_data(100) */
	writew(0, &wdt->tcnt); /* wdt_set_count(0); */
	writel(TCU_TSSR_WDTSC, &tcu->tscr); /* tcu_start_wdt_clock */
	writeb(readb(&wdt->tcer) | WDT_TCER_TCEN, &wdt->tcer); /* wdt start */

	while (1)
		;
}
