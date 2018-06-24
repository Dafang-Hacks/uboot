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
#include <malloc.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/arch/spi.h>
#include <asm/arch/base.h>
#include <linux/mtd/nand.h>
#include <asm/arch/sfc.h>

//#define COMMAND_MAX_LENGTH	8

/* wait time before read status (us) */

static void jz_reset(void)
{
	struct sfc_xfer xfer;

	xfer.cmd = 0xff;
	xfer.tran_dir = TRANSFER;
	xfer.addr_width = 0x0;
	xfer.data_en = DISABLE;
	xfer.poll_en = DISABLE;
	xfer.tran_len= 0x0;
	xfer.addr = 0x0;
	xfer.column = 0x0;

	writel((1<<7|1<<3),SFC_GLB);
	writel(0x7,SFC_DEV_CONF);
	writel((1<<24|0xff),SFC_TRAN_CONF0);
	writel(0x0,SFC_TRAN_LEN);//must add it

	sfc_start();
	udelay(100);
	wait_sfc_end();
	udelay(100);
	return ;
}

static void jz_block_unlock(void)//need write byte
{
	struct sfc_xfer xfer;
	unsigned char buf = 0x0;

	xfer.cmd = 0x1f;
	xfer.tran_dir = TRANSFER;
	xfer.addr_width = 0x1;
	xfer.data_en = ENABLE;
	xfer.poll_en = DISABLE;
	xfer.tran_len= 0x1;
	xfer.addr = 0xa0;
	xfer.column = 0x0;

	sfc_config(xfer);
	sfc_start();
	wait_sfc_requst(xfer.tran_dir);
	write_byte(&buf);
	wait_sfc_end();
	return ;
}

static void jz_read_status(void)//need read byte
{
	struct sfc_xfer xfer;
	unsigned char buf;

	xfer.cmd = 0x0f;
	xfer.tran_dir = RECEIVE;
	xfer.addr_width = 0x1;
	xfer.data_en = ENABLE;
	xfer.poll_en = DISABLE;
	xfer.tran_len= 0x1;
	xfer.addr = 0xc0;
	xfer.column = 0x0;

	sfc_config(xfer);
	sfc_start();
	wait_sfc_requst(xfer.tran_dir);
	read_byte(&buf,1);
	wait_sfc_end();
	return ;
}


static void jz_read_id()//need read byte
{
	int i;
	struct sfc_xfer xfer;
	unsigned char buf[2];
	xfer.cmd = 0x9f;
	xfer.tran_dir = RECEIVE;
	xfer.addr_width = 0x3;
	xfer.data_en = ENABLE;
	xfer.poll_en = DISABLE;
	xfer.tran_len= 0x2;
	xfer.addr = 0x0;
	xfer.column = 0x0;


		writel((1<<7|1<<3),SFC_GLB);
		writel(0x7,SFC_DEV_CONF);
		writel((1<<24|1<<16|0x9f),SFC_TRAN_CONF0);
		writel(0x1,SFC_TRAN_LEN);
	return ;
}

void sfc_page_read_to_cache(int page)
{

	writel(1<<THRESHOLD  | 1 <<PHASE_NUM,SFC_GLB) ;
	writel(1<<CE_DL | 1<<HOLD_DL | 1<<WP_DL,SFC_DEV_CONF);
	writel(3<<ADDR_WIDTH  | 1 <<CMD_EN |  0x13 <<TRAN_CMD,SFC_TRAN_CONF0);
	writel(0x0,SFC_TRAN_LEN);

	writel(page,SFC_DEV_ADR0);
	writel(1 <<26 | 1 <<25 | 1<<24 | 1<<16 |0xf <<TRAN_CMD,SFC_TRAN_CONF1);

	writel(0xc0,SFC_DEV_ADR1);
	writel(0x1,SFC_DEV_STA_MSK);
	writel(0x0,SFC_STA_EXP);
	sfc_start();
	wait_sfc_end();
	return ;
}

