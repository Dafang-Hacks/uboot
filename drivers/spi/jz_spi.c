/*
 * Ingenic JZ SPI driver
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

#include <config.h>
#include <common.h>
#include <spi.h>
#include <spi_flash.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/arch/spi.h>
#include <asm/arch/clk.h>
#include <asm/arch/base.h>
#include <malloc.h>
#include "jz_spi.h"

static struct jz_spi_support *gparams;

/* wait time before read status (us) for spi nand */
//static int t_reset = 500;
static int t_read  = 120;
static int t_write = 700;
static int t_erase = 5000;

static uint32_t jz_spi_readl(unsigned int offset)
{
	return readl(SSI_BASE + offset);
}

static void jz_spi_writel(unsigned int value, unsigned int offset)
{
	writel(value, SSI_BASE + offset);
}

static void jz_spi_flush(void )
{
	jz_spi_writel(jz_spi_readl(SSI_CR0) | SSI_CR0_TFLUSH | SSI_CR0_RFLUSH, SSI_CR0);
}

static unsigned int spi_rxfifo_empty(void )
{
	return (jz_spi_readl(SSI_SR) & SSI_SR_RFE);
}

static unsigned int spi_txfifo_full(void )
{
	return (jz_spi_readl(SSI_SR) & SSI_SR_TFF);
}

static unsigned int spi_get_rxfifo_count(void )
{
	return ((jz_spi_readl(SSI_SR) & SSI_SR_RFIFONUM_MASK) >> SSI_SR_RFIFONUM_BIT);
}

static inline struct jz_spi_slave *to_jz_spi(struct spi_slave *slave)
{
	return container_of(slave, struct jz_spi_slave, slave);
}

#ifdef CONFIG_SOFT_SPI
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	gpio_direction_input(GPIO_PE(20));	//spi-dr
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
	gpio_direction_output(GPIO_PE(29), 0); //cs
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	gpio_direction_output(GPIO_PE(29), 1); //cs
}
#endif

static int mem_compare(const void *buf0, const void *buf1, int len) {
	int i, errflag = 0, len1, len2;
	char *buf0_t2, *buf1_t2;
	int *buf0_t1 = (int *)buf0;
	int *buf1_t1 = (int *)buf1;

	len1 = len / sizeof(int);
	for (i = 0; i < len1; i++) {
		if (*buf0_t1++ != *buf1_t1++) {
			errflag |= 1;
			printf("ERROR: %08lx: %08lx | %08lx\n", i, *(buf1_t1 - 1), *(buf0_t1 - 1));
		}
		/*
		  if (i && !(i % 4)) {
		  printf("%s%08lx: %08lx %08lx %08lx %08lx | %08lx %08lx %08lx %08lx\n",
		  errflag ? "[ERROR]" : "[OK]", i - 4,
		  *(buf1_t1 - 4), *(buf1_t1 - 3), *(buf1_t1 - 2), *(buf1_t1 - 1),
		  *(buf0_t1 - 4), *(buf0_t1 - 3), *(buf0_t1 - 2), *(buf0_t1 - 1));
		  }
		*/
	}

	len2 = len % sizeof(int);
	if (len2) {
		buf0_t2 = (char *)buf0_t1;
		buf1_t2 = (char *)buf1_t1;
		for (i = 0; i < len2; i++) {
			if (*buf0_t2++ != *buf1_t2++) {
				errflag |= 1;
			}
		}
	}

	return errflag;
}

void spi_init(void )
{
#if DEBUG
	unsigned int errorpc;
	__asm__ __volatile__ (
			"mfc0  %0, $30,  0   \n\t"
			"nop                  \n\t"
			:"=r"(errorpc)
			:);

	printf("RESET ERROR PC:%x\n",errorpc);
#endif

#ifndef CONFIG_BURNER
	unsigned int ssi_rate = 70000000;
	clk_set_rate(SSI, ssi_rate);
#endif
	jz_spi_writel(~SSI_CR0_SSIE & jz_spi_readl(SSI_CR0), SSI_CR0);
	jz_spi_writel(11, SSI_GR);
	jz_spi_writel(SSI_CR0_EACLRUN | SSI_CR0_RFLUSH | SSI_CR0_TFLUSH, SSI_CR0);
	jz_spi_writel(SSI_FRMHL_CE0_LOW_CE1_LOW | SSI_GPCMD | SSI_GPCHL_HIGH | SSI_CR1_TFVCK_3 | SSI_CR1_TCKFI_3 | SSI_CR1_FLEN_8BIT | SSI_CR1_PHA | SSI_CR1_POL, SSI_CR1);
	jz_spi_writel(SSI_CR0_SSIE | jz_spi_readl(SSI_CR0), SSI_CR0);
}

