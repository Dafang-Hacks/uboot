 /*
  * JZ4775 LCDC DRIVER
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

#include <asm/io.h>
#include <config.h>
#include <serial.h>
#include <common.h>
#include <lcd.h>
#include <asm/arch-jz4775/lcdc.h>
#include <asm/arch-jz4775/gpio.h>
#include <jz_lcd/jz4775_lcd.h>

/*#define DEBUG*/

/* Frame buffer memory information */
struct jzfb_config_info lcd_config_info;
static int lcd_enable_state = 0;
void board_set_lcd_power_on(void);
void flush_cache_all(void);
void lcd_close_backlight(void);
void lcd_set_backlight_level(int num);
#define reg_write(addr,config) \
	writel(config,lcd_config_info.lcdbaseoff+addr)
#define reg_read(addr)	\
	readl(lcd_config_info.lcdbaseoff+addr)

#ifdef DEBUG
void dump_lcd_reg()
{
    printf("$$$dump_lcd_reg\n");
    int tmp;
    printf("LCDC_CFG:(0x%08x) \t0x%08x\n", LCDC_CFG,reg_read(LCDC_CFG));
    printf("LCDC_CTRL:(0x%08x)\t0x%08x\n",LCDC_CTRL,reg_read(LCDC_CTRL));
    printf("LCDC_STATE:(0x%08x)\t0x%08x\n",LCDC_STATE,reg_read(LCDC_STATE));
    printf("LCDC_OSDC:(0x%08x)\t0x%08x\n", LCDC_OSDC,reg_read(LCDC_OSDC));
    printf("LCDC_OSDCTRL:(0x%08x)\t0x%08x\n",LCDC_OSDCTRL,reg_read(LCDC_OSDCTRL));
    printf("LCDC_OSDS:(0x%08x)\t0x%08x\n",LCDC_OSDS,reg_read(LCDC_OSDS));
    printf("LCDC_BGC0:(0x%08x)\t0x%08x\n",LCDC_BGC0,reg_read(LCDC_BGC0));
    printf("LCDC_BGC1:(0x%08x)\t0x%08x\n",LCDC_BGC1,reg_read(LCDC_BGC1));
    printf("LCDC_KEY0:(0x%08x)\t0x%08x\n",LCDC_KEY0, reg_read(LCDC_KEY0));
    printf("LCDC_KEY1:(0x%08x)\t0x%08x\n",LCDC_KEY1, reg_read(LCDC_KEY1));
    printf("LCDC_ALPHA:(0x%08x)\t0x%08x\n",LCDC_ALPHA, reg_read(LCDC_ALPHA));
    printf("==================================\n");
    tmp = reg_read(LCDC_VAT);
    printf("LCDC_VAT:(0x%08x) \t0x%08x, HT = %d, VT = %d\n",LCDC_VAT, tmp,
            (tmp & LCDC_VAT_HT_MASK) >> LCDC_VAT_HT_BIT,
            (tmp & LCDC_VAT_VT_MASK) >> LCDC_VAT_VT_BIT);
    tmp = reg_read(LCDC_DAH);
    printf("LCDC_DAH:(0x%08x) \t0x%08x, HDS = %d, HDE = %d\n",LCDC_DAH, tmp,
            (tmp & LCDC_DAH_HDS_MASK) >> LCDC_DAH_HDS_BIT,
            (tmp & LCDC_DAH_HDE_MASK) >> LCDC_DAH_HDE_BIT);
    tmp = reg_read(LCDC_DAV);
    printf("LCDC_DAV:(0x%08x) \t0x%08x, VDS = %d, VDE = %d\n",LCDC_DAV, tmp,
            (tmp & LCDC_DAV_VDS_MASK) >> LCDC_DAV_VDS_BIT,
            (tmp & LCDC_DAV_VDE_MASK) >> LCDC_DAV_VDE_BIT);
    tmp = reg_read(LCDC_HSYNC);
    printf("LCDC_HSYNC:(0x%08x)\t0x%08x, HPS = %d, HPE = %d\n",LCDC_HSYNC, tmp,
            (tmp & LCDC_HSYNC_HPS_MASK) >> LCDC_HSYNC_HPS_BIT,
            (tmp & LCDC_HSYNC_HPE_MASK) >> LCDC_HSYNC_HPE_BIT);
    tmp = reg_read(LCDC_VSYNC);
    printf("LCDC_VSYNC:(0x%08x)\t0x%08x, VPS = %d, VPE = %d\n", LCDC_VSYNC,tmp,
            (tmp & LCDC_VSYNC_VPS_MASK) >> LCDC_VSYNC_VPS_BIT,
            (tmp & LCDC_VSYNC_VPE_MASK) >> LCDC_VSYNC_VPE_BIT);
    printf("==================================\n");
    printf("LCDC_XYP0:(0x%08x)\t0x%08x\n",LCDC_XYP0, reg_read(LCDC_XYP0));
    printf("LCDC_XYP1:(0x%08x)\t0x%08x\n",LCDC_XYP1, reg_read(LCDC_XYP1));
    printf("LCDC_SIZE0:(0x%08x)\t0x%08x\n",LCDC_SIZE0, reg_read(LCDC_SIZE0));
    printf("LCDC_SIZE1:(0x%08x)\t0x%08x\n",LCDC_SIZE1, reg_read(LCDC_SIZE1));
    printf("LCDC_RGBC:(0x%08x) \t0x%08x\n",LCDC_RGBC, reg_read(LCDC_RGBC));
    printf("LCDC_PS:(0x%08x)  \t0x%08x\n",LCDC_PS, reg_read(LCDC_PS));
    printf("LCDC_CLS:(0x%08x) \t0x%08x\n", LCDC_CLS,reg_read(LCDC_CLS));
    printf("LCDC_SPL:(0x%08x) \t0x%08x\n",LCDC_SPL, reg_read(LCDC_SPL));
    printf("LCDC_REV:(0x%08x) \t0x%08x\n",LCDC_REV, reg_read(LCDC_REV));
    printf("LCDC_IID:(0x%08x) \t0x%08x\n",LCDC_IID, reg_read(LCDC_IID));
    printf("==================================\n");
    printf("LCDC_DA0:(0x%08x) \t0x%08x\n",LCDC_DA0, reg_read(LCDC_DA0));
    printf("LCDC_SA0:(0x%08x) \t0x%08x\n",LCDC_SA0, reg_read(LCDC_SA0));
    printf("LCDC_FID0:(0x%08x)\t0x%08x\n",LCDC_FID0, reg_read(LCDC_FID0));
    printf("LCDC_CMD0:(0x%08x)\t0x%08x\n",LCDC_CMD0, reg_read(LCDC_CMD0));
    printf("LCDC_OFFS0:(0x%08x)\t0x%08x\n",LCDC_OFFS0, reg_read(LCDC_OFFS0));
    printf("LCDC_PW0:(0x%08x) \t0x%08x\n", LCDC_PW0,reg_read(LCDC_PW0));
    printf("LCDC_CNUM0:(0x%08x)\t0x%08x\n",LCDC_CNUM0, reg_read(LCDC_CNUM0));
    printf("LCDC_DESSIZE0:(0x%08x)\t0x%08x\n",LCDC_DESSIZE0, reg_read(LCDC_DESSIZE0));
    printf("==================================\n");
    printf("LCDC_DA1:(0x%08x) \t0x%08x\n", LCDC_DA1, reg_read(LCDC_DA1));
    printf("LCDC_SA1:(0x%08x) \t0x%08x\n",LCDC_SA1, reg_read(LCDC_SA1));
    printf("LCDC_FID1:(0x%08x)\t0x%08x\n",LCDC_FID1, reg_read(LCDC_FID1));
    printf("LCDC_CMD1:(0x%08x)\t0x%08x\n",LCDC_CMD1, reg_read(LCDC_CMD1));
    printf("LCDC_OFFS1:(0x%08x)\t0x%08x\n",LCDC_OFFS1, reg_read(LCDC_OFFS1));
    printf("LCDC_PW1:(0x%08x) \t0x%08x\n",LCDC_PW1, reg_read(LCDC_PW1));
    printf("LCDC_CNUM1:(0x%08x)\t0x%08x\n",LCDC_CNUM1, reg_read(LCDC_CNUM1));
    printf("LCDC_DESSIZE1:(0x%08x)\t0x%08x\n",LCDC_DESSIZE1, reg_read(LCDC_DESSIZE1));
    printf("==================================\n");
    printf("LCDC_PCFG:(0x%08x)\t0x%08x\n", LCDC_PCFG,reg_read(LCDC_PCFG));
    printf("==================================\n");
    printf("SLCDC_CFG:(0x%08x) \t0x%08x\n", SLCDC_CFG,reg_read(SLCDC_CFG));
    printf("SLCDC_CTRL:(0x%08x) \t0x%08x\n", SLCDC_CTRL,reg_read(SLCDC_CTRL));
    printf("SLCDC_STATE:(0x%08x) \t0x%08x\n", SLCDC_STATE,reg_read(SLCDC_STATE));
    printf("SLCDC_DATA:(0x%08x)\t0x%08x\n", SLCDC_DATA,reg_read(SLCDC_DATA));
    printf("==================================\n");
    printf("reg:0x10000020 value=0x%08x  (25bit) Clock Gate Register0\n",
            *(unsigned int *)0xb0000020);
    printf("reg:0x100000e4 value=0x%08x  (5bit_lcdc 21bit_lcdcs) Power Gate Register: \n",
            *(unsigned int *)0xb00000e4);
    printf("reg:0x100000b8 value=0x%08x  (10bit) SRAM Power Control Register0 \n",
            *(unsigned int *)0xb00000b8);
    printf("reg:0x10000064 value=0x%08x  Lcd pixclock \n",
            *(unsigned int *)0xb0000064);
    printf("==================================\n");
    printf("PCINT:\t0x%08x\n", *(unsigned int *)0xb0010210);
    printf("PCMASK:\t0x%08x\n",*(unsigned int *)0xb0010220);
    printf("PCPAT1:\t0x%08x\n",*(unsigned int *)0xb0010230);
    printf("PCPAT0:\t0x%08x\n",*(unsigned int *)0xb0010240);
    printf("==================================\n");
}/*end dump_lcd_reg*/
#endif

