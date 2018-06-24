/*
 * m200 clock common interface
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 * Based on: newxboot/modules/clk/jz4775_clk.c
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
/*#define DEBUG*/
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef DUMP_CGU_SELECT
static char clk_name[][10] = {
	[VPU] = {"vpu"},
	[OTG] = {"otg"},
	[I2S] = {"i2s"},
	[LCD] = {"lcd"},
	[UHC] = {"uhc"},
	[SSI] = {"ssi"},
	[CIM] = {"cim"},
	[PCM] = {"pcm"},
	[GPU] = {"gpu"},
	[ISP] = {"isp"},
	[DDR] = {"ddr"},
	[MSC] = {"msc"},
	[MSC1] = {"msc1"},
	[MSC2] = {"msc2"},
	[BCH] = {"bch"},
};

static char * cgu_name(int clk) {
	return clk_name[clk];
}
#endif

struct cgu cgu_clk_sel[CGU_CNT] = {
	[DDR] = {1, CPM_DDRCDR, 30, CONFIG_DDR_SEL_PLL, {0, APLL, MPLL, -1}, 29, 28, 27},
	[MSC] = {1, CPM_MSC0CDR, 31, CONFIG_CPU_SEL_PLL, {APLL, MPLL, -1, -1}, 29, 28, 27},
	[BCH] = {1, CPM_BCHCDR, 31, CONFIG_CPU_SEL_PLL, {APLL, MPLL, -1, -1}, 29, 28, 27},
	[VPU] = {1, CPM_VPUCDR, 31, CONFIG_CPU_SEL_PLL, {APLL, MPLL, -1, -1}, 29, 28, 27},
#ifndef CONFIG_BURNER
	[OTG] = {1, CPM_USBCDR, 30, EXCLK, {EXCLK, EXCLK, APLL, MPLL}, 29, 28, 27},
#endif
	[I2S] = {1, CPM_I2SCDR, 30, EXCLK, {APLL, MPLL, EXCLK, -1}, 29, 28, 27},
	[LCD] = {1, CPM_LPCDR,  31, CONFIG_CPU_SEL_PLL, {APLL, MPLL, -1, -1}, 28, 27, 26},
	[MSC1] = {0, CPM_MSC1CDR, 0, 0, {-1, -1, -1, -1}, 29, 28, 27},
	[MSC2] = {0, CPM_MSC2CDR, 0, 0, {-1, -1, -1, -1}, 29, 28, 27},
	[UHC] = {1, CPM_UHCCDR, 30, OTG, {APLL, MPLL, OTG, -1}, 29, 28, 27},
#ifdef CONFIG_BURNER
	[SSI] = {1, CPM_SSICDR, 30, EXCLK, {APLL, MPLL, EXCLK, -1}, 29, 28, 27},
#else
	[SSI] = {1, CPM_SSICDR, 30, CONFIG_CPU_SEL_PLL, {APLL, MPLL, EXCLK, -1}, 29, 28, 27},
#endif
	[CIM] = {1, CPM_CIMCDR, 31, CONFIG_CPU_SEL_PLL, {APLL, MPLL, -1, -1}, 30, 29, 28},
	[PCM] = {1, CPM_PCMCDR, 30, CONFIG_CPU_SEL_PLL, {APLL, MPLL, EXCLK, -1}, 28, 27, 26},
	[GPU] = {1, CPM_GPUCDR, 31, CONFIG_CPU_SEL_PLL, {APLL, MPLL, -1, -1}, 29, 28, 27},
	[ISP] = {1, CPM_ISPCDR, 31, CONFIG_CPU_SEL_PLL, {APLL, MPLL, -1, -1}, 29, 28, 27},
};

