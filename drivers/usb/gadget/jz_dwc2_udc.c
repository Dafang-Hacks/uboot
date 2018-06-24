/* Ingenic JZ DWC2 OTG Controller Driver
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
 *  Sun Jiwei <jwsun@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#if 1
#define DEBUG_SETUP	0
#define DEBUG_INTR	0
#define DEBUG_EP0	0
#else
#define DEBUG_EP0	1
#define DEBUG_SETUP	1
#define DEBUG_INTR	1
#endif
#define	DEBUG_REG	0

#include <common.h>
#include <malloc.h>

#include <asm/errno.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <asm/io.h>

#include <linux/list.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include <usb/lin_gadget_compat.h>
#include "jz47xx_dwc2_regs.h"

#define DRIVER_DESC "JZ47XX DWC2 USB OTG Device Driver, (C) Ingenic Semiconductor"
#define DRIVER_VERSION "24 October 2013"

/*-------------------------------------------------------------------------*/
/* DMA bounce buffer size, 16K is enough even for mass storage */
enum {
	SETUP_STAGE = 0,
	DATA_STAGE,
	STATUS_STAGE,
};

#define EP_FIFO_SIZE		4096
#define EP_FIFO0_SIZE		64

#define DEP_RXFIFO_SIZE         1064
#define DEP_NPTXFIFO_SIZE       1024
#define DEP_DTXFIFO_SIZE        768
#define DEP_NUM                 2

/* ep0-control, ep1in-bulk, ep2out-bulk, ep3in-int */
#define JZ_MAX_ENDPOINTS	4
#define JZ_MAX_HW_ENDPOINTS	16

#define WAIT_FOR_SETUP          0
#define DATA_STATE_XMIT         1
#define DATA_STATE_NEED_ZLP     2
#define WAIT_FOR_OUT_STATUS     3
#define DATA_STATE_RECV         4
#define WAIT_FOR_COMPLETE	5
#define WAIT_FOR_OUT_COMPLETE	6
#define WAIT_FOR_IN_COMPLETE	7
#define WAIT_FOR_NULL_COMPLETE	8

#define ep_is_in(EP) (((EP)->bEndpointAddress&USB_DIR_IN) == USB_DIR_IN)
#define ep_index(EP) ((EP)->bEndpointAddress&0xF)
#define ep_maxpacket(EP) ((EP)->ep.maxpacket)

static void dwc_stop_out_transfer(int ep);
static const char driver_name[] = "jz_dwc2_udc";
static const char driver_desc[] = DRIVER_DESC;
static const char ep0name[] = "ep0-control";

/* Max packet size*/
static unsigned int ep0_fifo_size = 64;
static unsigned int ep_fifo_size =  512;
/* static unsigned int ep_fifo_size2 = 1024; */

enum ep_type {
	ep_control, ep_bulk_in, ep_bulk_out, ep_interrupt
};

struct jz_ep {
	struct usb_ep ep;
	struct jz_udc *dev;

	const struct usb_endpoint_descriptor *desc;
	struct list_head queue;
	unsigned long pio_irqs;

	u8 stopped;
	u8 bEndpointAddress;
	u8 bmAttributes;

	enum ep_type ep_type;
	int fifo_num;
};

struct jz_request {
	struct usb_request req;
	struct list_head queue;
	bool	is_write_last;
	bool	is_read_last;
	u32	write_pktcnt;
	u32	read_pktcnt;
};

struct jz_udc {
	struct usb_gadget gadget;
	struct usb_gadget_driver *driver;

	struct jz_plat_otg_data *pdata;

	int ep0state;
	struct jz_ep ep[JZ_MAX_ENDPOINTS];

	unsigned char usb_address;

	struct usb_ctrlrequest *crq;

	int data_dir_out;
	/* unsigned req_pending:1, req_std:1; */
};

struct usb_ctrlrequest crq;

struct jz_udc	*the_controller;

#define	udc_read_reg(addr)	readl(OTG_BASE + addr)

#define udc_write_reg(value, addr)	writel(value, OTG_BASE + addr)


#if 1
static void pri_glb_reg(void)
{}

static void pri_dev_reg(u32 n)
{}
#else
static void pri_glb_reg(void)
{
	printf("\n\n==============Now the global REG status==========\n");
	printf("GOTGCTL	= 0x%08x\n", udc_read_reg(GOTG_CTL));
	printf("GOTGINT	= 0x%08x\n", udc_read_reg(GOTG_INTR));
	printf("GAHBCFG	= 0x%08x\n", udc_read_reg(GAHB_CFG));
	printf("GUSBCFG = 0x%08x\n", udc_read_reg(GUSB_CFG));
	printf("GRSTCTL = 0x%08x\n", udc_read_reg(GRST_CTL));
	printf("GINTSTS = 0x%08x\n", udc_read_reg(GINT_STS));
	printf("GINTMSK = 0x%08x\n", udc_read_reg(GINT_MASK));
	printf("GRXSTSR = 0x%08x\n", udc_read_reg(GRXSTS_READ));
	printf("GRXFSIZ = 0x%08x\n", udc_read_reg(GRXFIFO_SIZE));
	printf("GNPTXFIFO_SIZE = 0x%08x\n", udc_read_reg(GNPTXFIFO_SIZE));
	printf("GHW_CFG1 = 0x%08x\n", udc_read_reg(GHW_CFG1));
	printf("GHW_CFG2 = 0x%08x\n", udc_read_reg(GHW_CFG2));
	printf("GHW_CFG3 = 0x%08x\n", udc_read_reg(GHW_CFG3));
	printf("GHW_CFG4 = 0x%08x\n", udc_read_reg(GHW_CFG4));
	printf("GDFIFO_CFG = 0x%08x\n", udc_read_reg(GDFIFO_CFG));
}
static void pri_dev_reg(u32 n)
{
	printf("\n\n==============Now the device REG status==========\n");
	printf("DCFG = 0x%08x\n", udc_read_reg(OTG_DCFG));
	printf("DCTL = 0x%08x\n", udc_read_reg(OTG_DCTL));
	printf("DSTS = 0x%08x\n", udc_read_reg(OTG_DSTS));
	printf("DIEP_MASK = 0x%08x\n", udc_read_reg(DIEP_MASK));
	printf("DOEP_MASK = 0x%08x\n", udc_read_reg(DOEP_MASK));
	printf("DAINT = 0x%08x\n", udc_read_reg(OTG_DAINT));
	printf("DIEP_EMPMSK = 0x%08x\n", udc_read_reg(DIEP_EMPMSK));
	printf("DIEP_CTL(%d) = 0x%08x\n",n, udc_read_reg(DIEP_CTL(n)));
	printf("DOEP_CTL(%d) = 0x%08x\n",n, udc_read_reg(DOEP_CTL(n)));
	printf("DIEP_INT(%d) = 0x%08x\n",n, udc_read_reg(DIEP_INT(n)));
	printf("DOEP_INT(%d) = 0x%08x\n",n, udc_read_reg(DOEP_INT(n)));
	printf("DIEP_SIZE(%d) = 0x%08x\n",n, udc_read_reg(DIEP_SIZE(n)));
	printf("DOEP_SIZE(%d) = 0x%08x\n",n, udc_read_reg(DOEP_SIZE(n)));
	printf("DIEP_TXFSTS(%d) = 0x%08x\n",n, udc_read_reg(DIEP_TXFSTS(n)));
}
#endif

int enum_done_speed_detect(struct jz_udc *dev);
static void cpm_enable_otg_phy(struct jz_udc *dev)
{
	u32 reg_tmp;

	reg_tmp = readl(CPM_BASE + CPM_OPCR);
	reg_tmp |= OPCR_OTGPHY_ENABLE;
	writel(reg_tmp, CPM_BASE + CPM_OPCR);
}

static inline int dwc_get_utmi_width(void)
{
	u32	reg_tmp;

	reg_tmp = udc_read_reg(GHW_CFG4);

	return (reg_tmp >> 14) & 0x3;
}

static inline void fun_8bits(void)
{
	u32 reg_tmp;

	reg_tmp = udc_read_reg(GUSB_CFG);
	reg_tmp &= ~(1 << 3);
	udc_write_reg(reg_tmp, GUSB_CFG);

	reg_tmp = udc_read_reg(GUSB_CFG);
	reg_tmp |= USBCFG_TRDTIME_9;
	udc_write_reg(reg_tmp, GUSB_CFG);

	reg_tmp = readl(CPM_BASE + CPM_USBPCR1);
	reg_tmp &= ~(3 << 18);
	writel(reg_tmp, CPM_BASE + CPM_USBPCR1);
}

static inline void fun_16bits(void)
{
	u32 reg_tmp;

	reg_tmp = udc_read_reg(GUSB_CFG);
	reg_tmp |= (1 << 3);
	udc_write_reg(reg_tmp, GUSB_CFG);

	reg_tmp = udc_read_reg(GUSB_CFG);
	reg_tmp |= USBCFG_TRDTIME_6;
	udc_write_reg(reg_tmp, GUSB_CFG);

	reg_tmp = readl(CPM_BASE + CPM_USBPCR1);
	reg_tmp |= (3 << 18);
	writel(reg_tmp, CPM_BASE + CPM_USBPCR1);
}

static void dwc_otg_select_phy_width(struct jz_udc *dev)
{
	unsigned reg_tmp;

	reg_tmp = udc_read_reg(GHW_CFG2);

	if (((reg_tmp >> 6) & 0x3) == 1) {
		reg_tmp = udc_read_reg(GUSB_CFG);
		reg_tmp &= ~USBCFG_TRDTIME_MASK;
		udc_write_reg(reg_tmp, GUSB_CFG);
		fun_16bits();
	} else
		debug_cond(DEBUG_INTR != 0, "Unkonwn USB PHY Type\n");
}

static void dwc_otg_core_reset(struct jz_udc *dev)
{
        u32 greset = 0;
        u32 cnt = 0;

        do {
                udelay(10);

		greset = udc_read_reg(GRST_CTL);
                if (cnt++ > 100000) {
                        debug_cond(DEBUG_SETUP != 0, "GRESET wait IDLE timeout.\n");
                        return;
                }
        } while ((greset & RSTCTL_AHB_IDLE) == 0);

        cnt = 0;
	udc_write_reg(greset | RSTCTL_CORE_RST, GRST_CTL);
        do {
		greset = udc_read_reg(GRST_CTL);

                if (cnt++ > 10000  ) {
                        debug_cond(DEBUG_SETUP != 0, "GRESET wait IDLE timeout.\n");
                        return;
                }
                udelay(10);
        } while (greset & RSTCTL_CORE_RST);

        /* wait for 3 phy clocks */
        udelay(100);
}