#if defined(CONFIG_LCD_LOGO)
static void fbmem_set(void *_ptr, unsigned short val, unsigned count)
{
	int bpp = NBITS(panel_info.vl_bpix);
	if(bpp == 16){
		unsigned short *ptr = _ptr;

		while (count--)
			*ptr++ = val;
	} else if (bpp == 32){
		int val_32;
		int rdata, gdata, bdata;
		unsigned int *ptr = (unsigned int *)_ptr;

		if (lcd_config_info.fmt_order == FORMAT_X8B8G8R8) {
			/*fixed */
		} else if (lcd_config_info.fmt_order == FORMAT_X8R8G8B8) {
			rdata = val >> 11;
			gdata = val >> 5 & 0x003F;
			bdata = val & 0x001F;
			val_32 =
			    rdata << 19 | 0x7 << 16 | gdata << 10 | 0x3 << 8 | bdata <<
			    3 | 0x7;
		}

		while (count--){
			*ptr++ = val_32;
		}
	}
}
/* 565RLE image format: [count(2 bytes), rle(2 bytes)] */
void rle_plot_biger(unsigned short *src_buf, unsigned short *dst_buf, int bpp)
{
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int dis_width, dis_height;
	int flag_bit = (bpp == 16) ? 1 : 2;

	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;
	unsigned short *photo_ptr = (unsigned short *)src_buf;
	unsigned short *lcd_fb = (unsigned short *)dst_buf;

	photo_width = photo_ptr[0];
	photo_height = photo_ptr[1];
	photo_size = photo_ptr[3] << 16 | photo_ptr[2]; 	//photo size
	debug("photo_size =%d photo_width = %d, photo_height = %d\n", photo_size,
	      photo_width, photo_height);
	photo_ptr += 4;

	dis_height = photo_height < vm_height ? photo_height : vm_height;

	unsigned write_count = photo_ptr[0];
	while (photo_size > 0) {
			while (dis_height > 0) {
				dis_width = photo_width < vm_width ? photo_width : vm_width;
				while (dis_width > 0) {
					if (photo_size < 0)
						break;
					fbmem_set(lcd_fb, photo_ptr[1], write_count);
					lcd_fb += write_count * flag_bit;
					photo_ptr += 2;
					photo_size -= 2;
					write_count = photo_ptr[0];
					dis_width -= write_count;
				}
				if (dis_width < 0) {
					photo_ptr -= 2;
					photo_size += 2;
					lcd_fb += dis_width * flag_bit;
					write_count = - dis_width;
				}
				int extra_width = photo_width - vm_width;
				while (extra_width > 0) {
					photo_ptr += 2;
					photo_size -= 2;
					write_count = photo_ptr[0];
					extra_width -= write_count;
				}
				if (extra_width < 0) {
					photo_ptr -= 2;
					photo_size += 2;
					write_count = -extra_width;
				}
				dis_height -= 1;
			}
			if (dis_height <= 0)
				break;
		}
	return;
}
#ifdef CONFIG_LOGO_EXTEND
#ifndef  CONFIG_PANEL_DIV_LOGO
#define  CONFIG_PANEL_DIV_LOGO   (3)
#endif
void rle_plot_extend(unsigned short *src_buf, unsigned short *dst_buf, int bpp)
{
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int dis_width, dis_height, ewidth, eheight;
	unsigned short compress_count, compress_val, write_count;
	unsigned short compress_tmp_count, compress_tmp_val;
	unsigned short *photo_tmp_ptr;
	unsigned short *photo_ptr;
	unsigned short *lcd_fb;
	int rate0, rate1, extend_rate;
	int i;

	int flag_bit = 1;
	if(bpp == 16){
		flag_bit = 1;
	}else if(bpp == 32){
		flag_bit = 2;
	}

	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;
	photo_ptr = (unsigned short *)src_buf;
	lcd_fb = (unsigned short *)dst_buf;

	rate0 =  vm_width/photo_ptr[0] > CONFIG_PANEL_DIV_LOGO ? (vm_width/photo_ptr[0])/CONFIG_PANEL_DIV_LOGO : 1;
	rate1 =  vm_height/photo_ptr[1] > CONFIG_PANEL_DIV_LOGO ? (vm_width/photo_ptr[1])/CONFIG_PANEL_DIV_LOGO : 1;
	extend_rate = rate0 < rate1 ?  rate0 : rate1;
	debug("logo extend rate = %d\n", extend_rate);

	photo_width = photo_ptr[0] * extend_rate;
	photo_height = photo_ptr[1] * extend_rate;
	photo_size = ( photo_ptr[3] << 16 | photo_ptr[2]) * extend_rate; 	//photo size
	debug("photo_size =%d photo_width = %d, photo_height = %d\n", photo_size,
	      photo_width, photo_height);
	photo_ptr += 4;

	int test;
	dis_height = photo_height < vm_height ? photo_height : vm_height;
	ewidth = (vm_width - photo_width)/2;
	eheight = (vm_height - photo_height)/2;
	compress_count = compress_tmp_count= photo_ptr[0] * extend_rate;
	compress_val = compress_tmp_val= photo_ptr[1];
	photo_tmp_ptr = photo_ptr;
	write_count = 0;
	debug("0> photo_ptr = %08x, photo_tmp_ptr = %08x\n", photo_ptr, photo_tmp_ptr);
	while (photo_size > 0) {
			if (eheight > 0) {
				lcd_fb += eheight * vm_width * flag_bit;
			}
			while (dis_height > 0) {
				for(i = 0; i < extend_rate && dis_height > 0; i++){
					debug("I am  here\n");
					photo_ptr = photo_tmp_ptr;
					debug("1> photo_ptr = %08x, photo_tmp_ptr = %08x\n", photo_ptr, photo_tmp_ptr);
					compress_count = compress_tmp_count;
					compress_val = compress_tmp_val;

					dis_width = photo_width < vm_width ? photo_width : vm_width;
					if (ewidth > 0) {
						lcd_fb += ewidth*flag_bit;
					}
					while (dis_width > 0) {
						if (photo_size < 0)
							break;
						write_count = compress_count;
						if (write_count > dis_width)
							write_count = dis_width;

						fbmem_set(lcd_fb, compress_val, write_count);
						lcd_fb += write_count * flag_bit;
						if (compress_count > write_count) {
							compress_count = compress_count - write_count;
						} else {
							photo_ptr += 2;
							photo_size -= 2;
							compress_count = photo_ptr[0] * extend_rate;
							compress_val = photo_ptr[1];
						}

						dis_width -= write_count;
					}

					if (ewidth > 0) {
						lcd_fb += ewidth * flag_bit;
					} else {
						int xwidth = -ewidth;
						while (xwidth > 0) {
							unsigned write_count = compress_count;

							if (write_count > xwidth)
								write_count = xwidth;

							if (compress_count > write_count) {
								compress_count = compress_count - write_count;
							} else {
								photo_ptr += 2;
								photo_size -= 2;
								compress_count = photo_ptr[0];
								compress_val = photo_ptr[1];
							}
							xwidth -= write_count;
						}

					}
					dis_height -= 1;
				}
				photo_tmp_ptr = photo_ptr;
				debug("2> photo_ptr = %08x, photo_tmp_ptr = %08x\n", photo_ptr, photo_tmp_ptr);
				compress_tmp_count = compress_count;
				compress_tmp_val = compress_val;
			}

			if (eheight > 0) {
				lcd_fb += eheight * vm_width *flag_bit;
			}
			if (dis_height <= 0)
				return;
		}
	return;
}
#endif

