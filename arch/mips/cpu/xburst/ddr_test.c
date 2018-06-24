/*
 * DDR Test Driver. The driver can test the follow cases:
 * DDR_CACHE_TEST, ADDRLINE_TEST, DATALINE_TEST, CPU_TEST,
 * DMA_TEST, CPU_DMA, REMAP_TEST
 *
 * Used by Jz4775, JZ4780, M200, and so on ...
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author:
 *		Sun Jiwei <jwsun@ingenic.cn>
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

#define DEBUG
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <ddr/ddr_common.h>
//#include <generated/ddr_reg_values.h>
#if defined(CONFIG_DDR_TEST_DMA) || defined(CONFIG_DDR_TEST_CPU_DMA)
#include <asm/arch/dma.h>
#endif
#include <asm/arch/cpm.h>

DECLARE_GLOBAL_DATA_PTR;
extern struct ddr_params *ddr_params_p;
static void restore_remap(unsigned int *remap1, unsigned int *remap2, unsigned int *remap3,
		unsigned int *remap4, unsigned int *remap5)
{
	writel(*remap1, DDRC_BASE + DDRC_REMAP(1));
	writel(*remap2, DDRC_BASE + DDRC_REMAP(2));
	writel(*remap3, DDRC_BASE + DDRC_REMAP(3));
	writel(*remap4, DDRC_BASE + DDRC_REMAP(4));
	writel(*remap5, DDRC_BASE + DDRC_REMAP(5));
}

static void set_remap(unsigned int *remap1, unsigned int *remap2, unsigned int *remap3,
		unsigned int *remap4, unsigned int *remap5)
{
	*remap1 = readl(DDRC_BASE + DDRC_REMAP(1));
	*remap2 = readl(DDRC_BASE + DDRC_REMAP(2));
	*remap3 = readl(DDRC_BASE + DDRC_REMAP(3));
	*remap4 = readl(DDRC_BASE + DDRC_REMAP(4));
	*remap5 = readl(DDRC_BASE + DDRC_REMAP(5));

	writel((3 & 0x1f) << 24 | (2 & 0x1f) << 16 | (1 & 0x1f) << 8 | (0 & 0x1f),
			DDRC_BASE + DDRC_REMAP(1));
	writel((7 & 0x1f) << 24 | (6 & 0x1f) << 16 | (5 & 0x1f) << 8 | (4 & 0x1f),
			DDRC_BASE + DDRC_REMAP(2));
	writel((11 & 0x1f) << 24 | (10 & 0x1f) << 16 | (9 & 0x1f) << 8 | (8 & 0x1f),
			DDRC_BASE + DDRC_REMAP(3));
	writel((15 & 0x1f) << 24 | (14 & 0x1f) << 16 | (13 & 0x1f) << 8 | (12 & 0x1f),
			DDRC_BASE + DDRC_REMAP(4));
	writel((19 & 0x1f) << 24 | (18 & 0x1f) << 16 | (17 & 0x1f) << 8 | (16 & 0x1f),
			DDRC_BASE + DDRC_REMAP(5));
}

#ifdef CONFIG_DDR_TEST_DATALINE
static void dataline_test(void)
{
	/* Use the KSEG1 address(), 0xa0000000 <= testaddr < 0xb0000000
	 * but the testadd must not be too little, you can use 0xaf000000,
	 * becasue of the SPL code may be in the cache
	 * */
	unsigned int testaddr1 = 0xa5000000;
	/* other address, for floating bus error */
	unsigned int testaddr2 = 0xaafffff0;
	unsigned int i = 0, src = 0;
	unsigned int remap1, remap2, remap3, remap4, remap5;
	unsigned int err_data_line = 0;

	set_remap(&remap1, &remap2, &remap3, &remap4, &remap5);

	debug("Now test DDR data line\n");
	writel(0, testaddr1);
	writel(0, testaddr2);
	for (i = 0; i < 32; i++) {
		writel(1 << i, testaddr1);
		writel(1 << i, testaddr2);
		src = readl(testaddr1);
		if (src != (1 << i)) {
			err_data_line |= (1 << i) ^ src;
			debug("Error: now test data line %d\n", i);
			debug("The error data line is 0x%x, (Err:1 ; OK:0)\n", (1 << i) ^ src);
			if (i == 31) {
#ifdef CONFIG_BURNER
				gd->arch.gi->ddr_test.data_line_test.stat = 1;
				gd->arch.gi->ddr_test.data_line_test.data_line = err_data_line;
#endif
				goto DATA_ERR;
			}
		}
	}
	debug("Now the DDR data line test OK\n");