void clk_prepare(void)
{
	/*stop clk and set div max*/
	int id;
	struct cgu *cgu = NULL;
	unsigned regval = 0, reg = 0;

	for (id = 0; id < CGU_CNT; id++) {
		cgu = &(cgu_clk_sel[id]);
		reg = CPM_BASE + cgu->off;

#ifdef CONFIG_BURNER
		if (id == OTG)
			continue;
#endif
#ifdef CONFIG_BURNER
		if (id == SSI)
			continue;
#endif
		if (id != OTG) {
			regval = readl(reg);
			/*set div max*/
			regval |= 0xff | (1 << cgu->ce);
			while (readl(reg) & (1 << cgu->busy));
			writel(regval, reg);
		}

		/*stop clk*/
		while (readl(reg) & (1 << cgu->busy));
		regval = readl(reg);
		regval |= ((1 << cgu->stop) | (1 << cgu->ce));
		writel(regval, reg);
		while (readl(reg) & (1 << cgu->busy));

		/*clear ce*/
		regval = readl(reg);
		regval &= ~(1 << cgu->ce);
		writel(regval, reg);
#ifdef DUMP_CGU_SELECT
		printf("%s(0x%x) :0x%x\n",clk_name[id] ,reg,  readl(reg));
#endif
	}
}

void cgu_clks_set(struct cgu *cgu_clks, int nr_cgu_clks)
{
	int i, j, id;
	unsigned int xcdr = 0;
	unsigned int reg = 0;
	extern struct cgu_clk_src cgu_clk_src[];

	for (i = 0; cgu_clk_src[i].cgu_clk != SRC_EOF; i++) {
		id = cgu_clk_src[i].cgu_clk;
		cgu_clks[id].sel_src = cgu_clk_src[i].src;
	}

	for(i = 0; i < nr_cgu_clks; i++) {
		for (j = 0; j < 4; j++) {
			if (cgu_clks[i].sel_src == cgu_clks[i].sel[j] &&
					cgu_clks[i].en == 1) {
				reg = CPM_BASE + cgu_clks[i].off;
				xcdr = readl(reg);
				xcdr &= ~(3 << 30);
				xcdr |= j << cgu_clks[i].sel_bit;
				writel(xcdr, reg);
#ifdef DUMP_CGU_SELECT
				printf("%s: 0x%X: value=0x%X\n", cgu_name(i), reg, readl(reg));
#endif
				break;
			}
		}

	}
}

static unsigned int pll_get_rate(int pll)
{
	unsigned int cpxpcr = 0;
	unsigned int m, n, od0, od1;

	switch (pll) {
	case APLL:
		cpxpcr = cpm_inl(CPM_CPAPCR);
		break;
	case MPLL:
		cpxpcr = cpm_inl(CPM_CPMPCR);
		break;
	default:
		return 0;
	}

	m = (cpxpcr >> 20) & 0xfff;
	n = (cpxpcr >> 14) & 0x3f;
	od1 = (cpxpcr >> 11) & 0x7;
	od0 = (cpxpcr >> 8) & 0x7;
#ifdef CONFIG_BURNER
	return (unsigned int)((unsigned long)gd->arch.gi->extal * m / n / od0 / od1);
#else
	return (unsigned int)((unsigned long)CONFIG_SYS_EXTAL * m / n / od0 / od1);
#endif
}

static unsigned int get_ddr_rate(void)
{
	unsigned int ddrcdr  = cpm_inl(CPM_DDRCDR);

	switch ((ddrcdr >> 30) & 3) {
	case 1:
		return pll_get_rate(APLL) / ((ddrcdr & 0xf) + 1);
	case 2:
		return pll_get_rate(MPLL) / ((ddrcdr & 0xf) + 1);
	}
	return 0;
}

static unsigned int get_cclk_rate(void)
{
	unsigned int cpccr  = cpm_inl(CPM_CPCCR);

	switch ((cpccr >> 28) & 3) {
	case 1:
		return pll_get_rate(APLL) / ((cpccr & 0xf) + 1);
	case 2:
		return pll_get_rate(MPLL) / ((cpccr & 0xf) + 1);
	}
	return 0;
}

static unsigned int get_msc_rate(unsigned int xcdr)
{
	unsigned int msc0cdr  = cpm_inl(CPM_MSC0CDR);
	unsigned int mscxcdr  = cpm_inl(xcdr);
	unsigned int ret = 1;

	switch (msc0cdr >> 31) {
	case 0:
		ret = pll_get_rate(APLL) / (((mscxcdr & 0xff) + 1) * 2);
		break;
	case 1:
		ret = pll_get_rate(MPLL) / (((mscxcdr & 0xff) + 1) * 2);
		break;
	default:
		break;
	}

	return ret;
}

