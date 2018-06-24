/*
 * JZ common routines
 *
 * Copyright (c) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
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

#ifndef __JZ_LCD_V1_2_H__
#define __JZ_LCD_V1_2_H__

#include <common.h>
#include <linux/types.h>

void panel_pin_init(void);
void panel_power_on(void);
void panel_power_off(void);
void panel_init_set_sequence(struct dsi_device *dsi);
void board_set_lcd_power_on(void);
#if PWM_BACKLIGHT_CHIP
void lcd_set_backlight_level(int num);
void lcd_close_backlight(void);
#else
void lcd_init_backlight(int num);
void send_low_pulse(int num);
void lcd_set_backlight_level(int num);
void lcd_close_backlight(void);
#endif

struct jz_fb_dma_descriptor {
	u_long fdadr;		/* Frame descriptor address register */
	u_long fsadr;		/* Frame source address register */
	u_long fidr;		/* Frame ID register */
	u_long ldcmd;		/* Command register */
	u_long offsize;		/* Stride Offsize(in word) */
	u_long page_width;	/* Stride Pagewidth(in word) */
	u_long cmd_num;		/* Command Number(for SLCD) */
	u_long desc_size;	/* Foreground Size */
};

#ifdef CONFIG_TWO_FRAME_BUFFERS
#define NUM_FRAME_BUFFERS 2
#endif

#ifdef CONFIG_THREE_FRAME_BUFFERS
#define NUM_FRAME_BUFFERS 3
#endif

#define PIXEL_ALIGN 16
#define MODE_NAME_LEN 32

#define PICOS2KHZ(a) (1000000000/(a))
#define KHZ2PICOS(a) (1000000000/(a))
#define FB_SYNC_HOR_HIGH_ACT    1	/* horizontal sync high active  */
#define FB_SYNC_VERT_HIGH_ACT   2	/* vertical sync high active    */
#define FB_SYNC_EXT		4	/* external sync        */
#define FB_SYNC_COMP_HIGH_ACT   8	/* composite sync high active   */
#define FB_SYNC_BROADCAST	16	/* broadcast video timings      */
/* vtotal = 144d/288n/576i => PAL  */
/* vtotal = 121d/242n/484i => NTSC */
#define FB_SYNC_ON_GREEN	32	/* sync on green */

#define FB_VMODE_NONINTERLACED  0	/* non interlaced */
#define FB_VMODE_INTERLACED	1	/* interlaced       */
#define FB_VMODE_DOUBLE		2	/* double scan */
#define FB_VMODE_ODD_FLD_FIRST	4	/* interlaced: top line first */
#define FB_VMODE_MASK		255
struct jzfb_config_info lcd_config_info;
enum jzfb_format_order {
	FORMAT_X8R8G8B8 = 1,
	FORMAT_X8B8G8R8,
};

/* LCD controller supported display device output mode */
enum jzfb_lcd_type {
	LCD_TYPE_GENERIC_16_BIT = 0,
	LCD_TYPE_GENERIC_18_BIT = 0 | (1 << 7),
	LCD_TYPE_GENERIC_24_BIT = 0 | (1 << 6),
	LCD_TYPE_SPECIAL_TFT_1 = 1,
	LCD_TYPE_SPECIAL_TFT_2 = 2,
	LCD_TYPE_SPECIAL_TFT_3 = 3,
	LCD_TYPE_8BIT_SERIAL = 0xc,
	LCD_TYPE_SLCD = 0xd | (1 << 31),
};

/* smart lcd interface_type */
enum smart_lcd_type {
	SMART_LCD_TYPE_PARALLEL,
	SMART_LCD_TYPE_SERIAL,
};

/* smart lcd command width */
enum smart_lcd_cwidth {
	SMART_LCD_CWIDTH_16_BIT_ONCE = (0 << 8),
	SMART_LCD_CWIDTH_9_BIT_ONCE = SMART_LCD_CWIDTH_16_BIT_ONCE,
	SMART_LCD_CWIDTH_8_BIT_ONCE = (0x1 << 8),
	SMART_LCD_CWIDTH_18_BIT_ONCE = (0x2 << 8),
	SMART_LCD_CWIDTH_24_BIT_ONCE = (0x3 << 8),
};

/* smart lcd data width */
enum smart_lcd_dwidth {
	SMART_LCD_DWIDTH_18_BIT_ONCE_PARALLEL_SERIAL = (0 << 10),
	SMART_LCD_DWIDTH_16_BIT_ONCE_PARALLEL_SERIAL = (0x1 << 10),
	SMART_LCD_DWIDTH_8_BIT_THIRD_TIME_PARALLEL = (0x2 << 10),
	SMART_LCD_DWIDTH_8_BIT_TWICE_TIME_PARALLEL = (0x3 << 10),
	SMART_LCD_DWIDTH_8_BIT_ONCE_PARALLEL_SERIAL = (0x4 << 10),
	SMART_LCD_DWIDTH_24_BIT_ONCE_PARALLEL = (0x5 << 10),
	SMART_LCD_DWIDTH_9_BIT_TWICE_TIME_PARALLEL = (0x7 << 10),
	SMART_LCD_DWIDTH_MASK = (0x7 << 10),
};