void rle_plot_smaller(unsigned short *src_buf, unsigned short *dst_buf, int bpp)
{
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int dis_width, dis_height, ewidth, eheight;
	unsigned short compress_count, compress_val, write_count;
	unsigned short *photo_ptr;
	unsigned short *lcd_fb;
	int flag_bit = 1;
	if(bpp == 16){
		flag_bit = 1;
	}else if(bpp == 32){
		flag_bit = 2;
	}

	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;
	photo_ptr = (unsigned short *)src_buf;
	lcd_fb = (unsigned short *)dst_buf;

	photo_width = photo_ptr[0];
	photo_height = photo_ptr[1];
	photo_size = photo_ptr[3] << 16 | photo_ptr[2]; 	//photo size
	debug("photo_size =%d photo_width = %d, photo_height = %d\n", photo_size,
	      photo_width, photo_height);
	photo_ptr += 4;

	dis_height = photo_height < vm_height ? photo_height : vm_height;
	ewidth = (vm_width - photo_width)/2;
	eheight = (vm_height - photo_height)/2;
	compress_count = photo_ptr[0];
	compress_val = photo_ptr[1];
	write_count = 0;
	while (photo_size > 0) {
			if (eheight > 0) {
				lcd_fb += eheight * vm_width * flag_bit;
			}
			while (dis_height > 0) {
				dis_width = photo_width < vm_width ? photo_width : vm_width;
				if (ewidth > 0) {
					lcd_fb += ewidth*flag_bit;
				}
				while (dis_width > 0) {
					if (photo_size < 0)
						break;
					write_count = compress_count;
					if (write_count > dis_width)
						write_count = dis_width;

					fbmem_set(lcd_fb, compress_val, write_count);
					lcd_fb += write_count * flag_bit;
					if (compress_count > write_count) {
						compress_count = compress_count - write_count;
					} else {
						photo_ptr += 2;
						photo_size -= 2;
						compress_count = photo_ptr[0];
						compress_val = photo_ptr[1];
					}

					dis_width -= write_count;
				}

				if (ewidth > 0) {
					lcd_fb += ewidth * flag_bit;
				} else {
					int xwidth = -ewidth;
					while (xwidth > 0) {
						unsigned write_count = compress_count;

						if (write_count > xwidth)
							write_count = xwidth;

						if (compress_count > write_count) {
							compress_count = compress_count - write_count;
						} else {
							photo_ptr += 2;
							photo_size -= 2;
							compress_count = photo_ptr[0];
							compress_val = photo_ptr[1];
						}
						xwidth -= write_count;
					}

				}
				dis_height -= 1;
			}

			if (eheight > 0) {
				lcd_fb += eheight * vm_width *flag_bit;
			}
			if (dis_height <= 0)
				return;
		}
	return;
}
void rle_plot(unsigned short *buf, unsigned char *dst_buf)
{
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int flag;
	int bpp;

	unsigned short *photo_ptr = (unsigned short *)buf;
	unsigned short *lcd_fb = (unsigned short *)dst_buf;
	bpp = NBITS(panel_info.vl_bpix);
	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;

	flag =  photo_ptr[0] * photo_ptr[1] - vm_width * vm_height;
	if(flag <= 0){
#ifdef CONFIG_LOGO_EXTEND
		rle_plot_extend(photo_ptr, lcd_fb, bpp);
#else
		rle_plot_smaller(photo_ptr, lcd_fb, bpp);
#endif
	}else if(flag > 0){
		rle_plot_biger(photo_ptr, lcd_fb, bpp);
	}
	return;
}