unsigned int cpm_get_h2clk(void)
{
	int h2clk_div;
	unsigned int cpccr  = cpm_inl(CPM_CPCCR);

	h2clk_div = (cpccr >> 12) & 0xf;

	switch ((cpccr >> 24) & 3) {
		case 1:
			return pll_get_rate(APLL) / (h2clk_div + 1);
		case 2:
			return pll_get_rate(MPLL) / (h2clk_div + 1);
	}

}

unsigned int clk_get_rate(int clk)
{
	switch (clk) {
	case DDR:
		return get_ddr_rate();
	case CPU:
		return get_cclk_rate();
	case H2CLK:
		return cpm_get_h2clk();
	case MSC0:
		return get_msc_rate(CPM_MSC0CDR);
	case MSC1:
		return get_msc_rate(CPM_MSC1CDR);
	case MSC2:
		return get_msc_rate(CPM_MSC2CDR);
	case APLL:
		return pll_get_rate(APLL);
	case MPLL:
		return pll_get_rate(MPLL);

	}

	return 0;
}

static unsigned int set_bch_rate(int clk, unsigned long rate)
{
	unsigned int pll_rate = 0;
	unsigned int bchcdr = cpm_inl(CPM_BCHCDR);
	unsigned int cdr = 0;

	switch (bchcdr >> 31) {
	case 0:
		pll_rate = pll_get_rate(APLL);
		break;
	case 1:
		pll_rate = pll_get_rate(MPLL);
		break;
	default:
		return 0;
	}

	cdr = ((pll_rate + rate - 1)/rate - 1) & 0xf;
	bchcdr &= ~(0xf | 0x3 << 27);
	bchcdr |= (cdr | (1 << 29));
	cpm_outl(bchcdr , CPM_BCHCDR);

	while (cpm_inl(CPM_BCHCDR) & (1 << 28));

	debug("CPM_BCHCDR(%x) = %x\n",CPM_BCHCDR , cpm_inl(CPM_BCHCDR));
	return 0;
}

static unsigned int set_msc_rate(int clk, unsigned long rate)
{
	unsigned int msccdr  = 0;
	unsigned int msc0cdr  = cpm_inl(CPM_MSC0CDR);
	unsigned int xcdr_addr = 0;
	unsigned int pll_rate = 0;
	unsigned int cdr = 0;

	switch (clk) {
	case MSC0:
		xcdr_addr = CPM_MSC0CDR;
		break;
	case MSC1:
		xcdr_addr = CPM_MSC1CDR;
		break;
	case MSC2:
		xcdr_addr = CPM_MSC2CDR;
		break;
	default:
		return 0;
	}

	switch (msc0cdr >> 31) {
	case 0:
		pll_rate = pll_get_rate(APLL);
		break;
	case 1:
		pll_rate = pll_get_rate(MPLL);
		break;
	default:
		return 0;
	}

	msccdr = cpm_inl(xcdr_addr);
	msccdr &= ~(0x3 << 27 | 0xff);
	cdr = (((pll_rate + rate - 1)/rate)/2 - 1)& 0xff;
	msccdr |= (cdr | (1 << 29));
	cpm_outl(msccdr , xcdr_addr);
	while (cpm_inl(xcdr_addr) & (1 << 28));

	debug("CPM_MSC%dCDR(%x) = %x\n",(clk - MSC0), xcdr_addr, cpm_inl(xcdr_addr));
	return 0;
}

static unsigned int set_ddr_rate(int clk, unsigned long rate)
{
	unsigned int ddrcdr = cpm_inl(CPM_DDRCDR);
	unsigned int pll_rate;
	unsigned int cdr;

	switch (ddrcdr >> 30) {
	case 1:
		pll_rate = pll_get_rate(APLL);
		break;
	case 2:
		pll_rate = pll_get_rate(MPLL);
		break;
	case 0:
		printf("ddr clk is stop\n");
	default:
		return 0;
	}
	cdr = ((pll_rate + rate - 1)/rate - 1) & 0xf;
	ddrcdr &= ~(0xf | 0x3f << 24);
	ddrcdr |= (cdr | (1 << 29));
	cpm_outl(ddrcdr , CPM_DDRCDR);
	while (cpm_inl(CPM_DDRCDR) & (1 << 28));
	debug("CPM_DDRCDR(%x) = %x\n",CPM_DDRCDR, cpm_inl(CPM_DDRCDR));
	return 0;
}

