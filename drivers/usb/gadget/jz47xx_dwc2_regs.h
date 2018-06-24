/* arch/mips/include/asm/arch-jz4775/otg.h
 *
 * Ingenic JZ DWC2 USB Device Controller support
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Sun Jiwei <jwsun@ingenic.cn>
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

#ifndef __USB_OTG_H
#define __USB_OTG_H

#include <asm/arch/base.h>
#define BIT0	(1 << 0)
#define BIT1	(1 << 1)
#define BIT2	(1 << 2)
#define BIT3	(1 << 3)
#define BIT4	(1 << 4)
#define BIT5	(1 << 5)
#define BIT6	(1 << 6)
#define BIT7	(1 << 7)
#define BIT8	(1 << 8)
#define BIT9	(1 << 9)
#define BIT10	(1 << 10)
#define BIT11	(1 << 11)
#define BIT12	(1 << 12)
#define BIT13	(1 << 13)
#define BIT14	(1 << 14)
#define BIT15	(1 << 15)
#define BIT16	(1 << 16)
#define BIT17	(1 << 17)
#define BIT18	(1 << 18)
#define BIT19	(1 << 19)
#define BIT20	(1 << 20)
#define BIT21	(1 << 21)
#define BIT22	(1 << 22)
#define BIT23	(1 << 23)
#define BIT24	(1 << 24)
#define BIT25	(1 << 25)
#define BIT26	(1 << 26)
#define BIT27	(1 << 27)
#define BIT28	(1 << 28)
#define BIT29	(1 << 29)
#define BIT30	(1 << 30)
#define BIT31	(1 << 31)

/* Globle Regs define */
#define GOTG_CTL		(0x00)
#define GOTG_INTR		(0x04)
#define GAHB_CFG		(0x08)
#define GUSB_CFG		(0x0c)
#define GRST_CTL		(0x10)
#define GINT_STS		(0x14)
#define GINT_MASK		(0x18)
#define GRXSTS_READ		(0x1c)
#define GRXSTS_POP		(0x20)
#define GRXFIFO_SIZE		(0x24)
#define GNPTXFIFO_SIZE		(0x28)
#define GDTXFIFO_SIZE		(0x104)
#define DIEPTXF(n)              (0x104 + ((n) - 1) * 0x04)
#define GHW_CFG1		(0x44)
#define GHW_CFG2		(0x48)
#define GHW_CFG3		(0x4c)
#define GHW_CFG4		(0x50)
#define GDFIFO_CFG		(0x5c)

/* FIFO addr define */
#define EP_FIFO(n)		((n+1)*0x1000)

/* Device Regs define */
#define OTG_DCFG		(0x800)
#define OTG_DCTL		(0x804)
#define OTG_DSTS		(0x808)
#define DIEP_MASK		(0x810)
#define DOEP_MASK		(0x814)
#define OTG_DAINT		(0x818)
#define DAINT_MASK		(0x81c)
#define DIEP_EMPMSK		(0x834)
#define DIEP_CTL(n)		((0x900 + (n)*0x20))
#define DOEP_CTL(n)		((0xb00 + (n)*0x20))
#define DIEP_INT(n)		((0x908 + (n)*0x20))
#define DOEP_INT(n)		((0xb08 + (n)*0x20))
#define DIEP_SIZE(n)		((0x910 + (n)*0x20))
#define DOEP_SIZE(n)		((0xb10 + (n)*0x20))
#define DIEP_TXFSTS(n)		((0x918 + (n)*0x20))

/* Regs macro define */
/*************************************************/
#define AHBCFG_DMA_ENA		BIT5
#define	AHBCFG_GLOBLE_INTRMASK	BIT0

#define USBCFG_FORCE_DEVICE     BIT30
#define USBCFG_TRDTIME_MASK     (0xf << 10)
#define USBCFG_TRDTIME(n)       (((n) << 10) & USBCFG_TRDTIME_MASK)
#define USBCFG_TRDTIME_9	(9 << 10)
#define USBCFG_TRDTIME_6	(6 << 10)
#define USBCFG_HNP_EN           BIT9
#define USBCFG_SRP_EN           BIT8
#define USBCFG_PHY_SEL_USB1     BIT6
#define USBCFG_PHY_INF_UPLI     BIT4
#define USBCFG_16BIT_PHY        BIT3
#define USBCFG_TOUT_CAL_MASK (0x7)

/* GRSTCTL */
#define RSTCTL_AHB_IDLE		BIT31
#define RSTCTL_TXFNUM_ALL	(0x10 << 6)
#define RSTCTL_TXFIFO_FLUSH	BIT5
#define RSTCTL_RXFIFO_FLUSH	BIT4
#define RSTCTL_INTK_FLUSH	BIT3
#define RSTCTL_FRMCNT_RST	BIT2
#define RSTCTL_CORE_RST		BIT0