DATA_ERR:
	restore_remap(&remap1, &remap2, &remap3, &remap4, &remap5);
}
#endif /* CONFIG_DDR_TEST_DATALINE */
#define EMC_LOW_SDRAM_SPACE_SIZE        0x10000000 /* 256M */
#ifdef CONFIG_DDR_TEST_ADDRLINE
static void addrline_test(void)
{
	unsigned int row, col, bank, bank_size;
	unsigned int testaddr_base;
	unsigned int testaddr;
	unsigned int src;
	unsigned int memsize;
	unsigned int i;
	unsigned int err_bank_line = 0, err_addr_line = 0;

	unsigned int remap1, remap2, remap3, remap4, remap5;

	set_remap(&remap1, &remap2, &remap3, &remap4, &remap5);

#ifdef CONFIG_DDR_HOST_CC
	row = DDR_ROW;
	col = DDR_COL;
	bank = (DDR_BANK8 ? 8 : 4) * (CONFIG_DDR_CS0 + CONFIG_DDR_CS1);
	memsize = (unsigned int)(DDR_CHIP_0_SIZE) + (unsigned int)(DDR_CHIP_1_SIZE);
#else /* CONFIG_DDR_HOST_CC */
	row = ddr_params_p->row;
	col = ddr_params_p->col;
	bank = (ddr_params_p->bank8 ? 8 : 4) * (ddr_params_p->cs0 + ddr_params_p->cs1);
	memsize = ddr_params_p->size.chip0 + ddr_params_p->size.chip1;
#endif /* CONFIG_DDR_HOST_CC */
	if (memsize > EMC_LOW_SDRAM_SPACE_SIZE)
		memsize = EMC_LOW_SDRAM_SPACE_SIZE;
	bank_size = memsize/bank;

	debug("Now test the band line\n");
	debug("The all bank is %d\n", bank);
	testaddr_base = KSEG1 + bank_size / 2;
	for (i = 0; i < bank; i++) {
		testaddr = testaddr_base + (i << (row + col));
		writel(testaddr, testaddr);
	}

	for (i = 0; i < bank; i++) {
		testaddr = testaddr_base + (i << (row + col));
		src = readl(testaddr);

		if (src != testaddr) {
			err_bank_line |= (src ^ testaddr);
			debug("Error: now test bank %d\n", i);
			debug("The error bank line is 0x%x, (Err:1 ; OK:0)\n",
					((src ^ testaddr) >> (row + col)));
			if ((bank - 1) == i) {
#ifdef CONFIG_BURNER
				gd->arch.gi->ddr_test.address_line_test.stat = 1;
				gd->arch.gi->ddr_test.address_line_test.bank_line = err_bank_line;
#endif
				goto ADDR_TEST;
			}
		}
	}
	debug("The bank line is OK\n");

ADDR_TEST:
	debug("Now test the address line\n");
	testaddr_base = KSEG1 + bank_size;
	for (i = 0; i < row; i++) {
		testaddr = testaddr_base + (i << col);
		writel(testaddr, testaddr);
	}

	for (i = 0; i < row; i++) {
		testaddr = testaddr_base + (i << col);
		src = readl(testaddr);

		if (src != testaddr) {
			err_addr_line |= (src ^ testaddr) >> col;
			debug("Error: now test address line %d\n", i);
			debug("The error address line is 0x%x, (Err:1 ; OK:0)\n",
					((src ^ testaddr) >> col));
			if (i == (row - 1)) {
#ifdef CONFIG_BURNER
				gd->arch.gi->ddr_test.address_line_test.stat = 1;
				gd->arch.gi->ddr_test.address_line_test.addr_line = err_addr_line;
#endif
				goto ADDR_ERR;
			}
		}
	}
	debug("The address line is OK\n");
ADDR_ERR:
	restore_remap(&remap1, &remap2, &remap3, &remap4, &remap5);
}
#endif /* CONFIG_DDR_TEST_ADDRLINE */