static unsigned int set_lcd_rate(int clk, unsigned long rate)
{
	unsigned int lcdcdr = cpm_inl(CPM_LPCDR);
	unsigned int pll_rate;
	unsigned int cdr;

	switch (lcdcdr >> 31) {
	case 0:
		pll_rate = pll_get_rate(APLL);
		break;
	case 1:
		pll_rate = pll_get_rate(MPLL);
		break;
	}

	cdr = ((pll_rate + rate - 1)/rate - 1 )& 0xff;
	lcdcdr &= ~(0xff | (0x3 << 26));
	lcdcdr |= (cdr | (1 << 28));
	cpm_outl(lcdcdr , CPM_LPCDR);
	while (cpm_inl(CPM_LPCDR) & (1 << 27));
	debug("CPM_LPCDR(%x) = %x\n",CPM_LPCDR, cpm_inl(CPM_LPCDR));
	return 0;
}

static unsigned int set_ssi_rate(int clk, unsigned long rate)
{
	unsigned int cdr;
	unsigned int pll_rate;
	unsigned int ssicdr = cpm_inl(CPM_SSICDR);

	switch (ssicdr >> 30) {
	case 0:
		pll_rate = pll_get_rate(APLL);
		break;
	case 1:
		pll_rate = pll_get_rate(MPLL);
		break;
	case 2:
		pll_rate = CONFIG_SYS_EXTAL;
		break;
	default:
		printf("set_ssi_rate is error !\n");
	}

	cdr = ((pll_rate + rate - 1)/rate - 1 )& 0xff;
	ssicdr &= ~(3 << 27 | 0xff);
	ssicdr |= ((1 << 29) | cdr);
	cpm_outl(ssicdr, CPM_SSICDR);
	while (cpm_inl(CPM_SSICDR) & (1 << 28))
		;
	debug("CPM_SSICDR(%x) = %x\n",CPM_SSICDR, cpm_inl(CPM_SSICDR));
	return 0;
}

void clk_set_rate(int clk, unsigned long rate)
{
	switch (clk) {
	case SSI:
		set_ssi_rate(clk, rate);
		return;
	case DDR:
		set_ddr_rate(clk, rate);
		return;
	case LCD:
		set_lcd_rate(clk, rate);
		return;
	case MSC0:
	case MSC1:
	case MSC2:
		set_msc_rate(clk, rate);
		return;
	case BCH:
		set_bch_rate(clk, rate);
		return;
	default:
		break;
	}

	debug("%s: clk%d is not supported\n", __func__, clk);
}

void clk_init(void)
{
	unsigned int reg_clkgr = cpm_inl(CPM_CLKGR);
	unsigned int gate = 0
#ifdef CONFIG_JZ_MMC_MSC0
		| CPM_CLKGR_MSC0
#endif
#ifdef CONFIG_JZ_MMC_MSC1
		| CPM_CLKGR_MSC1
#endif
#ifdef CONFIG_JZ_MMC_MSC2
		| CPM_CLKGR_MSC2
#endif
#ifdef CONFIG_JZ_LCD_V12
		| CPM_CLKGR_LCD
#endif
#ifdef CONFIG_NAND_NFI
		|CPM_CLKGR_PDMA
		|CPM_CLKGR_BCH
#endif
#ifdef CONFIG_SPI_FLASH_INGENIC
		| CPM_CLKGR_SSI0
#endif

#ifdef CONFIG_JZ_SSI1_PE
		| CPM_CLKGR_SSI1
#endif
		;

	reg_clkgr &= ~gate;
	cpm_outl(reg_clkgr,CPM_CLKGR);
	cgu_clks_set(cgu_clk_sel, ARRAY_SIZE(cgu_clk_sel));
}

void enable_uart_clk(void)
{
	unsigned int clkgr = cpm_inl(CPM_CLKGR);

	switch (gd->arch.gi->uart_idx) {
#define _CASE(U, N) case U: clkgr &= ~N; break
		_CASE(0, CPM_CLKGR_UART0);
		_CASE(1, CPM_CLKGR_UART1);
		_CASE(2, CPM_CLKGR_UART2);
		_CASE(3, CPM_CLKGR_UART3);
		_CASE(4, CPM_CLKGR_UART4);
	default:
		break;
	}
	cpm_outl(clkgr, CPM_CLKGR);
}

