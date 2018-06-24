/*
 * MBR Creator.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

#define KBYTE			(1024LL)
#define MBYTE			((KBYTE)*(KBYTE))

#define LINUX_FS		0x83
#define FAT_FS			0x0b

#define ARRAY_SIZE(x)		((sizeof(x))/(sizeof(x[0])))

struct mbr_tab_item {
	unsigned long	offset;
	unsigned long	size;
	unsigned char	type;
};

struct mbr_tab_item	tab_item[4];

static int parse_mbr_string(const char *str) {
	int i1 = -1, i2 = -1, i3 = -1;
	int items = -1;
	unsigned long long offset = 0, end = 0;
	char type_str[100] = {0};

	/*
	 * format:p1off=xmb,p1end=ymb,p1type=fat
	 */
	printf("str =%s\n", str);
	if ((items = sscanf(str, "p%doff=%lldmb,p%dend=%lldmb,p%dtype=%s", &i1, &offset, &i2, &end, &i3, type_str)) != 6) {
		printf("itmes = %d\n", items);
		printf("i1 = %d\n", i1);
		printf("offset_str = %lld\n", offset);
		printf("i2 = %d\n", i2);
		printf("end_str = %lld\n", end);
		printf("i3 = %d\n", i3);
		printf("type_str = %s\n", type_str);
		printf("Error:%s is error format\n", str);
		printf("right format:pxoff=\"xmb\",pxend=\"ymb\",pxtype=\"fat\", x=1,2,3,4\n");
		return -1;
	}
	printf("itmes = %d\n", items);
	printf("i1 = %d\n", i1);
	printf("offset_str = %lld\n", offset);
	printf("i2 = %d\n", i2);
	printf("end_str = %lld\n", end);
	printf("i3 = %d\n", i3);
	printf("type_str = %s\n", type_str);

	tab_item[i1].offset = offset * MBYTE / 512;
	tab_item[i1].size = (end - offset) * MBYTE / 512;
	if (strcmp(type_str, "fat") == 0) {
		tab_item[i1].type = FAT_FS;
	}else if (strcmp(type_str, "linux") == 0) {
		tab_item[i1].type = LINUX_FS;
	}else {
		printf("no compatible filesystem type\n");
		return -1;
	}
	printf("tab_item[%d].offset = %u\n", i1, tab_item[i1].offset);
	printf("tab_item[%d].size = %u\n", i1, tab_item[i1].size);
	printf("tab_item[%d].type = %u\n", i1, tab_item[i1].type);

	return 0;
}
int main(int argc, char *argv[])
{
	int	i = 0;
	int	fd = -1;
	unsigned char	block[512];
	char	mbr_name[512] = {0};

	memset(tab_item, 0, ARRAY_SIZE(tab_item));
	memset(block, 0, ARRAY_SIZE(block));
	printf("argc = %d\n", argc);

	if (argc > 2) {
		if (strcmp("-o", argv[argc - 2]) == 0) {
			strcpy(mbr_name, argv[argc - 1]);
			argc -= 2;
		}else {
			strcpy(mbr_name, "mbr.bin");
		}
	}else {
		strcpy(mbr_name, "mbr.bin");
	}

	printf("argc = %d\n", argc);
	printf("mbr_name = %s\n", mbr_name);

	for (i = 1; i < argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
		/*
		 * format:p1off="xmb",p1end="ymb",p1type="fat"
		 */
		if (parse_mbr_string(argv[i]) < 0) {
			printf("Error:%s is error format\n", argv[i]);
			printf("right format:pxoff=\"xmb\",pxend=\"ymb\",pxtype=\"fat\", x=1,2,3,4\n");
			return  -1;
		}
	}

	block[0x1fe] = 0x55;                                                                                                             
	block[0x1ff] = 0xaa;
	memcpy(block+0x1c6,&tab_item[0].offset,sizeof(unsigned int));
	memcpy(block+0x1d6,&tab_item[1].offset,sizeof(unsigned int));
	memcpy(block+0x1e6,&tab_item[2].offset,sizeof(unsigned int));
	memcpy(block+0x1f6,&tab_item[3].offset,sizeof(unsigned int));

	memcpy(block+0x1ca,&tab_item[0].size,sizeof(unsigned int));
	memcpy(block+0x1da,&tab_item[1].size,sizeof(unsigned int));
	memcpy(block+0x1ea,&tab_item[2].size,sizeof(unsigned int));
	memcpy(block+0x1fa,&tab_item[3].size,sizeof(unsigned int));

	memcpy(block+0x1c2,&tab_item[0].type,sizeof(unsigned char));
	memcpy(block+0x1d2,&tab_item[1].type,sizeof(unsigned char));
	memcpy(block+0x1e2,&tab_item[2].type,sizeof(unsigned char));
	memcpy(block+0x1f2,&tab_item[3].type,sizeof(unsigned char));

	fd = open(mbr_name,O_RDWR | O_TRUNC | O_CREAT,0666);
	if(fd < 0)
	{
		printf("open %s failed.\n",mbr_name);
		exit(1);
	}

	if(write(fd,block,512) != 512)
		printf("write %s failed.\n",mbr_name);

	close(fd);

	return 0;
}
