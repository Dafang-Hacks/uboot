#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#include <generated/ddr_reg_values.h>

#include <asm/io.h>
#include <asm/arch/clk.h>


static unsigned int * test_data;
static unsigned int * test_data_uncache;

void dump_reg()
{
	printf("DDRC_DLP: %x\n",ddr_readl(DDRC_DLP));
	printf("DDRP_DSGCR: %x\n",ddr_readl(DDRP_DSGCR));
	printf("DDRP_ACDLLCR: %x\n",ddr_readl(DDRP_ACDLLCR));

	printf("DDRP_DX0DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(0)));
	printf("DDRP_DX1DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(1)));
	printf("DDRP_DX2DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(2)));
	printf("DDRP_DX3DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(3)));
}
#define TEST_SIZE (4 * 1024 * 1024)
void ddr_selfresh_test()
{
	unsigned int val;
	unsigned int run = 1;
	test_data = 0x80600000;
	test_data_uncache = (unsigned int *)((unsigned int)test_data | 0xa0000000);
	printf("%s %s",__FILE__,__DATE__);
	for(val = 0; val < TEST_SIZE / 4;val++)
		test_data_uncache[val] = (unsigned int)&test_data_uncache[val];
	dump_reg();
//-----------------------------------------
	ddr_writel(0xf003,DDRC_DLP);
	dump_reg();
//-----------------------------------------
/*
 *   ddr selrefret ok;
 */
	/* val = ddr_readl(DDRP_DSGCR); */
	/* val &= ~(1 << 4); */
	/* ddr_writel(val,DDRP_DSGCR); */
	/* dump_reg(); */
//-----------------------------------------
	while(run){
		val = ddr_readl(DDRC_CTRL);
		val |= 1 << 5;
		ddr_writel(val, DDRC_CTRL);
		printf("ddr self-refresh\n");
		mdelay(1000);


//---------------------------------------------------
		/* val = ddr_readl(DDRP_ACDLLCR); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_ACDLLCR); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_ACDLLCR); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_ACDLLCR); */
		/* mdelay(200); */

		/* val = ddr_readl(DDRP_DXDLLCR(0)); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(0)); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_DXDLLCR(0)); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(0)); */
		/* mdelay(1); */


		/* val = ddr_readl(DDRP_DXDLLCR(1)); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(1)); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_DXDLLCR(1)); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(1)); */
		/* mdelay(1); */


		/* val = ddr_readl(DDRP_DXDLLCR(2)); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(2)); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_DXDLLCR(2)); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(2)); */
		/* mdelay(1); */


		/* val = ddr_readl(DDRP_DXDLLCR(3)); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(3)); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_DXDLLCR(3)); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(3)); */
		/* mdelay(200); */

		/* dump_reg(); */
//---------------------------------------------------

		printf("1\n");
		val = ddr_readl(DDRC_CTRL);
		val &= ~(1 << 5);
		ddr_writel(val, DDRC_CTRL);
		printf("2\n");
		mdelay(10);
		*(volatile unsigned int *)0xb301102c |= (1 << 4);
		mdelay(1);


//-----------------------------------------
		/* *(volatile unsigned int *)(0xb3011000 + 0x80) |= (1 << 22) | 1; */
		/* printf("AAAA DDRC_PHYRST:%x\n",*(volatile unsigned int *)(0xb3011000 + 0x80)); */
		/* mdelay(10); */
		/* *(volatile unsigned int *)(0xb3011000 + 0x80) &= ~((1 << 22)  | 1); */
		/* mdelay(10); */
		/* printf("DDRC_PHYRST:%x\n",*(volatile unsigned int *)(0xb3011000 + 0x80)); */
//-----------------------------------------

		printf("3\n");

		for(val = 0; val < TEST_SIZE/ 4;val++) {
			if(test_data_uncache[val] != (unsigned int)&test_data_uncache[val])
			{
				printf("%d: d=%x  e=%x\n",val,test_data_uncache[val],(unsigned int)&test_data_uncache[val]);
				run = 1;
			}
		}
		printf("ddddddddddddddddddddddddddd============\n");
		for(val = 0; val < TEST_SIZE/ 4;val++) {
			if(test_data_uncache[val] != (unsigned int)&test_data_uncache[val])
			{
				printf("%d: d=%x  e=%x\n",val,test_data_uncache[val],(unsigned int)&test_data_uncache[val]);
				run = 1;
			}
		}
		/* for(val = 0; val < TEST_SIZE / 4;val++) */
		/* 	test_data_uncache[val] = (unsigned int)&test_data_uncache[val]; */
		printf("ddr test finish!\n");
	}
	while(1);
}