void spi_send_cmd(unsigned char *cmd, unsigned int count)
{
	unsigned int sum = count;
	jz_spi_flush();
	while(!spi_rxfifo_empty());
	while(count) {
		jz_spi_writel(*cmd, SSI_DR);
		while (spi_txfifo_full());
		cmd++;
		count--;
	}
	while (spi_get_rxfifo_count() != sum);
}

void spi_recv_cmd(unsigned char *read_buf, unsigned int count)
{
	unsigned int offset = 0;
	jz_spi_flush();
	while(!spi_rxfifo_empty());
	while (count) {
		jz_spi_writel(0, SSI_DR);
		while (spi_rxfifo_empty())
			;
		writeb(jz_spi_readl(SSI_DR), read_buf + offset);
		offset++;
		count--;
	}
}

void spi_load(unsigned int src_addr, unsigned int count, unsigned int dst_addr)
{
	unsigned char cmd;
	cmd = CMD_READ;
	src_addr = ((src_addr & 0xFF) << 16) | (src_addr & 0x0000FF00) | ((src_addr >> 16) & 0xFF);
	spi_init();
	jz_spi_writel(jz_spi_readl(SSI_CR1) | SSI_CR1_UNFIN, SSI_CR1);
	spi_send_cmd(&cmd, 1);
	spi_send_cmd((unsigned char *)&src_addr, 3);
	spi_recv_cmd((unsigned char *)dst_addr, count);
	jz_spi_writel(jz_spi_readl(SSI_CR1) & (~SSI_CR1_UNFIN), SSI_CR1);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	spi_init();
	struct jz_spi_slave *ss;

	ss = spi_alloc_slave(struct jz_spi_slave, bus, cs);
	if (!ss)
		return NULL;

	ss->mode = mode;
	ss->max_hz = max_hz;

	return &ss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct jz_spi_slave *ss = to_jz_spi(slave);

	free(ss);
}

int spi_claim_bus(struct spi_slave *slave)
{
	jz_spi_writel(jz_spi_readl(SSI_CR1) | SSI_CR1_UNFIN, SSI_CR1);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	jz_spi_writel(jz_spi_readl(SSI_CR1) & (~SSI_CR1_UNFIN), SSI_CR1);
	jz_spi_writel(jz_spi_readl(SSI_SR) & (~SSI_SR_UNDR) , SSI_SR);
}

int  spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	unsigned int count = bitlen / 8;
	unsigned char *cmd = (unsigned char *)dout;
	unsigned char *addr = din;
	unsigned int fifo_size = 64;

	if(dout != NULL) {
		while(count) {
			if(count > fifo_size) {
				spi_send_cmd(cmd, fifo_size);
				cmd += fifo_size;
				count -= fifo_size;
			} else {
				spi_send_cmd(cmd, count);
				break;
			}
		}
	}

	if(din != NULL) {
		spi_recv_cmd((unsigned char *)addr, count);
	}

	return 0;
}

static void jz_cs_reversal(void )
{
	spi_release_bus(NULL);

	udelay(1);

	spi_claim_bus(NULL);

	return ;
}

int jz_read_nand(struct spi_flash *flash, u32 offset, size_t len, void *data)
{
	unsigned int page_bak, page, page_num, column, i;
	unsigned char read_buf, cmd[COMMAND_MAX_LENGTH];

	if(offset % flash->page_size) {
		printf("offset must 0x%x align !\n", flash->page_size);
		return -1;
	}

	if(len % flash->page_size) {
		printf("len must 0x%x align !\n", flash->page_size);
		return -1;
	}

	column = 0;
	page_bak = offset / flash->page_size;
	page_num = len / flash->page_size;

	for(i = 0; i < page_num; i++) {
		if(page_bak > gparams->page_num) {
			printf("page : %x\n", page_bak);
			printf("page_num : %x\n", gparams->page_num);
			printf("space is full !\n");
			break;
		}

		page = gparams->page_list[page_bak];

		jz_cs_reversal();
		cmd[0] = 0x13;
		cmd[1] = (page >> 16) & 0xff;
		cmd[2] = (page >> 8) & 0xff;
		cmd[3] = page & 0xff;
		spi_send_cmd(cmd, 4);
		udelay(t_read);

		jz_cs_reversal();
		cmd[0] = 0x0f;
		cmd[1] = 0xc0;
		spi_send_cmd(cmd, 2);

		spi_recv_cmd(&read_buf, 1);
		while(read_buf & 0x1)
			spi_recv_cmd(&read_buf, 1);

		if((read_buf & 0x30) == 0x20) {
			printf("read error !!!\n");
			printf("please reset !!!\n");
			return -1;
		}

		jz_cs_reversal();
		cmd[0] = 0x03;
		cmd[1] = (column >> 8) & 0xf;
		cmd[1] |= 0x40;
		cmd[2] = column & 0xff;
		cmd[3] = 0x0;
		spi_send_cmd(cmd, 4);
		spi_recv_cmd(data, flash->page_size);
		data += flash->page_size;

		page_bak++;
	}

	return 0;
}

