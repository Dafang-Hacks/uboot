/*
 * Ingenic JZ SPI NAND driver
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
#include <asm/arch/base.h>
#include <nand.h>
#include <asm/arch/nand.h>

#define COMMAND_MAX_LENGTH	8
#define SSI_BASE CONFIG_SSI_BASE

/* wait time before read status (us) */
static int t_reset = 500;
static int t_read  = 120;
static int t_write = 700;
static int t_erase = 5000;

static int column_bak;
static int page_addr_bak;

static void jz_cs_reversal(void );
static void jz_erase_cmd(struct mtd_info *mtd, int page);
static void jz_read(struct mtd_info *mtd);
static void jz_read_id(void );
static void jz_read_status(void );
static void jz_reset(struct mtd_info *mtd);
static void mark_bad(struct mtd_info *mtd);

static void jz_cmdfunc(struct mtd_info *mtd, unsigned int command,int column, int page_addr)
{
	//printf("%s----%s---%d---%x\n", __FILE__, __func__, __LINE__, command);
	struct nand_chip *chip = mtd->priv;

	column_bak = column;
	page_addr_bak = page_addr;

	switch(command) {
		case NAND_CMD_RESET:
			jz_reset(mtd);
			break;

		case NAND_CMD_READID:
			jz_read_id();
			break;

		case NAND_CMD_READOOB:
			column_bak += mtd->writesize;
			jz_read(mtd);
			break;

		case NAND_CMD_READ0:
			jz_read(mtd);
			break;

		case NAND_CMD_STATUS:
			jz_read_status();
			break;

		case NAND_CMD_SEQIN:
			break;

		case NAND_CMD_ERASE1:
			jz_erase_cmd(mtd, page_addr);
			break;

		case NAND_CMD_ERASE2:
			printf("%s---%s---%d\n", __FILE__, __func__, __LINE__);
			break;

		default:
			printf("%s---%s---%d\n", __FILE__, __func__, __LINE__);
			printf("can not support cmd %x\n", command);
			break;
	}
	return ;
}

static void jz_reset(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	unsigned char cmd[COMMAND_MAX_LENGTH];

	jz_cs_reversal();
	cmd[0] = 0xff;
	spi_send_cmd(cmd, 1);
	udelay(t_erase);

	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xc0;
	spi_send_cmd(cmd, 2);
	while(chip->read_byte(mtd) & 0x1)
		;

	return ;
}

static void jz_read_status(void )
{
	unsigned char cmd[COMMAND_MAX_LENGTH];

	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xa0;
	spi_send_cmd(cmd, 2);

	return ;
}


static void jz_read_id(void )
{
	unsigned char cmd[COMMAND_MAX_LENGTH];
	unsigned char read_buf;

	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xa0;
	spi_send_cmd(cmd, 2);
	spi_recv_cmd(&read_buf, 1);
	read_buf &= ~(7 << 3);
	read_buf |= 0x80;

	jz_cs_reversal();
	cmd[0] = 0x1f;
	cmd[1] = 0xa0;
	cmd[2] = read_buf;
	spi_send_cmd(cmd, 3);

	jz_cs_reversal();
	cmd[0] = 0x9f;
	cmd[1] = 0x0;
	spi_send_cmd(cmd, 2);
}

static void jz_read(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	unsigned char cmd[COMMAND_MAX_LENGTH];

	unsigned char read_buf;

	jz_cs_reversal();
	cmd[0] = 0x13;
	cmd[1] = (page_addr_bak >> 16) & 0xff;
	cmd[2] = (page_addr_bak >> 8) & 0xff;
	cmd[3] = page_addr_bak & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_read);

	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xc0;
	spi_send_cmd(cmd, 2);
	read_buf = chip->read_byte(mtd);
	while(read_buf & 0x1)
		read_buf = chip->read_byte(mtd);

	if(read_buf & 0x20) {
		printf("data miss !!!\n");
		printf("page_addr_bak : %x\n", page_addr_bak);
		mark_bad(mtd);
		return ;
	}

	jz_cs_reversal();
	cmd[0] = 0x0b;
	cmd[1] = (column_bak >> 8) & 0xf;
	cmd[2] = column_bak & 0xff;
	cmd[3] = 0x0;
	spi_send_cmd(cmd, 4);

	return ;
}

