/*
 * Copyright (C) 2009 Ingenic Semiconductor Inc.
 * Author: <@ingenic.cn>
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

/* this file is based on sk6617 sd card controller*/
#include <common.h>
#include <mmc.h>

#define MMC_DEV_NUM 0
#define READ_BLOCK_LEN 512
#define HW_VERSION_SK6617 0x0107
#define FW_VERSION_SK6617 0x0702

static unsigned char sector_buff[512] = { 0 };

#define BULK_OUT_BUF_SIZE 0x21000
extern u32 Bulk_out_buf[BULK_OUT_BUF_SIZE];
typedef struct file_header_t {
	char name[32];
	int offset;
	int size;
} file_header_t;

#define HEADER_SECTION_SIZE    (1024*4)

file_header_t *get_file_header(unsigned char *buffer, int buffer_size,
			       char *name)
{
	int i;

	//find first empty header & last full header
	for (i = 0; i < (HEADER_SECTION_SIZE / sizeof(file_header_t)); i++) {
		file_header_t *header =
		    (file_header_t *) (buffer + i * sizeof(file_header_t));
		if (strcmp(header->name, name) == 0) {
			return header;
		}
	}

	return NULL;
}

void card_power_on(void)
{
}

void card_power_off(void)
{
}

void card_power_off_on(void)
{
	card_power_off();
	mdelay(1000);
	card_power_on();
	mdelay(1000);
}

int skymedi_set_func(struct mmc *mmc, unsigned int arg)
{
	struct mmc_cmd cmd;
	cmd.cmdidx = 60;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = arg;
	return mmc_send_cmd(mmc, &cmd, NULL);
}

unsigned int skymedi_get_hwversion(unsigned char *pbuf)
{
	unsigned int hwversion = 0;
	hwversion = (pbuf[0] << 8) + pbuf[1];
	return hwversion;
}

unsigned int skymedi_get_fwversion(unsigned char *pbuf)
{
	unsigned int fwversion = 0;
	fwversion = (pbuf[0] << 8) + pbuf[1];
	return fwversion;
}

unsigned int skymedi_get_flashid(unsigned char *pbuf)
{
	unsigned int flashid = 0;
	flashid = (pbuf[1] << 24) + (pbuf[2] << 16) + (pbuf[3] << 8) + pbuf[4];
	return flashid;
}

static ulong skymedi_read_one_block(struct mmc *mmc, void *dst, lbaint_t start)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	lbaint_t blkcnt = 1;
	int timeout = 3000;

	cmd.cmdidx = 8;		//read data
	cmd.cmdarg = start;
	cmd.resp_type = MMC_RSP_R1;

	data.dest = dst;
	data.blocks = blkcnt;
	data.blocksize = READ_BLOCK_LEN;
	data.flags = MMC_DATA_READ;
	if (mmc_send_cmd(mmc, &cmd, &data))
		return 0;

	if (mmc_send_status(mmc, timeout))	//check card status, timeout value is 3 second
		return 0;

	return blkcnt;
}

static ulong skymedi_read_blocks(struct mmc *mmc, void *dst, lbaint_t start,
				 lbaint_t blkcnt)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int timeout = 3000;

	if (blkcnt > 1)
		cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->read_bl_len;

	cmd.resp_type = MMC_RSP_R1;

	data.dest = dst;
	data.blocks = blkcnt;
	data.blocksize = mmc->read_bl_len;
	data.flags = MMC_DATA_READ;

	if (mmc_send_cmd(mmc, &cmd, &data))
		return 0;

	if (blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
			return 0;
		}
	}
	if (mmc_send_status(mmc, timeout))
		return 0;

	return blkcnt;
}