void sfc_page_read_from_cache(int len)//need read buffer
{
	int i;
	struct sfc_xfer xfer;
	xfer.cmd = 0x3;
	xfer.tran_dir = RECEIVE;
	xfer.addr_width = 0x3;
	xfer.data_en = ENABLE;
	xfer.poll_en = DISABLE;
	xfer.tran_len= 0;
	xfer.addr = 0x0;
	xfer.column = column_bak;

	writel(1<<THRESHOLD  | 1 <<PHASE_NUM,SFC_GLB);
	writel(1<<CE_DL | 1<<HOLD_DL | 1<<WP_DL,SFC_DEV_CONF);
	writel(3 <<ADDR_WIDTH | 1 <<CMD_EN |1<< DATA_EN |  0x3 <<TRAN_CMD,SFC_TRAN_CONF0);
	writel(len,SFC_TRAN_LEN);
	writel(0x0,SFC_DEV_ADR0);

	return ;
}

static void jz_page_read(struct mtd_info *mtd,int page)
{
	sfc_page_read_to_cache(page);
	sfc_page_read_from_cache(mtd->writesize);
	return ;
}

static void jz_read_oob(struct mtd_info *mtd)
{
	/* read to cache*/
	writel(1<<THRESHOLD  | 1 <<PHASE_NUM,SFC_GLB) ;
	writel(1<<CE_DL | 1<<HOLD_DL | 1<<WP_DL,SFC_DEV_CONF);
	writel(3<<ADDR_WIDTH  | 1 <<CMD_EN |  0x13 <<TRAN_CMD,SFC_TRAN_CONF0);
	writel(0x0,SFC_TRAN_LEN);

	writel(0x0,SFC_DEV_ADR0);
	writel(1 <<26 | 1 <<25 | 1<<24 | 1<<16 |0xf <<TRAN_CMD,SFC_TRAN_CONF1);

	writel(0xc0,SFC_DEV_ADR1);
	writel(0x1,SFC_DEV_STA_MSK);
	writel(0x0,SFC_STA_EXP);
	sfc_start();
	wait_sfc_end();

	/*read from cache*/
	writel(1<<THRESHOLD  | 1 <<PHASE_NUM,SFC_GLB);
	writel(1<<CE_DL | 1<<HOLD_DL | 1<<WP_DL,SFC_DEV_CONF);
	writel(3 <<ADDR_WIDTH | 1 <<CMD_EN |1<< DATA_EN |  0x3 <<TRAN_CMD,SFC_TRAN_CONF0);
	writel(64,SFC_TRAN_LEN);
	writel(0x0,SFC_DEV_ADR0);
	return ;
}

static void jz_read_buf(struct mtd_info *mtd,uint8_t *buf,int len)
{
	int i,j;
	unsigned char *data;
	unsigned int temp;


	sfc_start();
	for(i=0;i<len/4;i++)
	{
		while(!(__sfc_rece_req()))
		{
			udelay(100);
		}
	writel(0x1f,SFC_SCR);
		temp = readl(SFC_DR);
		for(j=0;j<4;j++)
		{
		*(buf +j+i*4)=*((unsigned char*)(&temp) +j);
		}
	}
	wait_sfc_end();
	return ;
}

void sfc_program_load(const uint8_t *buf, int len)//need write buffer
{
	int i;
	struct sfc_xfer xfer;
	xfer.cmd = 0x2;
	xfer.tran_dir = TRANSFER;
	xfer.addr_width = 0x2;
	xfer.data_en = ENABLE;
	xfer.poll_en = DISABLE;
	xfer.tran_len= len;
	xfer.addr = 0x0;
	xfer.column = column_bak;


	sfc_config(xfer);
	sfc_start();
	if(len>64)
	{
		for(i=0;i<64;i++)
		{
			wait_sfc_requst(xfer.tran_dir);
			write_byte(buf);
			buf +=64;
			len -=64;
		}
	}else
	{
		for(i=0;i<len;i++)
		{
			wait_sfc_requst(xfer.tran_dir);
			write_byte(buf);
		}
		len =0;
	}
	wait_sfc_end();
	return ;
}

