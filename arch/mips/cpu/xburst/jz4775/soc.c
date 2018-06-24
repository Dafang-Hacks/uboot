/*
 * JZ4775 common routines
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 * Based on: arch/mips/cpu/xburst/jz4780/jz4780.c
 *           Written by Paul Burton <paul.burton@imgtec.com>
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

/* #define DEBUG */
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
#include <spl.h>
#include <regulator.h>

#ifdef CONFIG_SPL_BUILD

/* Pointer to as well as the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;
gd_t gdata __attribute__ ((section(".data")));

#ifndef CONFIG_BURNER
struct global_info ginfo __attribute__ ((section(".data"))) = {
	.extal		= CONFIG_SYS_EXTAL,
	.cpufreq	= CONFIG_SYS_CPU_FREQ,
	.ddrfreq	= CONFIG_SYS_MEM_FREQ,
	.uart_idx	= CONFIG_SYS_UART_INDEX,
	.baud_rate	= CONFIG_BAUDRATE,
};

#endif

extern void gpio_init(void);
extern void pll_init(void);
extern void sdram_init(void);
extern void validate_cache(void);

void board_init_f(ulong dummy)
{
	/* Set global data pointer */
	gd = &gdata;

	/* Setup global info */
#ifndef CONFIG_BURNER
	gd->arch.gi = &ginfo;
#else
	gd->arch.gi = (struct global_info *)CONFIG_SPL_GINFO_BASE;
#endif
	gd->arch.gi->ddr_div = ((gd->arch.gi->cpufreq % gd->arch.gi->ddrfreq) == 0)
		? (gd->arch.gi->cpufreq / gd->arch.gi->ddrfreq)
		: (gd->arch.gi->cpufreq / gd->arch.gi->ddrfreq + 1);

	gpio_init();

	/* Init uart first */
	enable_uart_clk();

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#ifdef CONFIG_FPGA
	printf("Boot Select Is %d\n", *(volatile unsigned int *)0xf4000110);
#endif
#endif

	debug("Timer init\n");
	timer_init();

#ifdef CONFIG_SPL_CORE_VOLTAGE
	debug("Set core voltage:%dmv\n", CONFIG_SPL_CORE_VOLTAGE);
	spl_regulator_set_voltage(REGULATOR_CORE, CONFIG_SPL_CORE_VOLTAGE);
#endif
#ifdef CONFIG_SPL_MEM_VOLTAGE
	debug("Set mem voltage:%dmv\n", CONFIG_SPL_MEM_VOLTAGE);
	spl_regulator_set_voltage(REGULATOR_MEM, CONFIG_SPL_MEM_VOLTAGE);
#endif

	debug("PLL init\n");
	pll_init();

	debug("CLK init\n");
	clk_init();

	debug("SDRAM init\n");
	sdram_init();

#ifdef CONFIG_DDR_TEST
	ddr_basic_tests();
#endif
	debug("validate cache\n");
	validate_cache();
#ifndef CONFIG_BURNER
	/* Clear the BSS */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);

	debug("board_init_r\n");
	board_init_r(NULL, 0);
#else
	printf("run firmware finished\n");
	return ;
#endif
}

extern void flush_cache_all(void);

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
			(image_entry_noargs_t) spl_image->entry_point;

	flush_cache_all();

	debug("image entry point: 0x%X\n", spl_image->entry_point);
	image_entry();
}

#endif /* CONFIG_SPL_BUILD */

/*
 * U-Boot common functions
 */

void enable_interrupts(void)
{
}

int disable_interrupts(void)
{
	return 0;
}