/* GINTSTS */
#define GINTSTS_RSUME_DETE	BIT31
#define GINTSTS_CONID_STSCHG	BIT28
#define GINTSTS_RESET_DETE	BIT23
#define GINTSTS_FETCH_SUSPEND	BIT22
#define GINTSTS_OEP_INTR	BIT19
#define GINTSTS_IEP_INTR	BIT18
#define GINTSTS_EP_MISMATCH	BIT17
#define GINTSTS_ENUM_DONE	BIT13
#define GINTSTS_USB_RESET	BIT12
#define GINTSTS_USB_SUSPEND	BIT11
#define GINTSTS_USB_EARLYSUSPEND	BIT10
#define GINTSTS_I2C_INT		BIT9
#define GINTSTS_ULPK_CKINT	BIT8
#define GINTSTS_GOUTNAK_EFF	BIT7
#define GINTSTS_GINNAK_EFF	BIT6
#define GINTSTS_NPTXFIFO_EMPTY	BIT5
#define GINTSTS_RXFIFO_NEMPTY	BIT4
#define GINTSTS_START_FRAM	BIT3
#define GINTSTS_OTG_INTR	BIT2
#define GINTSTS_MODE_MISMATCH	BIT1

/* GINTMSK */
#define	GINTMSK_RSUME_DETE	BIT31
#define GINTMSK_RESET_DETE	BIT23
#define GINTMSK_OEP_INTR	BIT19
#define GINTMSK_IEP_INTR	BIT18
#define GINTMSK_ENUM_DONE	BIT13
#define GINTMSK_USB_RESET	BIT12
#define GINTMSK_USB_EARLYSUSPEND	BIT10
#define GINTMSK_RXFIFO_NEMPTY	BIT4
#define GINTMSK_START_FRAM	BIT3

/* DCTL */
#define DCTL_NAK_ON_BBLE        BIT16
#define DCTL_CLR_GONAK		BIT10
#define DCTL_SET_GONAK		BIT9
#define DCTL_CLR_GNPINNAK	BIT8
#define DCTL_SET_GNPINNAK	BIT7
#define DCTL_SOFT_DISCONN	BIT1

/* DCFG */
#define DCFG_HANDSHAKE_STALL_ERR_STATUS BIT2
#define DCFG_DEV_ADDR_MASK	(0x7f << 4)
#define DCFG_DEV_ADDR_BIT	4

/* DSTS */
#define DSTS_ENUM_SPEED_MASK		(0x3 << 1)
#define DSTS_ENUM_SPEED_BIT		BIT1
#define DSTS_ENUM_SPEED_HIGH		(0x0 << 1)
#define DSTS_ENUM_SPEED_FULL_30OR60	(0x1 << 1)
#define DSTS_ENUM_SPEED_LOW		(0x2 << 1)
#define DSTS_ENUM_SPEED_FULL_48		(0x3 << 1)

/* GRXSTSR/GRXSTSP */
#define GRXSTSP_PKSTS_MASK		(0xf << 17)
#define GRXSTSP_PKSTS_GOUT_NAK		(0x1 << 17)
#define GRXSTSP_PKSTS_GOUT_RECV		(0x2 << 17)
#define GRXSTSP_PKSTS_TX_COMP		(0x3 << 17)
#define GRXSTSP_PKSTS_SETUP_COMP	(0x4 << 17)
#define GRXSTSP_PKSTS_SETUP_RECV	(0x6 << 17)
#define GRXSTSP_BYTE_CNT_MASK		(0x7ff << 4)
#define GRXSTSP_BYTE_CNT_BIT		4
#define GRXSTSP_EPNUM_MASK		(0xf)
#define GRXSTSP_EPNUM_BIT		BIT0
#define	PKTCNT_BIT			19
#define PKTCNT_MASK			(0x3f << PKTCNT_BIT)
#define XFERSIZE_BIT			0
#define XFERSIZE_MASK			(0x7ffff << XFERSIZE_BIT)

/* DAINT_MASK */
#define DAINT_OUT_BIT		(16)
#define DAINT_OUT_MASK		(0xFFFF << DAINT_OUT_BIT)
#define DAINT_IN_BIT		(0)
#define DAINT_IN_MASK		(0xFFFF << DAINT_IN_BIT)

/* DIOEP_MASK */
#define DEPMSK_STUPPKTRCVD	BIT15
#define DEPMSK_NYETMSK		BIT14
#define DEPMSK_NAKMSK		BIT13
#define DEPMSK_TXFIFOEMTMSK	BIT7
#define DEPMSK_TXFIFOUNDRNMSK	BIT6
#define DEPMSK_B2BSETUPMSK	BIT6
#define DEPMSK_STSPHSERCVMSK	BIT5
#define DEPMSK_INTKNTXFEMPMSK	BIT4
#define DEPMSK_OUTTKNEPDISMSK   BIT4
#define DEPMSK_SETUPMSK		BIT3
#define DEPMSK_TIMEOUTMSK	BIT3
#define DEPMSK_AHBERRMSK	BIT2
#define DEPMSK_EPDISBLDMSK	BIT1
#define DEPMSK_XFERCOMLMSK	BIT0

