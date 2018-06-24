/*
 * SPL params fixer.
 * It's used for speed up M200 bootrom performance.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
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

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define FPGA

#define SEL_SCLKA		1
#define SEL_CPU			1
#define SEL_H0			1
#define SEL_H2			1
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
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

#ifndef FPGA
#define CONFIG_BOOTROM_PLLFREQ		576000000
#define CONFIG_BOOTROM_CPMCPCCR		CPCCR_CFG
#else
#define CONFIG_BOOTROM_PLLFREQ		0
#define CONFIG_BOOTROM_CPMCPCCR		0
#endif

struct desc {
	unsigned set_addr:16;
	unsigned poll_addr:16;
	unsigned value:32;
	unsigned poll_h_mask:32;
	unsigned poll_l_mask:32;
};

typedef union reg_cpccr {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned CDIV:4;
		unsigned L2CDIV:4;
		unsigned H0DIV:4;
		unsigned H2DIV:4;
		unsigned PDIV:4;
		unsigned CE_AHB2:1;
		unsigned CE_AHB0:1;
		unsigned CE_CPU:1;
		unsigned GATE_SCLKA:1;
		unsigned SEL_H2PLL:2;
		unsigned SEL_H0PLL:2;
		unsigned SEL_CPLL:2;
		unsigned SEL_SRC:2;
	} b;
} reg_cpccr_t;

typedef union nand_timing {
	/** raw register data */
	uint32_t nand_timing[4];
	/** register bits */
	struct {
		unsigned set_rw:8;
		unsigned wait_rw:8;
		unsigned hold_rw:8;
		unsigned set_cs:8;
		unsigned wait_cs:8;
		unsigned trr:8;
		unsigned tedo:8;
		unsigned trpre:8;
		unsigned twpre:8;
		unsigned tds:8;
		unsigned tdh:8;
		unsigned twpst:8;
		unsigned tdqsre:8;
		unsigned trhw:8;
		unsigned t1:8;
		unsigned t2:8;
	} b;
} nand_timing_t;

struct params {
	unsigned int id;
	unsigned int length;
	unsigned int pll_freq;
	reg_cpccr_t cpccr;
	nand_timing_t nand_timing;
	struct desc cpm_desc[0];
};

struct desc descriptors[14] = {
	/*
	 * saddr,	paddr,		value,		poll_h_mask,	poll_l_mask
	 */
#ifndef FPGA
	{0x20,		0xffff,		0x1fffffb4,	0,		0},		/* gate clk */
	{0x10,		0x10,		0x03004a01,	0x8,		0},		/* conf APLL */
	{0,		0xd4,		0x95773310,	0,		0x7},		/* conf DIV */
	{0,		0xffff,		0x55073310,	0,		0},		/* conf select */
#ifdef CONFIG_SPL_MMC_SUPPORT
#if (CONFIG_JZ_MMC_SPLMSC == 0)
	{0x68,		0x68,		0x20000017,	0,		0x10000000},	/* conf MSC0CDR */
#elif (CONFIG_JZ_MMC_SPLMSC == 1)
	{0xa4,		0xa4,		0x20000017,	0,		0x10000000},	/* conf MSC1CDR */
#endif /* CONFIG_JZ_MMC_SPLMSC == 1 */
#endif /* CONFIG_SPL_MMC_SUPPORT */
#ifdef CONFIG_SPL_NAND_SUPPORT
	{0xac,		0xac,		0x20000003,	0,		0x10000000},	/* conf BCHCDR */
#endif /* CONFIG_SPL_NAND_SUPPORT */
	{0x20,		0xffff,		0x1fffff80,	0,		0},		/* ungate clk */
#endif /* FPGA */
	{0xffff,	0xffff, 	0,		0,		0},
};