#ifdef CONFIG_DDR_TEST_CPU
static int test_ddr(unsigned int addr_start, unsigned int addr_end,
		unsigned int cache_flag, unsigned int with_remap)
{
	unsigned int i, src;

	for (i = addr_start; i <= addr_end; i += 4) {
		writel(i, i);
		if (cache_flag) {
			flush_dcache_all();
		}
	}

	for (i = addr_start; i <= addr_end; i += 4) {
		src = readl(i);
		if (src != i) {
			debug("Error: the want value is 0x%x \n", i);
			debug("But the real value is 0x%x \n", src);
			debug("The address is 0x%x, remap status:%d(1:unremap;0:remap)\n",
					i, with_remap);
			return 1;
		}
	}
	return 0;
}
static int dm_ddr_test()
{
	unsigned int i;
	unsigned int addr_start, offset, size;
	unsigned char src;

	offset = 0x100000;
	size   = 0x40000;

	addr_start = KSEG1 + offset;
	printf("addr_start = %x\n",addr_start);

	for (i = addr_start ; i <= addr_start + size; i++) {
		*(volatile unsigned char *)i = 0xaa;
	}
	for (i = addr_start; i <= addr_start + size; i++) {
		src = *(volatile unsigned char *)i;
		if (src != 0xaa) {
			debug("Error: ADDR:%x wanted value is 0x%x \n",i, 0xaa);
			debug("But the real value is 0x%x \n", src);
			debug("dm_ddr_test failed \n");
			return 1;
		}
	}

	for (i = addr_start ; i <= addr_start + size; i++) {
		*(volatile unsigned char *)i = (unsigned char)i;
	}
	for (i = addr_start; i <= addr_start + size; i++) {
		src = *(volatile unsigned char *)i;
		if (src != (unsigned char)i) {
			debug("Error: ADDR:%x wanted value is 0x%x \n",i, (unsigned char)i);
			debug("But the real value is 0x%x \n", src);
			debug("dm_ddr_test failed \n");
			return 1;
		}
	}
	return 0;
}

#define START_UNCACHE_ADDR	0x0
#define START_CACHE_ADDR	0x00100000
/* cache_flag:	0 - cache(KSEG0), 1 - uncache(KSEG1), 2 - uncache(KUSEG)
 * test_flag:	0 - address: 0x00100000 - 0x00200000
 *		1 - all_memory_size
 * with_remap:	0 - remap the row line and bank line
 *		1 - unremap
 */
static int ddr_test(unsigned int cache_flag, unsigned int test_flag, unsigned int with_remap)
{
	unsigned int addr_start, addr_end;
	unsigned int mem_size;
	unsigned int ret;

	unsigned int remap1, remap2, remap3, remap4, remap5;

	if (with_remap && cache_flag != 0) {
		set_remap(&remap1, &remap2, &remap3, &remap4, &remap5);
		debug("ddr test address unremap case\n");
	} else {
		debug("ddr test address remap case\n");
	}

#ifdef CONFIG_DDR_HOST_CC
	mem_size = (unsigned int)(DDR_CHIP_0_SIZE) + (unsigned int)(DDR_CHIP_1_SIZE);
#else
	mem_size = ddr_params_p->size.chip0 + ddr_params_p->size.chip1;
#endif

	switch (test_flag) {
	case 0:
		addr_start = START_CACHE_ADDR;
		addr_end = 0x00200000;
		break;
	case 1:
	default:
		addr_start = START_UNCACHE_ADDR;
		addr_end = mem_size;
		break;
	}

	if (cache_flag == 0) {
		addr_start += KSEG0;
		addr_end += KSEG0;
		if (addr_end >= 0x90000000) {
			addr_end = 0x8ffffffc;
		}
		debug("cache test the ddr ...\n");
		ret = test_ddr(addr_start, addr_end, 1, with_remap);
		if (ret == 0) {
			debug("cache test OK\n");
		} else {
			debug("cache test ERROR\n");
			goto ERR;
		}
	}

	if (cache_flag == 1) {
		addr_start += KSEG1;
		addr_end += KSEG1;
		if (addr_end >= 0xb0000000) {
			addr_end = 0xaffffffc;
		}
		debug("uncache test the ddr ...\n");
		ret = test_ddr(addr_start, addr_end, 0, with_remap);
		if (ret == 0) {
			debug("uncache test OK\n");
		} else {
			debug("uncache test ERROR\n");
			goto ERR;
		}
	}

	if (cache_flag == 2) {
		addr_start += 0x20008000;	//reserved spl zone
		debug("all memory size is 0x%x\n", mem_size);
		debug("uncache test the ddr(all size) ...\n");
		ret = test_ddr(addr_start, addr_end, 0, with_remap);
		if (ret == 0) {
			debug("all size of DDR OK\n");
		} else {
			debug("all size of DDR test ERROR\n");
			goto ERR;
		}
	}

	if (with_remap && cache_flag != 0) {
		restore_remap(&remap1, &remap2, &remap3, &remap4, &remap5);
		debug("ddr test address unremap case OK\n");
	} else {
		debug("ddr test address remap case OK\n");
	}

	return 0;

ERR:
#ifdef CONFIG_BURNER
	gd->arch.gi->ddr_test.cache_test.stat = 1;
	gd->arch.gi->ddr_test.cache_test.remap = with_remap;
#endif
	if (with_remap && cache_flag != 0) {
		restore_remap(&remap1, &remap2, &remap3, &remap4, &remap5);
	}
	return 1;
}
#endif /* CONFIG_DDR_TEST_CPU */

