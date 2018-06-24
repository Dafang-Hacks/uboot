/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

#ifndef __PCA953X_H_
#define __PCA953X_H_
#include <ingenic_soft_i2c.h>

#define PCA953X_IN		0x00
#define PCA953X_OUT		0x01
#define PCA953X_POL		0x02
#define PCA953X_CONF		0x03

#define PCA953X_OUT_LOW		0
#define PCA953X_OUT_HIGH	1
#define PCA953X_POL_NORMAL	0
#define PCA953X_POL_INVERT	1
#define PCA953X_DIR_OUT		0
#define PCA953X_DIR_IN		1

int pca953x_set_val(u8 chip, uint mask, uint data);
int pca953x_set_pol(u8 chip, uint mask, uint data);
int pca953x_set_dir(u8 chip, uint mask, uint data);
int pca953x_get_val(u8 chip);

int pca953x_init();
int pca953x_direction_output(int gpio, int val);
int pca953x_direction_input(int gpio);

int pca953x_set_value(int gpio, int value);
int pca953x_get_value(int gpio);

#define PCA953X_GPIO(gpio)	(CONFIG_PCA953X_GPIO_BASE + gpio)
#define TO_PCA953X_GPIO(gpio)	(gpio - CONFIG_PCA953X_GPIO_BASE)

#endif /* __PCA953X_H_ */
