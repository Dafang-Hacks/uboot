/*
 * Regulator driver for ACT8600
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: yqfu <yqfu@ingenic.cn>
 * Based on: u-boot-ak47/drivers/regulator/act8600.c
 *           Written by Kage Shen <kkshen@ingenic.cn>
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

#include <config.h>
#include <common.h>
#include <linux/err.h>
#include <linux/list.h>
#include <regulator.h>
#include <i2c.h>
#ifndef CONFIG_SPL_BUILD
#include <power/jz4780_battery.h>
#endif

#define ACT8600_I2C_ADDR    0x5a
static int act8600_write_reg(u8 reg, u8 *val);

#ifndef CONFIG_SPL_BUILD

#define ACT8600_NUM_LODX    (8 * 8)
#define ACT8600_NUM_LOD4    (6 * 32 - 7)

#define APCH_INTR0  0xa1
#define SUSCHG      (1 << 7)

#define APCH_STAT   0xaa
#define CHG_ACIN    (1 << 7)
#define CHG_USB     (1 << 6)

#define OTG_CON     0xb0
#define DBILIMQ3    (1 << 1)

enum {
	act8600_out1 = 1,
	act8600_out2,
	act8600_out3,
	act8600_out4,
	act8600_out5,
	act8600_out6,
	act8600_out7,
	act8600_out8,
	act8600_out9,
	act8600_out10,
	charger_out,
};

static struct regulator_ops act8600_ops;
static struct regulator_ops charger_ops;

static struct regulator act8600_regulators[] = {
	{
		.name = "OUT1",
		.id = act8600_out1,
		.n_voltages = ACT8600_NUM_LODX,
		.ops = &act8600_ops,
#if defined(CONFIG_PMU_ACT8600_OUT1_MIN) && defined(CONFIG_PMU_ACT8600_OUT1_MAX)
		.min_uV = CONFIG_PMU_ACT8600_OUT1_MIN,
		.max_uV = CONFIG_PMU_ACT8600_OUT1_MAX,
#else
		.min_uV = 1000000,
		.max_uV = 1300000,
#endif
	},
	{
		.name = "OUT2",
		.id = act8600_out2,
		.n_voltages = ACT8600_NUM_LODX,
		.ops = &act8600_ops,
#if defined(CONFIG_PMU_ACT8600_OUT2_MIN) && defined(CONFIG_PMU_ACT8600_OUT2_MAX)
		.min_uV = CONFIG_PMU_ACT8600_OUT2_MIN,
		.max_uV = CONFIG_PMU_ACT8600_OUT2_MAX,
#else
		.min_uV = 1200000,
		.max_uV = 1800000,
#endif
	},
	{
		.name = "OUT3",
		.id = act8600_out3,
		.n_voltages = ACT8600_NUM_LODX,
		.ops = &act8600_ops,
#if defined(CONFIG_PMU_ACT8600_OUT3_MIN) && defined(CONFIG_PMU_ACT8600_OUT3_MAX)
		.min_uV = CONFIG_PMU_ACT8600_OUT3_MIN,
		.max_uV = CONFIG_PMU_ACT8600_OUT3_MAX,
#else
		.min_uV = 3000000,
		.max_uV = 3500000,
#endif
	},
	{
		.name = "OUT4",
		.id = act8600_out4,
		.n_voltages = ACT8600_NUM_LOD4,
		.ops = &act8600_ops,
#if defined(CONFIG_PMU_ACT8600_OUT4_MIN) && defined(CONFIG_PMU_ACT8600_OUT4_MAX)
		.min_uV = CONFIG_PMU_ACT8600_OUT4_MIN,
		.max_uV = CONFIG_PMU_ACT8600_OUT4_MAX,
#else
		.min_uV = 0,
		.max_uV = 0,
#endif
	},
	{
		.name = "OUT5",
		.id = act8600_out5,
		.n_voltages = ACT8600_NUM_LODX,
		.ops = &act8600_ops,
#if defined(CONFIG_PMU_ACT8600_OUT5_MIN) && defined(CONFIG_PMU_ACT8600_OUT5_MAX)
		.min_uV = CONFIG_PMU_ACT8600_OUT5_MIN,
		.max_uV = CONFIG_PMU_ACT8600_OUT5_MAX,
#else
		.min_uV = 0,
		.max_uV = 0,
#endif
	},
	{
		.name = "OUT6",
		.id = act8600_out6,
		.n_voltages = ACT8600_NUM_LODX,
		.ops = &act8600_ops,
#if defined(CONFIG_PMU_ACT8600_OUT6_MIN) && defined(CONFIG_PMU_ACT8600_OUT6_MAX)
		.min_uV = CONFIG_PMU_ACT8600_OUT6_MIN,
		.max_uV = CONFIG_PMU_ACT8600_OUT6_MAX,
#else
		.min_uV = 0,
		.max_uV = 0,
#endif
	},
	{
		.name = "OUT7",
		.id = act8600_out7,
		.n_voltages = ACT8600_NUM_LODX,
		.ops = &act8600_ops,
#if defined(CONFIG_PMU_ACT8600_OUT7_MIN) && defined(CONFIG_PMU_ACT8600_OUT7_MAX)
		.min_uV = CONFIG_PMU_ACT8600_OUT7_MIN,
		.max_uV = CONFIG_PMU_ACT8600_OUT7_MAX,
#else
		.min_uV = 0,
		.max_uV = 0,
#endif
	},
	{
		.name = "OUT8",
		.id = act8600_out8,
		.n_voltages = ACT8600_NUM_LODX,
		.ops = &act8600_ops,
#if defined(CONFIG_PMU_ACT8600_OUT8_MIN) && defined(CONFIG_PMU_ACT8600_OUT8_MAX)
		.min_uV = CONFIG_PMU_ACT8600_OUT8_MIN,
		.max_uV = CONFIG_PMU_ACT8600_OUT8_MAX,
#else
		.min_uV = 0,
		.max_uV = 0,
#endif
	},
	{
		.name = "charger",
		.id = charger_out,
		.ops = &charger_ops,
		.min_uA = 1,
		.max_uA = 1000000,
	},
};

static int act8600_read_reg(u8 reg, u8 *val, u32 len)
{
	int ret;
	ret = i2c_read(ACT8600_I2C_ADDR, reg, 1, val, len);
	if(ret) {
		printf("act8600 read register error\n");
		return -EIO;
	}
	return 0;
}

static int selector2vol(int id, u32 sele, u8 *typ)
{
	u32 vol;

	if(id == act8600_out4) {
		if(sele < 96) {
			vol = 3000000;
			vol += sele * 100000;
		} else if(sele < 128) {
			vol = 12600000;
			vol += (sele - 96) * 200000;
		} else if(sele <= 192) {
			vol = 19000000;
			vol += (sele - 128) * 400000;
		} else {
			goto ERROR;
		}
		if(typ != NULL)
			*typ = sele + 0x40;
	} else {

		if(sele < 0x18) {                // 0b011000
			vol = 600000;
			vol += sele * 25000;
		} else if(sele < 0x30) {         // 0b110000
			vol = 1200000;
			vol += (sele - 0x18) * 50000;
		} else if(sele <= 0x3f) {        // 0b111111
			vol = 2400000;
			vol += (sele - 0x30) * 100000;
		} else {
			goto ERROR;
		}
		if(typ != NULL)
			*typ = sele;
	}
	return vol;

ERROR:
	printf("selector:0x%0x to voltage error! id:%d\n", sele, id);
	return 0;
}

static int act8600_list_voltage(struct regulator *reg, u32 sele)
{
	if (sele > reg->n_voltages)
		return -EINVAL;
	return selector2vol(reg->id, sele, NULL);
}

static int voltages_to_value(struct regulator *reg, int min_uV, int max_uV)
{
	u32 i;
	u32 vol;
	u32 val;

	for (i = 0; i <= reg->n_voltages; i++) {
		vol = selector2vol(reg->id, i, (u8 *)&val);
		if (vol >= min_uV) {
			return val & 0xff;
		}
	}
	return -EINVAL;
}

static int value_to_voltage(struct regulator *reg, u8 value)
{
	u32 i;
	u32 vol;
	u8 val;

	for (i = 0; i <= reg->n_voltages; i++) {
		vol = selector2vol(reg->id, i, &val);
		if (val == value) {
			return vol;
		}
	}
	return -EINVAL;
}

enum act8600_status {
	PMU_READ = 'A',
	PMU_ENABLE,
	PMU_DISABLE,
};

static int _act8600_change_status(struct regulator *reg,
		enum act8600_status st)
{
	u8 val;
	int ret;
	u32 addr;
	u32 flag;
	u32 index = reg->id;

	if(reg == NULL)
		return -EINVAL;

	switch(index) {
	case act8600_out1 ... act8600_out3:
		addr = (index << 4) + 2;
		flag = 0x80;
		break;
	case act8600_out4 ... act8600_out8:
		addr = (index << 4) + 1;
		flag = 0x80;
		break;
	case act8600_out9:
		addr = 0x91;
		flag = 0x80;
		break;
	case act8600_out10:
		addr = 0x91;
		flag = 0x40;
		break;
	default:
		return -EINVAL;
	}

	ret = act8600_read_reg(addr, &val, 1);
	if(ret)
		return ret;

	switch(st) {
	case PMU_READ:
		ret = val & flag;
		return ret ? true : false;
	case PMU_ENABLE:
		val |= flag;
		ret = act8600_write_reg(addr, &val);
		break;
	case PMU_DISABLE:
		val &= ~flag;
		ret = act8600_write_reg(addr, &val);
		break;
	default:
		return -EINVAL;
	}

	if(ret)
		return ret;
	return 0;
}

static int act8600_enable(struct regulator *reg)
{
	return _act8600_change_status(reg, PMU_ENABLE);
}

static int act8600_disable(struct regulator *reg)
{
	return _act8600_change_status(reg, PMU_DISABLE);
}

static int act8600_is_enabled(struct regulator *reg)
{
	return _act8600_change_status(reg, PMU_READ);
}

static int act8600_get_voltage(struct regulator *reg)
{
	u8 val;
	int ret = 0;
	u32 index = reg->id;

	if(reg == NULL)
		return -EINVAL;

	ret = act8600_read_reg(index << 4, &val, 1);
	if(ret)
		return ret;

	if(index != act8600_out4)
		val &= 0x3f;

	switch(index) {
	case act8600_out1 ... act8600_out8:
		ret = value_to_voltage(reg, val);
		break;
	case act8600_out9 ... act8600_out10:
		ret = -EINVAL;
		break;
	}

	if(ret < 0) {
		printf("%s can not get voltage value\n", reg->name);
	}

	return ret;
}

static int act8600_set_voltage(struct regulator *reg, int min_uV, int max_uV)
{
	int val;
	int ret;
	u32 index = reg->id;

	if(reg == NULL)
		return -EINVAL;

	ret = act8600_read_reg(index << 4, (u8 *)&val, 1);
	if(ret)
		return -EIO;

	switch(index) {
	case act8600_out1 ... act8600_out8:
		val = voltages_to_value(reg, min_uV, max_uV);
		if(val < 0) {
			printf("the voltage value is not supported\n");
			return val;
		}
		ret = act8600_write_reg(index << 4, (u8 *)&val);
		break;
	case act8600_out9 ... act8600_out10:
		ret = -EINVAL;
		break;
	}

	if(ret < 0) {
		printf("%s can not set voltage value\n", reg->name);
	}

	return ret;
}

static struct regulator_ops act8600_ops = {
	.list_voltage = act8600_list_voltage,
	.disable = act8600_disable,
	.enable = act8600_enable,
	.is_enabled = act8600_is_enabled,

	.set_voltage = act8600_set_voltage,
	.get_voltage = act8600_get_voltage,
};

static int act8600_charger_status(struct regulator *reg,
		enum act8600_status st)
{
	u8 value;
	int ret;

	ret = act8600_read_reg(APCH_INTR0, &value, 1);
	if(ret)
		return ret;

	switch(st) {
	case PMU_READ:
		return ((value & SUSCHG) ? false : true);
		break;
	case PMU_ENABLE:
		value &= ~SUSCHG;
		break;
	case PMU_DISABLE:
		value |= SUSCHG;
		break;
	}

	ret = act8600_write_reg(APCH_INTR0, &value);
	return ret;
}

static int act8600_charger_disable(struct regulator *reg)
{
	return act8600_charger_status(reg, PMU_DISABLE);
}

static int act8600_charger_enable(struct regulator *reg)
{
	return act8600_charger_status(reg, PMU_ENABLE);
}

static int act8600_charger_is_enabled(struct regulator *reg)
{
	return act8600_charger_status(reg, PMU_READ);
}

static int act8600_charger_source(void)
{
	int ret;
	u8 value;

	ret = act8600_read_reg(APCH_STAT, &value, 1);
	if(ret)
		return ret;

	switch((value & 0xc0) >> 6) {
	case 2:
		ret = CHARGER_AC;
		break;
	case 1:
		ret = CHARGER_USB;
		break;
	default:
		ret = CHARGER_NONE;
		break;
	}

	return ret;
}

static int act8600_charger_set_current(struct regulator *reg, int min_uA, int max_uA)
{
	int ret;
	u8 value;

	ret = act8600_charger_source();
	if(ret < 0)
		return ret;

	/* charger current is determined by the external circuit */
	if(ret != CHARGER_USB) {
		debug("%s cannot set charger current\n", reg->name);
		return 0;
	}

	ret = act8600_read_reg(OTG_CON, &value, 1);
	if(ret)
		return ret;

	if(min_uA < 400000){
		value &= ~DBILIMQ3;
		ret = act8600_write_reg(OTG_CON, &value);
	} else {
		value |= DBILIMQ3;
		ret = act8600_write_reg(OTG_CON, &value);
	}

	return 0;
}