#else
void rle_plot(unsigned short *buf, unsigned char *dst_buf)
{
}
#endif
void fb_fill(void *logo_addr, void *fb_addr, int count)
{
	//memcpy(logo_buf, fb_addr, count);
	int i;
	int *dest_addr = (int *)fb_addr;
	int *src_addr = (int *)logo_addr;
#ifndef CONFIG_SLCDC_CONTINUA
        int smart_ctrl = 0;
#endif
	for(i = 0; i < count; i = i + 4){
		*dest_addr =  *src_addr;
		src_addr++;
		dest_addr++;
	}
#ifndef CONFIG_SLCDC_CONTINUA
        smart_ctrl = reg_read(SLCDC_CTRL);
        smart_ctrl |= SLCDC_CTRL_DMA_START; //trigger a new frame
        reg_write(SLCDC_CTRL, smart_ctrl);
#endif

}

int jzfb_get_controller_bpp(unsigned int bpp)
{
	switch (bpp) {
	case 18:
	case 24:
		return 32;
	case 15:
		return 16;
	default:
		return bpp;
	}
}

static void jzfb_config_fg0(struct jzfb_config_info *info)
{
	unsigned int rgb_ctrl, cfg;

	/* OSD mode enable and alpha blending is enabled */
	cfg = LCDC_OSDC_OSDEN | LCDC_OSDC_ALPHAEN;	//|  LCDC_OSDC_PREMULTI0;
	cfg |= 1 << 16;		/* once transfer two pixels */
	cfg |= LCDC_OSDC_COEF_SLE0_1;
	/* OSD control register is read only */

	if (info->fmt_order == FORMAT_X8B8G8R8) {
		rgb_ctrl = LCDC_RGBC_RGBFMT | LCDC_RGBC_ODD_BGR |
		    LCDC_RGBC_EVEN_BGR;
	} else {
		/* default: FORMAT_X8R8G8B8 */
		rgb_ctrl = LCDC_RGBC_RGBFMT | LCDC_RGBC_ODD_RGB |
		    LCDC_RGBC_EVEN_RGB;
	}
	reg_write(LCDC_OSDC, cfg);
	reg_write(LCDC_RGBC, rgb_ctrl);
}