/*smart lcd init code type*/
enum smart_config_type {
	SMART_CONFIG_CMD =  0,
	SMART_CONFIG_DATA =  1,
	SMART_CONFIG_UDELAY =  2,
};

struct smart_lcd_data_table {
	enum smart_config_type type;
	uint32_t value;
};

struct fb_videomode {
	const char *name;	/* optional */
	u32 refresh;		/* optional */
	u32 xres;
	u32 yres;
	u32 pixclock;
	u32 left_margin;
	u32 right_margin;
	u32 upper_margin;
	u32 lower_margin;
	u32 hsync_len;
	u32 vsync_len;
	u32 sync;
	u32 vmode;
	u32 flag;
};

/* smart lcd new data width */
enum smart_lcd_new_dwidth {
	SMART_LCD_NEW_DWIDTH_24_BIT = (4 << 13),
	SMART_LCD_NEW_DWIDTH_18_BIT = (3 << 13),
	SMART_LCD_NEW_DWIDTH_16_BIT = (2 << 13),
	SMART_LCD_NEW_DWIDTH_9_BIT = (1 << 13),
	SMART_LCD_NEW_DWIDTH_8_BIT = (0 << 13),
};

/* smart lcd data times */
enum smart_lcd_new_dtimes {
	SMART_LCD_NEW_DTIMES_ONCE = (0 << 8),
	SMART_LCD_NEW_DTIMES_TWICE = (1 << 8),
	SMART_LCD_NEW_DTIMES_THICE = (2 << 8),
};

struct jzfb_config_info {
	int num_modes;
	struct fb_videomode *modes;	/* valid video modes */
	enum jzfb_format_order fmt_order;	/* frame buffer pixel format order */
	int lcdbaseoff;		/* lcd register base offset from LCD_BASE */

	enum jzfb_lcd_type lcd_type;	/* lcd type */
	unsigned int bpp;	/* bits per pixel for the lcd */
	unsigned pinmd:1;	/* 16bpp lcd data pin mapping. 0: LCD_D[15:0],1: LCD_D[17:10] LCD_D[8:1] */

	unsigned pixclk_falling_edge:1;	/* pixclk_falling_edge: pixel clock at falling edge */
	unsigned date_enable_active_low:1;	/* data enable active low */
	struct {
		enum smart_lcd_type smart_type;	/* smart lcd transfer type, 0: parrallel, 1: serial */

		unsigned clkply_active_rising:1;	/* smart lcd clock polarity:
							   0: Active edge is Falling,1: Active edge is Rasing */
		unsigned rsply_cmd_high:1;	/* smart lcd RS polarity.
						   0: Command_RS=0, Data_RS=1; 1: Command_RS=1, Data_RS=0 */
		unsigned csply_active_high:1;	/* smart lcd CS Polarity.
						   0: Active level is low, 1: Active level is high */

		unsigned newcfg_6800_md:1;
		unsigned newcfg_fmt_conv:1;
		unsigned newcfg_datatx_type:1;
		unsigned newcfg_cmdtx_type:1;
		unsigned newcfg_cmd_9bit:1;

		size_t length_cmd;
        unsigned long *write_gram_cmd;	/* write graphic ram command */
        unsigned bus_width;	/* bus width in bit */
        unsigned int length_data_table;	/* array size of data_table */
		struct smart_lcd_data_table *data_table;	/* init data table */
		int (*init) (void);
		int (*gpio_for_slcd) (void);
	} smart_config;

	unsigned dither_enable:1;	/* enable dither function: 1, disable dither function: 0 */
	/* when LCD'bpp is lower than 24, suggest to enable it */
	struct {
		unsigned dither_red;	/* 0:8bit dither, 1:6bit dither, 2: 5bit dither, 3: 4bit dither */
		unsigned dither_green;	/* 0:8bit dither, 1:6bit dither, 2: 5bit dither, 3: 4bit dither */
		unsigned dither_blue;	/* 0:8bit dither, 1:6bit dither, 2: 5bit dither, 3: 4bit dither */
	} dither;

	struct {
		unsigned spl;	/* special_tft SPL signal register setting */
		unsigned cls;	/* special_tft CLS signal register setting */
		unsigned ps;	/* special_tft PS signal register setting */
		unsigned rev;	/* special_tft REV signal register setting */
	} special_tft_config;

	unsigned long fdadr0;	/* physical address of frame/palette descriptor */
	unsigned long fdadr1;	/* physical address of frame descriptor */

	/* DMA descriptors */
	struct jz_fb_dma_descriptor *dmadesc_fblow;
	struct jz_fb_dma_descriptor *dmadesc_fbhigh;
	struct jz_fb_dma_descriptor *dmadesc_palette;
	struct jz_fb_dma_descriptor *dmadesc_cmd;
	struct jz_fb_dma_descriptor *dmadesc_cmd_tmp;

	unsigned long screen;	/* address of frame buffer */
	unsigned long palette;	/* address of palette memory */
	unsigned int palette_size;
	unsigned long dma_cmd_buf;	/* address of dma command buffer */

	void *par;
};

int jzfb_get_controller_bpp(unsigned int);
extern struct jzfb_config_info lcd_config_info;
extern struct jzfb_config_info jzfb1_init_data;
extern struct fb_videomode jzfb1_videomode;
extern struct dsi_device jz_dsi;
#endif /*__JZ_LCD_H__*/