static ulong skymedi_write_blocks(struct mmc *mmc, lbaint_t start,
				  lbaint_t blkcnt, const void *src,
				  u32 timeout_ms)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	if ((start + blkcnt) > mmc->block_dev.lba) {
		printf("MMC: block number 0x" LBAF " exceeds max(0x" LBAF ")\n",
		       start + blkcnt, mmc->block_dev.lba);
		return 0;
	}

	if (blkcnt == 0)
		return 0;
	else if (blkcnt == 1)
		cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->write_bl_len;

	cmd.resp_type = MMC_RSP_R1;

	data.src = src;
	data.blocks = blkcnt;
	data.blocksize = mmc->write_bl_len;
	data.flags = MMC_DATA_WRITE;

	if (mmc_send_cmd(mmc, &cmd, &data)) {
		printf("mmc write failed\n");
		return 0;
	}

	mdelay(1);

	if (blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
			printf("mmc fail to send stop cmd\n");
			return 0;
		}
	}

	/* Waiting for the ready status */
	if (mmc_send_status(mmc, timeout_ms))
		return 0;

	return blkcnt;
}

ulong skymedi_write_file(struct mmc * mmc, lbaint_t start, char *name,
			 u32 timeout_ms)
{
	unsigned char *buffer = (unsigned char *)Bulk_out_buf;	//???
	file_header_t *header =
	    get_file_header(buffer, BULK_OUT_BUF_SIZE, name);
	if (header == NULL) {
		printf("can't find file %s\n", name);
		return 0;
	}

	u32 size = header->size;
	void *src = buffer + header->offset;

	lbaint_t blkcnt = (size - 1) / 512 + 1;
	lbaint_t cur, blocks_todo = blkcnt;

	if (mmc_set_blocklen(mmc, mmc->write_bl_len))
		return 0;

	do {
		cur = (blocks_todo > mmc->b_max) ? mmc->b_max : blocks_todo;
		if (skymedi_write_blocks(mmc, start, cur, src, timeout_ms) !=
		    cur)
			return 0;
		blocks_todo -= cur;
		start += cur;
		src += cur * mmc->write_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}

int skymedi_perform_erase(struct mmc *mmc, u32 arg, u32 timeout_ms)
{
	struct mmc_cmd cmd;
	int ret;
	cmd.cmdidx = MMC_CMD_ERASE;
	cmd.cmdarg = arg;
	cmd.resp_type = MMC_RSP_R1b;

	ret = mmc_send_cmd(mmc, &cmd, NULL);
	if (ret) {
		printf("erase failed\n");
		return ret;
	}
	mdelay(10);

	ret = mmc_send_status(mmc, timeout_ms);
	if (ret != 0) {
		printf("erase timeout\n");
		return ret;
	}
	return 0;
}

int skymedi_judge_result(struct mmc *mmc)
{
	printf("skymedi_judge_result\n");
	struct mmc_cmd cmd;
	int ret;
	u8 cid[17] = "Ingenic open SD!";

	card_power_off_on();
	mmc->has_init = 0;
	mmc->init_in_progress = 0;
	ret = mmc_init(mmc);
	if (ret)
		return ret;

	if (!memcmp(cid, mmc->cid, sizeof(mmc->cid))) {
		printf("open SD card failed!\n");
		return -1;
	}
	card_power_on_off();
	printf("open SD card successed!\n");
	return 0;
}

/* 2. Validation Process */
int skymedi_validation_process(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int ret;
	unsigned int hwversion;
	unsigned int fwversion;
	unsigned int flashid;

	ret = skymedi_set_func(mmc, 0x00000055);	//entry debug mode
	if (ret)
		return ret;
	ret = skymedi_read_one_block(mmc, sector_buff, 0x00000068);	//read hw_version
	if (ret != 1)
		return -1;

	hwversion = skymedi_get_hwversion(sector_buff);
	if (hwversion != HW_VERSION_SK6617) {
		printf("error hw_version : %4x\n", hwversion);
		return -1;
	}

	ret = skymedi_read_one_block(mmc, sector_buff, 0x00000067);	//read hw_version
	if (ret != 1)
		return -1;
	fwversion = skymedi_get_fwversion(sector_buff);
	if (fwversion != FW_VERSION_SK6617) {
		printf("error hw_version : %4x\n", hwversion);
		return -1;
	}

	ret = skymedi_read_one_block(mmc, sector_buff, 0x00000065);	//read flash id
	if (ret != 1)
		return -1;

	flashid = skymedi_get_flashid(sector_buff);
	printf("flash_id = %x\n", flashid);
	/* Please verify the flash id. The function is not implemented */

	ret = skymedi_set_func(mmc, 0x000000aa);	//exit debug mode
	if (ret)
		return ret;
	cmd.cmdidx = MMC_CMD_APP_CMD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0x00020000;
	ret = mmc_send_cmd(mmc, &cmd, NULL);
	if (ret)
		return ret;
	ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_4);	//set bus width 4
	if (ret)
		return ret;
	mmc_set_bus_width(mmc, 4);

	ret = skymedi_set_func(mmc, 0x00000055);
	if (ret)
		return ret;
	ret = skymedi_read_one_block(mmc, sector_buff, 0x00000065);	//read flash id
	if (ret != 1)
		return -1;

	flashid = skymedi_get_flashid(sector_buff);
	printf("flash_id = %x\n", flashid);
	/* Please verify the flash id. The function is not implemented */

	return 0;
}