#define EMC_LOW_SDRAM_SPACE_SIZE        0x10000000 /* 256M */
#define DDR_DMA_BASE  (0xa0000000)              /*un-cached*/

#ifdef CONFIG_DDR_TEST_DMA
static int dma_check_result(void *src, void *dst, int size, int test_cnt)
{
	unsigned int addr1, addr2, i, err = 0;// count = 0;
	unsigned int data_expect,dsrc,ddst;

	addr1 = (unsigned int)src;
	addr2 = (unsigned int)dst;
	debug("check addr: 0x%x\n", addr1);

	for (i = 0; i < size; i += 4) {
		data_expect = addr1;
		dsrc = readl(addr1);
		ddst = readl(addr2);
		if ((dsrc != data_expect) || (ddst != data_expect)) {
			debug("\nDMA SRC ADDR: 0x%x\n", addr1);
			debug("DMA DST ADDR: 0x%x\n", addr2);
			debug("expect data: 0x%x\n", data_expect);
			debug("src data: 0x%x\n", dsrc);
			debug("dst data: 0x%x\n", ddst);

			err = 1;
			break;
		}

		addr1 += 4;
		addr2 += 4;
	}

	return err;
}

static void dma_nodesc_test(unsigned int dma_chan, unsigned int dma_src_addr, unsigned int dma_dst_addr, unsigned int size)
{
	unsigned int dma_src_phys_addr, dma_dst_phys_addr;
	unsigned int dma_chan_base = PDMA_BASE + dma_chan * 0x20;


	/* Allocate DMA buffers */
	dma_src_phys_addr = dma_src_addr & ~0xa0000000;
	dma_dst_phys_addr = dma_dst_addr & ~0xa0000000;

	/* Init DMA module */

	writel(0, dma_chan_base + CH_DCS);
	writel(DMAC_DRSR_RS_AUTO, dma_chan_base + CH_DRT);
	writel(dma_src_phys_addr, dma_chan_base + CH_DSA);
	writel(dma_dst_phys_addr, dma_chan_base + CH_DTA);
	writel(size / 32, dma_chan_base + CH_DTC);
	writel(DMAC_DCMD_SAI | DMAC_DCMD_DAI | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_32 |
			DMAC_DCMD_DS_32BYTE | DMAC_DCMD_TIE, dma_chan_base + CH_DCM);
	writel(DMAC_DCCSR_NDES | DMAC_DCCSR_EN, dma_chan_base + CH_DCS);

}

