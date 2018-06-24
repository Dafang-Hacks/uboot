#include "jzdma.h"
#include <clib.h>

static int firmware[] = {
#ifdef CONFIG_NAND_NFI
	#include "firmware_nfi.hex"
#else
	#include "firmware_nemc.hex"
#endif
};

static inline void jzdma_load_firmware(void)
{
	memcpy((void *)(PDMA_IOBASE + TCSM), firmware, sizeof(firmware));
}

static void set_programmable_channel(int program)
{
	writel(program, PDMA_IOBASE + DMACP);
}

static inline void set_gpioa_intc_by_mcu(void)
{
	unsigned int mask;

	/*mask gpioa interrupt for cpu*/
	mask = 1 << GPIOA_MAST_BIT;
	writel(mask, INTC_IOBASE + ICMSR0);

	/*unmask gpioa interrupt for mcu*/
	mask = readl(INTC_IOBASE + DMR0);
	mask &= ~(1 << GPIOA_MAST_BIT);
	writel(mask, INTC_IOBASE + DMR0);

}

static inline void jzdma_mcu_reset(void)
{
	unsigned int dmcs;
	dmcs = readl(PDMA_IOBASE + DMCS);
	dmcs |= 0x1;
	writel(dmcs, PDMA_IOBASE + DMCS);
}

static inline void jzdma_mcu_init(void)
{
	unsigned int dmcs;
	dmcs = readl(PDMA_IOBASE + DMCS);
	dmcs &= ~0x1;
	writel(dmcs, PDMA_IOBASE + DMCS);
}

static unsigned int get_channel_address(unsigned short channel, unsigned int offset)
{
	return PDMA_IOBASE + offset + channel * 0x20;
}

void jzdma_clear_status(unsigned short channel)
{
	unsigned int regaddr;
	int tmp;

	/*clear channel n, DCS:HAL,TT,AR,CTE*/
	regaddr = get_channel_address(channel, DCS_N);
	tmp = readl(regaddr);
	tmp &= ~((0x7 << DCS_HAL_BIT) + 0x1);
	writel(tmp, regaddr);
}
void clear_pdmam(int icsr)
{
	int dmint;
	if((icsr >> INTC_PDMAM_BIT) & 0x1){
		dmint = readl(PDMA_IOBASE + DMINT);
		/*check the mailbox if is normal mailbox*/
		if((dmint >> NORMAL_MAILBOX_BIT) & 0x1){
			/*clear DMINT.N_IP  bit*/
			dmint &= ~(0x1 << NORMAL_MAILBOX_BIT);
			writel(dmint, PDMA_IOBASE + DMINT);
			if((readl(PDMA_IOBASE + DMNMB) >> 16) == MCU_MSG_INTC){
				//printf("%s clear gpioa[%x] intterupt\n",__func__,*(volatile unsigned int *)0xb0010050);
				*(volatile unsigned int *)0xb0010058 = 0xffffffff;
			}
		}
	}

}
int jzdma_prep_memcpy(unsigned short channel, dma_addr_t srcaddr, dma_addr_t dstaddr, unsigned int len)
{
	unsigned int regaddr;
	int dataunit = 4;
	int icsr, tmp;
	//printf(" %s start! srcaddr= %x dstaddr= %x len= %d\n",__func__,srcaddr,dstaddr,len);

	icsr = readl(INTC_IOBASE + ICSR); /*pdmam*/
	clear_pdmam(icsr);
        /*choose no-descriptor, clear HAL,TT,AR*/
	regaddr = get_channel_address(channel, DCS_N);
	tmp = readl(regaddr);
	tmp &= ~(0xffffff << DCS_MASK_24BIT);
	tmp |= 0x1 << DCS_NDES_BIT;
	writel(tmp, regaddr);
	//printf("DCS: %x ----> %x\n",regaddr,tmp);

	/*clear channel n, DTC*/
//	regaddr = get_channel_address(channel, DTC_N);
//	writel(0, regaddr);
	//printf("DTC0: %x ----> %x\n",regaddr,*(volatile unsigned int*)regaddr);

	/*fill source address*/
	regaddr = get_channel_address(channel, DSA_N);
	writel(srcaddr, regaddr);
	//printf("DSA: %x ----> %x\n",regaddr,*(volatile unsigned int*)regaddr);

	/*fill target address*/
	regaddr = get_channel_address(channel, DTA_N);
	writel(dstaddr, regaddr);
	//printf("DTA: %x ----> %x\n",regaddr,*(volatile unsigned int*)regaddr);

	/*fill channel command*/
	regaddr = get_channel_address(channel, DCM_N);
	tmp = readl(regaddr);
	tmp &= ~((0x7 << TSZ_MASK_3BIT) + (0xff << DP_SP_RDIL_MASK_8BIT) + (0x3 << DAI_SAI_MASK_2BIT));
	tmp |= ((0x1 << DCM_TIE_BIT) + (0x0 << DCM_TSZ_BIT) +
		(0x1 << DCM_DAI_BIT) +(0x1 << DCM_SAI_BIT));
	tmp &= ~((0x3 << DCM_DP_BIT) + (0x3 << DCM_SP_BIT));
	//tmp |= 7 << DCM_RDIL_BIT; //dataunit = 4;
	writel(tmp, regaddr);
	//printf("DCA: %x ----> %x\n",regaddr,tmp);

	/*fill channel DTC*/
	regaddr = get_channel_address(channel, DTC_N);
	tmp = (len + dataunit - 1) / dataunit;
	if(tmp == 0){
		printf("ERROR:invalid DTC!!!!!!!!!!!!!!!! len=%d  dataunit=%d\n",len,dataunit);
		while(1);
	}
	writel(tmp, regaddr);
	//printf("DTC: %x ----> %x\n",regaddr,tmp);

	/*fill channel DRT*/
	regaddr = get_channel_address(channel, DRT_N);
	tmp = PDMA_AUTO_REQUEST;
	writel(tmp, regaddr);
	//printf("DRT: %x ----> %x\n",regaddr,tmp);

	//printf("%s ok!\n",__func__);
	return 0;
}

int jzdma_start_memcpy(unsigned short channel)
{
	unsigned int regaddr;
	int tmp;

	/*fill DCSn.CTE = 1*/
	regaddr = get_channel_address(channel, DCS_N);
	tmp = readl(regaddr);
	tmp |= 0x1;
	writel(tmp, regaddr);

	return 0;
}

int jzdma_init()
{
	int pdma_program = 0;
	int i, tmpreg;

	/*enable pdma clock*/
	tmpreg = readl(CPM_CLKGR);
	tmpreg &= ~(0x1 << PDMA_CLKGATE_BIT);
	writel(tmpreg,CPM_CLKGR);
	/*enable DMAC.DMAE*/
	writel(1, PDMA_IOBASE + DMAC);
	/* the corresponding dma channel(0,1,2,3,4) is set programmable */
	for(i = 0; i< 5; i++)
		pdma_program |= 1<<i;
	set_programmable_channel(pdma_program);
	/*set the gpioa interrupt take over by mcu */
	set_gpioa_intc_by_mcu();
	jzdma_mcu_reset();
	jzdma_load_firmware();
	jzdma_mcu_init();
	for(i = 0; i < 8; i++){
		*(((volatile unsigned long long *)MCU_TEST_DATA_DMA)+i) = 0;
	}
	printf("\n %s init ok!\n",__func__);
	return 0;
}
