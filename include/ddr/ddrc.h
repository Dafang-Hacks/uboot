/*
 * DDR Controller common data structure.
 * Used for JZ4780 JZ4775.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
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

#ifndef __DDRC_H__
#define __DDRC_H__

typedef union ddrc_timing1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tWL:6;
		unsigned reserved6_7:2;
		unsigned tWR:6;
		unsigned reserved14_15:2;
		unsigned tWTR:6;
		unsigned reserved22_23:2;
		unsigned tRTP:6;
		unsigned reserved30_31:2;
	} b;
} ddrc_timing1_t;

typedef union ddrc_timing2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tRL:6;
		unsigned reserved6_7:2;
		unsigned tRCD:6;
		unsigned reserved14_15:2;
		unsigned tRAS:6;
		unsigned reserved22_23:2;
		unsigned tCCD:6;
		unsigned reserved30_31:2;
	} b;

} ddrc_timing2_t;

typedef union ddrc_timing3 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tRC:6;
		unsigned reserved6_7:2;
		unsigned tRRD:6;
		unsigned reserved14_15:2;
		unsigned tRP:6;
		unsigned reserved22_23:2;
		unsigned tCKSRE:3;
		unsigned ONUM:4;
		unsigned reserved31:1;
	} b;
} ddrc_timing3_t;

typedef union ddrc_timing4 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tMRD:2;
		unsigned reserved2_3:2;
		unsigned tXP:3;
		unsigned reserved7:1;
		unsigned tMINSR:4;
		unsigned reserved12_15:4;
		unsigned tCKE:3;
		unsigned tRWCOV:2;
		unsigned tEXTRW:3;
		unsigned tRFC:6;
		unsigned reserved30_31:2;
	} b;
} ddrc_timing4_t;

typedef union ddrc_timing5 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tWDLAT:6;
		unsigned reserved6_7:2;
		unsigned tRDLAT:6;
		unsigned reserved14_15:2;
		unsigned tRTW:6;
		unsigned reserved22_23:2;
		unsigned tCTLUPD:8;
	} b;
} ddrc_timing5_t;

typedef union ddrc_timing6 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tCFGR:6;
		unsigned reserved6_7:2;
		unsigned tCFGW:6;
		unsigned reserved14_15:2;
		unsigned tFAW:6;
		unsigned reserved22_23:2;
		unsigned tXSRD:8;
	} b;
} ddrc_timing6_t;

typedef union ddrc_cfg {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned DW:1;
		unsigned BA0:1;
		unsigned CL:4;
		unsigned CS0EN:1;
		unsigned CS1EN:1;
		unsigned COL0:3;
		unsigned ROW0:3;
		unsigned reserved14:1;
		unsigned MISPE:1;
		unsigned ODTEN:1;
		unsigned TYPE:3;
		unsigned reserved20:1;
		unsigned BSL:1;
		unsigned IMBA:1;
		unsigned BA1:1;
		unsigned COL1:3;
		unsigned ROW1:3;
		unsigned reserved30_31:2;
	} b;
} ddrc_cfg_t;
typedef union ddrc_clkstp_cfg{
	uint32_t d32;
	struct {
		unsigned dly_time:12;
		unsigned reserved:16;
		unsigned sr_cond_en:1;
		unsigned lp_cond_en:1;
		unsigned idle_cond_en:1;
		unsigned clkstp_en:1;
	} b;
} ddrc_clkstp_cfg_t;
struct ddrc_reg {
	ddrc_cfg_t cfg;
	uint32_t ctrl;
	uint32_t refcnt;
	uint32_t mmap[2];
	uint32_t remap[5];
	ddrc_timing1_t timing1;
	ddrc_timing2_t timing2;
	ddrc_timing3_t timing3;
	ddrc_timing4_t timing4;
	ddrc_timing5_t timing5;
	ddrc_timing6_t timing6;
	uint32_t autosr_en;
	ddrc_clkstp_cfg_t clkstp_cfg;
};

#endif /* __DDRC_H__ */