int jz_read(struct spi_flash *flash, u32 offset, size_t len, void *data)
{
	unsigned char cmd[5];
	unsigned long read_len;

	cmd[0] = CMD_FAST_READ;
	cmd[1] = offset >> 16;
	cmd[2] = offset >> 8;
	cmd[3] = offset >> 0;
	cmd[4] = 0x00;

	read_len = flash->size - offset;

	if(len < read_len)
		read_len = len;

	jz_cs_reversal();
	spi_send_cmd(&cmd[0], 5);
	spi_recv_cmd(data, read_len);

	return 0;
}

int jz_write(struct spi_flash *flash, u32 offset, size_t len, const void *buf)
{
	unsigned char cmd[6], tmp;
	int chunk_len, actual, i;
	unsigned long byte_addr, page_size;
	unsigned char *send_buf = (unsigned char *)buf;

	page_size = flash->page_size;

	cmd[0] = CMD_WREN;

	cmd[1] = CMD_PP;

	cmd[5] = CMD_RDSR;

	for (actual = 0; actual < len; actual += chunk_len) {
		byte_addr = offset % page_size;
		chunk_len = min(len - actual, page_size - byte_addr);

		cmd[2] = offset >> 16;
		cmd[3] = offset >> 8;
		cmd[4] = offset >> 0;

		jz_cs_reversal();
		spi_send_cmd(&cmd[0], 1);

		jz_cs_reversal();
		spi_send_cmd(&cmd[1], 4);


		for(i = 0; i < chunk_len; i += 100) {
			if((chunk_len - i) < 100)
				spi_send_cmd((send_buf + actual + i), (chunk_len - i));
			else
				spi_send_cmd((send_buf + actual + i), 100);

		}

		jz_cs_reversal();
		spi_send_cmd(&cmd[5], 1);
		spi_recv_cmd(&tmp, 1);
		while(tmp & CMD_SR_WIP) {
			spi_recv_cmd(&tmp, 1);
		}

		offset += chunk_len;
	}
	return 0;
}

int jz_erase_nand(struct spi_flash *flash, u32 offset, size_t len)
{
	unsigned int block_bak, block, block_num, i;
	unsigned char cmd[COMMAND_MAX_LENGTH], read_buf;

	printf("jz_erase_nand: flash->sector_size = [%d], offset = [%d], len = [%d]\n", flash->sector_size, offset, len);

	if(offset % flash->sector_size) {
		printf("offset must 0x%x align !\n", flash->sector_size);
		return -1;
	}

	if(len % flash->sector_size) {
		printf("len must 0x%x align, len = [%d] !\n", flash->sector_size, len);
		return -1;
	}

	block_bak = offset / flash->page_size;
	block_num = len / flash->sector_size;

	for(i = 0; i < block_num; i++) {
		if(block_bak > gparams->page_num) {
			printf("space is full !\n");
			break;
		}

		block = gparams->page_list[block_bak];
		jz_cs_reversal();
		cmd[0] = 0x06;
		spi_send_cmd(cmd, 1);

		jz_cs_reversal();
		cmd[0] = 0xd8;
		cmd[1] = (block >> 16) & 0xff;
		cmd[2] = (block >> 8) & 0xff;
		cmd[3] = block & 0xc0;
		spi_send_cmd(cmd, 4);
		udelay(t_erase);

		jz_cs_reversal();
		cmd[0] = 0x0f;
		cmd[1] = 0xc0;
		spi_send_cmd(cmd, 2);
		spi_recv_cmd(&read_buf, 1);
		while(read_buf & 0x1)
			spi_recv_cmd(&read_buf, 1);

		if(read_buf & 0x4) {
			printf("erase fail !!!\n");
			printf("please reset !!!\n");
			return -1;
		}

		block_bak += (flash->sector_size / flash->page_size);
	}

	return 0;
}