static void mark_bad(struct mtd_info *mtd)
{
	unsigned char cmd[COMMAND_MAX_LENGTH];

	unsigned char read_buf;

	struct nand_chip *chip = mtd->priv;

	cmd[0] = 0x0;
	column_bak = mtd->writesize;
	page_addr_bak = 0x8000;
	jz_write_buf(mtd, cmd, 1);

}

void jz_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;

	unsigned char cmd[COMMAND_MAX_LENGTH];

	unsigned char read_buf;

	jz_cs_reversal();
	cmd[0] = 0x02;
	cmd[1] = (column_bak >> 8) & 0xf;
	cmd[2] = column_bak & 0xff;
	spi_send_cmd(cmd, 3);

	while(len) {
		if(len > 64) {
			spi_send_cmd(buf, 64);
			buf += 64;
			len -= 64;
		} else {
			spi_send_cmd(buf, len);
			len = 0;
		}
	}

	jz_cs_reversal();
	cmd[0] = 0x06;
	spi_send_cmd(cmd, 1);

	jz_cs_reversal();
	cmd[0] = 0x10;
	cmd[1] = (page_addr_bak >> 16) & 0xff;
	cmd[2] = (page_addr_bak >> 8) & 0xff;
	cmd[3] = page_addr_bak & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_write);

	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xc0;
	spi_send_cmd(cmd, 2);
	read_buf = chip->read_byte(mtd);
	while(read_buf & 0x1)
		read_buf = chip->read_byte(mtd);

	if(read_buf & 0x8) {
		printf("write fail !!!\n");
		while(1);
	}

	return ;
}

static int jz_dev_ready(struct mtd_info *mtd)
{
	printf("%s() not realize !!!\n", __func__);

	struct nand_chip *chip = mtd->priv;

	return 1;
}

static uint8_t jz_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	unsigned char read_buf;

	spi_recv_cmd(&read_buf, 1);

	return read_buf;
}

static void jz_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;

	spi_recv_cmd(buf, len);

	return ;
}

static void jz_select_chip(struct mtd_info *mtd, int chipnr)
{
	/* only one spi-nand, do not realize the function */
	/* if more than one spi-nand, we select one device. it is spi-cs# */

	struct nand_chip *chip = mtd->priv;

	return ;
}

static int jz_waitfunc(struct mtd_info *mtd, struct nand_chip *chip)
{
	printf("%s() not realize !!!\n", __func__);

	return 0;
}

static void jz_erase_cmd(struct mtd_info *mtd, int page)
{
	struct nand_chip *chip = mtd->priv;
	unsigned char cmd[COMMAND_MAX_LENGTH];
	unsigned char read_buf;

	jz_cs_reversal();
	cmd[0] = 0x06;
	spi_send_cmd(cmd, 1);

	jz_cs_reversal();
	cmd[0] = 0xd8;
	cmd[1] = (page >> 16) & 0xff;
	cmd[2] = (page >> 8) & 0xff;
	cmd[3] = page & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_erase);

	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xc0;
	spi_send_cmd(cmd, 2);
	read_buf = chip->read_byte(mtd);
	while(read_buf & 0x1)
		read_buf = chip->read_byte(mtd);

	if(read_buf & 0x8) {
		printf("write fail !!!\n");
		while(1);
	}

	return ;
}

static void jz_cs_reversal(void )
{
	spi_release_bus(NULL);

	udelay(t_reset);

	spi_claim_bus(NULL);

	return ;
}

int board_nand_init(struct nand_chip *nand)
{
	spi_init();

	nand->IO_ADDR_R		= (void *)(SSI_BASE + SSI_DR);

	nand->IO_ADDR_W		= (void *)(SSI_BASE + SSI_DR);

	nand->cmdfunc		= jz_cmdfunc;

	nand->dev_ready		= jz_dev_ready;

	nand->read_byte		= jz_read_byte;

	nand->read_buf		= jz_read_buf;

	nand->write_buf		= jz_write_buf;

	nand->select_chip	= jz_select_chip;

	/* default spi-nand buswide is 8 */;
	nand->options		= 0;

	nand->ecc.mode		= NAND_ECC_NONE;

	nand->waitfunc		= jz_waitfunc;

	return 0;
}