static void jzfb_config_tft_lcd_dma(struct jzfb_config_info *info)
{
	struct jz_fb_dma_descriptor *framedesc = info->dmadesc_fbhigh;

#define BYTES_PER_PANEL	 (((info->modes->xres * jzfb_get_controller_bpp(info->bpp) / 8 + 3) >> 2 << 2) * info->modes->yres)
	framedesc->fdadr = virt_to_phys((void *)info->dmadesc_fbhigh);
	framedesc->fsadr = virt_to_phys((void *)info->screen);
	framedesc->fidr = 0xda0;
	framedesc->ldcmd = LCDC_CMD_EOFINT | LCDC_CMD_FRM_EN;
	framedesc->ldcmd |= BYTES_PER_PANEL / 4;
	framedesc->offsize = 0;
	framedesc->page_width = 0;
	info->fdadr0 = virt_to_phys((void *)info->dmadesc_fbhigh);

	switch (jzfb_get_controller_bpp(info->bpp)) {
	case 16:
		framedesc->cmd_num = LCDC_CPOS_RGB_RGB565 | LCDC_CPOS_BPP_16;
		break;
	case 30:
		framedesc->cmd_num = LCDC_CPOS_BPP_30;
		break;
	default:
		framedesc->cmd_num = LCDC_CPOS_BPP_18_24;
		break;
	}
	/* global alpha mode */
	framedesc->cmd_num |= 0;
	/* data has not been premultied */
	framedesc->cmd_num |= LCDC_CPOS_PREMULTI;
	/* coef_sle 0 use 1 */
	framedesc->cmd_num |= LCDC_CPOS_COEF_SLE_1;

	/* fg0 alpha value */
	framedesc->desc_size = 0xff << LCDC_DESSIZE_ALPHA_BIT;
	framedesc->desc_size |=
	    (((info->modes->yres -
	       1) << LCDC_DESSIZE_HEIGHT_BIT & LCDC_DESSIZE_HEIGHT_MASK) |
	     ((info->modes->xres -
	       1) << LCDC_DESSIZE_WIDTH_BIT & LCDC_DESSIZE_WIDTH_MASK));
}

static void jzfb_config_smart_lcd_dma(struct jzfb_config_info *info)
{
	unsigned long bypes_per_panel;
	struct jz_fb_dma_descriptor *framedesc = info->dmadesc_fbhigh;
	struct jz_fb_dma_descriptor *framedesc_cmd[2];

	framedesc_cmd[0] = info->dmadesc_cmd;
	framedesc_cmd[1] = info->dmadesc_cmd_tmp;

	bypes_per_panel =
	    (((info->modes->xres * jzfb_get_controller_bpp(info->bpp)
	       / 8 + 3) >> 2 << 2) * info->modes->yres);
	framedesc->fdadr = virt_to_phys((void *)info->dmadesc_cmd);
	framedesc->fsadr = virt_to_phys((void *)info->screen);
	framedesc->fidr = 0xda0da0;
	framedesc->ldcmd = LCDC_CMD_EOFINT | LCDC_CMD_FRM_EN;
	framedesc->ldcmd |= bypes_per_panel / 4;
	framedesc->offsize = 0;
	framedesc->page_width = 0;

	switch (jzfb_get_controller_bpp(info->bpp)) {
	case 16:
		framedesc->cmd_num = LCDC_CPOS_RGB_RGB565 | LCDC_CPOS_BPP_16;
		break;
	case 30:
		framedesc->cmd_num = LCDC_CPOS_BPP_30;
		break;
	default:
		framedesc->cmd_num = LCDC_CPOS_BPP_18_24;
		break;
	}
	/* global alpha mode */
	framedesc->cmd_num |= 0;
	/* data has not been premultied */
	framedesc->cmd_num |= LCDC_CPOS_PREMULTI;
	/* coef_sle 0 use 1 */
	framedesc->cmd_num |= LCDC_CPOS_COEF_SLE_1;

	/* fg0 alpha value */
	framedesc->desc_size = 0xff << LCDC_DESSIZE_ALPHA_BIT;
	framedesc->desc_size |=
	    (((info->modes->yres -
	       1) << LCDC_DESSIZE_HEIGHT_BIT & LCDC_DESSIZE_HEIGHT_MASK) |
	     ((info->modes->xres -
	       1) << LCDC_DESSIZE_WIDTH_BIT & LCDC_DESSIZE_WIDTH_MASK));

	framedesc_cmd[0]->fdadr = virt_to_phys((void *)info->dmadesc_fbhigh);
	framedesc_cmd[0]->fsadr = 0;
	framedesc_cmd[0]->fidr = 0xda0da1;
	framedesc_cmd[0]->ldcmd = LCDC_CMD_CMD | LCDC_CMD_FRM_EN | 0;
	framedesc_cmd[0]->offsize = 0;
	framedesc_cmd[0]->page_width = 0;
	framedesc_cmd[0]->cmd_num = 0;
	framedesc_cmd[0]->desc_size = 0;

	framedesc_cmd[1]->fdadr = virt_to_phys((void *)info->dmadesc_fbhigh);
	framedesc_cmd[1]->fsadr = virt_to_phys((void *)info->dma_cmd_buf);
	framedesc_cmd[1]->fidr = 0xda0da2;
	framedesc_cmd[1]->offsize = 0;
	framedesc_cmd[1]->page_width = 0;
	framedesc_cmd[1]->desc_size = 0;
    switch (info->smart_config.cmd_width) {
		case SMART_LCD_CWIDTH_8_BIT_ONCE:
        case SMART_LCD_CWIDTH_9_BIT_ONCE://SMART_LCD_CWIDTH_16_BIT_ONCE
			framedesc_cmd[1]->ldcmd = LCDC_CMD_CMD | LCDC_CMD_FRM_EN | 1;
			framedesc_cmd[1]->cmd_num = 4;
			break;
		default://SMART_LCD_CWIDTH_18_BIT_ONCE,SMART_LCD_CWIDTH_24_BIT_ONCE
			framedesc_cmd[1]->ldcmd = LCDC_CMD_CMD | LCDC_CMD_FRM_EN | 1;
			framedesc_cmd[1]->cmd_num = 1;
			break;
	}

	info->fdadr0 = virt_to_phys((void *)info->dmadesc_cmd_tmp);
	flush_cache_all();
}