void otg_phy_init(enum otg_mode_t mode, unsigned extclk) {
#ifndef CONFIG_SPL_BUILD
	int ext_sel = 0;
	int tmp_reg = 0;
	int timeout = 0x7fffff;

	tmp_reg = cpm_inl(CPM_USBPCR1);
	tmp_reg &= ~(USBPCR1_REFCLKSEL_MSK | USBPCR1_REFCLKDIV_MSK);
	tmp_reg |= USBPCR1_REFCLKSEL_CORE | USBPCR1_WORD_IF0_16_30;
	switch (extclk/1000000) {
	case 12:
		tmp_reg |= USBPCR1_REFCLKDIV_12M;
		break;
	case 19:
		tmp_reg |= USBPCR1_REFCLKDIV_19_2M;
		break;
	case 48:
		tmp_reg |= USBPCR1_REFCLKDIV_48M;
		break;
	default:
		ext_sel = 1;
	case 24:
		tmp_reg |= USBPCR1_REFCLKDIV_24M;
		break;
	}
	cpm_outl(tmp_reg,CPM_USBPCR1);

	/*set usb cdr clk*/
	tmp_reg = cpm_inl(CPM_USBCDR);
	tmp_reg &= ~USBCDR_UCS_PLL;
	cpm_outl(tmp_reg, CPM_USBCDR);
	if (ext_sel) {
		unsigned int pll_rate = pll_get_rate(APLL);	//FIXME: default apll
		unsigned int cdr = pll_rate/24000000;
		cdr = cdr ? cdr - 1 : cdr;
		tmp_reg |= (cdr & USBCDR_USBCDR_MSK) | USBCDR_CE_USB;
		tmp_reg &= ~USBCDR_USB_STOP;
		cpm_outl(tmp_reg, CPM_USBCDR);
		while ((cpm_inl(CPM_USBCDR) & USBCDR_USB_BUSY) || timeout--);
		tmp_reg = cpm_inl(CPM_USBCDR);
		tmp_reg &= ~USBCDR_UPCS_MPLL;
		tmp_reg |= USBCDR_UCS_PLL;
		cpm_outl(tmp_reg, CPM_USBCDR);
	} else {
		tmp_reg |= USBCDR_USB_STOP;
		cpm_outl(tmp_reg, CPM_USBCDR);
		while ((cpm_inl(CPM_USBCDR) & USBCDR_USB_BUSY) || timeout--);
	}
	tmp_reg = cpm_inl(CPM_USBCDR);
	tmp_reg &= ~USBCDR_USB_DIS;
	cpm_outl(tmp_reg, CPM_USBCDR);
	if (!timeout)
		printf("USBCDR wait busy bit failed\n");

	tmp_reg = cpm_inl(CPM_USBPCR);
	switch (mode) {
	case OTG_MODE:
	case HOST_ONLY_MODE:
		tmp_reg |= USBPCR_USB_MODE_ORG;
		tmp_reg &= ~(USBPCR_VBUSVLDEXTSEL|USBPCR_VBUSVLDEXT|USBPCR_OTG_DISABLE);
		break;
	case DEVICE_ONLY_MODE:
		tmp_reg &= ~USBPCR_USB_MODE_ORG;
		tmp_reg |= USBPCR_VBUSVLDEXTSEL|USBPCR_VBUSVLDEXT|USBPCR_OTG_DISABLE;
	}
	cpm_outl(tmp_reg, CPM_USBPCR);

	tmp_reg = cpm_inl(CPM_OPCR);
	tmp_reg |= OPCR_SPENDN0;
	cpm_outl(tmp_reg, CPM_OPCR);

	tmp_reg = cpm_inl(CPM_USBPCR);
	tmp_reg |= USBPCR_POR;
	cpm_outl(tmp_reg, CPM_USBPCR);
	udelay(30);
	tmp_reg = cpm_inl(CPM_USBPCR);
	tmp_reg &= ~USBPCR_POR;
	cpm_outl(tmp_reg, CPM_USBPCR);
	udelay(300);

	tmp_reg = cpm_inl(CPM_CLKGR);
	tmp_reg &= ~CPM_CLKGR_OTG;
	cpm_outl(tmp_reg, CPM_CLKGR);
#endif
}
