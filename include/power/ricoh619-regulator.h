/*
 * linux/regulator/ricoh619-regulator.h
 *
 * Regulator driver for RICOH R5T619 power management chip.
 *
 * Copyright (C) 2012-2014 RICOH COMPANY,LTD
 *
 * Based on code
 *	Copyright (C) 2011 NVIDIA Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __LINUX_REGULATOR_RICOH619_H
#define __LINUX_REGULATOR_RICOH619_H


#define ricoh619_rails(_name) "RICOH619_"#_name

/* RICHOH Regulator IDs */
enum regulator_id {
	RICOH619_ID_DC1,
	RICOH619_ID_DC2,
	RICOH619_ID_DC3,
	RICOH619_ID_DC4,
	RICOH619_ID_DC5,
	RICOH619_ID_LDO1,
	RICOH619_ID_LDO2,
	RICOH619_ID_LDO3,
	RICOH619_ID_LDO4,
	RICOH619_ID_LDO5,
	RICOH619_ID_LDO6,
	RICOH619_ID_LDO7,
	RICOH619_ID_LDO8,
	RICOH619_ID_LDO9,
	RICOH619_ID_LDO10,
	RICOH619_ID_LDORTC1,
	RICOH619_ID_LDORTC2,
};


#endif