static void jzfb_config_fg1_dma(struct jzfb_config_info *info)
{
	struct jz_fb_dma_descriptor *framedesc = info->dmadesc_fblow;

	/*
	 * the descriptor of DMA 1 just init once
	 * and generally no need to use it
	 */

#define BYTES_PER_PANEL	 (((info->modes->xres * jzfb_get_controller_bpp(info->bpp) / 8 + 3) >> 2 << 2) * info->modes->yres)
	framedesc->fsadr =
	    virt_to_phys((void *)(info->screen + BYTES_PER_PANEL));
	framedesc->fdadr = (unsigned)virt_to_phys((void *)info->dmadesc_fblow);
	info->fdadr1 = (unsigned)virt_to_phys((void *)info->dmadesc_fblow);

	framedesc->fidr = 0xda1;

	framedesc->ldcmd = (LCDC_CMD_EOFINT & ~LCDC_CMD_FRM_EN)
	    | (BYTES_PER_PANEL / 4);
	framedesc->offsize = 0;
	framedesc->page_width = 0;

	/* global alpha mode, data has not been premultied, COEF_SLE is 11 */
	framedesc->cmd_num =
	    LCDC_CPOS_BPP_18_24 | LCDC_CPOS_COEF_SLE_3 | LCDC_CPOS_PREMULTI;
	framedesc->desc_size |=
	    (((info->modes->yres -
	       1) << LCDC_DESSIZE_HEIGHT_BIT & LCDC_DESSIZE_HEIGHT_MASK) |
	     ((info->modes->xres -
	       1) << LCDC_DESSIZE_WIDTH_BIT & LCDC_DESSIZE_WIDTH_MASK));

	framedesc->desc_size |= 0xff << LCDC_DESSIZE_ALPHA_BIT;

	flush_cache_all();
	reg_write(LCDC_DA1, framedesc->fdadr);
}

static int jzfb_prepare_dma_desc(struct jzfb_config_info *info)
{
	info->dmadesc_fblow =
	    (struct jz_fb_dma_descriptor *)((unsigned long)info->palette -
					    2 * 32);
	info->dmadesc_fbhigh =
	    (struct jz_fb_dma_descriptor *)((unsigned long)info->palette -
					    1 * 32);
	info->dmadesc_cmd =
	    (struct jz_fb_dma_descriptor *)((unsigned long)info->palette -
					    3 * 32);
	info->dmadesc_cmd_tmp =
	    (struct jz_fb_dma_descriptor *)((unsigned long)info->palette -
					    4 * 32);
	if (info->lcd_type != LCD_TYPE_LCM) {
		jzfb_config_tft_lcd_dma(info);
	} else {
		jzfb_config_smart_lcd_dma(info);
	}
	jzfb_config_fg1_dma(info);
	return 0;
}

/* Sent a command without data (18-bit bus, 16-bit index) */
static void slcd_send_mcu_command(struct jzfb_config_info *info,
				  unsigned long cmd)
{
	int count = 10000;
    cmd &= 0x3fffffff;
	while ((reg_read(SLCDC_STATE) & SLCDC_STATE_BUSY) && count--) {
		udelay(10);
	}
	if (count < 0) {
		serial_puts("SLCDC wait busy state wrong\n");
	}
	reg_write(SLCDC_DATA, SLCDC_DATA_RS_COMMAND | cmd);
}