void dump_params(struct params *p)
{
	int i;

	printf("SPL Params Fixer:\n");
	printf("id:\t\t0x%08X (%c%c%c%c)\n", p->id,
	       ((char *)(&p->id))[0],
	       ((char *)(&p->id))[1],
	       ((char *)(&p->id))[2],
	       ((char *)(&p->id))[3]);
	printf("length:\t\t%u\n", p->length);
	printf("pll_freq:\t%u\n", p->pll_freq);
	printf("CPM_CPCCR:\t0x%08X\n", p->cpccr.d32);

	for (i = 0; i < 4; i++)
		printf("nand_timing[%d]:\t0x%08X\n", i, p->nand_timing.nand_timing[i]);

	printf("descriptors:\n");
	for (i = 0; i < 14; i++) {
		struct desc *desc = &p->cpm_desc[i];

		if ((desc->set_addr == 0xffff) && (desc->poll_addr = 0xffff))
			break;

		printf("NO.%d:\n", i);
		printf("\tsaddr = 0x%04X\n", desc->set_addr);
		printf("\tsaddr = 0x%04X\n", desc->poll_addr);
		printf("\tpaddr = 0x%08X\n", desc->value);
		printf("\tpoll_h_mask = 0x%08X\n", desc->poll_h_mask);
		printf("\tpoll_l_mask = 0x%08X\n", desc->poll_l_mask);
	}
}

int main(int argc, char *argv[])
{
	int fd, i, offset, params_length;
	char *spl_path, *fix_file;
	unsigned int spl_length = 0;
	struct params *params;
	char valid_id[4] = {'I', 'N', 'G', 'E'};
	struct desc *desc;
	unsigned int *p;

	if (argc != 5) {
		printf("Usage: %s fix_file spl_path offset params_length\n",argv[0]);
		return 1;
	}

	fix_file = argv[1];
	spl_path = argv[2];
	offset = atoi(argv[3]);
	params_length = atoi(argv[4]);

	printf("fix_file:%s spl_path:%s offset:%d params_length=%d\n",
	       fix_file, spl_path, offset, params_length);

	params = (struct params *)malloc(params_length);

	p = (unsigned int *)valid_id;
	params->id = *p;

	fd = open(spl_path,O_RDONLY);
	if (fd < 0) {
		printf("open %s Error\n", spl_path);
		return -1;
	}
	spl_length = lseek(fd, 0, SEEK_END);
	close(fd);
	params->length = (spl_length & 0x1ff) == 0
		? spl_length
		: (spl_length & ~0x1ff) + 0x200;

	params->pll_freq = CONFIG_BOOTROM_PLLFREQ;
	params->cpccr.d32 = CONFIG_BOOTROM_CPMCPCCR;

	params->nand_timing.b.set_rw = 3;
	params->nand_timing.b.wait_rw = 14;
	params->nand_timing.b.hold_rw = 6;
	params->nand_timing.b.set_cs = 20;
	params->nand_timing.b.wait_cs = 6;
	params->nand_timing.b.trr = 12;
	params->nand_timing.b.tedo = 15;
	params->nand_timing.b.trpre = 0;
	params->nand_timing.b.twpre = 0;
	params->nand_timing.b.tds = 0;
	params->nand_timing.b.tdh = 0;
	params->nand_timing.b.twpst = 0;
	params->nand_timing.b.tdqsre = 0;
	params->nand_timing.b.trhw = 30;
	params->nand_timing.b.t1 = 0;
	params->nand_timing.b.t2 = 0;

	desc = params->cpm_desc;

	for (i = 0; i < 14; i++) {
		memcpy(&desc[i], &descriptors[i], sizeof(struct desc));
	}
	dump_params(params);

	fd = open(fix_file, O_RDWR);
	if (fd < 0) {
		printf("open %s Error\n", fix_file);
		return -1;
	}

	i = lseek(fd, offset, SEEK_SET);
	if (i != offset) {
		printf("lseek to %d Error\n", offset);
		return -1;
	}

	if (write(fd, params, params_length) != params_length) {
		printf("write %s Error\n", spl_path);
		return -1;
	}

	close(fd);

	return 0;
}