int jz_write_nand(struct spi_flash *flash, u32 offset, size_t len, const void *buf)
{
	unsigned char *send_buf = (unsigned char *)buf;
	unsigned char read_buf, cmd[COMMAND_MAX_LENGTH];
	unsigned int page_bak, page, page_num, column, i, read_num;
#ifdef CONFIG_SPI_WRITE_CHECK
	int error_count = 0;
#endif

	if(offset % flash->page_size) {
		printf("offset must 0x%x align !\n", flash->page_size);
		return -1;
	}

	if(len % flash->page_size) {
		printf("len must 0x%x align !\n", flash->page_size);
		return -1;
	}
rewrite:
	column = 0;
	page_bak = offset / flash->page_size;
	page_num = len / flash->page_size;

	for(i = 0; i < page_num; i++) {
		if(page_bak > gparams->page_num) {
			printf("page : %x\n", page_bak);
			printf("page_num : %x\n", gparams->page_num);
			printf("space is full !\n");
			break;
		}

		page = gparams->page_list[page_bak];

		jz_cs_reversal();
		cmd[0] = 0x02;
		cmd[1] = (column >> 8) & 0xf;
		cmd[2] = column & 0xff;
		spi_send_cmd(cmd, 3);

		read_num = flash->page_size + 64;
		while(read_num) {
			if(read_num > FIFI_THRESHOLD) {
				spi_send_cmd(send_buf, FIFI_THRESHOLD);
				send_buf += FIFI_THRESHOLD;
				read_num -= FIFI_THRESHOLD;
			} else {
				spi_send_cmd(send_buf, read_num);
				read_num = 0;
			}
		}

		jz_cs_reversal();
		cmd[0] = 0x06;
		spi_send_cmd(cmd, 1);

		jz_cs_reversal();
		cmd[0] = 0x10;
		cmd[1] = (page >> 16) & 0xff;
		cmd[2] = (page >> 8) & 0xff;
		cmd[3] = page & 0xff;
		spi_send_cmd(cmd, 4);
		udelay(t_write);

		jz_cs_reversal();
		cmd[0] = 0x0f;
		cmd[1] = 0xc0;
		spi_send_cmd(cmd, 2);

		spi_recv_cmd(&read_buf, 1);
		while(read_buf & 0x1)
			spi_recv_cmd(&read_buf, 1);

		if(read_buf & 0x8) {
			printf("write fail !!!\n");
			printf("please reset !!!\n");
			return -1;
		}

		page_bak++;
	}

#ifdef CONFIG_SPI_WRITE_CHECK
	printf("write check !\n");

	int j;
	unsigned char *check_buf = (unsigned char *)0x80000000;

	send_buf = (unsigned char *)buf;

	if(error_count > SPI_WRITE_CHECK_TIMES) {
		printf("write error more than %d! times\n", SPI_WRITE_CHECK_TIMES);
		return -1;
	}

#if 0
	for(i = 0; i < len; i += flash->page_size) {
		jz_read_nand(flash, offset, flash->page_size, check_buf);
		for(j = 0; j < flash->page_size; j++) {
			if(strcmp(send_buf[j], check_buf[j])) {
				printf("write error : %x!\n", i);
				error_count++;
				jz_erase_nand(flash, offset, ((len + flash->sector_size - 1)/flash->sector_size) * flash->sector_size);
				goto rewrite;
			}
		}
	}
#else
	for(i = 0; i < page_num; i++) {
		memset(check_buf, 0xff, flash->page_size);
		jz_read_nand(flash, offset + i * flash->page_size, flash->page_size, check_buf);
		printf("write check: check page [%d] ...\n", i);
		if (mem_compare((send_buf + i * flash->page_size), check_buf, flash->page_size)) {
			printf("check error : page [%d]!\n", i);
			error_count++;
			jz_erase_nand(flash, offset, ((len + flash->sector_size - 1)/flash->sector_size) * flash->sector_size);
			goto rewrite;
		}
	}
#endif
#endif
	return 0;
}

int jz_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	unsigned long erase_size;
	unsigned char cmd[6], buf;

#ifdef CONFIG_BURNER
	erase_size = len;
#else
	erase_size = flash->sector_size;
