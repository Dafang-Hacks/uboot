/*
 * JZ4775 pll configuration
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: cli <cli@ingenic.cn>
 *
 * Note: Burner's firmware is too big , for reduce code.
 *	 Burner does not support cpu freq lower than 600.
 *       and burner parameter must be right,
 *       there are not check or adjust when select burnner.
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
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;
#define CPCCR_CFG(a,b,c,d,e,f,g,h,i)	\
	(((a & 3) << 30)                \
	 | ((b & 3) << 28)                \
	 | ((c & 3) << 26)                 \
	 | ((d & 3) << 24)                 \
	 | (((e - 1) & 0xf) << 16)       \
	 | (((f - 1) & 0xf) << 12)         \
	 | (((g - 1) & 0xf) << 8)          \
	 | (((h - 1) & 0xf) << 4)          \
	 | (((i - 1) & 0xf) << 0))

#ifndef CONFIG_BURNER
static unsigned int nfro(unsigned int _fin, unsigned int _fout)
{
	unsigned int nrok = 0;
	unsigned int mnod = 0;
	int nf = 1, nr = 0, no = 1;
	int fvco = 0, bs = 0;
	int fin = _fin/1000000;
	int fout = _fout/1000000;

	if ((fin < 10) || (fin > 50) || (fout < 36))
		goto err;

	do {
		nrok++;
		nf = (fout * nrok)/fin;
		if ((nf > 128)) goto err;

		if (fin * nf != fout * nrok)
			continue;

		if (nrok <= 64) {
			no = 0;
			nr = nrok;
			fvco = fout * 1;
		} else if (nrok <= 128 && nf%2 == 0) {
			if (nf%2) goto err;
			no = 1;
			nr = nrok/2;
			fvco = fout * 2;
		} else if (nrok <= 256 && nf%4 == 0) {
			if (nf%4) goto err;
			no = 2;
			nr = nrok/4;
			fvco = fout * 4;
		} else if (nrok <= 512 && nf%8 == 0) {
			no = 3;
			nr = nrok/8;
			fvco = fout * 8;
		} else {
			goto err;
		}

		debug("fvco = %d\n", fvco);
		if (fout >= 63 && fvco >= 500) {
			bs = 1;
			break;
		} else if (fout >= 36 && fvco >= 300 && fvco <= 600) {
			bs = 0;
			break;
		}
	} while (1);

	mnod = (bs << 31) | ((nf - 1) << 24) | ((nr - 1) << 18) | (no << 16);

	return mnod;
err:
	printf("no adjust parameter to the fout:%dM fin: %d\n", fout, fin);
	while(1);
	return 0;
}
#else /* nCONFIG_BURNER */
static unsigned int nfro(unsigned int fin, unsigned int fout)
{
	unsigned int nf = 0;
	unsigned mnod = 0;

	nf = fout/fin;
	mnod = (1 << 31) | ((nf - 1) << 24);
	return mnod;
}
#endif /* CONFIG_BURNER */

extern int set_spl_cgu_array(unsigned reg, unsigned sel, unsigned div);