static int ddr_dma_test(void)
{
	int i, err = 0, banks;
	int times, test_cnt;
	unsigned int dma_chan;
	unsigned int addr, ddr_dma0_src, ddr_dma0_dst;
	unsigned int memsize, banksize, testsize;
	unsigned int tmp;

	debug("ddr_dma_test...\n");
	tmp = readl(CPM_CLKGR + CPM_BASE);
	tmp &= ~CPM_CLKGR_PDMA;
	writel(tmp, CPM_CLKGR + CPM_BASE); //Open the PDMA clock

#ifdef CONFIG_DDR_HOST_CC
	banks = (DDR_BANK8 ? 8 : 4) * (CONFIG_DDR_CS0 + CONFIG_DDR_CS1);
	memsize = (unsigned int)(DDR_CHIP_0_SIZE) + (unsigned int)(DDR_CHIP_1_SIZE);
#else /* CONFIG_DDR_HOST_CC */
	banks = (ddr_params_p->bank8 ? 8 : 4) * (ddr_params_p->cs0 + ddr_params_p->cs1);
	memsize = ddr_params_p->size.chip0 + ddr_params_p->size.chip1;
#endif /* CONFIG_DDR_HOST_CC */
	if (memsize > EMC_LOW_SDRAM_SPACE_SIZE)
		memsize = EMC_LOW_SDRAM_SPACE_SIZE;
	banksize = memsize/banks;
	testsize = banksize / 2;

	for(test_cnt = 0; test_cnt < 1; test_cnt++) {
		for(times = 0; times < banks; times++) {
			ddr_dma0_src = DDR_DMA_BASE + banksize * times;
			ddr_dma0_dst = DDR_DMA_BASE + banksize * (times + 1) - testsize;
			debug("dma src addr: 0x%x\n", ddr_dma0_src);
			debug("dma dst addr: 0x%x\n", ddr_dma0_dst);
			debug("test bank: %d\n", times);

			addr = ddr_dma0_src;

			for (i = 0; i < testsize; i += 4) {
				*(volatile unsigned int *)(addr + i) = (addr + i);
			}

			dma_chan = 3;

			writel(0, PDMA_BASE + DMAC);

			writel(0, PDMA_BASE + DMACP);

			/* Init target buffer */
			memset((void *)ddr_dma0_dst, 0, testsize);
			dma_nodesc_test(dma_chan, ddr_dma0_src, ddr_dma0_dst, testsize);

			writel(1, PDMA_BASE + DMAC);

			while(!(readl(PDMA_BASE + dma_chan * 0x20 + CH_DCS) & 0x8));

			err = dma_check_result((void *)ddr_dma0_src,
					(void *)ddr_dma0_dst, testsize, test_cnt);

			//REG_DMAC_DCCSR(0) &= ~DMAC_DCCSR_EN;  /* disable DMA */
			writel(0, PDMA_BASE + DMAC);
			writel(0, PDMA_BASE + dma_chan * 0x20 + CH_DCS);
			writel(0, PDMA_BASE + dma_chan * 0x20 + CH_DCM);
			writel(0, PDMA_BASE + dma_chan * 0x20 + CH_DRT);

			if (err != 0) {
#ifdef CONFIG_BURNER
				gd->arch.gi->ddr_test.dma_test.stat = 1;
#endif
				return err;
			}
		}
	}
	return err;

}
#endif /* CONFIG_DDR_TEST_DMA */

#ifdef CONFIG_DDR_TEST_CPU_DMA
typedef struct {
	volatile u32 dcmd;      /* DCMD value for the current transfer */
	volatile u32 dsadr;     /* DSAR value for the current transfer */
	volatile u32 dtadr;     /* DTAR value for the current transfer */
	volatile u32 ddadr;     /* Points to the next descriptor + transfer count */
	volatile u32 dstrd;     /* DMA source and target stride address */
	volatile u32 dreqt;     /* DMA request type for current transfer */
	volatile u32 reserved0; /* Reserved */
	volatile u32 reserved1; /* Reserved */
} jz_dma_desc;

