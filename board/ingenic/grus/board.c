/*
 * Ingenic grus setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <ptkang@ingenic.cn>
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

#include <common.h>
#include <nand.h>
#include <net.h>
#include <netdev.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/arch/nand.h>
#include <asm/arch/mmc.h>
#include <spi_flash.h>

extern int act8600_regulator_init(void);
extern int jz_net_initialize(bd_t *bis);
#ifdef CONFIG_BOOT_ANDROID
extern void boot_mode_select(void);
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
static void battery_init_gpio(void)
{
}
#endif

int board_early_init_f(void)
{
	/* Power on TF-card */
	gpio_direction_output(GPIO_PF(19), 1);
	act8600_regulator_init();

	return 0;
}

#ifdef CONFIG_USB_GADGET
int jz_udc_probe(void);
void board_usb_init(void)
{
	printf("USB_udc_probe\n");
	jz_udc_probe();
}
#endif /* CONFIG_USB_GADGET */

int misc_init_r(void)
{
#if 0 /* TO DO */
	uint8_t mac[6] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };

	/* set MAC address */
	eth_setenv_enetaddr("ethaddr", mac);
#endif
#ifdef CONFIG_BOOT_ANDROID
	boot_mode_select();
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
	battery_init_gpio();
#endif
	return 0;
}

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rv;
#ifndef  CONFIG_USB_ETHER
	/* reset grus DM9000 */
	gpio_direction_output(CONFIG_GPIO_DM9000_RESET, CONFIG_GPIO_DM9000_RESET_ENLEVEL);
	mdelay(10);
	gpio_set_value(CONFIG_GPIO_DM9000_RESET, !CONFIG_GPIO_DM9000_RESET_ENLEVEL);
	mdelay(10);

	/* init grus gpio */
	gpio_set_func(GPIO_PORT_A, GPIO_FUNC_0, 0x040300ff);
	gpio_set_func(GPIO_PORT_B, GPIO_FUNC_0, 0x00000002);
	rv = dm9000_initialize(bis);
#else
	rv = usb_eth_initialize(bis);
#endif
	return rv;
}

/* U-Boot common routines */
int checkboard(void)
{
	puts("Board: grus (Ingenic XBurst JZ4780 SoC)\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
}
#endif /* CONFIG_SPL_BUILD */

#ifdef CONFIG_SPL_SPI_SUPPORT
void spl_spi_load_image(void)
{
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	spl_parse_image_header(header);

	spi_load(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN, CONFIG_SYS_TEXT_BASE);
}
#endif

#ifdef CONFIG_SOFT_SPI
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	gpio_direction_input(GPIO_PA(20));
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	gpio_direction_output(GPIO_PA(23), 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	gpio_direction_output(GPIO_PA(23), 1);
}

struct spi_flash *spi_flash_probe_ingenic(struct spi_slave *spi, u8 *idcode)
{
	struct spi_flash *flash;

	flash = spi_flash_alloc_base(spi, "ingenic");
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	flash->page_size = 256;
	flash->sector_size = 4 * 1024;
	flash->size = 32 * 1024 * 1024;

	return flash;
}
#endif
