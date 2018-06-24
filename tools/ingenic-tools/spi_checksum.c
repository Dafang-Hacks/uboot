/*
 * SPI SPL check tool.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Based on: u-boot-1.1.6/tools/spi_checksum/spi_checksum.c
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

#include <fcntl.h>
#include <stdio.h>
#include <config.h>
#include <string.h>
#include <unistd.h>

#ifdef CONFIG_JZ4775
#define SKIP_SIZE 16
#endif
#ifdef CONFIG_JZ4780
#define SKIP_SIZE 16
#endif
#ifdef CONFIG_M200
#define SKIP_SIZE 2048
#endif
#ifdef CONFIG_T10
#define SKIP_SIZE 2048
#endif
#ifdef CONFIG_T15
#define SKIP_SIZE 2048
#endif
#ifdef CONFIG_T20
#define SKIP_SIZE 2048
#endif

#define BUFFER_SIZE 4

#ifndef CONFIG_SPL_SFC_SUPPORT
int main(int argc, char *argv[])
{
	int fd, count;
	int bytes_read;
	char buffer[BUFFER_SIZE];
	unsigned int check = 0;
	volatile int t = 0;

	if (argc != 2) {
		printf("Usage: %s fromfile tofile\n\a",argv[0]);
		return 1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		printf("Open %s Error\n", argv[1]);
		return 1;
	}

	count = 0;

	while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
		if (t >= SKIP_SIZE)
			check += *((unsigned int *)buffer);
		else
			t += BUFFER_SIZE;

		count += bytes_read;
	}

	printf("spi spl count = %d \n", count);
	printf("spi spl check = %#x \n", check);

	lseek(fd, 8, SEEK_SET);

	if ((t = write(fd, &count, 4)) != 4) {
		printf("Write %s Error\n",argv[1]);
		return 1;
	}

	check = 0 - check;
	if ((t = write(fd, &check, 4)) != 4) {
		printf("Check: Write %s Error\n",argv[1]);
		return 1;
	}

#if 0
	lseek( fd, 8, SEEK_SET);

	if ((t = read(fd,buffer,BUFFER_SIZE) < 0)) {
		printf("read %d \n",t);
	}
	printf("%#x\t", *(unsigned int *)buffer);

	if ((t = read(fd,buffer,BUFFER_SIZE) < 0)) {
		printf("read %d \n",t);
	}
	printf("%#x\n", *(unsigned int *)buffer);

#endif
	close(fd);

	return 0;
}
#else
#define CRC_POSITION		9	/*  9th bytes */
#define SPL_LENGTH_POSITION	12	/* 12th bytes */

typedef unsigned char u8;

static const u8 crc7_syndrome_table[256] = {
	0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
	0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
	0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
	0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
	0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
	0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
	0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
	0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
	0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
	0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
	0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
	0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
	0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
	0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
	0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
	0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
	0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
	0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
	0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
	0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
	0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
	0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
	0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
	0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
	0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
	0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
	0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
	0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
	0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
	0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
	0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
	0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};

static inline u8 crc7_byte(u8 crc, u8 data)
{
	return crc7_syndrome_table[(crc << 1) ^ data];
}

static u8 crc7(u8 crc, u8 *buffer, int len)
{
	while (len--)
		crc = crc7_byte(crc, *buffer++);
	return crc;
}

int main(int argc, char *argv[])
{
	u8 crc, buffer[BUFFER_SIZE];
	int t, fd, count, bytes_read;

	if (argc != 2) {
		printf("Usage: %s fromfile tofile\n\a",argv[0]);
		return 1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		printf("Open %s Error\n", argv[1]);
		return 1;
	}

	t = 0;
	crc = 0;
	count = 0;

	while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
		if (t >= SKIP_SIZE) {
			crc = crc7(crc, buffer, bytes_read);
		} else {
			t += BUFFER_SIZE;
		}
		count += bytes_read;
	}

	printf("spi spl crc7 = %x \n", crc);
	printf("spi spl count = %08x \n", count);

	/*set crc*/
	lseek(fd, CRC_POSITION, SEEK_SET);

	if (write(fd, &crc, 1) != 1) {
		printf("Write %s Error\n",argv[1]);
		return 1;
	}

	/*set spl len*/
	lseek(fd, SPL_LENGTH_POSITION, SEEK_SET);
	if (write(fd, &count, 4) != 4) {
		printf("Check: Write %s Error\n",argv[1]);
		return 1;
	}

	close(fd);

	return 0;
}
#endif
