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

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_CPCCR_SEL
/**
 * default CPCCR configure.
 * It is suggested if you are NOT sure how it works.
 */
#define SEL_SCLKA		2
#define SEL_CPU			2
#define SEL_H0			2
#define SEL_H2			2
#if (CONFIG_SYS_CPU_FREQ > 1000000000)
#define DIV_PCLK		12
#define DIV_H2			6
#else
#define DIV_PCLK		8
#define DIV_H2			4
#endif
#ifdef CONFIG_SYS_MEM_DIV
#define DIV_H0			CONFIG_SYS_MEM_DIV
#else
#define DIV_H0			gd->arch.gi->ddr_div
#endif
#define DIV_L2			2
#define DIV_CPU			1

#define CPCCR_CFG		(((SEL_SCLKA & 3) << 30)		\
				 | ((SEL_CPU & 3) << 28)		\
				 | ((SEL_H0 & 3) << 26)			\
				 | ((SEL_H2 & 3) << 24)			\
				 | (((DIV_PCLK - 1) & 0xf) << 16)	\
				 | (((DIV_H2 - 1) & 0xf) << 12)		\
				 | (((DIV_H0 - 1) & 0xf) << 8)		\
				 | (((DIV_L2 - 1) & 0xf) << 4)		\
				 | (((DIV_CPU - 1) & 0xf) << 0))
#else
/**
 * Board CPCCR configure.
 * CONFIG_SYS_CPCCR_SEL should be define in [board].h
 */
#define CPCCR_CFG CONFIG_SYS_CPCCR_SEL
#endif

struct m_n_od {
	unsigned int m;
	unsigned int n;
	unsigned int od;
};

static void get_mnod(unsigned int fin, unsigned long fout, struct m_n_od *p)
{
	p->n = 1;
	p->od = fout < 600 ? 2 : 1;
	p->m = (fout / fin) * p->od * p->n;
}

void pll_init(void)
{
	cpm_cpxpcr_t cpxpcr;
	unsigned int cpccr = cpm_inl(CPM_CPCCR);
	unsigned int cppcr = cpm_inl(CPM_CPPCR);
	struct m_n_od m_n_od = {0, 0, 0};

	cppcr &= ~(0xfffff);
	cppcr |= 18 << 8 | 0x20;
	cpm_outl(cppcr,CPM_CPPCR);

	debug("pll init...");
#define GET_M_N_OD_DIRECTLY(X)						\
	cpxpcr.b.PLLM = CONFIG_SYS_##X##_M - 1;				\
	cpxpcr.b.PLLN = CONFIG_SYS_##X##_N - 1;				\
	cpxpcr.b.PLLOD = CONFIG_SYS_##X##_OD - 1
#define GET_M_N_OD_CALC(X)						\
	get_mnod(gd->arch.gi->extal, CONFIG_SYS_##X##_FREQ, &m_n_od);	\
	cpxpcr.b.PLLM = m_n_od.m - 1;					\
	cpxpcr.b.PLLN = m_n_od.n - 1;					\
	cpxpcr.b.PLLOD = m_n_od.od - 1
#define INIT_PLL(X)							\
	cpxpcr.b.PLLEN = 1;						\
	cpm_outl(cpxpcr.d32,CPM_CP##X##PCR);				\
	while(!(cpm_inl(CPM_CP##X##PCR) & (0x1<<4)))

#ifdef CONFIG_SYS_APLL_FREQ
	cpxpcr.d32 = 0;
  #if defined (CONFIG_SYS_APLL_M) && (CONFIG_SYS_APLL_N) && (CONFIG_SYS_APLL_OD)
	GET_M_N_OD_DIRECTLY(APLL);
  #else
    #if (CONFIG_SYS_APLL_FREQ > 0)
	GET_M_N_OD_CALC(APLL);
    #else
	/* APLL on JZ4780 should be enabled, 48m as default */
    #endif
  #endif
	INIT_PLL(A);
	debug("CPM_CPAPCR %x\n",cpm_inl(CPM_CPAPCR));
#endif /* CONFIG_SYS_APLL_FREQ */

#if (CONFIG_SYS_MPLL_FREQ > 0)
	cpxpcr.d32 = 0;
  #if defined (CONFIG_SYS_MPLL_M) && (CONFIG_SYS_MPLL_N) && (CONFIG_SYS_MPLL_OD)
	GET_M_N_OD_DIRECTLY(MPLL);
  #else
	GET_M_N_OD_CALC(MPLL);
  #endif
	INIT_PLL(M);
	debug("CPM_CPMPCR %x\n",cpm_inl(CPM_CPMPCR));
#endif /* CONFIG_SYS_MPLL_FREQ */

#if (CONFIG_SYS_EPLL_FREQ > 0)
	cpxpcr.d32 = 0;
  #if defined (CONFIG_SYS_EPLL_M) && (CONFIG_SYS_EPLL_N) && (CONFIG_SYS_EPLL_OD)
	GET_M_N_OD_DIRECTLY(EPLL);
  #else
	GET_M_N_OD_CALC(EPLL);
  #endif
	INIT_PLL(E);
	debug("CPM_CPEPCR %x\n",cpm_inl(CPM_CPEPCR));
#endif /* CONFIG_SYS_EPLL_FREQ */

#if (CONFIG_SYS_VPLL_FREQ > 0)
  #if defined (CONFIG_SYS_VPLL_M) && (CONFIG_SYS_VPLL_N) && (CONFIG_SYS_VPLL_OD)
	GET_M_N_OD_DIRECTLY(VPLL);
  #else
	GET_M_N_OD_CALC(VPLL);
  #endif
	INIT_PLL(V);
	debug("CPM_CPVPCR %x\n",cpm_inl(CPM_CPVPCR));
#endif /* CONFIG_SYS_VPLL_FREQ */

	cpccr = CPCCR_CFG | (7 << 20);
	cpm_outl(cpccr,CPM_CPCCR);
	while(cpm_inl(CPM_CPCSR) & 0x7);

	debug("pll init ok\n");
}