int skymedi_low_level_process(struct mmc *mmc)
{
	int ret;
	card_power_off_on();
	mmc->has_init = 0;
	mmc->init_in_progress = 0;
	ret = mmc_init(mmc);
	if (ret)
		return ret;
	mmc_set_bus_width(mmc, 1);

	ret = skymedi_set_func(mmc, 0x00000055);
	if (ret)
		return ret;
	ret = skymedi_set_func(mmc, 0x00000057);	//disable special function
	if (ret)
		return ret;
	ret = skymedi_write_file(mmc, 0x80000014, "ERASE.bin", 3000);
	if (ret == 0)
		return -1;
	ret = skymedi_set_func(mmc, 0x00000056);
	if (ret)
		return ret;
	ret = skymedi_write_file(mmc, 0xE0000001, "LLF_Parameter.bin", 3000);
	if (ret == 0)
		return -1;

	ret = skymedi_perform_erase(mmc, 0x00000000, 100000);
	if (ret)
		return ret;
	ret = skymedi_set_func(mmc, 0x00000057);
	if (ret)
		return ret;

	card_power_off_on();
	mmc->has_init = 0;
	mmc->init_in_progress = 0;
	ret = mmc_init(mmc);
	if (ret)
		return ret;

	ret = skymedi_set_func(mmc, 0x00000055);
	if (ret)
		return ret;
	ret = skymedi_set_func(mmc, 0x00000057);
	if (ret)
		return ret;
	ret = skymedi_write_file(mmc, 0x80000014, "LLF1.bin", 3000);
	if (ret == 0)
		return -1;
	ret = skymedi_set_func(mmc, 0x00000056);
	if (ret)
		return ret;
	ret = skymedi_write_file(mmc, 0xE0000001, "LLF_Parameter.bin", 100000);
	if (ret == 0)
		return -1;

	ret = skymedi_read_one_block(mmc, sector_buff, 0x00000072);	//Temp.bin
	if (sector_buff[0] != 0x65 || sector_buff[1] != 0x67) {
		printf("LLF STATUS = %x %x\n", sector_buff[0], sector_buff[1]);
		return -1;
	}

	ret = skymedi_set_func(mmc, 0x00000057);
	if (ret)
		return ret;
	ret = skymedi_write_file(mmc, 0x80000014, "ERASE.bin", 3000);
	if (ret == 0)
		return -1;
	ret = skymedi_set_func(mmc, 0x00000056);
	if (ret)
		return ret;
	ret = skymedi_write_file(mmc, 0xFFFFFF10, "FDM.bin", 3000);
	if (ret == 0)
		return -1;
	ret =
	    skymedi_read_blocks(mmc, sector_buff, 0xFFFFFF10, 64 * 1024 / 512);
	if (ret == 0)
		return -1;
	//Compare temp1.bin with FDM.bin ?????

	ret = skymedi_write_file(mmc, 0xFFFFFF20, "FDM.bin", 3000);
	if (ret == 0)
		return -1;
	ret =
	    skymedi_read_blocks(mmc, sector_buff, 0xFFFFFF20, 64 * 1024 / 512);
	if (ret == 0)
		return -1;
	//Compare temp1.bin with FDM.bin ?????

	return 0;
}