static void dwc_otg_cpm_init(struct jz_udc *dev)
{
	u32	reg_tmp;

#ifdef CONFIG_JZ4780
	reg_tmp = readl(CPM_BASE + CPM_USBPCR1);
	reg_tmp |= (1 << 28);
	writel(reg_tmp, CPM_BASE + CPM_USBPCR1);
#endif /* CONFIG_JZ4780 */
	reg_tmp = readl(CPM_BASE + CPM_USBPCR);
	reg_tmp &= ~(1 << 31);
	writel(reg_tmp, CPM_BASE + CPM_USBPCR);

	reg_tmp = readl(CPM_BASE + CPM_USBPCR);
	reg_tmp |= USBPCR_VBUSVLDEXT;
	writel(reg_tmp, CPM_BASE + CPM_USBPCR);

	reg_tmp = readl(CPM_BASE + CPM_USBPCR);
	reg_tmp |= USBPCR_POR;
	writel(reg_tmp, CPM_BASE + CPM_USBPCR);

	udelay(30);

	reg_tmp = readl(CPM_BASE + CPM_USBPCR);
	reg_tmp &= ~USBPCR_POR;
	writel(reg_tmp, CPM_BASE + CPM_USBPCR);
	udelay(300);
}

static void dwc_otg_core_init(struct jz_udc *dev)
{
	u32 reg_tmp;

	reg_tmp = udc_read_reg(GUSB_CFG);
	reg_tmp &= ~((1 << 4) | (1 << 6) | (1 << 8) | (1 << 9));
	udc_write_reg(reg_tmp, GUSB_CFG);

        dwc_otg_select_phy_width(dev);

        dwc_otg_core_reset(dev);

	udc_write_reg(1 << 7, GAHB_CFG);
	udc_write_reg(0, GINT_MASK);
}

void handle_rxfifo_nempty(struct jz_udc *dev);
void dwc_otg_flush_tx_fifo(unsigned char txf_num)
{
	unsigned int gintsts;
	unsigned int grstctl;
	int tmp;
	gintsts = udc_read_reg(GINT_STS);

	if ((gintsts & GINTSTS_GINNAK_EFF))
	{
		tmp = udc_read_reg(OTG_DCTL);
		tmp |= GINTSTS_GINNAK_EFF;
		udc_write_reg(tmp,OTG_DCTL);
		while(!(udc_read_reg(GINT_STS) & GINTSTS_GINNAK_EFF))
			udelay(1);
	}

	while (!(udc_read_reg(GRST_CTL) & RSTCTL_AHB_IDLE));

	grstctl = udc_read_reg(GRST_CTL);
	if (!(grstctl & RSTCTL_TXFIFO_FLUSH))
	{
		tmp = udc_read_reg(GRST_CTL);
		tmp |= (txf_num << 6);
		udc_write_reg(tmp,GRST_CTL);
	}
	tmp = udc_read_reg(GRST_CTL);
	tmp |= RSTCTL_TXFIFO_FLUSH;
	udc_write_reg(tmp,GRST_CTL);

	while (udc_read_reg(GRST_CTL) & RSTCTL_TXFIFO_FLUSH);

	tmp = udc_read_reg(OTG_DCTL);
	tmp |= DCTL_CLR_GNPINNAK;
	udc_write_reg(tmp,OTG_DCTL);
	debug_cond(DEBUG_SETUP != 0,"flush tx fifo\n");
}

void dwc_otg_flush_rx_fifo(void)
{
	udc_write_reg(DCTL_SET_GONAK,OTG_DCTL);
	while(!(udc_read_reg(GINT_STS) & GINTSTS_GOUTNAK_EFF)) {
		if ((udc_read_reg(GINT_STS) & GINTSTS_RXFIFO_NEMPTY)) {
			handle_rxfifo_nempty(the_controller);
		}
		udelay(1);
	};
	udc_write_reg(RSTCTL_RXFIFO_FLUSH, GRST_CTL);
        while (udc_read_reg(GRST_CTL) & RSTCTL_RXFIFO_FLUSH);
	udc_write_reg(DCTL_CLR_GONAK,OTG_DCTL);
	debug_cond(DEBUG_SETUP != 0,"flush rx fifo\n");
}

void disable_all_ep(void)
{
	int i;
	unsigned int	reg_tmp;

	for (i = 0; i < DEP_NUM; i++) {
		reg_tmp = udc_read_reg(DIEP_CTL(i));
		reg_tmp |= (DEPCTL_EPDIS | DEPCTL_SNAK);
		udc_write_reg(reg_tmp, DIEP_CTL(i));

		reg_tmp = udc_read_reg(DOEP_CTL(i));
		reg_tmp |= (DEPCTL_EPDIS | DEPCTL_SNAK);
		udc_write_reg(reg_tmp, DOEP_CTL(i));

		udc_write_reg(0, DIEP_SIZE(i));
		udc_write_reg(0, DOEP_SIZE(i));

		udc_write_reg(0xff, DIEP_INT(i));
		udc_write_reg(0xff, DOEP_INT(i));
	}
}

static void dwc_otg_device_init(struct jz_udc *dev)
{
	u32 reg_tmp;
        u16 epinfobase, gdfifocfg;


	udc_write_reg(DEP_RXFIFO_SIZE, GRXFIFO_SIZE);
	udc_write_reg((DEP_NPTXFIFO_SIZE << 16) | DEP_RXFIFO_SIZE,
			GNPTXFIFO_SIZE);
	udc_write_reg((DEP_DTXFIFO_SIZE << 16) |
			(DEP_RXFIFO_SIZE + DEP_NPTXFIFO_SIZE), GDTXFIFO_SIZE);

	reg_tmp = udc_read_reg(GHW_CFG3);
	gdfifocfg = (reg_tmp >> 16);

	reg_tmp = udc_read_reg(GRXFIFO_SIZE);
	epinfobase = (reg_tmp & 0xffff);
	reg_tmp = udc_read_reg(GNPTXFIFO_SIZE);
	epinfobase += reg_tmp >> 16;
	udc_write_reg(epinfobase << 16 | gdfifocfg, GDFIFO_CFG);

        dwc_otg_flush_tx_fifo(0x10);
        dwc_otg_flush_rx_fifo();

        /* clear irq and mask all ep intr */
	udc_write_reg(0, DOEP_MASK);
	udc_write_reg(0, DIEP_MASK);
	udc_write_reg(0xffffffff, OTG_DAINT);
	udc_write_reg(0, DAINT_MASK);

        /* disable all in and out ep */
        disable_all_ep();

	udc_write_reg(0xffffffff, GINT_STS);
	udc_write_reg((1<<2), OTG_DCFG);
	udc_write_reg(0, OTG_DCTL);

	reg_tmp = udc_read_reg(GINT_MASK);
	reg_tmp |= (1 << 4);
	udc_write_reg(reg_tmp, GINT_MASK);
}

static void dwc_otg_enable_common_irq(struct jz_udc *dev)
{
	u32 reg_tmp;

	reg_tmp = udc_read_reg(GAHB_CFG);
	reg_tmp |= 1;
	udc_write_reg(reg_tmp, GAHB_CFG);

	reg_tmp = udc_read_reg(GINT_MASK);

        /*	    CONIDSTS    OUTEP      INEP         enum      usbreset     goutnak */
	reg_tmp |= (1 << 28) | (1 << 19) | (1 << 18) | (1 << 13) | (1 << 12) | (1<<7);
	udc_write_reg(reg_tmp, GINT_MASK);
}

/* until it's enabled, this UDC should be completely invisible
 * to any USB host.
 */
static int udc_enable(struct jz_udc *dev)
{
	debug_cond(DEBUG_SETUP != 0, "%s: %p\n", __func__, dev);

	dwc_otg_cpm_init(dev);

	cpm_enable_otg_phy(dev);

	dwc_otg_core_init(dev);

	dwc_otg_device_init(dev);

	dwc_otg_enable_common_irq(dev);

	dev->gadget.speed = USB_SPEED_UNKNOWN;

	return 0;
}

/*
 *	udc_disable - disable USB device controller
 */
static void udc_disable(struct jz_udc *dev)
{
	return;
}

/*
 *	udc_reinit - initialize software state
 */
static void udc_reinit(struct jz_udc *dev)
{
	unsigned int i;

	/* device/ep0 records init */
	INIT_LIST_HEAD(&dev->gadget.ep_list);
	INIT_LIST_HEAD(&dev->gadget.ep0->ep_list);
	dev->ep0state = SETUP_STAGE;

	/* basic endpoint records init */
	for (i = 0; i < JZ_MAX_ENDPOINTS; i++) {
		struct jz_ep *ep = &dev->ep[i];

		if (i != 0)
			list_add_tail(&ep->ep.ep_list, &dev->gadget.ep_list);

		ep->desc = 0;
		ep->stopped = 0;
		INIT_LIST_HEAD(&ep->queue);
		ep->pio_irqs = 0;
	}

	/* the rest was statically initialized, and is read-only */
}

/*
 *	udc_done - retire a request; caller blocked irqs
 */
static void udc_done(struct jz_ep *ep, struct jz_request *req, int status)
{
	unsigned int stopped = ep->stopped;

	debug_cond(DEBUG_INTR != 0, "%s: %s %p, req = %p, stopped = %d\n",
	      __func__, ep->ep.name, ep, &req->req, stopped);

	list_del_init(&req->queue);

	if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	if (status && status != -ESHUTDOWN) {
		printf("complete %s req %p stat %d len %u/%u\n",
		      ep->ep.name, &req->req, status,
		      req->req.actual, req->req.length);
	}

	/* don't modify queue heads during completion callback */
	ep->stopped = 1;

	debug_cond(DEBUG_INTR != 0, "%s callback completed\n",__func__);

	spin_unlock(&ep->dev->lock);
	req->req.complete(&ep->ep, &req->req);
	spin_lock(&ep->dev->lock);
	ep->stopped = stopped;
}

/*
 *	nuke - dequeue ALL requests
 */
static void nuke(struct jz_ep *ep, int status)
{
	struct jz_request *req;

	/* called with irqs blocked */
	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct jz_request, queue);
		udc_done(ep, req, status);
	}
}

static void stop_activity(struct jz_udc *dev,
			  struct usb_gadget_driver *driver)
{
	int i;

	/* don't disconnect drivers more than once */
	if (dev->gadget.speed == USB_SPEED_UNKNOWN)
		driver = 0;
	dev->gadget.speed = USB_SPEED_UNKNOWN;

	/* prevent new request submissions, kill any outstanding requests  */
	for (i = 0; i < JZ_MAX_ENDPOINTS; i++) {
		struct jz_ep *ep = &dev->ep[i];
		ep->stopped = 1;
		nuke(ep, -ESHUTDOWN);
	}

