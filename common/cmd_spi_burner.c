/*
 * Ingenic JZ SPI burner
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Tiger <lbh@ingenic.cn>
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
#include <div64.h>
#include <malloc.h>
#include <spi_flash.h>
#include <asm/io.h>

static int spi_burner_nand(int argc, char * const argv[])
{
	int ret;
	char *endp;
	struct spi_flash *flash;

	flash = spi_flash_probe(0, 0, 0, 0);
	if (!flash)
		return -1;

	unsigned long addr = simple_strtoul(argv[2], &endp, 16);
	if (*argv[1] == 0 || *endp != 0) {
		printf("addr error !\n");
		return -1;
	}

	unsigned long offset = simple_strtoul(argv[3], &endp, 16);
	if (*argv[1] == 0 || *endp != 0) {
		printf("offset error !\n");
		return -1;
	}

	unsigned long len = simple_strtoul(argv[4], &endp, 16);
	if (*argv[1] == 0 || *endp != 0) {
		printf("len error !\n");
		return -1;
	}

	ret = spi_flash_erase(flash, offset, len);
	if(ret) {
		printf("erase error !\n");
		return -1;
	}

	printf("spi_burner: 0x%x bytes @ %#x Erased: OK\n", len, (u32)offset);

	ret = spi_flash_write(flash, offset, len, addr);
	if(ret) {
		printf("write error !\n");
		return -1;
	}

	printf("spi_burner: 0x%x bytes @ %#x Write at 0x%x: OK\n", len, (u32)offset, addr);

	return 0;
}

static int do_spi_burner(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	const char *cmd;

	if (argc < 2)
		goto usage;

	if (!strcmp(argv[1], "nand")) {
		ret = spi_burner_nand(argc, argv);
		if(ret < 0) {
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	spi_burner,	5,	1,	do_spi_burner,
	"SPI burner sub-system\n",
	"spi_burner nand addr offset len\n"
	"write `len' bytes from memory at `addr' to spi-flash at `offset'\n"
);
