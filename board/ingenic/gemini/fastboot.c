/*
 * Ingenic gemini setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#include <common.h>
#include <fastboot.h>

struct partition_info partition_info[PARTITION_NUM] = {
	[0] = {.pname = "ndxboot", .offset = 0*1024*1024,.size = 3*1024*1024},
	[1] = {.pname = "ndboot",.offset = 3*1024*1024,.size = 8*1024*1024},
	[2] = {.pname = "ndrecovery",.offset = 11*1024*1024,.size = 8*1024*1024},
	[3] = {.pname = "ndmisc",.offset = 19*1024*1024,.size = 4*1024*1024},
	[4] = {.pname = "ndbattery",.offset = 23*1024*1024,.size = 1*1024*1024},
	[5] = {.pname = "ndcache",.offset = 24*1024*1024,.size = 30*1024*1024},
	[6] = {.pname = "nddevice_id",.offset = 54*1024*1024,.size = 2*1024*1024},
	[7] = {.pname = "ndsystem",.offset = 56*1024*1024,.size = 512*1024*1024},
	[8] = {.pname = "nddata",.offset = 568*1024*1024,.size = 1024*1024*1024},
	[9] = {.pname = "ndstorage1",.offset = 1592*1024*1024,.size = 1024*1024*1024},
};
