/*
 * (C) Copyright 2003
 * cli@ingenic.cn
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
#include <efuse.h>

enum efuse_state {
	EFUSE_INVALID = 0,
	EFUSE_INIT,
	EFUSE_READ,
	EFUSE_WRITE,
	EFUSE_DEBUG,
};

static int do_efuse(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	enum efuse_state state;
	void *addr = NULL;
	int length = 0;
	off_t offset = 0;
	int pin = -1;
	int enable = 0;
	int ret = 0;

	state = EFUSE_INVALID;
	if (argc == 5 && strcmp(argv[1], "read") == 0)
		state = EFUSE_READ;
	else if (argc == 5 && strcmp(argv[1], "write") == 0)
		state = EFUSE_WRITE;
	else if ((argc == 3 || argc == 2)&& strcmp(argv[1], "init") == 0)
		state = EFUSE_INIT;
	else if (argc == 3 && strcmp(argv[1], "debug") == 0)
		state = EFUSE_DEBUG;

	switch (state) {
	case EFUSE_READ:
	case EFUSE_WRITE:
		addr = (void *)simple_strtoul(argv[2], NULL, 16);
		length = (int)simple_strtoul(argv[3], NULL, 10);
		offset = (int)simple_strtoul(argv[4], NULL, 16);
		if (state == EFUSE_READ) {
			printf("efuse read %d byte data from offset 0x%x of efuse to addr 0x%p of memory\n",
					length,(int)offset,addr);
			ret = efuse_read(addr, length, offset);
		} else {
			printf("efuse write %d byte data from  addr 0x%p of memory to offset 0x%x of efuse\n",
					length,addr,(int)offset);
			ret = efuse_write(addr, length, offset);
		}
		if (ret)
			printf("efuse ops failed\n");
		break;
	case EFUSE_INIT:
		if (argc == 3) {
			pin = (int)simple_strtoul(argv[2], NULL, 10);
		} else {
#if defined(CONFIG_EFUSE_GPIO)
			pin = CONFIG_EFUSE_GPIO;
#endif
		}
		ret = efuse_init(pin);
		if (ret)
			printf("efuse init failed\n");
		break;
	case EFUSE_DEBUG:
		enable = (int)simple_strtoul(argv[2], NULL, 10);
		efuse_debug_enable(enable);
		break;
	default:
		return CMD_RET_USAGE;
	}
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	efuse, 5, 1, do_efuse,
	"efuse subsystem,try efuse --help\n",
	"efuse init pin\n"
	"efuse debug is_open\n"
	"efuse read addr len off\n"
	"efuse write addr len off\n"
	"pin : efuse gpio pin num\n"
	"addr: the address of data store in memory\n"
	"len: the length of data to be read/write\n"
	"off: the offset of data to be read/write in efuse\n"
	);
