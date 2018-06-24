/* include/mach/jz_dsim.h
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _JZ_MIPI_DSIM_H
#define _JZ_MIPI_DSIM_H

//#include <linux/fb.h>

#define DSI_BASE (0xb3014000)
#define DATALANE_BYTECLOCK_KHZ	(CONFIG_DATALANE_BPS_MHZ * 1000 / 8)

#define REFERENCE_FREQ (24000)  //24MHZ, ext
#define DPHY_DIV_UPPER_LIMIT    (40000)
#define DPHY_DIV_LOWER_LIMIT    (1000)
#define MIN_OUTPUT_FREQ		(80)

#define DSIH_FIFO_ACTIVE_WAIT	500

#define PRECISION_FACTOR	(1000)
#define DSIH_PIXEL_TOLERANCE	2

#define VIDEO_PACKET_OVERHEAD   6
#define NULL_PACKET_OVERHEAD    6
#define SHORT_PACKET		4
#define BLANKING_PACKET         6
#define MAX_NULL_SIZE		1023
#define TX_ESC_CLK_DIV		7
#define MAX_WORD_COUNT     150
#define PANEL_NAME_SIZE     (32)

enum dsi_interface_type {
	DSIM_COMMAND,
	DSIM_VIDEO
};

enum dsi_virtual_ch_no {
	DSIM_VIRTUAL_CH_0,
	DSIM_VIRTUAL_CH_1,
	DSIM_VIRTUAL_CH_2,
	DSIM_VIRTUAL_CH_3
};

enum dsi_no_of_data_lane {
	DSIM_DATA_LANE_1,
	DSIM_DATA_LANE_2,
	DSIM_DATA_LANE_3,
	DSIM_DATA_LANE_4
};

enum dsi_byte_clk_src {
	DSIM_PLL_OUT_DIV8,
	DSIM_EXT_CLK_DIV8,
	DSIM_EXT_CLK_BYPASS
};

struct dsi_config {
	unsigned char max_lanes;
	unsigned char max_hs_to_lp_cycles;
	unsigned char max_lp_to_hs_cycles;
	unsigned short max_bta_cycles;
	int color_mode_polarity;
	int shut_down_polarity;
};

typedef enum {
	OK = 0,
	ERR_DSI_COLOR_CODING,
	ERR_DSI_OUT_OF_BOUND,
	ERR_DSI_OVERFLOW,
	ERR_DSI_INVALID_INSTANCE,
	ERR_DSI_CORE_INCOMPATIBLE,
	ERR_DSI_VIDEO_MODE,
	ERR_DSI_INVALID_COMMAND,
	ERR_DSI_INVALID_EVENT,
	ERR_DSI_INVALID_HANDLE,
	ERR_DSI_PHY_POWERUP,
	ERR_DSI_PHY_INVALID,
	ERR_DSI_PHY_FREQ_OUT_OF_BOUND,
	ERR_DSI_TIMEOUT
} dsih_error_t;

typedef enum {
	NOT_INITIALIZED = 0,
	INITIALIZED,
	ON,
	OFF
} dsih_state_t;

typedef enum {
	VIDEO_NON_BURST_WITH_SYNC_PULSES = 0,
	VIDEO_NON_BURST_WITH_SYNC_EVENTS,
	VIDEO_BURST_WITH_SYNC_PULSES
} dsih_video_mode_t;
/**
 *  * Color coding type (depth and pixel configuration)
 *   */
typedef enum {
	COLOR_CODE_16BIT_CONFIG1,
	COLOR_CODE_16BIT_CONFIG2,
	COLOR_CODE_16BIT_CONFIG3,
	COLOR_CODE_18BIT_CONFIG1,
	COLOR_CODE_18BIT_CONFIG2,
	COLOR_CODE_24BIT
} dsih_color_coding_t;

struct video_config {
	unsigned char no_of_lanes;
	unsigned char virtual_channel;
	dsih_video_mode_t video_mode;
	int receive_ack_packets;
	unsigned int byte_clock;
	unsigned int pixel_clock;
	dsih_color_coding_t color_coding;
	int is_18_loosely;
	int data_en_polarity;
	int h_polarity;
	unsigned short h_active_pixels;	/* hadr */
	unsigned short h_sync_pixels;
	unsigned short h_back_porch_pixels;	/* hbp */
	unsigned short h_total_pixels;	/* h_total */
	int v_polarity;
	unsigned short v_active_lines;	/* vadr */
	unsigned short v_sync_lines;
	unsigned short v_back_porch_lines;	/* vbp */
	unsigned short v_total_lines;	/* v_total */
};

struct dsi_device {
	unsigned int  address;

	struct dsi_config *dsi_config;
	struct dsi_phy *dsi_phy;
	struct video_config *video_config;

	unsigned int state;
	unsigned int data_lane;

};

struct dsi_phy {
	unsigned int reference_freq;
	unsigned int address;
	void (*bsp_pre_config) (struct dsi_device * dsi, void *param);

};

struct dsi_cmd_packet {
	unsigned char packet_type;
	unsigned char cmd0_or_wc_lsb;
	unsigned char cmd1_or_wc_msb;
	unsigned char cmd_data[MAX_WORD_COUNT];
};

typedef struct {
	/** Register offset */
	unsigned int addr;
	/** Register data [in or out]*/
	unsigned int data;
} register_config_t;

struct freq_ranges {
	unsigned int freq;	/* upper margin of frequency range */
	unsigned char hs_freq;	/* hsfreqrange */
	unsigned char vco_range;	/* vcorange */
};

struct loop_band {
	unsigned int loop_div;	/* upper limit of loop divider range */
	unsigned char cp_current;	/* icpctrl */
	unsigned char lpf_resistor;	/* lpfctrl */
};

#endif /* _JZ_MIPI_DSIM_H */