void start_dma(unsigned int size)
{
	int i;
	unsigned int tmp;
	unsigned int dma_chan = 3;

	debug("start dma...\n");
	debug("size is 0x%x\n", size);

	tmp = readl(CPM_CLKGR + CPM_BASE);
	tmp &= ~CPM_CLKGR_PDMA;
	writel(tmp, CPM_CLKGR + CPM_BASE); //Open the PDMA clock

	for (i = 0; i < size / 2; i += 4)
		writel(0x20000000 + i, 0x20000000 + i);

	/* at mcu */
	jz_dma_desc *desc = (jz_dma_desc *)0xb3422000;
	jz_dma_desc *next = desc;

	writel(0, PDMA_BASE + DMAC);
	writel(0, PDMA_BASE + DMACP);
	writel(0, PDMA_BASE + DDRS);

	writel(0, PDMA_BASE + dma_chan * 0x20 + CH_DDA);
	writel(0, PDMA_BASE + dma_chan * 0x20 + CH_DCS);
	writel(DMAC_DCCSR_DES8, PDMA_BASE + dma_chan * 0x20 + CH_DCS);
	tmp = readl(PDMA_BASE + dma_chan * 0x20 + CH_DCS);
	tmp &= ~DMAC_DCCSR_NDES;
	writel(tmp, PDMA_BASE + dma_chan * 0x20 + CH_DCS);
	writel(DMAC_DRSR_RS_AUTO, PDMA_BASE + dma_chan * 0x20 + CH_DRT);
	writel(0, PDMA_BASE + dma_chan * 0x20 + CH_DTC);
	writel(DMAC_DCMD_LINK, PDMA_BASE + dma_chan * 0x20 + CH_DCM);

	desc->dcmd = DMAC_DCMD_SAI | DMAC_DCMD_DAI | DMAC_DCMD_RDIL_IGN | DMAC_DCMD_SWDH_32 |
		DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS_128BYTE | DMAC_DCMD_LINK | DMAC_DCMD_TIE;
	desc->dsadr = 0x20000000;
	desc->dtadr = 0x20000000 + size / 2;
	desc->dreqt = 0x8;
	desc->ddadr = (((unsigned int)next >> 4) << 24) | (size / 256);

//	flush_dcache_all();

	writel((unsigned int)desc & 0x1fffffff, PDMA_BASE + dma_chan * 0x20 + CH_DDA);

	writel(1 << 3, PDMA_BASE + DDRS);
	tmp = readl(PDMA_BASE + DMAC);
	tmp |= 1;
	writel(tmp, PDMA_BASE + DMAC);

	tmp = readl(PDMA_BASE + dma_chan * 0x20 + CH_DCS);
	tmp |= 1;
	writel(tmp, PDMA_BASE + dma_chan * 0x20 + CH_DCS);
}

static int cpu_and_dma(void)
{
	int i;
	unsigned int memsize;
	unsigned int cnt = 0;
	unsigned int cpu_addr = 0;
	unsigned int a = 5;

	debug("cpu_and_dma ...\n");
#ifdef CONFIG_DDR_HOST_CC
	memsize = (unsigned int)(DDR_CHIP_0_SIZE) + (unsigned int)(DDR_CHIP_1_SIZE);
#else /* CONFIG_DDR_HOST_CC */
	memsize = ddr_params_p->size.chip0 + ddr_params_p->size.chip1;
#endif /* CONFIG_DDR_HOST_CC */

	debug("size is 0x%x\n", memsize);

	memset((void *)0x20100000, 0, (memsize - 0x100000));

	start_dma(memsize / 2);

	cpu_addr = 0x20000000 + memsize / 2;
	while (a--) {
		debug("cpu access...\n");
		for (i = 0; i < (memsize / 2); i += 4) {
			writel(cpu_addr + i, cpu_addr + i);
		}

		for (i = 0; i < (memsize / 2); i += 4) {
			if (readl(cpu_addr + i) != cpu_addr + i) {
#ifdef CONFIG_BURNER
				gd->arch.gi->ddr_test.cpu_dma_test.stat = 1;
#endif
				debug("Error: addr 0x%x, value 0x%x\n", cpu_addr + i, cpu_addr + i);
				cnt++;
				if (cnt == 10)
					return -1;
			}
		}
	}

	return 0;
}
#endif /* CONFIG_DDR_TEST_CPU_DMA */

#if 0
#ifdef CONFIG_DDR_TEST_REMAP
/* FIXME */
static int memory_post_remap_test(void)
{
	unsigned int i;

	for (i = 0; i < 10; i++) {
		set_remap();

		writel(0x123456, 0xa0000000 | (1 << (i + 12)));
		writel(0x778899aa, 0xa0000000 | (1 << (i + 1 + 12)));
		writel(0xbbccddee, 0xa0000000 | (1 << (i + 1 + 12)) | (1 << (i + 12)));

		remap_swap(i, 10);
		remap_swap(i + 1, 11);
		debug("===> %d<=>10 %d<=>11 :", i, i + 1);
		if (readl(0xa0400000) != 0x123456)
			debug ("\tERROR: should: 0x123456, act: 0x%x, src: 0x%x\n",
					readl(0xa0400000), readl(0xa0000000 | (1 << (i + 12))));
		if (readl(0xa0800000) != 0x778899aa)
			debug ("\tERROR: should: 0x778899aa, act: 0x%x, src: 0x%x\n",
					readl(0xa0800000), readl(0xa0000000 | (1 << (i + 1 + 12))));
		if (readl(0xa0c00000) != 0xbbccddee)
			debug ("\tERROR: should: 0xbbccddee, act: 0x%x, src: 0x%x\n",
					readl(0xa0800000),
					readl(0xa0000000 | (1 << (i + 12)) | (1 << (i + 1 + 12))));
		if (ddr_dma_test() == 0)
			debug("remap ok!\n");
		else
			debug("remap fail!\n");
		debug ("done\n");
	}

	set_remap();
}
#endif /* CONFIG_DDR_TEST_REMAP */