	/* report disconnect; the driver is already quiesced */
	if (driver) {
		spin_unlock(&dev->lock);
		driver->disconnect(&dev->gadget);
		spin_lock(&dev->lock);
	}

	/* re-init driver-visible data structures */
	/* udc_reinit(dev) */
}

/*
 * Unregister entry point for the peripheral controller driver.
 */
int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct jz_udc *dev = the_controller;
	unsigned long flags;

	if (!dev)
		return -ENODEV;
	if (!driver || driver != dev->driver)
		return -EINVAL;

	spin_lock_irqsave(&dev->lock, flags);
	dev->driver = 0;
	stop_activity(dev, driver);
	spin_unlock_irqrestore(&dev->lock, flags);

	driver->unbind(&dev->gadget);

	/* disable_irq(IRQ_OTG); */

	udc_disable(dev);
	return 0;
}

static void jz_udc_ep_activate(struct jz_ep *ep)
{
	u32		reg_tmp;
	u32		eptype;
	u32		epnum;

	epnum = ep->bEndpointAddress & 0x7f;

	switch (ep->ep_type) {
	case ep_control:
		eptype = 0;
		break;
	case ep_bulk_in:
	case ep_bulk_out:
		eptype = 2;
		break;
	case ep_interrupt:
		eptype = 3;
		break;
	default:
		printf("%s:ERROR ep type is not defined\n", __func__);
		return;
	}

	if (ep_is_in(ep)) {
		reg_tmp = udc_read_reg(DIEP_CTL(epnum));
	} else {
		dwc_stop_out_transfer(epnum);
		reg_tmp = udc_read_reg(DOEP_CTL(epnum));
	}

	reg_tmp &= ~(DEPCTL_MPS_MASK | DEPCTL_EPENA);
	reg_tmp |= (ep->ep.maxpacket << DEPCTL_MPS_BIT);

	reg_tmp |= DEPCTL_USBACTEP;

	reg_tmp |= DEPCTL_SETD0PID;

	reg_tmp &=~DEPCTL_TYPE_MASK;
	reg_tmp |= (eptype << DEPCTL_TYPE_BIT);

	reg_tmp |= (DEPCTL_EPDIS | DEPCTL_SNAK);//queue will enable this endpoint

	if (ep_is_in(ep)) {
		reg_tmp &= ~DIEPCTL_TX_FIFO_NUM_MASK;
		reg_tmp |= DIEPCTL_TX_FIFO_NUM(ep->fifo_num);
		udc_write_reg(reg_tmp, DIEP_CTL(epnum));
	} else {
		udc_write_reg(reg_tmp, DOEP_CTL(epnum));
	}
}

static void jz_udc_set_nak(struct jz_ep *ep)
{

}

