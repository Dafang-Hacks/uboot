#ifndef _AUO_X163_H_
#define _AUO_X163_H_

#include <jz_lcd/jz_dsim.h>

struct auo_x163_platform_data {
	int gpio_rst;
};

extern struct auo_x163_platform_data auo_x163_pdata;
extern void auo_x163_sleep_in(struct dsi_device *dsi);
extern void auo_x163_sleep_out(struct dsi_device *dsi);
extern void auo_x163_display_on(struct dsi_device *dsi);
extern void auo_x163_display_off(struct dsi_device *dsi);
extern void auo_x163_set_pixel_off(struct dsi_device *dsi); /* set_pixels_off */
extern void auo_x163_set_pixel_on(struct dsi_device *dsi); /* set_pixels_on */
extern void auo_x163_set_brightness(struct dsi_device *dsi, unsigned int brightness); /* set brightness */


#endif /* _AUO_X163_H_ */