#endif
	if (offset % erase_size || len % erase_size) {
		printf("Erase offset/length not multiple of erase size\n");
		return -1;
	}

	cmd[0] = CMD_WREN;

	switch(erase_size) {
	case 0x1000 :
		cmd[1] = CMD_ERASE_4K;
		break;
	case 0x8000 :
		cmd[1] = CMD_ERASE_32K;
		break;
	case 0x10000 :
		cmd[1] = CMD_ERASE_64K;
		break;
	default:
		printf("unknown erase size !\n");
		return -1;
	}

	cmd[5] = CMD_RDSR;

	while(len) {
		cmd[2] = offset >> 16;
		cmd[3] = offset >> 8;
		cmd[4] = offset >> 0;

		printf("erase %x %x %x %x %x %x %x \n", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], offset);

		jz_cs_reversal();
		spi_send_cmd(&cmd[0], 1);

		jz_cs_reversal();
		spi_send_cmd(&cmd[1], 4);

		jz_cs_reversal();
		spi_send_cmd(&cmd[5], 1);
		spi_recv_cmd(&buf, 1);
		while(buf & CMD_SR_WIP) {
			spi_recv_cmd(&buf, 1);
		}

		offset += erase_size;
		len -= erase_size;
	}

	return 0;
}
#ifdef CONFIG_SPI_FLASH_INGENIC
static void jz_spi_nand_init(void )
{
	unsigned char cmd[COMMAND_MAX_LENGTH];
	unsigned char read_cmd[COMMAND_MAX_LENGTH];

	/* disable write protect */
	jz_cs_reversal();
	cmd[0] = 0x1f;
	cmd[1] = 0xa0;
	cmd[2] = 0x0;
	spi_send_cmd(cmd, 3);

	/* enable ECC */
	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xb0;
	cmd[2] = 0x10;
	spi_send_cmd(cmd, 2);

	/* check */
	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xa0;
	spi_send_cmd(cmd, 2);
	spi_recv_cmd(read_cmd, 1);
	if(read_cmd[0] != 0x0)
		printf("read status 0xa0 : %x\n", read_cmd[0]);

	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xb0;
	spi_send_cmd(cmd, 2);
	spi_recv_cmd(read_cmd, 1);
	if(read_cmd[0] != 0x10)
		printf("read status 0xb0 : %x\n", read_cmd[0]);

}

static void jz_spi_nand_scan_bad_block(struct jz_spi_support *params)
{
	int page, block;
	int page_size = params->page_size;
	int block_size = params->block_size;
	int size = params->size;
	unsigned char cmd[COMMAND_MAX_LENGTH], read_buf;

	params->page_list = calloc(size / page_size, sizeof(unsigned int));
	if(params->page_list == NULL) {
		printf("calloc error !\n");
		return ;
	}

	for(block = 0; block < (size / page_size); block += (block_size / page_size)) {
		/* erase */
		jz_cs_reversal();
		cmd[0] = 0x06;
		spi_send_cmd(cmd, 1);

		jz_cs_reversal();
		cmd[0] = 0xd8;
		cmd[1] = (block >> 16) & 0xff;
		cmd[2] = (block >> 8) & 0xff;
		cmd[3] = block & 0xc0;
		spi_send_cmd(cmd, 4);
		udelay(t_erase);

		jz_cs_reversal();
		cmd[0] = 0x0f;
		cmd[1] = 0xc0;
		spi_send_cmd(cmd, 2);
		spi_recv_cmd(&read_buf, 1);
		while(read_buf & 0x1)
			spi_recv_cmd(&read_buf, 1);

		/* can not erase */
		if(read_buf & 0x4) {
			printf("erase bad block id : %d\n", (block / 0x40));
			continue;
		}
#if 0
		/* write */
		int send_len;
		int column = 0;
		unsigned char write_buf = 0x5a;
		for(page = 0; page < (block_size / page_size); page++) {
			jz_cs_reversal();
			cmd[0] = 0x02;
			cmd[1] = (column >> 8) & 0xf;
			cmd[2] = column & 0xff;
			spi_send_cmd(cmd, 3);

			for(send_len = 0; send_len < page_size; send_len++)
				spi_send_cmd(&write_buf, 1);

			jz_cs_reversal();
			cmd[0] = 0x06;
			spi_send_cmd(cmd, 1);

			jz_cs_reversal();
			cmd[0] = 0x10;
			cmd[1] = ((page + block) >> 16) & 0xff;
			cmd[2] = ((page + block) >> 8) & 0xff;
			cmd[3] = (page + block) & 0xff;
			spi_send_cmd(cmd, 4);
			udelay(t_write);

			jz_cs_reversal();
			cmd[0] = 0x0f;
			cmd[1] = 0xc0;
			spi_send_cmd(cmd, 2);

			spi_recv_cmd(&read_buf, 1);
			while(read_buf & 0x1)
				spi_recv_cmd(&read_buf, 1);

			/* can not write */
			if(read_buf & 0x8) {
				printf("write bad block id : %d\n", (block / 0x40));
				break;
			}
		}

		if(page < (block_size / page_size)) {
			printf("write bad block \n");
			continue ;
		}

		/* read */
		for(page = 0; page < (block_size / page_size); page++) {
			jz_cs_reversal();
			cmd[0] = 0x13;
			cmd[1] = ((page + block) >> 16) & 0xff;
			cmd[2] = ((page + block) >> 8) & 0xff;
			cmd[3] = (page + block) & 0xff;
			spi_send_cmd(cmd, 4);
			udelay(t_read);

			jz_cs_reversal();
			cmd[0] = 0x0f;
			cmd[1] = 0xc0;
			spi_send_cmd(cmd, 2);

			spi_recv_cmd(&read_buf, 1);
			while(read_buf & 0x1)
				spi_recv_cmd(&read_buf, 1);

			/* can not correct */
			if((read_buf & 0x30) == 0x20) {
				printf("read bad block : %d\n", (block / 0x40));
				break;
			}
		}

		if(page < (block_size / page_size)) {
			printf("read bad block\n");
			continue ;
		}
#endif
		for(page = 0; page < (block_size / page_size); page++) {
			params->page_list[params->page_num] = (block + page);
			params->page_num = params->page_num + 1;
		}

	}

	printf("spi nand space : 0x%x\n", params->page_num * params->page_size);
}