static int act8600_charger_get_current(struct regulator *reg)
{
	int ret;
	u8 value;

	ret = act8600_charger_source();
	if(ret < 0)
		return ret;

	/* charger current is determined by the external circuit */
	if(ret != CHARGER_USB) {
		debug("%s cannot set charger current\n", reg->name);
		return 0;
	}

	ret = act8600_read_reg(OTG_CON, &value, 1);
	if(ret)
		return ret;

	return (value & DBILIMQ3) ? 800000 : 400000;
}

static int act8600_charger_get_voltage (struct regulator *reg)
{
	return 0;
	/* return get_battery_voltage(); */
}

static struct regulator_ops charger_ops = {
	.disable = act8600_charger_disable,
	.enable = act8600_charger_enable,
	.is_enabled = act8600_charger_is_enabled,

	.set_current = act8600_charger_set_current,
	.get_current = act8600_charger_get_current,

	.get_voltage = act8600_charger_get_voltage,
};

/*
static struct charger_ops act8600_battery_ops = {
	.get_charger_status = act8600_charger_source,
	.charger_disable = act8600_charger_disable,
	.charger_enable = act8600_charger_enable,
	.charger_is_enabled = act8600_charger_is_enabled,

	.set_current = act8600_charger_set_current,
	.get_current = act8600_charger_get_current,
};
*/

