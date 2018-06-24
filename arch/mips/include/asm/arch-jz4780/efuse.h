/*
 * JZ4780 efuse definitions
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
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

#ifndef __EFUSE_H__
#define __EFUSE_H__

#include <asm/arch/base.h>

/*************************************************************************
 * EFUSE (EFUSE Slave Interface)
 *************************************************************************/
#define EFUSE_EFUCTRL	0xD0
#define EFUSE_EFUCFG	0xD4
#define EFUSE_EFUSTATE	0xD8
#define EFUSE_EFUDATA0	0xDC
#define EFUSE_EFUDATA1	0xE0
#define EFUSE_EFUDATA2	0xE4
#define EFUSE_EFUDATA3	0xE8
#define EFUSE_EFUDATA4	0xEC
#define EFUSE_EFUDATA5	0xF0
#define EFUSE_EFUDATA6	0xF4
#define EFUSE_EFUDATA7	0xF8

/* EFUSE Control Register */
#define EFUSE_EFUCTRL_ADDRESS_BIT	21
#define EFUSE_EFUCTRL_ADDRESS_MASK	(0x1ff << EFUSE_EFUCTRL_ADDRESS_BIT)
#define EFUSE_EFUCTRL_LENGTH_BIT	16
#define EFUSE_EFUCTRL_LENGTH_MASK	(0x1f << EFUSE_EFUCTRL_LENGTH_BIT)
#define EFUSE_EFUCTRL_RD_EN		(1 << 0)
#define EFUSE_EFUCTRL_RD_EN_BIT		0

/* EFUSE Configure Register */
#define EFUSE_EFUCFG_RD_ADJ_BIT		20
#define EFUSE_EFUCFG_RD_ADJ_MASK	(0xf << EFUSE_EFUCFG_RD_ADJ_BIT)
#define EFUSE_EFUCFG_RD_STROBE_BIT	16
#define EFUSE_EFUCFG_RD_STROBE_MASK	(0xf << EFUSE_EFUCFG_RD_STROBE_BIT)

#endif /* __EFUSE_H__ */
