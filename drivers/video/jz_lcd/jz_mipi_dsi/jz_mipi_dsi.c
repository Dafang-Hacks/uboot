/* /drivers/video/jz_lcd/jz_mipi_dsi/jz_mipi_dsi.c
 *
 * Ingenic SoC MIPI-DSI dsim driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <asm/io.h>
#include <config.h>
#include <serial.h>
#include <common.h>

#include <jz_lcd/jz_dsim.h>
#include <jz_lcd/jz_lcd_v1_2.h>
#include "jz_mipi_dsi_lowlevel.h"
#include "jz_mipi_dsih_hal.h"
#include "jz_mipi_dsi_regs.h"

int jz_dsi_video_cfg(struct dsi_device *dsi)
{
	dsih_error_t err_code = OK;
	unsigned short bytes_per_pixel_x100 = 0;	/* bpp x 100 because it can be 2.25 */
	unsigned short video_size = 0;
	unsigned int ratio_clock_xPF = 0;	/* holds dpi clock/byte clock times precision factor */
	unsigned short null_packet_size = 0;
	unsigned char video_size_step = 1;
	unsigned int total_bytes = 0;
	unsigned int bytes_per_chunk = 0;
	unsigned int no_of_chunks = 0;
	unsigned int bytes_left = 0;
	unsigned int chunk_overhead = 0;

	struct video_config *video_config;
	video_config = dsi->video_config;

	/* check DSI controller dsi */
	if ((dsi == NULL) || (video_config == NULL)) {
		return ERR_DSI_INVALID_INSTANCE;
	}
	if (dsi->state != INITIALIZED) {
		return ERR_DSI_INVALID_INSTANCE;
	}

	ratio_clock_xPF =
	    (video_config->byte_clock * PRECISION_FACTOR) /
	    (video_config->pixel_clock);
	video_size = video_config->h_active_pixels;
	/*  disable set up ACKs and error reporting */
	mipi_dsih_hal_dpi_frame_ack_en(dsi, video_config->receive_ack_packets);
	if (video_config->receive_ack_packets) {	/* if ACK is requested, enable BTA, otherwise leave as is */
		mipi_dsih_hal_bta_en(dsi, 1);
	}
	/*0:switch to high speed transfer, 1 low power mode */
	mipi_dsih_write_word(dsi, R_DSI_HOST_CMD_MODE_CFG, 0);

	/*0:enable video mode, 1:enable cmd mode */
	mipi_dsih_hal_gen_set_mode(dsi, 0);

	err_code =
	    jz_dsi_video_coding(dsi, &bytes_per_pixel_x100, &video_size_step,
				&video_size);
	if (err_code) {
		return err_code;
	}


	jz_dsi_dpi_cfg(dsi, &ratio_clock_xPF, &bytes_per_pixel_x100);

	/* TX_ESC_CLOCK_DIV must be less than 20000KHz */
	jz_dsih_hal_tx_escape_division(dsi, 7);

	/* video packetisation   */
	if (video_config->video_mode == VIDEO_BURST_WITH_SYNC_PULSES) {	/* BURST */
		//mipi_dsih_hal_dpi_null_packet_en(dsi, 0);
		mipi_dsih_hal_dpi_null_packet_size(dsi, 0);
		//mipi_dsih_hal_dpi_multi_packet_en(dsi, 0);
		err_code =
		    err_code ? err_code : mipi_dsih_hal_dpi_chunks_no(dsi, 1);
		err_code =
		    err_code ? err_code :
		    mipi_dsih_hal_dpi_video_packet_size(dsi, video_size);
		if (err_code != OK) {
			return err_code;
		}
		/* BURST by default, returns to LP during ALL empty periods - energy saving */
		mipi_dsih_hal_dpi_lp_during_hfp(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_hbp(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_vactive(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_vfp(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_vbp(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_vsync(dsi, 1);
	} else {		/* non burst transmission */
		null_packet_size = 0;
		/* bytes to be sent - first as one chunk */
		bytes_per_chunk =
		    (bytes_per_pixel_x100 * video_config->h_active_pixels) /
		    100 + VIDEO_PACKET_OVERHEAD;
		/* bytes being received through the DPI interface per byte clock cycle */
		total_bytes =
		    (ratio_clock_xPF * video_config->no_of_lanes *
		     (video_config->h_total_pixels -
		      video_config->h_back_porch_pixels -
		      video_config->h_sync_pixels)) / PRECISION_FACTOR;
		printf("---->total_bytes:%d, bytes_per_chunk:%d\n", total_bytes,
		       bytes_per_chunk);
		/* check if the in pixels actually fit on the DSI link */
		if (total_bytes >= bytes_per_chunk) {
			chunk_overhead = total_bytes - bytes_per_chunk;
			/* overhead higher than 1 -> enable multi packets */
			if (chunk_overhead > 1) {
				for (video_size = video_size_step; video_size < video_config->h_active_pixels; video_size += video_size_step) {	/* determine no of chunks */
					if ((((video_config->h_active_pixels *
					       PRECISION_FACTOR) / video_size) %
					     PRECISION_FACTOR) == 0) {
						no_of_chunks =
						    video_config->h_active_pixels
						    / video_size;
						bytes_per_chunk =
						    (bytes_per_pixel_x100 *
						     video_size) / 100 +
						    VIDEO_PACKET_OVERHEAD;
						if (total_bytes >=
						    (bytes_per_chunk *
						     no_of_chunks)) {
							bytes_left =
							    total_bytes -
							    (bytes_per_chunk *
							     no_of_chunks);
							break;
						}
					}
				}
				/* prevent overflow (unsigned - unsigned) */
				if (bytes_left >
				    (NULL_PACKET_OVERHEAD * no_of_chunks)) {
					null_packet_size =
					    (bytes_left -
					     (NULL_PACKET_OVERHEAD *
					      no_of_chunks)) / no_of_chunks;
					if (null_packet_size > MAX_NULL_SIZE) {	/* avoid register overflow */
						null_packet_size =
						    MAX_NULL_SIZE;
					}
				}
			} else {	/* no multi packets */
				no_of_chunks = 1;
				/* video size must be a multiple of 4 when not 18 loosely */
				for (video_size = video_config->h_active_pixels;
				     (video_size % video_size_step) != 0;
				     video_size++) {
					;
				}
			}
		} else {
			printf
			    ("resolution cannot be sent to display through current settings");
			err_code = ERR_DSI_OVERFLOW;

		}
	}
	err_code =
	    err_code ? err_code : mipi_dsih_hal_dpi_chunks_no(dsi,
							      no_of_chunks);
	err_code =
	    err_code ? err_code : mipi_dsih_hal_dpi_video_packet_size(dsi,
								      video_size);
	err_code =
	    err_code ? err_code : mipi_dsih_hal_dpi_null_packet_size(dsi,
								     null_packet_size);

	// mipi_dsih_hal_dpi_null_packet_en(dsi, null_packet_size > 0? 1: 0);
	// mipi_dsih_hal_dpi_multi_packet_en(dsi, (no_of_chunks > 1)? 1: 0);
#ifdef  CONFIG_DSI_DPI_DEBUG
	/*      D E B U G               */
	{
		printf("total_bytes %d ,", total_bytes);
		printf("bytes_per_chunk %d ,", bytes_per_chunk);
		printf("bytes left %d ,", bytes_left);
		printf("null packets %d ,", null_packet_size);
		printf("chunks %d ,", no_of_chunks);
		printf("video_size %d ", video_size);
	}
#endif
	mipi_dsih_hal_dpi_video_vc(dsi, video_config->virtual_channel);
	jz_dsih_dphy_no_of_lanes(dsi, video_config->no_of_lanes);
	/* enable high speed clock */
	mipi_dsih_dphy_enable_hs_clk(dsi, 1);
	printf("video configure is ok!\n");
	return err_code;

}

/* set all register settings to MIPI DSI controller. */
void jz_dsi_phy_cfg(struct dsi_device *dsi)
{
	jz_dsi_set_clock(dsi);
	jz_init_dsi(dsi);
}

void jz_dsi_phy_open(struct dsi_device *dsi)
{
	struct video_config *video_config;
	video_config = dsi->video_config;

	debug("entry %s()\n", __func__);
	jz_dsih_dphy_reset(dsi, 0);
	jz_dsih_dphy_stop_wait_time(dsi, 0x1c);	/* 0x1c: */
	jz_dsih_dphy_no_of_lanes(dsi, video_config->no_of_lanes);
	jz_dsih_dphy_clock_en(dsi, 1);
	jz_dsih_dphy_shutdown(dsi, 1);
	jz_dsih_dphy_reset(dsi, 1);
}

void set_base_dir_tx(struct dsi_device *dsi, void *param)
{
	int i = 0;
	register_config_t phy_direction[] = {
		{0xb4, 0x02},
		{0xb8, 0xb0},
		{0xb8, 0x100b0},
		{0xb4, 0x00},
		{0xb8, 0x000b0},
		{0xb8, 0x0000},
		{0xb4, 0x02},
		{0xb4, 0x00}
	};
	i = mipi_dsih_write_register_configuration(dsi, phy_direction,
						   (sizeof(phy_direction) /
						    sizeof(register_config_t)));
	if (i != (sizeof(phy_direction) / sizeof(register_config_t))) {
		printf("ERROR setting up testchip %d", i);
	}
}

void jz_dsi_init(struct dsi_device *dsi)
{
	int retry = 5;
	int st_mask = 0;
	unsigned int tmp = 0;
	debug("entry jz_dsi_init()\n");

	dsi->state = NOT_INITIALIZED;
	dsi->address = dsi->dsi_phy->address = DSI_BASE;
	dsi->dsi_phy->bsp_pre_config = set_base_dir_tx;
	dsi->dsi_phy->reference_freq = REFERENCE_FREQ;
	dsi->video_config->byte_clock = DATALANE_BYTECLOCK_KHZ,	/* KHz  */
	dsi->video_config->video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
	dsi->video_config->pixel_clock = PICOS2KHZ(jzfb1_videomode.pixclock); // dpi_clock
	dsi->video_config->h_polarity = jzfb1_videomode.sync & FB_SYNC_HOR_HIGH_ACT;
	dsi->video_config->h_active_pixels = jzfb1_videomode.xres;
	dsi->video_config->h_sync_pixels = jzfb1_videomode.hsync_len;
	dsi->video_config->h_back_porch_pixels = jzfb1_videomode.left_margin;
	dsi->video_config->h_total_pixels = jzfb1_videomode.xres + jzfb1_videomode.hsync_len + jzfb1_videomode.left_margin + jzfb1_videomode.right_margin;

	dsi->video_config->v_active_lines = jzfb1_videomode.yres;
	dsi->video_config->v_polarity =  jzfb1_videomode.sync & FB_SYNC_VERT_HIGH_ACT;
	dsi->video_config->v_sync_lines = jzfb1_videomode.vsync_len;
	dsi->video_config->v_back_porch_lines = jzfb1_videomode.upper_margin;
	dsi->video_config->v_total_lines = jzfb1_videomode.yres + jzfb1_videomode.upper_margin + jzfb1_videomode.lower_margin + jzfb1_videomode.vsync_len;

	debug("GATE0: 0x10000020 = %x\n", *(volatile unsigned int *)0xb0000020);
	*(volatile unsigned int *)0xb0000020 &= ~(1<<26); //open gate for clk
	debug("GATE0: 0x10000020 = %x\n", *(volatile unsigned int *)0xb0000020);

	/*select mipi dsi */
	*((volatile unsigned int *)0xb30500a4) = 1 << 7;	//MCTRL
	jz_dsi_phy_open(dsi);

	/*set command mode */
	mipi_dsih_write_word(dsi, R_DSI_HOST_MODE_CFG, 0x1);
	/*set this register for cmd size, default 0x6 */
	mipi_dsih_write_word(dsi, R_DSI_HOST_EDPI_CMD_SIZE, 0x6);

	/*
	 * jz_dsi_phy_cfg: PLL programming
	 * */
	jz_dsi_phy_cfg(dsi);

	debug("wait for phy config ready\n");
	if (dsi->video_config->no_of_lanes == 2)
		st_mask = 0x95;
	else {
		st_mask = 0x15;
	}
	/*checkout phy clk lock and  clklane, datalane stopstate  */
	while ((mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_STATUS) & st_mask) !=
	       st_mask && retry) {
		retry--;
	}
	if (!retry) {
		printf("jz mipi dsi init failed\n");
		return -1;
	}

    mipi_dsih_write_word(dsi, R_DSI_HOST_CMD_MODE_CFG,
				                     0xffffff0);


	dsi->state = INITIALIZED;
	debug("---------------dsi init oK-----------------\n");

#ifdef DPI_DEBUG
	mipi_dsih_write_word(dsi, R_DSI_HOST_DPI_CFG_POL, 0x0);
	panel_init_set_sequence(dsi);

	mipi_dsih_dphy_enable_hs_clk(dsi, 1);
	mipi_dsih_write_word(dsi, R_DSI_HOST_CMD_MODE_CFG, 1);
	jz_dsi_video_cfg(dsi);
	mipi_dsih_hal_power(dsi, 1);

	tmp = mipi_dsih_read_word(dsi, R_DSI_HOST_VID_MODE_CFG);
	tmp |= 1 << 16 | 0 << 20 | 1 << 24;
	mipi_dsih_write_word(dsi, R_DSI_HOST_VID_MODE_CFG, tmp);
#endif

	return 0;
}
