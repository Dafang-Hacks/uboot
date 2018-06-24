/*
 * drivers/regulator/ricoh619-regulator.c
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

/* #define DEBUG			1 */
/*#define VERBOSE_DEBUG		1*/
#include <config.h>
#include <common.h>
#include <linux/err.h>
#include <linux/list.h>
#include <regulator.h>
//#include <i2c.h>
//
#include <ingenic_soft_i2c.h>
#ifndef CONFIG_SPL_BUILD
#include <power/ricoh619.h>
#include <power/ricoh619-regulator.h>
#endif

#define RICOH61x_I2C_ADDR    0x32

#define RICOH619_DC1  0x36
#define RICOH619_DC2  0x37
#define RICOH619_DC3  0x38
#define RICOH619_DC4  0x39
#define	RICOH619_DC5  0x3A
#define	RICOH619_LDO1 0x4c
#define	RICOH619_LDO2 0x4d
#define	RICOH619_LDO3 0x4e
#define	RICOH619_LDO4 0x4f
#define	RICOH619_LDO5 0x50
#define	RICOH619_LDO6 0x51
#define	RICOH619_LDO7 0x52
#define	RICOH619_LDO8 0x53
#define	RICOH619_LDO9 0x54
#define	RICOH619_LDO10 0x55
#define	RICOH619_LDORTC1 0x56
#define	RICOH619_LDORTC2 0x57

#ifndef CONFIG_SPL_BUILD
struct ricoh61x_regulator {
	int		id;
	int		sleep_id;
	/* Regulator register address.*/
	u8		reg_en_reg;
	u8		en_bit;
	u8		reg_disc_reg;
	u8		disc_bit;
	u8		vout_reg;
	u8		vout_mask;
	u8		vout_reg_cache;
	u8		sleep_reg;
	u8		eco_reg;
	u8		eco_bit;
	u8		eco_slp_reg;
	u8		eco_slp_bit;

	/* chip constraints on regulator behavior */
	int			min_uV;
	int			max_uV;
	int			step_uV;
	int			nsteps;

	/* regulator specific turn-on delay */
	u16			delay;

	/* used by regulator core */
	struct regulator	desc;

	/* Device */
	struct device		*dev;
};

enum regulator_type {
	REGULATOR_VOLTAGE,
	REGULATOR_CURRENT,
};
#endif

static struct i2c ricoh61x_i2c;
static struct i2c *i2c;

#ifndef CONFIG_BATTERYDET_LED
static int ricoh61x_write_reg(u8 reg, u8 *val)
#else
int ricoh61x_write_reg(u8 reg, u8 *val)
#endif
{
	unsigned int  ret;
	ret = i2c_write(i2c, RICOH61x_I2C_ADDR, reg, 1, val, 1);
	if(ret) {
		debug("ricoh61x write register error\n");
		return -EIO;
	}
	return 0;
}

#ifndef CONFIG_SPL_BUILD
#ifndef CONFIG_BATTERYDET_LED
static int ricoh61x_read_reg(u8 reg, u8 *val, u32 len)
#else
int ricoh61x_read_reg(u8 reg, u8 *val, u32 len)
#endif
{
	int ret;
	ret = i2c_read(i2c, RICOH61x_I2C_ADDR, reg, 1, val, len);
	if(ret) {
		printf("ricoh61x read register error\n");
		return -EIO;
	}
	return 0;
}
void *rdev_get_drvdata(struct regulator *rdev)
{
	    return rdev->reg_data;
}

int rdev_set_drvdata(struct regulator *rdev, void *data)
{

	rdev->reg_data = data;
	return 0;
}

int ricoh61x_set_bits(u8 reg, uint8_t bit_mask)
{
	uint8_t reg_val;
	int ret = 0;

		ret = ricoh61x_read_reg( reg, &reg_val,1);
		if (ret){
			debug("ricoh61x read error\n");
			return 1;
		}

		if ((reg_val & bit_mask) != bit_mask) {
			reg_val |= bit_mask;
			ret = ricoh61x_write_reg(reg,&reg_val);
			if(ret){
				debug("ricoh61x write error\n");
				return 1;
			}
		}
		return 0;
}

