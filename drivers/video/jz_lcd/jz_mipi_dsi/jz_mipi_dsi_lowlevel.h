/* linux/drivers/video/exynos/exynos_mipi_dsi_lowlevel.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _JZ_MIPI_DSI_LOWLEVEL_H
#define _JZ_MIPI_DSI_LOWLEVEL_H

void jz_dsih_dphy_reset(struct dsi_device *dsi, int reset);

void jz_dsih_dphy_stop_wait_time(struct dsi_device *dsi,
				 unsigned char no_of_byte_cycles);

void jz_dsih_dphy_no_of_lanes(struct dsi_device *dsi,
			      unsigned char no_of_lanes);
void jz_dsih_dphy_clock_en(struct dsi_device *dsi, int en);
void jz_dsih_dphy_shutdown(struct dsi_device *dsi, int powerup);
void jz_dsih_hal_power(struct dsi_device *dsi, int on);
int jz_dsi_init_config(struct dsi_device *dsi);
int jz_init_dsi(struct dsi_device *dsi);
int jz_dsi_set_clock(struct dsi_device *dsi);
void jz_mipi_dsi_init_interrupt(struct dsi_device *dsi);
int jz_dsi_video_coding(struct dsi_device *dsi,
			unsigned short *bytes_per_pixel_x100,
			unsigned char *video_size_step,
			unsigned short *video_size);
void jz_dsi_dpi_cfg(struct dsi_device *dsi, unsigned int *ratio_clock_xPF,
		    unsigned short *bytes_per_pixel_x100);
void jz_dsih_hal_tx_escape_division(struct dsi_device *dsi,
				    unsigned char tx_escape_division);
dsih_error_t jz_dsih_dphy_configure(struct dsi_device *dsi,
				    unsigned char no_of_lanes,
				    unsigned int output_freq);
#endif /* _JZ_MIPI_DSI_LOWLEVEL_H */
