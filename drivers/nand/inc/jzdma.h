#ifndef _JZDMA_H_
#define _JZDMA_H_

#include "dma_msg_handler.h"

#define INTC_IOBASE     0xB0000000
#define PDMA_IOBASE	0xB3420000

#define TCSM	0x2000

#define DMAC	0x1000
#define DIRQP	0x1004
#define DDR	0x1008
#define DDRS	0x100C
#define DMACP	0x101C
#define DSIRQP	0x1020
#define DSIRQM	0x1024
#define DCIRQP	0x1028
#define DCIRQM	0x102C
#define DMAC_AR_BIT 2

/* MCU of PDMA */
#define DMCS	0x1030
#define DMNMB	0x1034
#define DMSMB	0x1038
#define DMINT	0x103C
#define NORMAL_MAILBOX_BIT 16

/*channel n base*/
#define DSA_N     0x0
#define DTA_N     0x4
#define DTC_N     0x8
#define DRT_N     0xc
#define DCS_N     0x10
#define DCM_N     0x14
#define DDA_N     0x18
#define DSD_N     0x1c
#define DCS_HAL_BIT  2
#define DCS_NDES_BIT 31
#define DCM_LINK_BIT 0
#define DCM_TIE_BIT  1
#define DCM_TSZ_BIT  8
#define DCM_DP_BIT   12
#define DCM_SP_BIT   14
#define DCM_RDIL_BIT 16
#define DCM_DAI_BIT  22
#define DCM_SAI_BIT  23
#define DCS_MASK_24BIT 8
#define TSZ_MASK_3BIT 8
#define DP_SP_RDIL_MASK_8BIT 12
#define DAI_SAI_MASK_2BIT    22

#define PDMA_AUTO_REQUEST 0x8

/*INTC*/
#define ICMSR0  0x1008
#define ICSR    0x1020
#define ICMCR1	0x102C
#define ICPR1   0x1030
#define DMR0    0x1038
#define GPIOA_MAST_BIT 17
#define INTC_PDMAM_BIT 29

/* INTC MailBox msg */
#define MCU_MSG_NORMAL          0x1
#define MCU_MSG_INTC            0x2
#define MCU_MSG_RB  	        0x3
#define MCU_MSG_PDMA            0x4

#define CPM_CLKGR  0xB0000020
#define PDMA_CLKGATE_BIT 20

#define readl(addr) (*(volatile unsigned int *)(addr))
#define writel(value,addr) ((*(volatile unsigned int *)(addr)) = (value))

#define MCU_TEST_DATA_DMA 0xB34257C0 //PDMA_BANK7 - 0x40

int jzdma_init(void);
int jzdma_prep_memcpy(unsigned short channel, dma_addr_t srcaddr, dma_addr_t dstaddr, unsigned int len);
int jzdma_start_memcpy(unsigned short channel);
void jzdma_clear_status(unsigned short channel);
#endif