struct spi_flash *spi_flash_probe_ingenic_nand(struct spi_slave *spi, u8 *idcode)
{
	int i;
	struct spi_flash *flash;
	struct jz_spi_support *params;

	for (i = 0; i < ARRAY_SIZE(jz_spi_nand_support_table); i++) {
		params = &jz_spi_nand_support_table[i];
		if ( (params->id_manufactory == idcode[0]) && (params->id_device == idcode[1]) )
			break;
	}

	if (i == ARRAY_SIZE(jz_spi_nand_support_table)) {
			printf("ingenic: Unsupported ID %04x\n", idcode[0]);
			return NULL;
	}

	flash = spi_flash_alloc_base(spi, params->name);
	if (!flash) {
		printf("ingenic: Failed to allocate memory\n");
		return NULL;
	}

	flash->erase = jz_erase_nand;
	flash->write = jz_write_nand;
	flash->read  = jz_read_nand;

	flash->page_size = params->page_size;
	flash->sector_size = params->block_size;
	flash->size = params->size;

	params->page_num = 0;
	params->page_list = NULL;

	jz_spi_nand_init();

	jz_spi_nand_scan_bad_block(params);

	gparams = params;

	return flash;
}

struct spi_flash *spi_flash_probe_ingenic(struct spi_slave *spi, u8 *idcode)
{
	int i;
	struct spi_flash *flash;
	struct jz_spi_support *params;

	for (i = 0; i < ARRAY_SIZE(jz_spi_support_table); i++) {
		params = &jz_spi_support_table[i];
		if (params->id_manufactory == idcode[0])
			break;
	}

	if (i == ARRAY_SIZE(jz_spi_support_table)) {
#ifdef CONFIG_BURNER
		if (idcode[0] != 0){
			printf("unsupport ID is %04x if the id not be 0x00,the flash is ok for burner\n",idcode[0]);
			params = &jz_spi_support_table[1];
		}else{
			printf("ingenic: Unsupported ID %04x\n", idcode[0]);
			return NULL;

		}
#else
			printf("ingenic: Unsupported ID %04x\n", idcode[0]);
			return NULL;
#endif
	}

	flash = spi_flash_alloc_base(spi, params->name);
	if (!flash) {
		printf("ingenic: Failed to allocate memory\n");
		return NULL;
	}

	flash->erase = jz_erase;
	flash->write = jz_write;
	flash->read  = jz_read;

	flash->page_size = params->page_size;
	flash->sector_size = params->sector_size;
	flash->size = params->size;

	return flash;
}
#endif

#ifdef CONFIG_SPL_SPI_SUPPORT
void spl_spi_load_image(void)
{
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	spl_parse_image_header(header);

	spi_load(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN, CONFIG_SYS_TEXT_BASE);
}
#endif