#endif
#if 0
#ifdef CONFIG_DDR_TEST_CACHE
#define CACHE_STARTDDR	0x8c000000
#define CACHE_ENDDDR	0x8ffffffc

static void dcache_sync(void)
{
	unsigned int i, value;
	debug("Now test data is transfered between cache and DDR\n");

	for (i = 0; (CACHE_STARTDDR + i) < CACHE_ENDDDR; i += 4) {
		writel(i, CACHE_STARTDDR + i);
		flush_cache_all();
	}

	for (i = 0; (CACHE_STARTDDR + i) < CACHE_ENDDDR; i += 4) {
		value = readl(CACHE_STARTDDR + i);
		if (value != i) {
			debug("Now a error is found, please check the DDR or flush_cache_all()\n");
			debug("The right value is 0x%x\n", i);
			debug("But now the value is 0x%x\n", value);
			hang();
		}
	}
	debug("Now the test case is OK\n");
}
#endif /* CONFIG_DDR_TEST_CACHE */
#endif

#ifdef CONFIG_DDR_TEST
void ddr_basic_tests(void)
{
	debug("Now test the DDR\n");

#ifdef CONFIG_DDR_TEST_CPU
#ifdef CONFIG_BURNER
	if (gd->arch.gi->ddr_test.uncache_test.enable)
		ddr_test(1, 0, 1);
#else
	ddr_test(1, 0, 1);
#endif
	dm_ddr_test();
#endif
	/* The dataline test must run before all test */
#ifdef CONFIG_DDR_TEST_DATALINE
#ifdef CONFIG_BURNER
	if (gd->arch.gi->ddr_test.data_line_test.enable)
		dataline_test();
#else
	dataline_test();
#endif
#endif /* CONFIG_DDR_TEST_DATALINE */

#ifdef CONFIG_DDR_TEST_ADDRLINE
#ifdef CONFIG_BURNER
	if (gd->arch.gi->ddr_test.address_line_test.enable)
		addrline_test();
#else
	addrline_test();
#endif
#endif /* CONFIG_DDR_TEST_ADDRLINE */

#ifdef CONFIG_DDR_TEST_CPU
#ifdef CONFIG_BURNER
	if (gd->arch.gi->ddr_test.uncache_all_test.enable)
		/* First test the address unremap case, then test the remap case */
		ddr_test(2, 1, 1);
	if (gd->arch.gi->ddr_test.cache_test.enable)
		ddr_test(0, 0, 0);
	if (gd->arch.gi->ddr_test.uncache_test.enable)
		ddr_test(1, 0, 0);
	if (gd->arch.gi->ddr_test.uncache_all_test.enable)
		ddr_test(2, 1, 0);
#else
	/* First test the address unremap case, then test the remap case */
	ddr_test(2, 1, 1);
	ddr_test(0, 0, 0);
	ddr_test(1, 0, 0);
	ddr_test(2, 1, 0);
#endif
#endif /* CONFIG_DDR_TEST_CPU */

#ifdef CONFIG_DDR_TEST_DMA
#ifdef CONFIG_BURNER
	if (gd->arch.gi->ddr_test.dma_test.enable)
		ddr_dma_test();
#else
	ddr_dma_test();
#endif
#endif /* CONFIG_DDR_TEST_DMA */

#ifdef CONFIG_DDR_TEST_CPU_DMA
#ifdef CONFIG_BURNER
	if (gd->arch.gi->ddr_test.cpu_dma_test.enable)
		cpu_and_dma();
#else
	cpu_and_dma();
#endif
#endif /* CONFIG_DDR_TEST_CPU_DMA */

#if 0
#ifdef CONFIG_DDR_TEST_REMAP
#ifdef CONFIG_BURNER
	if (gd->arch.gi->ddr_test.uncache_test.enable)
		memory_post_remap_test();
#else
	memory_post_remap_test();
#endif
#endif /* CONFIG_DDR_TEST_REMAP */
#endif

	debug("Now the DDR test over\n");
}
#endif