void sfc_write_enable()
{
	struct sfc_xfer xfer;
	xfer.cmd = 0x6;
	xfer.tran_dir = RECEIVE;
	xfer.addr_width = 0x0;
	xfer.data_en = DISABLE;
	xfer.poll_en = DISABLE;
	xfer.tran_len= 0x0;
	xfer.addr = 0x0;
	xfer.column = 0x0;


	sfc_config(xfer);
	sfc_start();
	wait_sfc_end();
	return ;
}

void sfc_program_execute()
{
	struct sfc_xfer xfer;
	xfer.cmd = 0x10;
	xfer.tran_dir = RECEIVE;
	xfer.addr_width = 0x3;
	xfer.data_en = ENABLE;
	xfer.poll_en = ENABLE;
	xfer.tran_len= 0x0;
	xfer.addr = page_addr_bak;
	xfer.column = 0x0;


	sfc_config(xfer);
	sfc_start();
	wait_sfc_end();
	return ;
}

static void jz_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	jz_block_unlock();
	sfc_program_load(buf,len);
	sfc_write_enable();
	sfc_program_execute();
	return ;
}

static void jz_block_erase(struct mtd_info *mtd, int page)
{
	struct sfc_xfer xfer;
	xfer.cmd = 0xd8;
	xfer.tran_dir = RECEIVE;
	xfer.addr_width = 0x3;
	xfer.data_en = DISABLE;
	xfer.poll_en = ENABLE;
	xfer.tran_len= 0x0;
	xfer.addr = 0x0;
	xfer.column = 0x0;


	sfc_config(xfer);
	sfc_start();
	wait_sfc_end();
	return ;
}


static void jz_cmdfunc(struct mtd_info *mtd, unsigned int command,int column, int page_addr)
{
	int page_addr_bak = page_addr;
	int column_bak = column;
	switch(command) {
		case NAND_CMD_RESET:
			jz_reset();
			break;

		case NAND_CMD_READID:
			jz_read_id();
			break;

		case NAND_CMD_GET_FEATURES:
			jz_read_status();
			break;

		case NAND_CMD_SET_FEATURES:
			jz_block_unlock();
			break;

		case NAND_CMD_ERASE1:
			jz_block_erase(mtd, page_addr_bak);
			break;
		case NAND_CMD_READOOB:
			column_bak += mtd->writesize;
			jz_read_oob(mtd);
			break;

		case NAND_CMD_READ0:
			jz_page_read(mtd,page_addr);
			break;

		default:
			printf("can not support cmd %x\n", command);
			break;
	}
	return ;
}


void jz_select_chip(struct mtd_info *mtd, int chip)
{
return ;
}

unsigned char jz_read_byte(struct mtd_info mtd)
{
	unsigned char data;
	sfc_start();
	while(!(__sfc_rece_req()))
	{
		printf("rdid have no fifo space!\n");
		udelay(100);
	}
	writel(0x1f,SFC_SCR);
	read_byte(&data,1);

	wait_sfc_end();

	writel(0x1f,SFC_SCR);

	return data;
}
int board_nand_init(struct nand_chip *nand)
{
	sfc_init();

	nand->IO_ADDR_R		= (void *)(SFC_DR);

	nand->IO_ADDR_W		= (void *)(SFC_DR);

	nand->cmdfunc		= jz_cmdfunc;


	nand->read_byte		= jz_read_byte;

	nand->read_buf		= jz_read_buf;

	nand->write_buf		= jz_write_buf;

	nand->select_chip	= jz_select_chip;

	nand->ecc.mode		= NAND_ECC_NONE;

	return 0;
}
