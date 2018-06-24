/*
 * Command for mmc_spi setup.
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <spi.h>
extern void spi_load(unsigned int src_addr, unsigned int count, unsigned int dst_addr);
static int do_spinor(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int src_addr,count,dst_addr;
	if(argc < 5){
		return CMD_RET_USAGE;
	}
	if(!strcmp(argv[1],"read")){
		src_addr = simple_strtoul(argv[2],NULL,16);
		count = simple_strtoul(argv[3],NULL,16);
		dst_addr = simple_strtoul(argv[4],NULL,16);
		printf("nor load Image form 0x%x to  0x%x size is 0x%x ...\n",src_addr,dst_addr,count);
		spi_load(src_addr,count,dst_addr);
		printf("Image load ok!\n");
		return 0;
	}else{
		return CMD_RET_USAGE;
	}
usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	spinor, 5,	0,	do_spinor,
	"load Image from spi nor",
	"spinor read [src:0x..] [bytes:0x..] [dst:0x..]"
);