void pll_init(void)
{
	unsigned int pll_value;
	unsigned int tmp;
	unsigned int cpccr = 0;
	unsigned int sel = 0x1/*cpu sel apll default*/, sel_a = 0x2/*sclk_a use exclk default*/;
#ifdef CONFIG_JZ_SLT
	/*Slt select cpu freq on gpio*/
	unsigned int freq_sel = 0;
	gpio_direction_input(GPIO_FREQ_SEL0);
	gpio_direction_input(GPIO_FREQ_SEL1);
	freq_sel = gpio_get_value(GPIO_FREQ_SEL0) & 0x1;
	freq_sel |= (gpio_get_value(GPIO_FREQ_SEL1) & 0x1) << 1;

	printf("freq_sel = 0x%x\n", freq_sel);
	switch (freq_sel) {
	case 0:
		gd->arch.gi->cpufreq = SLT_CPU_FREQ0;
		gd->arch.gi->ddrfreq = SLT_CPU_FREQ0/SLT_DDR_DIV0;
		tmp = gd->arch.gi->ddr_div = SLT_DDR_DIV0;
		sel = 0x2;
		break;
	case 1:
		gd->arch.gi->cpufreq = SLT_CPU_FREQ1;
		gd->arch.gi->ddrfreq = SLT_CPU_FREQ1/SLT_DDR_DIV1;
		tmp = gd->arch.gi->ddr_div = SLT_DDR_DIV1;
		break;
	case 2:
		gd->arch.gi->cpufreq = SLT_CPU_FREQ2;
		gd->arch.gi->ddrfreq = SLT_CPU_FREQ2/SLT_DDR_DIV2;
		tmp = gd->arch.gi->ddr_div = SLT_DDR_DIV2;
		sel = 0x2;
		break;
	case 3:
		gd->arch.gi->cpufreq = SLT_CPU_FREQ3;
		gd->arch.gi->ddrfreq = SLT_CPU_FREQ3/SLT_DDR_DIV3;
		tmp = gd->arch.gi->ddr_div = SLT_DDR_DIV3;
	default:
		break;
	}
#else
	tmp = gd->arch.gi->cpufreq/gd->arch.gi->ddrfreq;
	if (tmp < 1)
		gd->arch.gi->ddr_div = tmp = 1;
	gd->arch.gi->cpufreq = gd->arch.gi->ddrfreq * tmp;
	gd->arch.gi->ddr_div = tmp;
#endif

#ifndef CONFIG_BURNER
	{
		unsigned int tmp2;
		tmp2 = gd->arch.gi->cpufreq/gd->arch.gi->extal;
		if (gd->arch.gi->cpufreq%gd->arch.gi->extal >= gd->arch.gi->extal/2) {
			gd->arch.gi->cpufreq = gd->arch.gi->extal * (tmp2 + 1);
		} else {
			gd->arch.gi->cpufreq = gd->arch.gi->extal * tmp2;
		}
		gd->arch.gi->ddrfreq = gd->arch.gi->cpufreq/gd->arch.gi->ddr_div;
	}
#endif

	pll_value = nfro(gd->arch.gi->extal, gd->arch.gi->cpufreq);

	if (cpm_inl(CPM_CPAPCR) & (1 << 8)) {
		sel_a = 0x1;
		sel = 0x2;
	}

	if (sel == 0x1) {
		sel_a = 0x1;
		cpm_outl(pll_value | (1 << 8) | 0x20, CPM_CPAPCR);
		while(!(cpm_inl(CPM_CPAPCR) & (1 << 10)));
	} else {
		cpm_outl(pll_value | (1 << 7), CPM_CPMPCR);
		while(!(cpm_inl(CPM_CPMPCR) & 1));
	}

	if(gd->arch.gi->cpufreq > 1000000000) {
		cpccr = CPCCR_CFG(sel_a,sel,sel,sel,10,5,tmp,2,1) | (7 << 20);
	} else if(gd->arch.gi->cpufreq > 800000000) {
		cpccr =	CPCCR_CFG(sel_a,sel,sel,sel, 8,4,tmp,2,1) | (7 << 20);
	} else if(gd->arch.gi->cpufreq > 600000000) {
		cpccr = CPCCR_CFG(sel_a,sel,sel,sel, 6,3,tmp,2,1) | (7 << 20);
	} else {
		cpccr = CPCCR_CFG(sel_a,sel,sel,sel, 4,2,tmp,2,1) | (7 << 20);
	}

	tmp = (cpm_inl(CPM_CPCCR) & (0xff << 24))
		| (cpccr & ~(0xff << 24))
		| (7 << 20);
	cpm_outl(tmp, CPM_CPCCR);
	while(cpm_inl(CPM_CPCSR) & 0x7);

	tmp = (cpccr & (0xff << 24)) | (cpm_inl(CPM_CPCCR) & ~(0xff << 24));
	cpm_outl(tmp ,CPM_CPCCR);
	while(cpm_inl(CPM_CPCSR) & 0x7);

	printf("cpuspeed:%d\n",gd->arch.gi->cpufreq);
	printf("ddrspeed:%d\n",gd->arch.gi->ddrfreq);
	printf("ddrdiv:%d\n", gd->arch.gi->ddr_div);
	printf("cpapcr %x\n", cpm_inl(CPM_CPAPCR));
	printf("cpmpcr %x\n", cpm_inl(CPM_CPMPCR));
	printf("cpccr:%x\n", cpccr);
	set_spl_cgu_array(CPM_DDRCDR, sel, -1);
#ifdef CGU_MSC_FREQ
#define RESET_MSC_DIV (gd->arch.gi->cpufreq / CGU_MSC_FREQ / 2 - 1)
	set_spl_cgu_array(CPM_MSC0CDR, sel, RESET_MSC_DIV);
	set_spl_cgu_array(CPM_MSC1CDR, sel, RESET_MSC_DIV);
#undef RESET_MSC_DIV
#endif
	set_spl_cgu_array(CPM_BCHCDR, sel, -1);

#ifndef CONFIG_BURNER
#ifdef CONFIG_SYS_PCLK_FREQ
#define RESET_LCD_DIV (gd->arch.gi->cpufreq / CONFIG_SYS_PCLK_FREQ - 1)
	if (sel == 0x1)
		set_spl_cgu_array(CPM_LPCDR, 0, RESET_LCD_DIV);
	else
		set_spl_cgu_array(CPM_LPCDR, 1, RESET_LCD_DIV);
#undef RESET_LCD_DIV
#endif
	if (sel == 0x1)
		set_spl_cgu_array(CPM_I2SCDR, 2, -1);
	else
		set_spl_cgu_array(CPM_I2SCDR, 3, -1);
#endif
}