int act8600_regulator_init(void)
{
	int ret;
	int i;

	ret = i2c_probe(ACT8600_I2C_ADDR);
	if(ret) {
		printf("probe act8600 error, i2c addr ox%x\n", ACT8600_I2C_ADDR);
		return -EIO;
	}
	for (i = 0; i < ARRAY_SIZE(act8600_regulators); i++) {
		ret = regulator_register(&act8600_regulators[i], NULL);
		if(ret)
			printf("%s regulator_register error\n",
					act8600_regulators[i].name);
	}

	/* jz4780_battery_cherger(&act8600_battery_ops); */

	return 0;
}
#endif /* !CONFIG_SPL_BUILD */

#ifdef CONFIG_SPL_BUILD
#define ACT8600_REG1_VSET       0x10
#define ACT8600_REG2_VSET       0x20
#define ACT8600_REG3_VSET       0x30

int spl_regulator_set_voltage(enum regulator_outnum outnum, int vol_mv)
{
	char reg;
	u8 regvalue;
	switch(outnum) {
	case REGULATOR_CORE:
		reg = ACT8600_REG1_VSET;
		if ((vol_mv < 1000) || (vol_mv >1300)) {
			debug("voltage for core is out of range\n");
			return -EINVAL;
		}
		break;
	case REGULATOR_MEM:
		reg = ACT8600_REG2_VSET;
		break;
	case REGULATOR_IO:
		reg = ACT8600_REG3_VSET;
		break;
	default:return -EINVAL;
	}

	if ((vol_mv < 600) || (vol_mv > 3900)) {
		debug("unsupported voltage\n");
		return -EINVAL;
	} else if (vol_mv < 1200) {
		regvalue = (vol_mv - 600) / 25;
	} else if (vol_mv < 2400) {
		regvalue = (vol_mv - 1200) / 50 + 24;
	} else {
		regvalue = (vol_mv - 2400) / 100 + 48;
	}

	return act8600_write_reg(reg, &regvalue);
}
#endif /* CONFIG_SPL_BUILD */

static int act8600_write_reg(u8 reg, u8 *val)
{
	int ret;
	ret = i2c_write(ACT8600_I2C_ADDR, reg, 1, val, 1);
	if(ret) {
		debug("act8600 write register error\n");
		return -EIO;
	}
	return 0;
}