/* DIOEPCTL */
#define DEPCTL_EPENA		BIT31
#define DEPCTL_EPDIS		BIT30
#define DEPCTL_SETD1PID		BIT29
#define DEPCTL_SETD0PID		BIT28
#define DEPCTL_SNAK		BIT27
#define DEPCTL_CNAK		BIT26
#define DIEPCTL_TX_FIFO_NUM(x)	((x)<<22)
#define DIEPCTL_TX_FIFO_NUM_MASK	(0xf << 22)
#define DEPCTL_STALL		BIT21
#define DEPCTL_SNP		BIT20
#define DEPCTL_TYPE_BIT		(18)
#define DEPCTL_TYPE_MASK	(0x3 << DEPCTL_TYPE_BIT)
#define DEPCTL_NAKSTS		BIT17
#define DEPCTL_DPID		BIT16
#define DEPCTL_USBACTEP		BIT15
#define DEPCTL_MPS_BIT		(0)
#define DEPCTL_MPS_MASK		(0x7ff << DEPCTL_MPS_BIT)
#define DEPCTL_EP0_MPS_64	(0x0 << DEPCTL_MPS_BIT)
#define DEPCTL_EP0_MPS_32	(0x1 << DEPCTL_MPS_BIT)
#define DEPCTL_EP0_MPS_16	(0x2 << DEPCTL_MPS_BIT)
#define DEPCTL_EP0_MPS_8	(0x3 << DEPCTL_MPS_BIT)

/* DIOEPINT */
#define DEP_STUPPKTRCVD_INT	BIT15
#define DEP_NYET_INT		BIT14
#define DEP_NAK_INT		BIT13
#define DEP_BABBLE_ERR_INT	BIT12
#define DEP_PKT_DROP_STATUS	BIT11
#define DEP_BNA_INT		BIT9
#define DEP_TXFIFO_UNDRN	BIT8		// Only for INEP
#define DEP_OUTPKT_ERR		BIT8		// Only for OUTEP
#define DEP_TXFIFO_EMPTY	BIT7
#define DEP_INEP_NAKEFF		BIT6		// Only for INEP
#define DEP_B2B_SETUP_RECV	BIT6		// Only for OUTEP0
#define DEP_INTOKEN_EPMISATCH	BIT5		// Only for INEP
#define DEP_STATUS_PHASE_RECV	BIT5		// Only for OUTEP0
#define DEP_INTOKEN_RECV_TXFIFO_EMPTY	BIT4	// Only for INEP
#define DEP_OUTTOKEN_RECV_EPDIS	BIT4		// Only for OUTEP
#define DEP_TIME_OUT		BIT3		// Only for INEP
#define DEP_SETUP_PHASE_DONE	BIT3		// Only for OUTEP0
#define DEP_AHB_ERR		BIT2
#define DEP_EPDIS_INT		BIT1
#define DEP_XFER_COMP		BIT0		// Used by INEP and OUTEP

/* DOEPSIZ0 */
#define DOEPSIZE0_SUPCNT_1	(0x1 << 29)
#define DOEPSIZE0_SUPCNT_2	(0x2 << 29)
#define DOEPSIZE0_SUPCNT_3	(0x3 << 29)
#define DOEPSIZE0_PKTCNT	BIT19

/* OTG parameter control register(USBPCR) */
#define USBPCR_USB_MODE         BIT31
#define USBPCR_AVLD_REG         BIT30
#define USBPCR_INCRM            BIT27   /* INCR_MASK bit */
#define USBPCR_CLK12_EN         BIT26
#define USBPCR_COMMONONN        BIT25
#define USBPCR_VBUSVLDEXT       BIT24
#define USBPCR_VBUSVLDEXTSEL    BIT23
#define USBPCR_POR              BIT22
#define USBPCR_SIDDQ            BIT21
#define USBPCR_OTG_DISABLE      BIT20
#define USBPCR_TXPREEMPHTUNE    BIT6

#define CPM_OPCR                (0x24) /* Oscillator and Power Control Register */
#define CPM_USBPCR              (0x3c) /* USB parameter control register       */
#define CPM_USBRDT              (0x40) /* USB reset detect timer register      */
#define CPM_USBVBFIL            (0x44) /* USB jitter filter register           */
#define CPM_USBPCR1             (0x48) /* USB parameter control register 1     */
#define CPM_USBCDR              (0x50) /* USB OTG PHY clock divider register   */
/* Oscillator and power control register(OPCR) */
#define OPCR_OTGPHY_ENABLE      BIT7    /* SPENDN bit */
#define OPCR_GPSEN              BIT6
#define OPCR_UHCPHY_DISABLE     BIT5    /* SPENDH bit */
#define OPCR_O1SE               BIT4
#define OPCR_PD                 BIT3
#define OPCR_ERCS               BIT2
#endif
