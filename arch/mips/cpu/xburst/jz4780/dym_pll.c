/*
 * JZ4780 pll configuration
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <ptkang@ingenic.cn>
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
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>

#ifdef CONFIG_SPL_BUILD

#define SEL_SCLKA               2
#define SEL_CPU                 2
#define SEL_H0                  2
#define SEL_H2                  2
#define DIV_PCLK                12
#define DIV_H2                  6
#define DIV_H0                  3
#define DIV_L2                  2
#define DIV_CPU                 1

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

unsigned int nfro(unsigned int fin, unsigned int fout)
{
	int nfk = 1, nrok = 0;
	unsigned int mnod = 0;
	int nr = 0, no = 0;
	if ((fin == 0) || (fout == 0)) return 0;

	do {
		nrok++;
		nfk = (fout * nrok)/fin;
		if ((nfk > 8192) || (nrok > 16 * 64)) goto err;
	}while (fin * nfk != fout * nrok);


	nr = nrok;
	no = 1;

	while ((nr % 2 == 0) && (nr > 1) && (no < 16)) {
		nr = nr >> 1;
		no = no << 1;
	}
	if ((nr > 64) || (no > 16)) goto err;
	mnod = ((nfk - 1) << 19 | (nr - 1) << 13 | (no - 1) << 9);

	return mnod;
err:
	printf("no adjust parameter to the fout:%ld\n", fout);
	while(1);
	return 0;
}

void pll_init(void)
{
	unsigned int cpccr;
	unsigned int cppcr = cpm_inl(CPM_CPPCR);
	cppcr &= ~(0xfff << 8);                 //BWADJ value
	cppcr |= 16 << 8 | 0xff | (0x1 << 30);  //stable delay to MAX, Fastlock mode enable
	cpm_outl(cppcr,CPM_CPPCR);

	struct global_info *ginfo = CONFIG_SPL_GINFO_BASE;

	unsigned int mpll_value;
	unsigned int tmp = ginfo->cpufreq / ginfo->ddrfreq;

	if(tmp < 1)
		tmp = 1;
	mpll_value = nfro(ginfo->extal,ginfo->ddrfreq * tmp);
	
	cpm_outl(mpll_value | 0x1,CPM_CPMPCR);
	while(!(cpm_inl(CPM_CPMPCR) & (0x1<<4)));

	if(ginfo->cpufreq > 1000000000) {
		cpccr = CPCCR_CFG(2,2,2,2,12,6,tmp,2,1) | (7 << 20);
	}else if(ginfo->cpufreq > 800000000){
		cpccr =	CPCCR_CFG(2,2,2,2,8,4,tmp,2,1) | (7 << 20);
	}else if(ginfo->cpufreq > 600000000){
		cpccr = CPCCR_CFG(2,2,2,2,6,3,tmp,2,1) | (7 << 20);
	}else {
		cpccr = CPCCR_CFG(2,2,2,2,4,2,tmp,2,1) | (7 << 20);
	}

	printf("cpuspeed:%d\n",ginfo->cpufreq);
	printf("ddrspeed:%d\n",ginfo->ddrfreq);
	printf("tmp:%d\n",tmp);
	printf("cpccr:%x\n",cpccr);

	cpm_outl(cpccr,CPM_CPCCR);
	while(cpm_inl(CPM_CPCSR) & 0x7);
}

#endif


