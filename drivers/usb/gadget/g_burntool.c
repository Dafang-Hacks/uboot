/*
 * Ingenic burner Gadget Code
 *
 * Copyright (c) 2013 cli <cli@ingenic.cn>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#include <errno.h>
#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <part.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/compiler.h>
#include <linux/usb/composite.h>
#include "gadget_chips.h"
#include "composite.c"

#define MANUFACTURER_IDX	0
#define PRODUCT_IDX		1

static const char shortname[] = {"jz_usb_burner_"};
static const char manufacturer[] = {"Ingenic"};
static const char product[] = {CONFIG_BURNER_PRIDUCT_INFO};

struct usb_device_descriptor device_desc= {
	.bLength = sizeof(device_desc),
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = __constant_cpu_to_le16(0x0200),
	.idVendor = __constant_cpu_to_le16(CONFIG_USB_VENDOR_ID),
	.idProduct = __constant_cpu_to_le16(CONFIG_USB_PRODUCT_ID),
	.bNumConfigurations = 1,
};

struct usb_string g_bt_string_defs[] = {
	[MANUFACTURER_IDX].s = manufacturer,
	[PRODUCT_IDX].s = product,
	{}
};

struct usb_gadget_strings g_bt_string = {
	.language = 0x0409, /* en-us */
	.strings = g_bt_string_defs,
};

struct usb_gadget_strings *g_bt_string_tab[] = {
	&g_bt_string,
	NULL,
};

static struct usb_composite_dev *g_cdev;

extern int jz_cloner_add(struct usb_configuration *c);
static int g_do_burntool_config(struct usb_configuration *c)
{
	int ret = -ENODEV;
	const char *s = c->cdev->driver->name;
	debug("%s: configuration: 0x%p composite dev: 0x%p\n",
			__func__, c, c->cdev);

	if (!strcmp(s, "jz_usb_burner_vdr")) {
		 ret = jz_cloner_add(c);
	}
	return ret;
}

static void g_do_burntool_unconfig(struct usb_configuration *cdev)
{
	return;
}

static int g_burntool_config(struct usb_composite_dev *cdev)
{
	static struct usb_configuration config = {
		.label = "usb_burntool",
		.bConfigurationValue = 1,
		.bmAttributes	=  USB_CONFIG_ATT_ONE,
		.bMaxPower	=  0xFA,
		.bind	=	g_do_burntool_config,
		.unbind =	g_do_burntool_unconfig,
	};
	return usb_add_config(cdev,&config);
}

static int burntool_unbind(struct usb_composite_dev * cdev)
{
	struct usb_gadget *gadget = cdev->gadget;
	debug("%s: calling usb_gadget_disconnect for "
			"controller '%s'\n", shortname, gadget->name);
	g_cdev = NULL;
	return usb_gadget_disconnect(gadget);
}

static int burntool_bind(struct usb_composite_dev * cdev)
{
	struct usb_gadget *gadget = cdev->gadget;
	int id = -ENODEV;
	int ret,gcnum;

	debug("%s: gadget: 0x%p cdev: 0x%p\n", __func__, gadget, cdev);

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	g_bt_string_defs[MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	g_bt_string_defs[PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	device_desc.iSerialNumber = 0;

	g_cdev = cdev;
	ret = g_burntool_config(cdev);
	if (ret < 0)
		goto error;

	gcnum = usb_gadget_controller_number(gadget);
	debug("gcnum: %d\n", gcnum);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0200 + gcnum);
	else
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	debug("%s: calling usb_gadget_connect for "
			"controller '%s'\n", shortname, gadget->name);

	/* udc is already connect in bootrom*/
	//usb_gadget_connect(gadget);
	return 0;
error:
	burntool_unbind(cdev);
	return -ENOMEM;
}

static struct usb_composite_driver g_burntool_driver = {
	.name = NULL,
	.dev = &device_desc,
	.strings = g_bt_string_tab,
	.bind = burntool_bind,
	.unbind = burntool_unbind,
};

int g_burntool_register(const char *type)
{
	static char name[sizeof(shortname) + 3];
	int ret;

	if (!strcmp(type, "vdr")) {//vendor
		strcpy(name, shortname);
		strcat(name, type);
	} else {
		printf("%s: unknown command: %s\n", __func__, type);
	}
	g_burntool_driver.name = name;
	debug("%s: g_dnl_driver.name: %s\n", __func__, g_burntool_driver.name);

	ret = usb_composite_register(&g_burntool_driver);
	if (ret) {
		printf("%s: failed!, error: %d\n", __func__, ret);
		return ret;
	}
	return 0;
}

void  g_burntool_virtual_set_config(const char *type)
{
	struct usb_composite_dev *cdev = g_cdev;
	struct usb_configuration *c = NULL;
	struct usb_function *f = NULL;
	struct usb_descriptor_header **descriptors = NULL;
	u32 tmp = 0;

	if (!cdev) {
		printf("BURNTOOL: there no cdev\n");
		return;
	}
	/*virtual set config because enumned in bootrom*/
	list_for_each_entry(c, &cdev->configs, list) {
		if (c->bConfigurationValue == 1) {
			break;
		}
	}
	if (!c) {
		printf("BURNTOOL: there no default 1 configuration\n");
		return;
	}

	/*set config*/
	cdev->config = c;
	/*set_alt*/
	for (tmp = 0; tmp < MAX_CONFIG_INTERFACES; tmp++) {
		f = c->interface[tmp];
		if (!f)
			break;

		if (cdev->gadget->speed == USB_SPEED_HIGH)
			descriptors = f->hs_descriptors;
		else
			descriptors = f->descriptors;

		for (; *descriptors; ++descriptors) {
			struct usb_endpoint_descriptor *ep;
			u32 addr;
			if ((*descriptors)->bDescriptorType != USB_DT_ENDPOINT)
				continue;

			ep = (struct usb_endpoint_descriptor *)*descriptors;
			addr = ((ep->bEndpointAddress & 0x80) >> 3)
				|	(ep->bEndpointAddress & 0x0f);
			__set_bit(addr, f->endpoints);
		}

		if (f->set_alt(f, tmp, 0)) {
			list_for_each_entry(f, &cdev->config->functions, list) {
				if (f->disable)
					f->disable(f);

				bitmap_zero(f->endpoints, 32);
			}
			cdev->config = NULL;
			return;
		}
	}

	return;
}

void g_burntool_unregister(void)
{
	usb_composite_unregister(&g_burntool_driver);
	return;
}
