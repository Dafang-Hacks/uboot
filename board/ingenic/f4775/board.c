/*
 * Ingenic mensa setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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
#include <asm/arch/cpm.h>
#include <asm/arch/nand.h>
#include <asm/arch/mmc.h>

extern int jz_net_initialize(bd_t *bis);
int board_early_init_f(void)
{
	return 0;
}

int misc_init_r(void)
{
	return 0;
}

int board_nand_init(struct nand_chip *nand)
{
	return 0;
}


#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

#ifdef CONFIG_SPL_SPI_SUPPORT
void spl_spi_load_image(void)
{
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	spl_parse_image_header(header);

	spi_load(CONFIG_SPL_PAD_TO, CONFIG_SYS_MONITOR_LEN, CONFIG_SYS_TEXT_BASE);
}
#endif

#ifdef CONFIG_SPL_NOR_SUPPORT
int spl_start_uboot(void)
{
	return 1;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rv;
	rv = jz_net_initialize(bis);
	return rv;
}

/* U-Boot common routines */
int checkboard(void)
{
	puts("Board: f4775 (Ingenic XBurst JZ4775 SoC)\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
}
#endif /* CONFIG_SPL_BUILD */