int ricoh61x_clr_bits(u8 reg, uint8_t bit_mask)
{
	uint8_t reg_val;
	int ret = 0;

	ret = ricoh61x_read_reg( reg, &reg_val,1);
	if (ret){
		debug("ricoh61x read error\n");
		return 1;
	}

	if (reg_val & bit_mask) {
		reg_val &= ~bit_mask;
		ret = ricoh61x_write_reg(reg,&reg_val);
		if(ret){
			debug("ricoh61x write error\n");
			return 1;
		}

	}
	return 0 ;
}

int ricoh619_power_off(void)
{
	int ret,g_soc = 100,g_fg_on_mode = 0;
	uint8_t reg_val;
	reg_val = g_soc;
	reg_val &= 0x7f;

	ret = ricoh61x_write_reg(RICOH61x_PSWR, &reg_val);
	if (ret < 0)
		printf("Error in writing PSWR_REG\n");
	if (g_fg_on_mode == 0) {
		/* Clear RICOH61x_FG_CTRL 0x01 bit */
		ret = ricoh61x_read_reg(RICOH61x_FG_CTRL, &reg_val,1);
		if (reg_val & 0x01) {
			reg_val &= ~0x01;
			ret = ricoh61x_write_reg(RICOH61x_FG_CTRL, &reg_val);
		}
		if (ret < 0)
			printf("Error in writing FG_CTRL\n");
	}
	/* set rapid timer 300 min */
	ret = ricoh61x_read_reg(TIMSET_REG, &reg_val,1);
	reg_val |= 0x03;
	ret = ricoh61x_write_reg(TIMSET_REG, &reg_val);
	if (ret < 0)
		printf("Error in writing the TIMSET_Reg\n");

	/* Disable all Interrupt */
	reg_val = 0;
	ricoh61x_write_reg(RICOH61x_INTC_INTEN, &reg_val);
	/* Not repeat power ON after power off(Power Off/N_OE) */
	reg_val = 0x0;
	ricoh61x_write_reg(RICOH61x_PWR_REP_CNT, &reg_val);
	/* Power OFF */
	reg_val = 0x1;
	ricoh61x_write_reg(RICOH61x_PWR_SLP_CNT, &reg_val);
	return 0;
}

static int ricoh61x_reg_enable(struct regulator *rdev)
{
	struct ricoh61x_regulator *ri = rdev_get_drvdata(rdev);
	u8 value;
	int ret;

	ret = ricoh61x_set_bits(ri->reg_en_reg, (1 << ri->en_bit));
	if(ret){
		printf("ricoh61x set bit is error\n");
	}
	return ret;
}

static int ricoh61x_reg_disable(struct regulator *rdev)
{
	struct ricoh61x_regulator *ri = rdev_get_drvdata(rdev);
	u8 value;
	int ret;

	ret = ricoh61x_clr_bits(ri->reg_en_reg, (1 << ri->en_bit));
	if(ret){
		printf("ricoh61x set clr is error\n");
	}

return ret;
}


static int __ricoh61x_set_voltage( struct ricoh61x_regulator *ri,
		int min_uV, int max_uV, unsigned *selector)
{
	int vsel;
	int ret;
	uint8_t vout_val;

	if ((min_uV < ri->min_uV) || (max_uV > ri->max_uV))
		return -EDOM;

	vsel = (min_uV - ri->min_uV + ri->step_uV - 1)/ri->step_uV;
	if (vsel > ri->nsteps)
		return -EDOM;

	if (selector)
		*selector = vsel;

	vout_val =  (vsel & ri->vout_mask);
	ret = ricoh61x_write_reg(ri->vout_reg, &vout_val);
	if (ret < 0)
		printf("Error in writing the Voltage register\n");

	return ret;
}

static  int ricoh61x_set_voltage(struct regulator *rdev,
		int min_uV, int max_uV, unsigned *selector)
{
	struct ricoh61x_regulator *ri = rdev_get_drvdata(rdev);

	return __ricoh61x_set_voltage(ri, min_uV, max_uV, selector);
}

void test_richo()
{
	int val =0;
	//spl_ricoh61x_regulator_set_voltage(RICOH619_LDO4, 1200);
	ricoh61x_read_reg(0x2c, &val, 1);
	printf("val:%d\n", val);
	ricoh61x_read_reg(0xbd, &val, 1);
	printf("val:%d\n", val);
//	ricoh61x_read_reg(u8 reg, u8 *val, u32 len);
}
int ricoh61x_reg_charge_status(u8 reg, uint8_t bit_status)
{
	int ret = 0;
	uint8_t reg_val;
	ret = ricoh61x_read_reg( reg, &reg_val,1);
	return (((reg_val >> bit_status) & 1) == 1);
}