static void slcd_send_mcu_data(struct jzfb_config_info *info,
			       unsigned long data)
{
	int count = 10000;
    data &= 0x3fffffff;
	while ((reg_read(SLCDC_STATE) & SLCDC_STATE_BUSY) && count--) {
		udelay(10);
	}
	if (count < 0) {
		serial_puts("SLCDC wait busy state wrong\n");
	}
	reg_write(SLCDC_DATA, SLCDC_DATA_RS_DATA | data);
}

/* Sent a command with data (18-bit bus, 16-bit index, 16-bit register value) */
static void slcd_set_mcu_register(struct jzfb_config_info *info,
				  unsigned long cmd, unsigned long data)
{
	slcd_send_mcu_command(info, cmd);
	slcd_send_mcu_data(info, data);
}

/* slcd: restart dma */
void soc_lcd_pan_display(void)
{
    int slcd_dma_mode_is_single_dma;
    unsigned int reg;

    slcd_dma_mode_is_single_dma =
        SMART_LCD_DMA_MODE_SINGLE == lcd_config_info.smart_config.dma_mode;

    /* slcd single dma */
    if (lcd_config_info.lcd_type == LCD_TYPE_LCM && slcd_dma_mode_is_single_dma) {
        reg = SLCDC_CTRL_DMA_MODE | SLCDC_CTRL_DMA_EN;
        reg |= SLCDC_CTRL_DMA_START;
        reg_write(SLCDC_CTRL, reg);
    }
}

void lcd_enable(void)
{
	unsigned ctrl;
	if (lcd_enable_state == 0) {
		reg_write(LCDC_STATE, 0);
		reg_write(LCDC_DA0, lcd_config_info.fdadr0);
		ctrl = reg_read(LCDC_CTRL);
		ctrl |= LCDC_CTRL_ENA;
		ctrl &= ~LCDC_CTRL_DIS;
		reg_write(LCDC_CTRL, ctrl);
		serial_puts("dump_lcdc_registers\n");
    }
    lcd_enable_state = 1;
}

void lcd_disable(void)
{
    unsigned ctrl;
    if (lcd_enable_state == 1) {
        if (lcd_config_info.lcd_type != LCD_TYPE_LCM) {
            ctrl = reg_read(LCDC_CTRL);
            ctrl |= LCDC_CTRL_DIS;
            reg_write(LCDC_CTRL, ctrl);
            while (!(reg_read(LCDC_STATE) & LCDC_STATE_LDD)) ;
        } else {
            /* SLCD and TVE only support quick disable */
            ctrl = reg_read(LCDC_CTRL);
            ctrl &= ~LCDC_CTRL_ENA;
            reg_write(LCDC_CTRL, ctrl);
        }
    }
    lcd_enable_state = 0;
}

static void jzfb_slcd_mcu_init(struct jzfb_config_info *info)
{
	unsigned int is_enabled, i;
	unsigned long tmp;

	if (info->lcd_type != LCD_TYPE_LCM)
		return;

	is_enabled = lcd_enable_state;
	if (!is_enabled) {
		lcd_enable();
	}

	if (info->smart_config.length_data_table &&
	    info->smart_config.data_table) {
		for (i = 0; i < info->smart_config.length_data_table; i++) {
			switch (info->smart_config.data_table[i].type) {
			case 0:
				slcd_set_mcu_register(info,
						      info->smart_config.
						      data_table[i].reg,
						      info->smart_config.
						      data_table[i].value);
				break;
			case 1:
				slcd_send_mcu_command(info,
						      info->smart_config.
						      data_table[i].value);
				break;
			case 2:
				slcd_send_mcu_data(info,
						   info->smart_config.
						   data_table[i].value);
				break;
			default:
				serial_puts("Unknow SLCD data type\n");
				break;
			}
			udelay(info->smart_config.data_table[i].udelay);
		}
        {
            int count = 10000;
            while ((reg_read(SLCDC_STATE) & SLCDC_STATE_BUSY)
                    && count--) {
                udelay(10);
            }
            if (count < 0) {
                serial_puts("SLCDC wait busy state wrong");
            }
        }
	}
	 if(info->smart_config.data_width2){
        int tmp = reg_read(SLCDC_CFG);
        tmp &=  ~SMART_LCD_DWIDTH_MASK;
        tmp |= info->smart_config.data_width2;
        reg_write(SLCDC_CFG, tmp);
        printf("mcu init over. the cfg is %08x\n",tmp);
    }

	if (!is_enabled) {
		lcd_disable();
	}
}

