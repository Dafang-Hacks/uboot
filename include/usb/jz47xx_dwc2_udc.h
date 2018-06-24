#ifndef JZ47XX_DWC2_UDC_H
#define JZ47XX_DWC2_UDC_H
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/arch/base.h>

struct dwc2_request {
	struct usb_request req;
	struct dwc2_ep *dep;
	struct list_head queue;
	int xfersize;
	bool zlp_transfered;
};

struct dwc2_udc;
struct dwc2_ep {
	struct usb_ep ep;
	struct list_head urb_list;
	const struct usb_endpoint_descriptor *desc;
	int wait_inxfer_complete;
	struct dwc2_udc *dev;
	int max_xfer_once;		//contorller limit
	char name[16];
	u32 flags;
#define DWC2_EP_ACTIVE	(1 << 0)
#define DWC2_EP_BUSY	(1 << 1)
	u8 bEndpointAddress;
	u8 bmAttributes;
};

#define to_dwc2_ep(ep) container_of(ep, struct dwc2_ep, ep)
#define to_dwc2_request(req) container_of(req, struct dwc2_request, req)

static inline int need_send_zlp(struct dwc2_request *request) {
	struct dwc2_ep *dep = request->dep;
	return ( (request->req.zero) &&
			(request->req.length != 0) &&
			(!request->zlp_transfered) &&
			((request->req.length % dep->ep.maxpacket) == 0));
}

static inline struct dwc2_request* next_request(struct list_head *head) {
	if (list_empty(head))
		return NULL;
	return list_first_entry(head ,struct dwc2_request ,queue);
}

struct dwc2_udc {
	struct usb_gadget gadget;
	struct usb_gadget_driver *driver;
	char name[20];
#define IDLE_STAGE	0X0
#define SETUP_STAGE	0X1
#define DATA_STAGE	0x2
#define STATUS_STAGE	0X3
	int ep0state;
	/* ep0-control, ep1in-bulk, ep1out-bulk, ep2in-int */
#define DWC2_MAX_IN_ENDPOINTS	3	/*include control endpoint*/
#define DWC2_MAX_OUT_ENDPOINTS	2	/*include control endpoint*/
#define DWC2_MAX_ENDPOINTS	(DWC2_MAX_IN_ENDPOINTS + DWC2_MAX_OUT_ENDPOINTS)
	struct dwc2_ep ep_attr[DWC2_MAX_ENDPOINTS];
	struct dwc2_ep *ep_in_attr[DWC2_MAX_IN_ENDPOINTS];
	struct dwc2_ep *ep_out_attr[DWC2_MAX_OUT_ENDPOINTS];
	struct usb_ctrlrequest crq;
};

#define RXFIFO_SIZE         1047    /* 4*1 + (7 + 1)*(512/4 + 1) + 2*2 + 1 fix*/
#define NPTXFIFO_SIZE       128     /* (7 + 1)*(64/4) fix*/
#define DTXFIFO1_SIZE       1024    /* (7 + 1)*(512/4) fix*/
#define DTXFIFO2_SIZE       128     /* (7 + 1)*(64/4) fix*/

#define ep_num(EP)   (((EP)->bEndpointAddress)&0xF)
#define ep_type	     ((ep_num(EP) == 0) ? USB_ENDPOINT_XFER_CONTROL : \
			((EP)->desc->bmAttributes | USB_ENDPOINT_XFERTYPE_MASK))
#define ep_maxpacket(EP) ((EP)->ep.maxpacket)

static inline int ep_is_in(struct dwc2_ep *ep) {
	if (!ep_num(ep)) {
		if (ep->dev->ep0state & USB_DIR_IN)
			return 1;
	} else {
		if (ep->bEndpointAddress&USB_DIR_IN)
			return 1;
	}
	return 0;
}

#define	udc_read_reg(addr)		readl(OTG_BASE + addr)
#define udc_write_reg(value, addr)	writel(value, OTG_BASE + addr)
static inline void udc_set_reg(int c_bit,int s_bit,int addr)
{
	int temp_reg = udc_read_reg(addr);
	if (c_bit)
		temp_reg &= ~c_bit;
	if (s_bit)
		temp_reg |= s_bit;
	udc_write_reg(temp_reg, addr);
}
#define udc_test_reg(value,addr)	!!(readl(OTG_BASE + addr) & value)

#define pr_err(fmt,args...)	\
		printf("\033[31m [E]udc:" fmt"\033[0m",##args)
#define pr_warn(fmt,args...)	\
		printf("\033[33m [W]udc:" fmt"\033[0m",##args)
#define pr_warn_start()		\
		printf("\033[33m");
#define pr_warn_end()		\
		printf("\033[0m");

#ifdef DWC2_DEBUG
#define pr_info(fmt,args...)	\
		printf("[I]udc:"fmt,##args)
#define pr_debug(fmt,args...)	\
		printf("\033[01m [D]udc:" fmt"\033[0m",##args)
#define pr_ep0(fmt,args...)	\
		printf("\033[32m [EP0]udc:" fmt"\033[0m",##args)
#else
#define pr_info(fmt,args...)	{}
#define pr_ep0(fmt,args...)	{}
#define pr_debug(fmt,args...)	{}
#endif

#endif /*JZ47XX_DWC2_UDC_H*/
