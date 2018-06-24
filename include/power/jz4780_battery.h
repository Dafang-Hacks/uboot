/*
 * Battery measurement code for Ingenic JZ SoC
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Kage Shen <kkshen@ingenic.cn>
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

#ifndef __JZ4780_BATTERY_H__
#define __JZ4780_BATTERY_H__

enum battery_status {
	CHARGER_AC = 'B',
	CHARGER_USB,
	CHARGER_NONE,
};

struct charger_ops {
	/* enumerate supported voltages */
	int (*get_charger_status) (void);

	/* enable/disable regulator */
	int (*charger_enable) (struct regulator *);
	int (*charger_disable) (struct regulator *);
	int (*charger_is_enabled) (struct regulator *);

	int (*set_current) (struct regulator *, int min_uA, int max_uA);
	int (*get_current) (struct regulator *);
};

struct battery_dec {
	char *name;
	struct regulator *reg;
	/* enumerate supported voltages */
	struct charger_ops *ops;
};

extern int jz4780_battery_cherger(struct charger_ops *);
extern int get_battery_voltage(void);

#endif