static int jzfb_set_par(struct jzfb_config_info *info)
{
	struct fb_videomode *mode = info->modes;
	unsigned short hds, vds;
	unsigned short hde, vde;
	unsigned short ht, vt;
	unsigned cfg, ctrl;
	unsigned size0, size1;
	unsigned smart_cfg = 0, smart_ctrl = 0;;
	unsigned pcfg;

	hds = mode->hsync_len + mode->left_margin;
	hde = hds + mode->xres;
	ht = hde + mode->right_margin;

	vds = mode->vsync_len + mode->upper_margin;
	vde = vds + mode->yres;
	vt = vde + mode->lower_margin;

	/*
	 * configure LCDC config register
	 * use 8words descriptor, not use palette
	 */
	cfg = LCDC_CFG_NEWDES | LCDC_CFG_RECOVER;
	cfg |= info->lcd_type;

	if (!(mode->sync & FB_SYNC_HOR_HIGH_ACT))
		cfg |= LCDC_CFG_HSP;

	if (!(mode->sync & FB_SYNC_VERT_HIGH_ACT))
		cfg |= LCDC_CFG_VSP;

	if (lcd_config_info.pixclk_falling_edge)
		cfg |= LCDC_CFG_PCP;

	if (lcd_config_info.date_enable_active_low)
		cfg |= LCDC_CFG_DEP;

	/* configure LCDC control register */
	ctrl = LCDC_CTRL_BST_64 | LCDC_CTRL_OFUM;
	if (lcd_config_info.pinmd)
		ctrl |= LCDC_CTRL_PINMD;

	ctrl |= LCDC_CTRL_BPP_18_24;
	/* configure smart LCDC registers */
	if (info->lcd_type == LCD_TYPE_LCM) {
		smart_cfg = lcd_config_info.smart_config.smart_type |
		    lcd_config_info.smart_config.cmd_width |
		    lcd_config_info.smart_config.data_width;

		if (lcd_config_info.smart_config.clkply_active_rising)
			smart_cfg |= SLCDC_CFG_CLK_ACTIVE_RISING;
		if (lcd_config_info.smart_config.rsply_cmd_high)
			smart_cfg |= SLCDC_CFG_RS_CMD_HIGH;
		if (lcd_config_info.smart_config.csply_active_high)
			smart_cfg |= SLCDC_CFG_CS_ACTIVE_HIGH;
		/* SLCD DMA mode select 0 */
		smart_ctrl |= SLCDC_CTRL_DMA_MODE | SLCDC_CTRL_DMA_EN;
	}
	if (info->lcd_type != LCD_TYPE_LCM) {
		reg_write(LCDC_VAT, (ht << 16) | vt);
		reg_write(LCDC_DAH, (hds << 16) | hde);
		reg_write(LCDC_DAV, (vds << 16) | vde);

		reg_write(LCDC_HSYNC, mode->hsync_len);
		reg_write(LCDC_VSYNC, mode->vsync_len);
	} else {
		reg_write(LCDC_VAT, (mode->xres << 16) | mode->yres);
		reg_write(LCDC_DAH, mode->xres);
		reg_write(LCDC_DAV, mode->yres);

		reg_write(LCDC_HSYNC, 0);
		reg_write(LCDC_VSYNC, 0);

		reg_write(SLCDC_CFG, smart_cfg);
		reg_write(SLCDC_CTRL, smart_ctrl);
	}

	reg_write(LCDC_CFG, cfg);
	reg_write(LCDC_CTRL, ctrl);

	pcfg = 0xC0000000 | (511 << 18) | (400 << 9) | (256 << 0);
	reg_write(LCDC_PCFG, pcfg);

	size0 =
	    (info->modes->xres << LCDC_SIZE_WIDTH_BIT) & LCDC_SIZE_WIDTH_MASK;
	size0 |=
	    ((info->modes->
	      yres << LCDC_SIZE_HEIGHT_BIT) & LCDC_SIZE_HEIGHT_MASK);
	size1 = size0;
	reg_write(LCDC_SIZE0, size0);
	reg_write(LCDC_SIZE1, size1);

	jzfb_config_fg0(info);

    jzfb_prepare_dma_desc(info);

    if (info->lcd_type == LCD_TYPE_LCM) {
        jzfb_slcd_mcu_init(info);
        lcd_enable();
        if(lcd_config_info.smart_config.dma_mode == SMART_LCD_DMA_MODE_CONTINUOUS)
        {
            unsigned int reg;
            reg &= ~SLCDC_CTRL_DMA_MODE;
            reg |= SLCDC_CTRL_DMA_EN;
            reg |= SLCDC_CTRL_DMA_START;
            reg_write(SLCDC_CTRL, reg);
        }
    }
    return 0;
}

static int jz_lcd_init_mem(void *lcdbase, struct jzfb_config_info *info)
{
	unsigned long palette_mem_size;
	int fb_size =
	    (info->modes->xres * (jzfb_get_controller_bpp(info->bpp) / 8)) *
	    info->modes->yres;

	info->screen = (unsigned long)lcdbase;
	info->palette_size = 256;
	palette_mem_size = info->palette_size * sizeof(u16);

	/* locate palette and descs at end of page following fb */
	info->palette =
	    (unsigned long)lcdbase + fb_size + PAGE_SIZE - palette_mem_size;
    info->dma_cmd_buf = (((unsigned long)lcdbase + fb_size)
            + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if (info->lcd_type == LCD_TYPE_LCM) {
        int i;
        unsigned long cmd[2] = { 0, 0 }, *ptr;

        ptr = (unsigned long *)info->dma_cmd_buf;
        cmd[0] = info->smart_config.write_gram_cmd;
        cmd[1] = cmd[0];
        for (i = 0; i < 4; i += 2) {
            ptr[i] = cmd[0];
            ptr[i + 1] = cmd[1];
        }
        flush_cache_all();
    }
    return 0;
}

void lcd_ctrl_init(void *lcd_base)
{
	/* init registers base address */
	lcd_config_info = jzfb1_init_data;
	lcd_config_info.lcdbaseoff = 0;
	lcd_set_flush_dcache(1);

	lcd_close_backlight();
	panel_pin_init();

#ifdef CONFIG_LCD_FORMAT_X8B8G8R8
	lcd_config_info.fmt_order = FORMAT_X8B8G8R8;
#else
	lcd_config_info.fmt_order = FORMAT_X8R8G8B8;
#endif

	jz_lcd_init_mem(lcd_base, &lcd_config_info);

	board_set_lcd_power_on();

	panel_power_on();

	jzfb_set_par(&lcd_config_info);

	flush_cache_all();

#ifdef DEFAULT_BACKLIGHT_LEVEL
	lcd_set_backlight_level(CONFIG_SYS_BACKLIGHT_LEVEL);
#else
	lcd_set_backlight_level(80);
#endif

	return;
}

void lcd_show_board_info(void)
{
	return;
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	return;
}