/* FIXME */
static int jz_udc_set_halt(struct usb_ep *_ep, int value)
{
#if 0
	struct jz_ep	*ep;
	struct jz_udc	*dev;
	unsigned long	flags;
	u8		ep_num;

	ep = container_of(_ep, struct jz_ep, ep);
	ep_num = ep_index(ep);

	if (unlikely(!_ep || !ep->desc || ep_num == EP0_CON ||
		     ep->desc->bmAttributes == USB_ENDPOINT_XFER_ISOC)) {
		debug("%s: %s bad ep or descriptor\n", __func__, ep->ep.name);
		return -EINVAL;
	}

	/* Attempt to halt IN ep will fail if any transfer requests
	 * are still queue */
	if (value && ep_is_in(ep) && !list_empty(&ep->queue)) {
		debug("%s: %s queue not empty, req = %p\n",
			__func__, ep->ep.name,
			list_entry(ep->queue.next, struct jz_request, queue));

		return -EAGAIN;
	}

	dev = ep->dev;
	debug("%s: ep_num = %d, value = %d\n", __func__, ep_num, value);

	spin_lock_irqsave(&dev->lock, flags);

	if (value == 0) {
		ep->stopped = 0;
		jz_udc_ep_clear_stall(ep);
	} else {
		if (ep_num == 0)
			dev->ep0state = WAIT_FOR_SETUP;

		ep->stopped = 1;
		jz_udc_ep_set_stall(ep);
	}

	spin_unlock_irqrestore(&dev->lock, flags);
#endif

	return 0;
}
#ifdef CONFIG_BURNER
void jz_udc_disable_bootrom_ep(struct usb_ep *_ep)
{
	struct jz_ep *ep = container_of(_ep, struct jz_ep, ep);
	int epnum = (ep->bEndpointAddress & 0xf);
	int dir = (ep->bEndpointAddress & 0x80);
	if (epnum) {
		int tmp = 0;
		if (dir) {
			tmp = udc_read_reg(DIEP_CTL(epnum));
			if (tmp & DEPCTL_EPENA) {
				/*check it*/
				tmp &= ~DEPCTL_EPENA;
				tmp |= DEPCTL_EPDIS | DEPCTL_SNAK;
				udc_write_reg(tmp, DIEP_CTL(epnum));
				while(!(udc_read_reg(DIEP_INT(epnum)) & DEP_EPDIS_INT))
				udc_write_reg(DEP_EPDIS_INT,DIEP_INT(epnum));
			}
		} else {
			tmp = udc_read_reg(DOEP_CTL(epnum));

			if (tmp & DEPCTL_EPENA) {
				/*set global out nak*/
				udc_write_reg(DCTL_SET_GONAK,OTG_DCTL);
				while(!(udc_read_reg(GINT_STS) & GINTSTS_GOUTNAK_EFF)) {
					if ((udc_read_reg(GINT_STS) & GINTSTS_RXFIFO_NEMPTY)) {
						handle_rxfifo_nempty(the_controller);
					}
				}
				/*disable endpoint*/
				tmp &= ~DEPCTL_EPENA;
				tmp |= DEPCTL_EPDIS | DEPCTL_SNAK;
				udc_write_reg(tmp, DOEP_CTL(epnum));
				while(!(udc_read_reg(DOEP_INT(epnum)) & DEP_EPDIS_INT));
				udc_write_reg(DEP_EPDIS_INT,DOEP_INT(epnum));

				/*flush rx fifo*/
				udc_write_reg(RSTCTL_RXFIFO_FLUSH, GRST_CTL);
				while (udc_read_reg(GRST_CTL) & RSTCTL_RXFIFO_FLUSH);

				/*clear global out nak*/
				udc_write_reg(DCTL_CLR_GONAK,OTG_DCTL);
			}
		}
	}
}
#endif
static int jz_ep_enable_part(struct usb_ep *_ep,
			 const struct usb_endpoint_descriptor *desc,
			 int hw_enable)
{
	struct jz_ep *ep;
	struct jz_udc *dev;
	unsigned long flags;

	ep = container_of(_ep, struct jz_ep, ep);
	if (!_ep || !desc || ep->desc || _ep->name == ep0name
			|| desc->bDescriptorType != USB_DT_ENDPOINT
			|| ep->bEndpointAddress != desc->bEndpointAddress
			|| ep_maxpacket(ep) <
			le16_to_cpu(get_unaligned(&desc->wMaxPacketSize))) {

		printf("%s: bad ep or descriptor\n", __func__);
		return -EINVAL;
	}

	/* xfer types must match, except that interrupt ~= bulk */
	if (ep->bmAttributes != desc->bmAttributes
	    && ep->bmAttributes != USB_ENDPOINT_XFER_BULK
	    && desc->bmAttributes != USB_ENDPOINT_XFER_INT) {


		printf("%s: %s type mismatch\n", __func__, _ep->name);
		return -EINVAL;
	}

	/* hardware _could_ do smaller, but driver doesn't */
	if ((desc->bmAttributes == USB_ENDPOINT_XFER_BULK
	     && le16_to_cpu(get_unaligned(&desc->wMaxPacketSize)) !=
	     ep_maxpacket(ep)) || !get_unaligned(&desc->wMaxPacketSize)) {

		printf("%s: bad %s maxpacket\n", __func__, _ep->name);
		return -ERANGE;
	}

	dev = ep->dev;
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {

		printf("%s: bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

	ep->stopped = 0;
	ep->desc = desc;
	ep->pio_irqs = 0;
	ep->ep.maxpacket = le16_to_cpu(get_unaligned(&desc->wMaxPacketSize));

	/* Reset halt state */
	//if (hw_enable) {
		jz_udc_set_nak(ep);
		jz_udc_set_halt(_ep, 0);
		spin_lock_irqsave(&ep->dev->lock, flags);
		jz_udc_ep_activate(ep);
		spin_unlock_irqrestore(&ep->dev->lock, flags);
	//}

	debug_cond(DEBUG_SETUP != 0, "%s: enabled %s, stopped = %d, maxpacket = %d\n",
	      __func__, _ep->name, ep->stopped, ep->ep.maxpacket);
	return 0;
}

static int jz_ep_vir_enable(struct usb_ep *_ep,
		const struct usb_endpoint_descriptor *desc)
{
	return jz_ep_enable_part(_ep,desc,0);
}

int f_ep_vir_enable(struct usb_ep *_ep,
		const struct usb_endpoint_descriptor *desc)
{
	return jz_ep_vir_enable(_ep,desc);
}

static int jz_ep_enable(struct usb_ep *_ep,
		const struct usb_endpoint_descriptor *desc)
{
	return jz_ep_enable_part(_ep,desc,1);
}

static void set_max_pktsize(struct jz_udc *dev, enum usb_device_speed speed)
{
	int i;

	if (speed == USB_SPEED_HIGH) {
		ep0_fifo_size = 64;
		ep_fifo_size = 512;
		/* ep_fifo_size2 = 1024; */
		dev->gadget.speed = USB_SPEED_HIGH;
	} else {
		ep0_fifo_size = 64;
		ep_fifo_size = 64;
		/* ep_fifo_size2 = 64; */
		dev->gadget.speed = USB_SPEED_FULL;
	}

	dev->ep[0].ep.maxpacket = ep0_fifo_size;
	for (i = 1; i < JZ_MAX_ENDPOINTS; i++)
		dev->ep[i].ep.maxpacket = ep_fifo_size;
}

/* FIXME */
/*
 * Disable EP
 */
static int jz_ep_disable(struct usb_ep *_ep)
{
	struct jz_ep *ep;
	unsigned long flags;

	printf("%s: %p\n", __func__, _ep);

	ep = container_of(_ep, struct jz_ep, ep);
	if (!_ep || !ep->desc) {
		printf("%s: %s not enabled\n", __func__,
		      _ep ? ep->ep.name : NULL);
		return -EINVAL;
	}

	spin_lock_irqsave(&ep->dev->lock, flags);

	/* Nuke all pending requests */
	nuke(ep, -ESHUTDOWN);

	ep->desc = 0;
	ep->stopped = 1;

	spin_unlock_irqrestore(&ep->dev->lock, flags);

	printf("%s: disabled %s\n", __func__, _ep->name);
	return 0;

}

static struct usb_request *jz_alloc_request(struct usb_ep *ep,
					     gfp_t gfp_flags)
{
	struct jz_request *req;

	req = kzalloc(sizeof(*req), gfp_flags);
	if (!req)
		return NULL;

	INIT_LIST_HEAD(&req->queue);
	return &req->req;

}

static void jz_free_request(struct usb_ep *ep, struct usb_request *_req)
{
	struct jz_request *req;

	debug("%s: %p\n", __func__, ep);

	req = container_of(_req, struct jz_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}

static void handle_ep_data(struct jz_ep *ep, struct jz_request *req,
		u32 *pktcnt, u32 *xfersize)
{
	if (ep->ep.maxpacket > 512) {
		if ((req->req.length - req->req.actual) > 0x7ffff) {
			*xfersize = 0x7ffff;
		} else {
			*xfersize = req->req.length;
		}
		*pktcnt = (*xfersize + ep->ep.maxpacket  - 1) / (ep->ep.maxpacket);
	} else {
		*pktcnt = ((req->req.length - req->req.actual) + ep->ep.maxpacket  - 1) /
			(ep->ep.maxpacket);
		if (*pktcnt > 256) {
			*pktcnt = 256;
			*xfersize = (*pktcnt) * ep->ep.maxpacket;
		} else {
			*xfersize = req->req.length - req->req.actual;
		}
		debug_cond(DEBUG_INTR != 0, "%s: req->req.length = %d, actual = %d\n",
				__func__, req->req.length, req->req.actual);
	}
}

/* FIXME: the pkcnt > 0x3ff, and xfersize > 0x7ffff, How to deal with this case? */
static void handle_ep_data_in_phase(struct jz_ep *ep, struct jz_request *req)
{
        u32 pktcnt, xfersize;
	u32 epnum;
	u32 reg_tmp;

	if ((!ep) || (!req)) {
		debug_cond(DEBUG_INTR != 0, "%s:Error ep or req is NULL\n", __func__);
		return ;
	}
	epnum = ep->bEndpointAddress & 0x7f;
	debug_cond(DEBUG_INTR != 0, "%s: epnum is %d\n", __func__, epnum);

	if (req->req.length) {
		handle_ep_data(ep, req, &pktcnt, &xfersize);
	} else {
		debug_cond(DEBUG_INTR != 0, "%s: Deal with zero package\n", __func__);
		pktcnt = 1;
		xfersize = 0;
		req->is_write_last = 1;
	}
	req->write_pktcnt = pktcnt;
	debug_cond(DEBUG_INTR != 0, "%s: the pktcnt is %d, xfersize is %d\n",
			__func__, pktcnt, xfersize);

	debug_cond(DEBUG_INTR != 0, "pkcnt = %d, xfersize = %d\n", pktcnt, xfersize);
	udc_write_reg(pktcnt << 19 | xfersize, DIEP_SIZE(epnum));
	asm volatile ("nop\n\t");
	reg_tmp = udc_read_reg(DIEP_CTL(epnum));
	reg_tmp |= DEPCTL_EPENA | DEPCTL_CNAK;
	udc_write_reg(reg_tmp, DIEP_CTL(epnum));
	debug_cond(DEBUG_REG != 0, "%s: IEP_CTL is 0x%x, epnum is %d\n",
			__func__, udc_read_reg(DIEP_CTL(epnum)),epnum);

	reg_tmp = udc_read_reg(DIEP_EMPMSK);
	reg_tmp |= (1 << epnum);
	udc_write_reg(reg_tmp, DIEP_EMPMSK);
}

void handle_ep_data_out_phase(struct jz_ep *ep, struct jz_request *req)
{
        u32 pktcnt, xfersize;
	u32 epnum;
	u32 reg_tmp;

	if ((!ep) || (!req)) {
		debug_cond(DEBUG_INTR != 0, "%s:Error ep or req is NULL\n", __func__);
		return ;
	}
	epnum = ep->bEndpointAddress & 0x7f;
	debug_cond(DEBUG_INTR != 0, "%s: epnum is %d\n", __func__, epnum);

	if (!req->req.length) {
		printf("%s: Warning: The request length is 0, yes?\n", __func__);
		return;
	}
	handle_ep_data(ep, req, &pktcnt, &xfersize);
	if (epnum) {
		debug_cond(DEBUG_INTR != 0, "%s: the pktcnt is %d = 0x%x, xfersize is %d = 0x%x\n",
				__func__, pktcnt, pktcnt, xfersize, xfersize);
		req->read_pktcnt = pktcnt;
	}

	udc_write_reg(pktcnt << 19 | xfersize, DOEP_SIZE(epnum));
	asm volatile ("nop\n\t");
	reg_tmp = udc_read_reg(DOEP_CTL(epnum));
	reg_tmp |= DEPCTL_EPENA | DEPCTL_CNAK;
	udc_write_reg(reg_tmp, DOEP_CTL(epnum));
}

static inline int read_ep_fifo(unsigned int addr, unsigned char *buf, unsigned int len)
{
	unsigned int	*buf_tmp = (unsigned int *)buf;
	unsigned int	total = (len + 3) / 4;
	unsigned int	remain = 4 - len % 4;

	while (total--) {
		*buf_tmp = udc_read_reg(addr);
		buf_tmp++;
	}

	buf_tmp--;

	switch (remain)	{
	case 1:
		*buf_tmp &= 0x00ffffff;
		break;
	case 2:
		*buf_tmp &= 0x0000ffff;
		break;
	case 3:
		*buf_tmp &= 0x000000ff;
		break;
	default:
		break;
	}
	return 0;
}

/* FIXME */
static int jz_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct jz_request *req;
	struct jz_ep *ep;
	struct jz_udc *dev;
	int is_in = 0;
	unsigned long flags;
	req = container_of(_req, struct jz_request, req);
	if (unlikely(!_req || !_req->complete || !_req->buf
				|| !list_empty(&req->queue))) {

		printf("req = %p, _req->complete = %p, _req->buf = %p, list = %d\n",
				_req,_req->complete,_req->buf,!list_empty(&req->queue));
		printf("%s: bad params\n", __func__);
		return -EINVAL;
	}

	ep = container_of(_ep, struct jz_ep, ep);

	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {

		printf("%s: bad ep: %s, %d, %p\n", __func__,
		      ep->ep.name, !ep->desc, _ep);
		return -EINVAL;
	}

	dev = ep->dev;
	if (unlikely(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)) {

		printf("%s: bogus device addr %p,speed %d\n", __func__,
				dev,dev->gadget.speed);
		return -ESHUTDOWN;
	}

	spin_lock_irqsave(&dev->lock, flags);

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	if (!(ep->bEndpointAddress & 0xf)) {
		/*Assume only data stage or two stage status can be queue*/
		if (dev->data_dir_out)
			is_in = 0;
		 else
			is_in = 1;
	} else {
		is_in = ep_is_in(ep);
	}

	/* kickstart this i/o queue? */
	debug_cond(DEBUG_INTR != 0, "\n*** %s: %s-%s req = %p, len = %d, buf = %p"
		"Q empty = %d, stopped = %d\n",
		__func__, _ep->name,is_in ? "in" : "out",
		_req, _req->length, _req->buf,
		list_empty(&ep->queue), ep->stopped);

	/* kickstart this i/o queue? */
	if (list_empty(&ep->queue)) {
		if (ep->bEndpointAddress == 0 /* ep0 */) {
			if (dev->ep0state == DATA_STAGE && dev->data_dir_out) {
				/*OUT DATA*/
				debug_cond(DEBUG_EP0 != 0 ,
						"|ep0 %p| *SET UP DONE* data out \"enable out data\"\n",ep);
				handle_ep_data_out_phase(ep, req);
			} else {
				/*IN DATA OR STATUS*/
				debug_cond(DEBUG_EP0 != 0 ,
						"|ep0 %p| *SET UP DONE* in %s \"enable in %s\" \n",
						ep,
						dev->ep0state == DATA_STAGE ? "data" : "status",
						dev->ep0state == DATA_STAGE ? "data" : "status");
				handle_ep_data_in_phase(ep, req);
			}
		/* can the FIFO can satisfy the request immediately? */
		} else if ((ep->bEndpointAddress & USB_DIR_IN) != 0) {
			handle_ep_data_in_phase(ep, req);
		} else {
			handle_ep_data_out_phase(ep, req);
		}
	}

	/* pio or dma irq handler advances the queue. */
	if (likely(req != NULL)) {
		list_add_tail(&req->queue, &ep->queue);
		req->is_read_last = 0;
	} else {
		printf("%s: Warning, request is NULL\n", __func__);
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	return 0;
}

static void dwc_out_endpoint(int epnum)
{
	int timeout = 5000;
	unsigned int ctl = udc_read_reg(DOEP_CTL(epnum));
	udc_write_reg(ctl | (DEPCTL_EPDIS | DEPCTL_SNAK),DOEP_CTL(epnum));

	do {
		udelay(1);
	} while ( (!(udc_read_reg(DOEP_INT(epnum)) & DEP_EPDIS_INT)) && (--timeout > 0));
	udc_write_reg(DEP_EPDIS_INT,DOEP_INT(epnum));
}

void dwc_stop_out_transfer(int ep)
{
	if (!(udc_read_reg(DOEP_CTL(ep)) & DEPCTL_EPENA))
		return;

	unsigned int reg_tmp = udc_read_reg(GINT_MASK);
	reg_tmp |= (1<<7);
	udc_write_reg(reg_tmp, GINT_MASK);

	udc_write_reg(DCTL_SET_GONAK,OTG_DCTL);
	while(!(udc_read_reg(GINT_STS) & GINTSTS_GOUTNAK_EFF)) {
		if ((udc_read_reg(GINT_STS) & GINTSTS_RXFIFO_NEMPTY)) {
			handle_rxfifo_nempty(the_controller);
		}
		udelay(1);
	};

	dwc_out_endpoint(ep);
	udc_write_reg(DCTL_CLR_GONAK,OTG_DCTL);
#if 0
	if (udc_read_reg(DOEP_CTL(ep)) & DEPCTL_EPENA)
		printf("ep%d dwc_stop_out_transfer failed.\n",ep);
	else
		printf("ep%d dwc_stop_out_transfer ok.\n",ep);
#endif
}


/* FIXME */
/* dequeue JUST ONE request */
static int jz_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct jz_ep *ep = container_of(_ep, struct jz_ep, ep);
	struct jz_udc	*dev = ep->dev;
	struct jz_request *req;
	unsigned long flags;

	debug("%s: %p\n", __func__, _ep);

	if (!_ep)
		return -EINVAL;

	spin_lock_irqsave(&ep->dev->lock, flags);

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		spin_unlock_irqrestore(&ep->dev->lock, flags);
		return -EINVAL;
	}

	if(!ep_is_in(ep) && (ep_index(ep) == 1)) {
		dwc_stop_out_transfer(1);
	}

	udc_done(ep, req, -ECONNRESET);

	spin_unlock_irqrestore(&ep->dev->lock, flags);
	return 0;

}

/*
 * Return bytes in EP FIFO
 */
/* FIXME */
static int jz_fifo_status(struct usb_ep *_ep)
{
#if 0
	struct jz_ep        *ep;

	ep = container_of(_ep, struct jz_ep, ep);
	if (!_ep) {
		debug_cond(DEBUG_INTR != 0, "%s, bad ep\n", __func__);
		return -ENODEV;
	}
	/* pxa can't report unclaimed bytes from IN fifos */
	if ((ep->bEndpointAddress & USB_DIR_IN) != 0)
		return -EOPNOTSUPP;
	if (ep->dev->gadget.speed == USB_SPEED_UNKNOWN
			|| (readl(ep->reg_udccs) & UDCCS_BO_RFS) == 0)
		return 0;
	else
		return (readl(ep->reg_ubcr) & 0xfff) + 1;
#endif
	return 0;

}

/*
 * Flush EP FIFO
 */
/* FIXME */
static void jz_fifo_flush(struct usb_ep *_ep)
{
#if 0
	struct jz_ep *ep;

	ep = container_of(_ep, struct jz_ep, ep);
	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {
		debug("%s: bad ep\n", __func__);
		return;
	}

	debug("%s: %d\n", __func__, ep_index(ep));
#endif
}

static const struct usb_gadget_ops jz_udc_ops = {
	/* current versions must always be self-powered */
};

static struct usb_ep_ops jz_ep_ops = {
	.enable = jz_ep_enable,
	.disable = jz_ep_disable,

	.alloc_request = jz_alloc_request,
	.free_request = jz_free_request,

	.queue = jz_queue,
	.dequeue = jz_dequeue,

	.set_halt = jz_udc_set_halt,
	.fifo_status = jz_fifo_status,
	.fifo_flush = jz_fifo_flush,
};

static struct jz_udc memory = {
	.usb_address = 0,
	.gadget = {
		.ops = &jz_udc_ops,
		.ep0 = &memory.ep[0].ep,
		.name = driver_name,
	},

	/* control endpoint */
	.ep[0] = {
		.ep = {
			.name = ep0name,
			.ops = &jz_ep_ops,
			.maxpacket = EP_FIFO0_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = 0,
		.bmAttributes = 0,

		.ep_type = ep_control,
	},

	/* first group of endpoints */
	.ep[1] = {
		.ep = {
			.name = "ep1in-bulk",
			.ops = &jz_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = USB_DIR_IN | 1,
		.bmAttributes = USB_ENDPOINT_XFER_BULK,

		.ep_type = ep_bulk_out,
		.fifo_num = 1,
	},

	.ep[2] = {
		.ep = {
			.name = "ep1out-bulk",
			.ops = &jz_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = USB_DIR_OUT | 1,
		.bmAttributes = USB_ENDPOINT_XFER_BULK,

		.ep_type = ep_bulk_in,
	},

	.ep[3] = {
		.ep = {
			.name = "ep2in-int",
			.ops = &jz_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = USB_DIR_IN | 2,
		.bmAttributes = USB_ENDPOINT_XFER_INT,

		.ep_type = ep_interrupt,
		.fifo_num = 2,
	},
};

int jz_udc_probe(void)
{
	struct jz_udc *dev = &memory;
	int retval = 0;

	printf("jz_dwc2_udc\n");
	dev->gadget.is_dualspeed = 1;	/* Hack only*/
	dev->gadget.is_otg = 0;
	dev->gadget.is_a_peripheral = 0;
	dev->gadget.b_hnp_enable = 0;
	dev->gadget.a_hnp_support = 0;
	dev->gadget.a_alt_hnp_support = 0;

	the_controller = dev;

	udc_reinit(dev);

	return retval;
}

static void handle_early_suspend_intr(struct jz_udc *dev)
{
	debug_cond(DEBUG_INTR != 0,
		"%s: Handle early suspend intr, mask EARLYSUSPEND bit\n", __func__);

	udc_write_reg(GINTSTS_USB_EARLYSUSPEND, GINT_STS);
}

static void handle_start_frame_intr(struct jz_udc *dev)
{
	debug_cond(DEBUG_INTR != 0,
		"%s: Handle start frame intr, mask START_FRAM bit\n", __func__);

	udc_write_reg(GINTSTS_START_FRAM, GINT_STS);
}

static void handle_reset_intr(struct jz_udc *dev)
{
	u32 reg_tmp;
	debug_cond(DEBUG_INTR != 0, "%s: Handle reset intr\n", __func__);

	/* Step 1: SET NAK for all OUT ep */
	reg_tmp = udc_read_reg(DOEP_CTL(0));
	reg_tmp |= DEPCTL_SNAK;
	udc_write_reg(reg_tmp, DOEP_CTL(0));
	reg_tmp = udc_read_reg(DOEP_CTL(1));
	reg_tmp |= DEPCTL_SNAK;
	udc_write_reg(reg_tmp, DOEP_CTL(1));

	/* Step 2: unmask intr. */
	reg_tmp = udc_read_reg(DAINT_MASK);
	reg_tmp |= (1 << 0) | (1 << 16);
	udc_write_reg(reg_tmp, DAINT_MASK);

	reg_tmp = udc_read_reg(DOEP_MASK);
	reg_tmp |= (1 << 0) | (1 << 3) | (1 << 6);
	/* reg_tmp |= (1 << 0) | (1 << 3); */
	udc_write_reg(reg_tmp, DOEP_MASK);

	reg_tmp = udc_read_reg(DIEP_MASK);
	reg_tmp |= DEPMSK_XFERCOMLMSK;
	/* reg_tmp |= (1 << 0) | (1 << 3); */
	udc_write_reg(reg_tmp, DIEP_MASK);

	/* Step 3: device init nothing to do */

	/* Step 4: dfifo dynamic allocated */

	/* Step 5: Reset Device Address */
	reg_tmp = udc_read_reg(OTG_DCFG);
	reg_tmp &= ~DCFG_DEV_ADDR_MASK;
	udc_write_reg(reg_tmp, OTG_DCFG);

	/*Step 6: setup EP0 to receive SETUP packets*/
	udc_write_reg(DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT | (8 * 3),
			DOEP_SIZE(0));
	udc_write_reg(GINTSTS_USB_RESET, GINT_STS);

#if 0
	/* flush all txfifos */
	dwc_otg_flush_tx_fifo(0x10);
	udc_write_reg(DEPCTL_EPENA | DEPCTL_CNAK, DOEP_CTL(0));

#endif
}

int enum_done_speed_detect(struct jz_udc *dev)
{
	u32 dsts = udc_read_reg(OTG_DSTS);

	switch(dsts & DSTS_ENUM_SPEED_MASK) {
	case DSTS_ENUM_SPEED_HIGH:
		debug_cond(DEBUG_INTR != 0, "%s: High Speed.\n", __func__);
		set_max_pktsize(dev, USB_SPEED_HIGH);
		break;
	case DSTS_ENUM_SPEED_FULL_30OR60:
	case DSTS_ENUM_SPEED_FULL_48:
		debug_cond(DEBUG_INTR != 0, "%s: Full Speed.\n", __func__);
		set_max_pktsize(dev, USB_SPEED_FULL);
		break;
	case DSTS_ENUM_SPEED_LOW:
		debug_cond(DEBUG_INTR != 0, "%s: LOW Speed\n", __func__);
		set_max_pktsize(dev, USB_SPEED_LOW);
		break;
	default:
		debug_cond(DEBUG_INTR != 0, "%s: Fault speed enumeration\n", __func__);
		return -1;
	}

	return 0;
}

void handle_enum_done_intr(struct jz_udc *dev)
{
	u32 reg_tmp;
	u32 ret;
	debug_cond(DEBUG_INTR != 0, "%s: Handle enum done intr.\n", __func__);

	ret = enum_done_speed_detect(dev);
	if (ret)
		return;

	reg_tmp = udc_read_reg(OTG_DCTL);
	reg_tmp |= DCTL_CLR_GNPINNAK;
	udc_write_reg(reg_tmp, OTG_DCTL);

	reg_tmp = udc_read_reg(GINT_MASK);
	reg_tmp |= GINTSTS_RXFIFO_NEMPTY | GINTSTS_IEP_INTR | GINTSTS_OEP_INTR;
	udc_write_reg(reg_tmp,GINT_MASK);

	reg_tmp = udc_read_reg(DIEP_CTL(0));
	reg_tmp |= DEPCTL_EP0_MPS_64 | DIEPCTL_TX_FIFO_NUM(0);
	udc_write_reg(reg_tmp,  DIEP_CTL(0));

	reg_tmp = udc_read_reg(DOEP_CTL(0));
	reg_tmp |= DEPCTL_EPENA | DEPCTL_CNAK | DEPCTL_EP0_MPS_64;
	udc_write_reg(reg_tmp, DOEP_CTL(0));

	udc_write_reg(GINTSTS_ENUM_DONE, GINT_STS);
}

static void udc_get_status_complete(struct usb_ep *ep,
		struct usb_request *req)
{
	usb_ep_free_request(ep, req);
}

static int udc_get_status(struct jz_udc *dev,
		struct usb_ctrlrequest *crq)
{
	u8 ep_num = crq->wIndex & 0x7F;
	struct usb_request *req;
	char *buf;

	req = usb_ep_alloc_request(dev->gadget.ep0, 0);
	if (!req) {
		printf("%s: Error, alloc request\n", __func__);
		return -1;
	}

	req->length = 2;
	req->complete = udc_get_status_complete;
	req->buf = malloc(req->length);

	buf = (char *)req->buf;

	debug_cond(DEBUG_SETUP != 0,
			"%s: *** USB_REQ_GET_STATUS\n", __func__);
	debug_cond(DEBUG_SETUP != 0,
			"crq->brequest:0x%x\n", crq->bRequestType & USB_RECIP_MASK);
	switch (crq->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_INTERFACE:
		buf[0] = 0;
		buf[1] = 0;
		break;

	case USB_RECIP_DEVICE:
		buf[0] = 1;
		buf[1] = 0;
		break;

	case USB_RECIP_ENDPOINT:
		if (crq->wLength > 2) {
			printf("GET_STATUS:Not support EP or wLength\n");
			return 1;
		}

		buf[0] = dev->ep[ep_num].stopped;
		buf[1] = 0;

		break;

	default:
		return 1;
	}

	usb_ep_queue(dev->gadget.ep0, req, 0);

	return 0;
}

static int udc_setup_status(int epnum,
		int is_in)
{
	int reg_tmp = 0;
	if (is_in) {
		udc_write_reg(1 << 19, DIEP_SIZE(epnum));
		asm volatile ("nop\n\t");
		reg_tmp = udc_read_reg(DIEP_CTL(epnum));
		reg_tmp |= DEPCTL_EPENA | DEPCTL_CNAK;
		udc_write_reg(reg_tmp, DIEP_CTL(epnum));
	} else {
		udc_write_reg((1 << 19),DOEP_SIZE(epnum));
		asm volatile ("nop\n\t");
		reg_tmp = udc_read_reg(DOEP_CTL(epnum));
		reg_tmp |= DEPCTL_EPENA | DEPCTL_CNAK;
		udc_write_reg(reg_tmp, DOEP_CTL(epnum));
	}
	return 0;
}

void udc_start_new_setup(int epnum)
{
	int reg_tmp = 0;
	reg_tmp = udc_read_reg(DOEP_CTL(epnum));
	reg_tmp |= DEPCTL_EPENA | DEPCTL_CNAK;
	udc_write_reg(reg_tmp, DOEP_CTL(epnum));
}

void udc_handle_ep0_idle(struct jz_udc *dev, struct jz_ep *ep)
{
	int ret = 0;
	u32 reg_tmp;
	debug_cond(DEBUG_INTR != 0,"requesttype %x request %x\n",dev->crq->bRequestType,
			dev->crq->bRequest);

#ifdef CONFIG_BURNER
	if (dev->crq->bRequestType == 0xc0 && dev->crq->bRequest == 0x00) {
		jz_udc_disable_bootrom_ep(&memory.ep[2].ep);
	}
#endif
	if (dev->crq->wLength) {
		debug_cond(DEBUG_EP0 != 0 , "|ep0 %p| *SET UP DONE* data ===START===\n",ep);
		dev->ep0state = DATA_STAGE;
	} else {
		debug_cond(DEBUG_EP0 != 0 , "|ep0 %p| *SET UP DONE* status ===START===\n",ep);
		dev->ep0state = STATUS_STAGE;
	}

	if (!(dev->crq->bRequestType & USB_DIR_IN))
		dev->data_dir_out = 1;
	else
		dev->data_dir_out = 0;

	if ((dev->crq->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (dev->crq->bRequest) {
		case USB_REQ_SET_ADDRESS:
			debug_cond(DEBUG_INTR != 0, "USB_REQ_SET_ADDRESS\n");
			if (dev->crq->bRequestType == USB_RECIP_DEVICE) {
				reg_tmp = udc_read_reg(OTG_DCFG);
				reg_tmp &=~DCFG_DEV_ADDR_MASK;
				reg_tmp |= dev->crq->wValue << DCFG_DEV_ADDR_BIT;
				udc_write_reg(reg_tmp, OTG_DCFG);
				debug_cond(DEBUG_INTR != 0, "Set ADDRESS : 0x%x\n",dev->crq->wValue);
				debug_cond(DEBUG_EP0 != 0 ,
						"|ep0 %p| *SET UP DONE* setup, \" enable in status\" \n",
						ep);
				udc_setup_status(0,1);
			}
			break;
		case USB_REQ_GET_STATUS:
			debug_cond(DEBUG_INTR != 0, "USB_REQ_GET_STATUS\n");
			ret = udc_get_status(dev, dev->crq);
			break;
		case USB_REQ_CLEAR_FEATURE:
			debug_cond(DEBUG_INTR != 0, "USB_REQ_CLEAR_FEATURE\n");
			ret = -1;
			break;
		case USB_REQ_SET_FEATURE:
			debug_cond(DEBUG_INTR != 0, "USB_REQ_SET_FEATURE\n");
			ret = -1;
			break;
		default:
			ret = dev->driver->setup(&dev->gadget, dev->crq);
			break;
		}
	} else {
		ret = dev->driver->setup(&dev->gadget, dev->crq);
	}

	if (ret) {
		debug_cond(DEBUG_EP0 != 0 ,
			"|ep0 %p| *SET UP DONE* STALL  \"do nothing\" ===END===\n",
			ep);
		/*usb_stall_ep0(dir);*/
	}

	if (dev->ep0state == DATA_STAGE && !dev->data_dir_out) {
		debug_cond(DEBUG_EP0 != 0 ,
			"|ep0 %p| *SET UP DONE* data in \"enable out status\" ===END===\n",
			ep);
		udc_setup_status(0,0);
	} else if (dev->ep0state == STATUS_STAGE) {
		debug_cond(DEBUG_EP0 != 0 ,
			"|ep0 %p| *SET UP DONE* status \"enable next setup\" ===END===\n",
			ep);
		udc_start_new_setup(0);
	} else {
		debug_cond(DEBUG_EP0 != 0 ,
			"|ep0 %p| *SET UP DONE* data out \"do nothing\" ===END===\n",
			ep);
	}
}

static inline int write_ep_fifo(unsigned int addr, unsigned char *buf, unsigned int len)
{
	unsigned int	total = (len + 3) / 4;
	unsigned int	*buf_tmp = (unsigned int *)buf;

	while (total--) {
		debug_cond(DEBUG_INTR != 0, "now write 0x%x\n",*buf_tmp);
		udc_write_reg(*buf_tmp, addr);
		buf_tmp++;
	}

	return 0;
}

static int udc_write_fifo(struct jz_ep *ep, struct jz_request *req)
{
	u8	*buf;
	int	bufferspace;
	int	txsizes = 0;
	int	idx;
	u32	max_packet;
	int	fifo_status;
	u32	fifo_reg;

	idx = ep->bEndpointAddress & 0x7f;

	switch (idx) {
	default:
		idx = 0;
	case 0:
		fifo_reg = EP_FIFO(0);
		break;
	case 1:
		fifo_reg = EP_FIFO(1);
		break;
	case 2:
		fifo_reg = EP_FIFO(2);
		break;
	case 3:
		fifo_reg = EP_FIFO(3);
		break;
	}

	while (req->write_pktcnt--) {
		fifo_status = udc_read_reg(DIEP_TXFSTS(idx));
		debug_cond(DEBUG_REG != 0, "fifo_status is 0x%x\n",fifo_status);

		bufferspace = req->req.length - req->req.actual;
		debug_cond(DEBUG_INTR != 0, "bufferspace is 0x%x\n", bufferspace);

		buf = ((unsigned char *)req->req.buf) + req->req.actual;

		if (fifo_status < bufferspace) {
			static int i;
			debug_cond(DEBUG_INTR != 0, "Now fifo is not enough, wait\n");
			mdelay(10);
			i++;
			if (i >= 1000) {
				printf("Error: Timeout\n");
				i = 0;
				req->is_write_last = 1;
				break;
			} else
				return 0;
		}

		if (bufferspace <= 0) {
			printf("Txfer is over\n");
			req->is_write_last = 1;
			break;
		}

		max_packet = ep->ep.maxpacket;
		txsizes = min(bufferspace, max_packet);
		debug_cond(DEBUG_INTR != 0, "txsizes is 0x%x\n",txsizes);

		write_ep_fifo(fifo_reg, buf, txsizes);

		req->req.actual += txsizes;
		debug_cond(DEBUG_INTR != 0, "req.actual is %d,req.length is %d\n",
			req->req.actual, req->req.length);
	}

	if (req->req.length <= ep->ep.maxpacket)
		req->is_write_last = 1;
	else if (req->req.length <= req->req.actual)
		req->is_write_last = 1;
	else
		req->is_write_last = 0;
	debug_cond(DEBUG_INTR != 0, "is_write_last is %d\n", req->is_write_last);

	return 0;
}

static inline int udc_read_packet(int fifo, u8 *buf,
		struct jz_request *req, unsigned int avail)
{
	unsigned int len;

	len = min(req->req.length - req->req.actual, avail);
	req->req.actual += len;

	read_ep_fifo(fifo, buf, len);
	return len;
}

static int udc_read_fifo(struct jz_ep *ep, struct jz_request *req, u32 fifo_count)
{
	u8	*buf;
	u32	avail;
	u32	idx;
	u32	fifo_reg;
	unsigned int bufferspace;

	idx = ep->bEndpointAddress & 0x7f;

	switch (idx) {
	default:
		idx = 0;
	case 0:
		fifo_reg = EP_FIFO(0);
		break;
	case 1:
		fifo_reg = EP_FIFO(1);
		break;
	}

	if (!req->req.length) {
		printf("%s: ERROR, request length is 0\n", __func__);
		return -EINVAL;
	}

	buf = (unsigned char *)req->req.buf + req->req.actual;
	if (req->req.length <= req->req.actual) {
		bufferspace = 0;
	} else
		bufferspace = req->req.length - req->req.actual;

	/* if Buffer is full, how to deal with the fifo? */
	if (!bufferspace) {
		debug_cond(DEBUG_INTR != 0,
			"%s: Buffer full, Flush rxfifo and return\n", __func__);
		req->is_read_last = 1;
	} else {
		debug_cond(DEBUG_INTR != 0,
			"%s: GRXSTSP_BYTE_CNT is %d\n",__func__, fifo_count);

		avail = udc_read_packet(fifo_reg, buf, req, fifo_count);
		debug_cond(DEBUG_INTR != 0,
			"%s: read buf count is %d\n",__func__, avail);

		if (req->req.length <= ep->ep.maxpacket) {
			req->is_read_last = 1;
		} else {
			if (avail < ep->ep.maxpacket) {
				req->is_read_last = 1;
			} else {
				req->is_read_last =
					(req->req.length <= req->req.actual) ? 1 : 0;
			}
		}
	}

	if (idx == 1) {
		int size = udc_read_reg(DOEP_SIZE(1));
		debug_cond(DEBUG_INTR != 0,"read 1 fifo paket %d size = %d,ctl = %x\n",
				(size>>19)&0x3ff,
				(size&0x7ffff),
				udc_read_reg(DOEP_CTL(1)));
	}

	if (req->is_read_last) {
		debug_cond(DEBUG_INTR != 0,"%s:The last read packet now epnum %d\n", __func__,idx);
	}

	return req->is_read_last;
}

static void handle_ep0(struct jz_udc *dev)
{
	struct jz_ep		*ep = &dev->ep[0];
	udc_handle_ep0_idle(dev, ep);
}

static void udc_read_fifo_ep(struct jz_ep *ep, u32 fifo_count)
{
	struct jz_request *req;

	if (likely(!list_empty(&ep->queue)))
		req = list_entry(ep->queue.next,
				struct jz_request, queue);
	else
		req = NULL;

	if (likely(req)) {
		udc_read_fifo(ep, req, fifo_count);
	}
}

static void udc_read_fifo_ep0_crq(struct usb_ctrlrequest *crq)
{
	unsigned char *buf = (unsigned char *)crq;
	read_ep_fifo(EP_FIFO(0), buf, sizeof(struct usb_ctrlrequest));
}

void handle_rxfifo_nempty(struct jz_udc *dev)
{
	u32	rxsts_pop = udc_read_reg(GRXSTS_POP);
	int	ep_num = rxsts_pop & 0xf;
	u32	fifo_count = (rxsts_pop & GRXSTSP_BYTE_CNT_MASK)
				>> GRXSTSP_BYTE_CNT_BIT;

	debug_cond(DEBUG_INTR != 0, "%s: rxfifo is not empty\n", __func__);
	debug_cond(DEBUG_INTR != 0, "%s: GRXSTS_POP is 0x%x\n", __func__, rxsts_pop);

	switch(rxsts_pop & GRXSTSP_PKSTS_MASK) {
	case GRXSTSP_PKSTS_GOUT_NAK:
		debug_cond(DEBUG_INTR != 0, "%s: OUT NAK\n", __func__);
		break;
	case GRXSTSP_PKSTS_GOUT_RECV:
		debug_cond(DEBUG_INTR != 0, "%s: GRXSTSP_PKSTS_GOUT_RECV\n", __func__);
		debug_cond(DEBUG_INTR != 0, "%s: The ep is %d, fifo_count is %d\n",
				__func__, ep_num, fifo_count);

		if (fifo_count != 0) {
			if (ep_num != 0) {
				udc_read_fifo_ep(&dev->ep[ep_num + 1], fifo_count);
			} else {
				udc_read_fifo_ep(&dev->ep[ep_num], fifo_count);
			}
		}
		break;
	case GRXSTSP_PKSTS_TX_COMP:
		debug_cond(DEBUG_INTR != 0, "%s: TX complete epnum %d\n", __func__,ep_num);
		break;
        case GRXSTSP_PKSTS_SETUP_COMP:
		debug_cond(DEBUG_INTR != 0, "%s: SETUP complete\n", __func__);
                break;
        case GRXSTSP_PKSTS_SETUP_RECV:
		debug_cond(DEBUG_INTR != 0, "%s: SETUP receive\n", __func__);
		udc_read_fifo_ep0_crq(dev->crq);
                break;
        default:
		debug_cond(DEBUG_INTR != 0, "%s: Warring, have not intr\n", __func__);
                break;
        }
	udc_write_reg(GINTSTS_RXFIFO_NEMPTY, GINT_STS);
}

void udc_write_fifo_ep(struct jz_ep *ep)
{
	struct jz_request *req;

	if (likely(!list_empty(&ep->queue))){
		req = list_entry(ep->queue.next,
				struct jz_request, queue);
	} else
		req = NULL;
	if (req){
		if (req->req.length > req->req.actual)
			udc_write_fifo(ep, req);
		else
			req->is_write_last = 1;
	}
}

void inep_transfer_complete(struct jz_ep *ep)
{
	struct jz_request	*req;
	struct jz_udc	*dev = ep->dev;

	debug_cond(DEBUG_INTR != 0,"jz_udc is right %d line %d\n",&memory == dev,__LINE__);
	if (likely(!list_empty(&ep->queue)))
		req = list_entry(ep->queue.next,
			struct jz_request, queue);
	else {
#ifdef CONFIG_BURNER
		static volatile int xx = 1;
		if (xx) {
			debug_cond(DEBUG_EP0 != 0 ,
				"|ep0 %p| in status state xfercomplet, \"bootrom last complete\" \n",
				ep);
			xx -= 1;
		}
		else
#endif
		{
			if (dev->ep0state == STATUS_STAGE) {
				dev->ep0state = SETUP_STAGE;
				debug_cond(DEBUG_EP0 != 0 ,
					"|ep0 %p| in status state xfercomplet, \"out setup already enable\" \n",
					ep);
			} else {
				debug_cond(DEBUG_EP0 != 0 ,
						"|ep0 %p| Unkonwn xfercomplet mark 1, \"warinnnnnnnnng\" \n",
						ep);
			}
		}
		return;
	}

	if (!(ep->bEndpointAddress &0xf)) {
		if (dev->ep0state == DATA_STAGE) {
			debug_cond(DEBUG_EP0 != 0 ,
				"|ep0 %p| in data state xfercomplet, \"out status already enable\" \n",
				ep);
			dev->ep0state = STATUS_STAGE;
		} else if (dev->ep0state == STATUS_STAGE) {
			debug_cond(DEBUG_EP0 != 0 ,
				"|ep0 %p| in status state xfercomplet, \"out setup already enable\" \n",
				ep);
			dev->ep0state = SETUP_STAGE;
		} else {
			debug_cond(DEBUG_EP0 != 0 ,
				"|ep0 %p| Unkonwn xfercomplet mark 2, \"warinnnnnnnnng\" \n",
				ep);
		}
	}

	if (req->is_write_last) {
		req->is_write_last = 0;
		udc_done(ep, req, 0);
	} else if (req->req.length <= req->req.actual) {
		printf("%s: Error, Now it should be the last transfer, but not\n"
				, __func__);
	} else {
		handle_ep_data_in_phase(ep, req);
	}
}

static void dwc_set_in_nak(int epnum)
{
	int  timeout = 5000;
	unsigned int diep_ctl = udc_read_reg(DIEP_CTL(epnum));

	if (!(diep_ctl & DEPCTL_EPENA))
		return ;

	udc_write_reg(DEPCTL_SNAK,DIEP_CTL(epnum));

	do
	{
		udelay(1);
		if (timeout < 2) {
			debug_cond(DEBUG_EP0,"dwc set in nak timeout\n");
		}
	} while ( (!(udc_read_reg(DIEP_INT(epnum)) & DEP_INEP_NAKEFF)) && (--timeout > 0));

	udc_write_reg(DEP_INEP_NAKEFF,DIEP_INT(epnum));
}

static void dwc_disable_in_ep(int epnum)
{
	int  timeout = 100000;
	unsigned int diep_ctl = udc_read_reg(DIEP_CTL(epnum));

	if (!(diep_ctl & DEPCTL_EPENA))
		return;

	udc_write_reg(diep_ctl|DEPCTL_EPDIS, DIEP_CTL(epnum));

	do
	{
		udelay(1);
		if (timeout < 2) {
			debug_cond(DEBUG_EP0,"dwc disable in ep timeout\n");
		}
	} while ( (!(udc_read_reg(DIEP_INT(epnum)) & DEP_EPDIS_INT)) && (--timeout > 0));

	udc_write_reg(DEP_EPDIS_INT,DIEP_INT(epnum));

	dwc_otg_flush_tx_fifo(epnum);
}

static void dwc_stop_in_transfer(int epnum)
{
	dwc_set_in_nak(epnum);
	dwc_disable_in_ep(epnum);
}

void outep_transfer_complete(struct jz_ep *ep)
{
	struct jz_request	*req;
	struct jz_udc *dev = ep->dev;
	static int out_data_cover = 0;

	debug_cond(DEBUG_INTR != 0,"jz_udc is right %d line %d\n",&memory == dev,__LINE__);
	if (likely(!list_empty(&ep->queue))) {
		req = list_entry(ep->queue.next,
				struct jz_request, queue);
	} else {
		if (!(ep->bEndpointAddress&0xf)) {
			if (dev->ep0state == STATUS_STAGE && dev->data_dir_out) {
				debug_cond(DEBUG_EP0 != 0 ,
					"|ep0 %p| out status state xfercomplet, \"enable new setup\" \n",
					ep);
				dev->ep0state = SETUP_STAGE;
				udc_start_new_setup(0);
			} else if (dev->ep0state == SETUP_STAGE) {
				debug_cond(DEBUG_EP0 != 0 ,
					"|ep0 %p| setup state xfercomplet, \"wait setup done\" \n",
					ep);
			} else {
				/*if dev->data_dir_out there may be a out token recive when ep disable*/
				debug_cond(DEBUG_EP0 != 0 ,
					"|ep0 %p| unkown out xfercomplet mark 1 \"warinnnnnnnnng\" \n",
					ep);
			}
		}
		return ;
	}

	if (!(ep->bEndpointAddress&0xf)) {
		if (dev->ep0state == DATA_STAGE && dev->data_dir_out) {
			if (!out_data_cover) {
				debug_cond(DEBUG_EP0 != 0 ,
					"|ep0 %p| out data state xfercomplet first time,"
					"\" do nothing\"\n",
					ep);
				out_data_cover = 1;
			} else {
				debug_cond(DEBUG_EP0 != 0 ,
					"|ep0 %p| out data state xfercomplet,"
					"\"enable new setup and in status\" \n",
					ep);
				out_data_cover = 0;
				dev->ep0state  = STATUS_STAGE;
				udc_done(ep, req, 0);
				udc_setup_status(0,1);
				udc_start_new_setup(0);
			}
		} else {
			debug_cond(DEBUG_EP0 != 0 ,
				"|ep0 %p| unkown out xfercomplet status %d dir_out=%d\"warinnnnnnnnng\" \n",
				ep,dev->ep0state,dev->data_dir_out);
		}
		return;
	}

	if (req->is_read_last) {
		req->is_read_last = 0;
		udc_done(ep, req, 0);
	} else if (req->req.length <= req->req.actual) {
		if (req->req.length == 0) {
			req->is_read_last = 0;
			udc_done(ep, req, 0);
			return;
		} else {
			printf("%s: Error, Now it should be the last transfer,"
					"but not, req.length = %d, req.actual = %d\n",
					__func__, req->req.length, req->req.actual);
		}
	} else {
		handle_ep_data_out_phase(ep, req);
		debug_cond(DEBUG_INTR != 0, "%s: Transfer is not last\n", __func__);
	}
}

void handle_inep_intr(struct jz_udc *dev)
{
        u32 ep_intr, intr;
        int epnum = 15;
	u32 ep_empmsk;
	u32 reg_tmp;

	ep_intr = udc_read_reg(OTG_DAINT);
	debug_cond(DEBUG_REG != 0, "%s: DAINT = 0x%x.\n", __func__, ep_intr);
	ep_intr &= DAINT_IN_MASK;

        while(ep_intr) {
		if(!(ep_intr & (0x1 << epnum))) {
			epnum--;
			continue;
		}
		debug_cond(DEBUG_REG != 0, "%s: epnum=%d\n", __func__, epnum);
		intr = udc_read_reg(DIEP_INT(epnum));
		ep_empmsk = udc_read_reg(DIEP_EMPMSK);
		debug_cond(DEBUG_REG != 0, "%s:IEP_INT = 0x%x,IEP_EMPMSK = 0x%x\n",
				__func__, intr, ep_empmsk);

		if (intr & DEP_EPDIS_INT) {
			debug_cond(DEBUG_INTR != 0, "%s: DEP_EPDIS_INT\n", __func__);
			udc_write_reg(DEP_EPDIS_INT, DIEP_INT(epnum));
		}

		if (intr & DEP_AHB_ERR) {
			debug_cond(DEBUG_INTR != 0, "%s: DEP_AHB_ERR\n", __func__);
			udc_write_reg(DEP_AHB_ERR, DIEP_INT(epnum));
		}



		if ((intr & DEP_TIME_OUT) & udc_read_reg(DIEP_MASK)) {
			debug_cond(DEBUG_INTR != 0, "%s: DEP_TIME_OUT\n", __func__);
			reg_tmp = udc_read_reg(DIEP_MASK);
			reg_tmp &= ~DEP_TIME_OUT;
			udc_write_reg(reg_tmp, DIEP_MASK);
			udc_write_reg(DEP_TIME_OUT, DIEP_INT(epnum));
		}

		if ((intr & DEP_TXFIFO_EMPTY) && (ep_empmsk & (1 << epnum))) {
			unsigned int timeout = 0x7ffff;
			debug_cond(DEBUG_INTR != 0,
					"%s: In TXFIFO_EMPTY intr.\n", __func__);
			udc_write_fifo_ep(&dev->ep[epnum]);

			udc_write_reg(DEP_TXFIFO_EMPTY, DIEP_INT(epnum));

			/* FIXME: Using the BULK transferation, When DIEP_INT.DEP_XFER_COMP is 1,
			 * but GINT_STS.GINTSTS_IEP_INTR is not 1, why? */
			while (!(udc_read_reg(DIEP_INT(epnum)) & DEP_XFER_COMP) && --timeout)
				;
			if(!timeout) {
				printf("handle inep timeout\n");
				dwc_stop_in_transfer(epnum);
				reg_tmp = udc_read_reg(DIEP_EMPMSK);
				reg_tmp &= ~(1 << epnum);
				udc_write_reg(reg_tmp, DIEP_EMPMSK);
				inep_transfer_complete(&dev->ep[epnum]);
			}
		}

		if (intr & DEP_NAK_INT) {
			debug_cond(DEBUG_INTR != 0,
					"%s: In DEP_NAK_INT intr.\n", __func__);
			udc_write_reg(DEP_NAK_INT, DIEP_INT(epnum));
		}

		if ((udc_read_reg(DIEP_INT(epnum)) & DEP_XFER_COMP) & udc_read_reg(DIEP_MASK)) {
			debug_cond(DEBUG_INTR != 0,
					"%s DEP_XFER_COMP %d\n",__func__,epnum);
			udc_write_reg(DEP_XFER_COMP, DIEP_INT(epnum));

			reg_tmp = udc_read_reg(DIEP_EMPMSK);
			reg_tmp &= ~(1 << epnum);
			udc_write_reg(reg_tmp, DIEP_EMPMSK);
			inep_transfer_complete(&dev->ep[epnum]);
		}

                ep_intr &= ~(0x1<<epnum);
		epnum--;
	}
}


int handle_outep_intr(struct jz_udc *dev)
{
	u32 ep_intr, intr;
	int epnum = 15;

	ep_intr = udc_read_reg(OTG_DAINT);
	ep_intr = (ep_intr & DAINT_OUT_MASK) >> DAINT_OUT_BIT;

	while (ep_intr) {

		if(!(ep_intr & (0x1 << epnum))) {
			epnum--;
			continue;
		}

		intr = udc_read_reg(DOEP_INT(epnum));
		debug_cond(DEBUG_REG != 0, "%s: DOEP_INT(%d)=0x%08x\n",__func__, epnum, intr);

		if (intr & DEP_XFER_COMP) {
			debug_cond(DEBUG_INTR != 0,"%s: DEP_XFER_COMP epnum %d\n", __func__,epnum);
			udc_write_reg(DEP_XFER_COMP, DOEP_INT(epnum));
			if (!epnum)
				udc_write_reg(DEP_STATUS_PHASE_RECV, DOEP_INT(epnum));
			if (epnum == 0) {
				outep_transfer_complete(&dev->ep[epnum]);
			} else {
				outep_transfer_complete(&dev->ep[epnum + 1]);
			}
		}

		if (intr & DEP_SETUP_PHASE_DONE) {
                        debug_cond(DEBUG_INTR != 0, "%s:DEP_SETUP_PHASE_DONE.\n", __func__);
			udc_write_reg(DEP_SETUP_PHASE_DONE, DOEP_INT(epnum));
			if (!epnum) {
				if (intr & DEP_B2B_SETUP_RECV) {
					printf("%s: Handle epnum 0 back to back .\n",
							__func__);
					udc_write_reg(DEP_B2B_SETUP_RECV, DOEP_INT(epnum));
				}
				handle_ep0(dev);
			}
                }

		if (intr & DEP_OUTTOKEN_RECV_EPDIS) {
			debug_cond(DEBUG_INTR !=0, "%s:DEP_OUTTOKEN_RECV_EPDIS.\n",__func__);
			udc_write_reg(DEP_SETUP_PHASE_DONE, DOEP_INT(epnum));
		}

		if (intr & DEP_EPDIS_INT) {
			debug_cond(DEBUG_INTR != 0, "%s: DEP_EPDIS_INT\n", __func__);
			udc_write_reg(DEP_EPDIS_INT, DOEP_INT(epnum));
		}

		if (intr & DEP_AHB_ERR) {
			debug_cond(DEBUG_INTR != 0, "%s: DEP_AHB_ERR\n", __func__);
			udc_write_reg(DEP_AHB_ERR, DOEP_INT(epnum));
		}

		if (intr & DEP_BABBLE_ERR_INT) {
			debug_cond(DEBUG_INTR != 0, "%s: DEP_BABBLE_ERR_INT\n", __func__);
			udc_write_reg(DEP_BABBLE_ERR_INT, DOEP_INT(epnum));
		}

		if (intr & DEP_NAK_INT) {
			debug_cond(DEBUG_INTR != 0, "%s: DEP_NAK_INT\n", __func__);
			udc_write_reg(DEP_NAK_INT, DOEP_INT(epnum));
		}

		if (intr & DEP_NYET_INT) {
			debug_cond(DEBUG_INTR != 0, "%s: DEP_NYET_INT\n", __func__);
			udc_write_reg(DEP_NYET_INT, DOEP_INT(epnum));
		}

		ep_intr &= ~(0x1<<epnum);
		epnum--;
	}

	return 0;
}

int jz_udc_irq(int irq, void *_dev)
{
	struct jz_udc *dev = _dev;
	u32 intsts;
	u32 gintmsk;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);

	intsts = udc_read_reg(GINT_STS);
	debug_cond(DEBUG_REG != 0, "%s: GINT_STS is 0x%x\n", __func__, intsts);
	gintmsk = udc_read_reg(GINT_MASK);
	debug_cond(DEBUG_REG != 0, "%s: GINT_MASK is 0x%x\n", __func__, gintmsk);

	if (!(intsts & gintmsk)) {
		spin_unlock_irqrestore(&dev->lock, flags);
		return IRQ_HANDLED;
	}

	if ((intsts & GINTSTS_USB_EARLYSUSPEND) &&
			(gintmsk & GINTMSK_USB_EARLYSUSPEND)) {
		handle_early_suspend_intr(dev);
	}

	if ((intsts & GINTSTS_START_FRAM) &&
			(gintmsk & GINTMSK_START_FRAM)) {
		handle_start_frame_intr(dev);
	}

	if ((intsts & GINTSTS_USB_RESET) &&
			(gintmsk & GINTMSK_USB_RESET)) {
		handle_reset_intr(dev);
	}

	if ((intsts & GINTSTS_ENUM_DONE) &&
			(gintmsk & GINTMSK_ENUM_DONE)) {
		handle_enum_done_intr(dev);
	}


	if ((intsts & GINTSTS_IEP_INTR) &&
			(gintmsk & GINTMSK_IEP_INTR)) {
		handle_inep_intr(dev);
	}

	if ((intsts & GINTSTS_OEP_INTR) &&
			(gintmsk & GINTMSK_OEP_INTR)) {
		handle_outep_intr(dev);
	}

	if ((intsts & GINTSTS_RXFIFO_NEMPTY) &&
			(gintmsk & GINTMSK_RXFIFO_NEMPTY)) {
		handle_rxfifo_nempty(dev);
	}

	return IRQ_HANDLED;
}

/*
  Register entry point for the peripheral controller driver.
*/
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct jz_udc *dev = the_controller;
	dev->crq = &crq;
	int retval = 0;
	unsigned long flags;

	if (!driver
	    || (driver->speed != USB_SPEED_FULL
		&& driver->speed != USB_SPEED_HIGH)
	    || !driver->bind || !driver->disconnect || !driver->setup)
		return -EINVAL;
	if (!dev)
		return -ENODEV;
	if (dev->driver)
		return -EBUSY;

	spin_lock_irqsave(&dev->lock, flags);
	/* first hook up the driver ... */
	dev->driver = driver;
	spin_unlock_irqrestore(&dev->lock, flags);

	retval = driver->bind(&dev->gadget);
	if (retval) {
		debug_cond(DEBUG_SETUP != 0,
			   "%s: bind to driver --> error %d\n",
			    dev->gadget.name, retval);
		dev->driver = NULL;
		return retval;
	}

	/* enable_irq(IRQ_OTG); */
	debug_cond(DEBUG_SETUP != 0,
		   "Registered gadget driver %s\n", dev->gadget.name);
#ifndef CONFIG_BURNER
	udc_enable(dev);
#else
	enum_done_speed_detect(dev);
	{
		int reg_tmp;
		reg_tmp = udc_read_reg(OTG_DCFG);
		reg_tmp |= (1 << 2);
		udc_write_reg(reg_tmp, OTG_DCFG);
	}

	debug_cond(DEBUG_EP0 != 0,"EP 0 %p\n",&memory.ep[0].ep);
	debug_cond(DEBUG_EP0 != 0,"EP 1 %p\n",&memory.ep[1].ep);
	debug_cond(DEBUG_EP0 != 0,"EP 2 %p\n",&memory.ep[2].ep);
#endif /* CONFIG_BURNER */

	return 0;
}

int usb_gadget_handle_interrupts(void)
{
#if 0
	static int i = 0;
	if((i++%0x7fffff) == 0)
		printf("usb_gadget_handle_interrupts\n");
#endif
	return jz_udc_irq(1, (void *)the_controller);
}