static int ricoh61x_reg_is_enabled(struct regulator *regulator)
{
	uint8_t control;
	struct ricoh61x_regulator *ri = rdev_get_drvdata(regulator);

	ricoh61x_read_reg(ri->reg_en_reg, &control,1);
	return (((control >> ri->en_bit) & 1) == 1);
}

static struct regulator_ops ricoh61x_ops = {

	.disable = ricoh61x_reg_disable,
	.enable = ricoh61x_reg_enable,
	.set_voltage = ricoh61x_set_voltage,
	.is_enabled	= ricoh61x_reg_is_enabled,

#if 0
	.list_voltage	= ricoh61x_list_voltage,
	.set_voltage	= ricoh61x_set_voltage,
	.get_voltage	= ricoh61x_get_voltage,
	.enable		= ricoh61x_reg_enable,
	.disable	= ricoh61x_reg_disable,
	.enable_time	= ricoh61x_regulator_enable_time,
#endif

};

#define RICOH61x_REG(_id, _en_reg, _en_bit, _disc_reg, _disc_bit, _vout_reg, \
		_vout_mask, _ds_reg, _min_mv, _max_mv, _step_uV, _nsteps,    \
		_ops, _delay, _eco_reg, _eco_bit, _eco_slp_reg, _eco_slp_bit) \
{								\
	.reg_en_reg	= _en_reg,				\
	.en_bit		= _en_bit,				\
	.reg_disc_reg	= _disc_reg,				\
	.disc_bit	= _disc_bit,				\
	.vout_reg	= _vout_reg,				\
	.vout_mask	= _vout_mask,				\
	.sleep_reg	= _ds_reg,				\
	.step_uV	= _step_uV,				\
	.nsteps		= _nsteps,				\
	.delay		= _delay,				\
	.id		= RICOH619_ID_##_id,			\
	.sleep_id	= RICOH619_DS_##_id,			\
	.eco_reg	=  _eco_reg,				\
	.eco_bit	=  _eco_bit,				\
	.min_uV		= _min_mv * 1000,			\
	.max_uV		= _max_mv * 1000,			\
	.eco_slp_reg	=  _eco_slp_reg,			\
	.eco_slp_bit	=  _eco_slp_bit,			\
	.desc = {						\
		.name = ricoh619_rails(_id),			\
		.id = RICOH619_ID_##_id,			\
		.min_uV		= _min_mv * 1000,			\
		.max_uV		= _max_mv * 1000,			\
		.n_voltages = _nsteps,				\
		.ops = &_ops,					\
	},							\
}

static struct ricoh61x_regulator ricoh61x_regulator[] = {
	RICOH61x_REG(DC1, 0x2C, 0, 0x2C, 1, 0x36, 0xFF, 0x3B,
			600, 3500, 12500, 0xE8, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),

	RICOH61x_REG(DC2, 0x2E, 0, 0x2E, 1, 0x37, 0xFF, 0x3C,
			600, 3500, 12500, 0xE8, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),

	RICOH61x_REG(DC3, 0x30, 0, 0x30, 1, 0x38, 0xFF, 0x3D,
			600, 3500, 12500, 0xE8, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),

	RICOH61x_REG(DC4, 0x32, 0, 0x32, 1, 0x39, 0xFF, 0x3E,
			600, 3500, 12500, 0xE8, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),

	RICOH61x_REG(DC5, 0x34, 0, 0x34, 1, 0x3A, 0xFF, 0x3F,
			600, 3500, 12500, 0xE8, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),

	RICOH61x_REG(LDO1, 0x44, 0, 0x46, 0, 0x4C, 0x7F, 0x58,
			900, 3500, 25000, 0x68, ricoh61x_ops, 500,
			0x48, 0, 0x4A, 0),

	RICOH61x_REG(LDO2, 0x44, 1, 0x46, 1, 0x4D, 0x7F, 0x59,
			900, 3500, 25000, 0x68, ricoh61x_ops, 500,
			0x48, 1, 0x4A, 1),