int skymedi_verify_llf_and_fw(struct mmc *mmc)
{
	int ret;
	card_power_off_on();
	mmc->has_init = 0;
	mmc->init_in_progress = 0;
	ret = mmc_init(mmc);
	if (ret)
		return ret;
	mmc_set_bus_width(mmc, 1);

	ret = skymedi_set_func(mmc, 0x00000055);
	if (ret)
		return ret;
	ret = skymedi_set_func(mmc, 0x00000057);
	if (ret)
		return ret;
	ret = skymedi_write_file(mmc, 0x80000014, "ERASE.bin", 3000);
	if (ret == 0)
		return -1;
	ret = skymedi_set_func(mmc, 0x00000056);
	if (ret)
		return ret;
	ret = skymedi_read_blocks(mmc, sector_buff, 0xFFFFFF47, 512 / 512);
	if (ret == 0)
		return -1;
	//check version  ???

	//compare test from address 0 at least 10 sectors data ????

	return 0;
}

int skymedi_cid_program(struct mmc *mmc)
{
	int ret;
	struct mmc_cmd cmd;
	card_power_off_on();
	mmc->has_init = 0;
	mmc->init_in_progress = 0;
	ret = mmc_init(mmc);
	if (ret)
		return ret;

	ret = skymedi_set_func(mmc, 0x00000055);
	if (ret)
		return ret;
	ret = skymedi_set_func(mmc, 0x00000057);
	if (ret)
		return ret;
	ret = skymedi_write_file(mmc, 0x80000014, "ERASE.bin", 3000);
	if (ret == 0)
		return -1;
	ret = skymedi_set_func(mmc, 0x00000056);
	if (ret)
		return ret;
	cmd.cmdidx = 32;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0x00000001;
	ret = mmc_send_cmd(mmc, &cmd, NULL);
	if (ret)
		return ret;
	ret = skymedi_write_file(mmc, 0x80000001, "CID.bin", 3000);
	if (ret == 0)
		return -1;

	return 0;
}

int skymedi_open_mmc_card(void)
{
	int ret;
	struct mmc *mmc;
	mmc = find_mmc_device(MMC_DEV_NUM);
	if (!mmc) {
		printf("no mmc device %x\n", MMC_DEV_NUM);
		return -1;
	}
	memset(sector_buff, 0, sizeof(sector_buff));

	/* 1. Card Initial */
	mmc->has_init = 0;
	mmc->init_in_progress = 0;
	ret = mmc_init(mmc);
	if (ret)
		return -1;
	mmc_set_bus_width(mmc, 1);
	/* 2. Validation Process */
	ret = skymedi_validation_process(mmc);
	if (ret)
		return -1;
	/* 3. Low Level Process */
	ret = skymedi_low_level_process(mmc);
	if (ret)
		return -1;
	/* 4. Verify LLF result & FW Patch Version */
	ret = skymedi_verify_llf_and_fw(mmc);
	if (ret)
		return -1;
	/* 5. CID Programming */
	ret = skymedi_cid_program(mmc);
	if (ret)
		return -1;

	ret = skymedi_judge_result(mmc);
	if (ret)
		return -1;

	return 0;
}
