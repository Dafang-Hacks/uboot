
/*
 * Ingenic burn command
 *
 * Copyright (c) 2013 jykang <jykang@ingenic.cn>
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

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/errno.h>

#include <asm/arch/gpio.h>
#include <asm/arch/cpm.h>

int start_serial_cloner();
static int serial_burner()
{
	start_serial_cloner();
}

static int do_serial_burn(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *str_env;
	char *s = "vdr";	//vendor
	char *env_bkp;
	int ret;

	if (argc > 1)
		return CMD_RET_USAGE;
	serial_burner();
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(serial_burn, CONFIG_SYS_MAXARGS, 1, do_serial_burn,
	"Ingenic serial burner",
	"No params\n"
);