	RICOH61x_REG(LDO3, 0x44, 2, 0x46, 2, 0x4E, 0x7F, 0x5A,
			900, 3500, 25000, 0x68, ricoh61x_ops, 500,
			0x48, 2, 0x4A, 2),

	RICOH61x_REG(LDO4, 0x44, 3, 0x46, 3, 0x4F, 0x7F, 0x5B,
			900, 3500, 25000, 0x68, ricoh61x_ops, 500,
			0x48, 3, 0x4A, 3),

	RICOH61x_REG(LDO5, 0x44, 4, 0x46, 4, 0x50, 0x7F, 0x5C,
			600, 3500, 25000, 0x74, ricoh61x_ops, 500,
			0x48, 4, 0x4A, 4),

	RICOH61x_REG(LDO6, 0x44, 5, 0x46, 5, 0x51, 0x7F, 0x5D,
			600, 3500, 25000, 0x74, ricoh61x_ops, 500,
			0x48, 5, 0x4A, 5),

	RICOH61x_REG(LDO7, 0x44, 6, 0x46, 6, 0x52, 0x7F, 0x5E,
			900, 3500, 25000, 0x68, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),

	RICOH61x_REG(LDO8, 0x44, 7, 0x46, 7, 0x53, 0x7F, 0x5F,
			900, 3500, 25000, 0x68, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),

	RICOH61x_REG(LDO9, 0x45, 0, 0x47, 0, 0x54, 0x7F, 0x60,
			900, 3500, 25000, 0x68, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),

	RICOH61x_REG(LDO10, 0x45, 1, 0x47, 1, 0x55, 0x7F, 0x61,
			900, 3500, 25000, 0x68, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),


	RICOH61x_REG(LDORTC1, 0x45, 4, 0x00, 0, 0x56, 0x7F, 0x00,
			1700, 3500, 25000, 0x48, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),

	RICOH61x_REG(LDORTC2, 0x45, 5, 0x00, 0, 0x57, 0x7F, 0x00,
			900, 3500, 25000, 0x68, ricoh61x_ops, 500,
			0x00, 0, 0x00, 0),
};

int ricoh61x_regulator_init(void)
{

	int ret,i;
	ricoh61x_i2c.scl = CONFIG_RICOH61X_I2C_SCL;
	ricoh61x_i2c.sda = CONFIG_RICOH61X_I2C_SDA;
	i2c = &ricoh61x_i2c;
	i2c_init(i2c);

	ret = i2c_probe(i2c, RICOH61x_I2C_ADDR);

	if(ret) {
		printf("probe richo61x error, i2c addr ox%x\n", RICOH61x_I2C_ADDR);
		return -EIO;
	}
	for (i = 0; i < ARRAY_SIZE(ricoh61x_regulator); i++) {
		ret = regulator_register(&ricoh61x_regulator[i].desc, NULL);
		rdev_set_drvdata(&ricoh61x_regulator[i].desc,&ricoh61x_regulator[i]);
		if(ret)
			printf("%s regulator_register error\n",
					ricoh61x_regulator[i].desc.name);
	}

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
int spl_regulator_init()
{
	int ret;

	ricoh61x_i2c.scl = CONFIG_RICOH61X_I2C_SCL;
	ricoh61x_i2c.sda = CONFIG_RICOH61X_I2C_SDA;
	i2c = &ricoh61x_i2c;
	i2c_init(i2c);

	ret = i2c_probe(i2c, RICOH61x_I2C_ADDR);

	return ret;
}
int spl_regulator_set_voltage(enum regulator_outnum outnum, int vol_mv)
{
	char reg;
	u8 regvalue;

	switch(outnum) {
		case REGULATOR_CORE:
			reg = RICOH619_DC1;
			if ((vol_mv < 1000) || (vol_mv >1300)) {
				debug("voltage for core is out of range\n");
				return -EINVAL;
			}
			break;
		case REGULATOR_MEM:
			reg = RICOH619_DC2;
			break;
		case REGULATOR_IO:
			reg = RICOH619_DC4;
			break;
		default:return -EINVAL;
	}

	if ((vol_mv < 600) || (vol_mv > 3500)) {
		debug("unsupported voltage\n");
		return -EINVAL;
	} else {
		regvalue = ((vol_mv - 600) * 10)/ 125;
	}

	return ricoh61x_write_reg(reg, &regvalue);
}
#endif
