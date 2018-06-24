/*
 * PMIC driver for D2041 or DA9024
 */
#include <config.h>
#include <common.h>
#include <linux/err.h>
#include <regulator.h>
#include <i2c.h>
#include <power/d2041_pmic.h>
#include <power/d2041_reg.h>
#include <power/d2041_core.h>

#define D2041_NUM_BUCKX  0x3F
#define D2041_NUM_BUCK4  0x53
#define D2041_NUM_LODX   0x2A
#define D2041_I2C_ADDR   0x49

#define __write_32bit_c0_register(register, sel, value)         \
do {                                    \
    if (sel == 0)                           \
        __asm__ __volatile__(                   \
            "mtc0\t%z0, " #register "\n\t"          \
            : : "Jr" ((unsigned int)(value)));      \
    else                                \
        __asm__ __volatile__(                   \
            ".set\tmips32\n\t"              \
            "mtc0\t%z0, " #register ", " #sel "\n\t"    \
            ".set\tmips0"                   \
            : : "Jr" ((unsigned int)(value)));      \
} while (0)

#define __read_32bit_c0_register(source, sel)               \
({ int __res;                               \
    if (sel == 0)                           \
        __asm__ __volatile__(                   \
            "mfc0\t%0, " #source "\n\t"         \
            : "=r" (__res));                \
    else                                \
        __asm__ __volatile__(                   \
            ".set\tmips32\n\t"              \
            "mfc0\t%0, " #source ", " #sel "\n\t"       \
            ".set\tmips0\n\t"               \
            : "=r" (__res));                \
    __res;                              \
})

#define read_c0_status_1()     __read_32bit_c0_register($12, 4)
#define write_c0_status_1(val) __write_32bit_c0_register($12, 4, val)

static int d2041_regulator_disable(struct regulator* regulator);
static int d2041_regulator_enable(struct regulator *regulator);
static int d2041_set_voltage(struct regulator* regulator,unsigned int min_uV, unsigned int max_uV);
struct regl_register_map {
	u8 pm_reg;
	u8 mctl_reg;
	u8 dsm_opmode;
};

static struct regulator_ops d2041_ops = {
	.disable = d2041_regulator_disable,
	.enable = d2041_regulator_enable,
	.set_voltage = d2041_set_voltage,
};

#define D2041_REGULATOR(_name, _id, _n_step, _min_uV, _max_uV) \
	{	\
		.name = _name,	\
		.id = _id,	\
		.n_voltages = _n_step, \
                .min_uV = _min_uV, \
		.max_uV = _max_uV, \
		.ops = & d2041_ops,\
	}


struct regulator d2041_regulator[] = {

	D2041_REGULATOR( "BUCK_1",   D2041_BUCK_1,   D2041_NUM_BUCKX,  500000,  2075000),
	D2041_REGULATOR( "BUCK_2",   D2041_BUCK_2,   D2041_NUM_BUCKX,  500000,  2075000),
	D2041_REGULATOR( "BUCK_3",   D2041_BUCK_3,   D2041_NUM_BUCKX,  925000,  2500000),
	D2041_REGULATOR( "BUCK_4",   D2041_BUCK_4,   D2041_NUM_BUCK4,  1200000, 3300000),
	D2041_REGULATOR( "LDO_1",    D2041_LDO_1,    D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_2",    D2041_LDO_2,    D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_3",    D2041_LDO_3,    D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_4",    D2041_LDO_4,    D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_5",    D2041_LDO_5,    D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_6",    D2041_LDO_6,    D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_7",    D2041_LDO_7,    D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_8",    D2041_LDO_8,    D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_9",    D2041_LDO_9,    D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_10",   D2041_LDO_10,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_11",   D2041_LDO_11,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_12",   D2041_LDO_12,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_13",   D2041_LDO_13,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_14",   D2041_LDO_14,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_15",   D2041_LDO_15,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_16",   D2041_LDO_16,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_17",   D2041_LDO_17,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_18",   D2041_LDO_18,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_19",   D2041_LDO_19,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_20",   D2041_LDO_20,   D2041_NUM_LODX,   1200000, 3300000),
	D2041_REGULATOR( "LDO_AUD",  D2041_LDO_AUD,  D2041_NUM_LODX,   1200000, 3300000)
};

#define get_regulator_reg(n)        (regulator_register_map[n].pm_reg)
#define get_regulator_mctl_reg(n)   (regulator_register_map[n].mctl_reg)
#define get_regulator_dsm_opmode(n) (regulator_register_map[n].dsm_opmode)

#define D2041_DEFINE_REGL(_name, _pm_reg, _mctl_reg) \
	[D2041_##_name] = \
{ \
	.pm_reg = _pm_reg, \
	.mctl_reg = _mctl_reg, \
}

static struct regl_register_map regulator_register_map[] = {
	D2041_DEFINE_REGL(BUCK_1,  D2041_BUCK1_TURBO_REG,   D2041_BUCK1_MCTL_REG  ),
	D2041_DEFINE_REGL(BUCK_2,  D2041_BUCK2_REG,   D2041_BUCK2_MCTL_REG  ),
	D2041_DEFINE_REGL(BUCK_3,  D2041_BUCK3_REG,   D2041_BUCK3_MCTL_REG  ),
	D2041_DEFINE_REGL(BUCK_4,  D2041_BUCK4_REG,   D2041_BUCK4_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_1,   D2041_LDO1_REG,    D2041_LDO1_MCTL_REG   ),
	D2041_DEFINE_REGL(LDO_2,   D2041_LDO2_REG,    D2041_LDO2_MCTL_REG   ),
	D2041_DEFINE_REGL(LDO_3,   D2041_LDO3_REG,    D2041_LDO3_MCTL_REG   ),
	D2041_DEFINE_REGL(LDO_4,   D2041_LDO4_REG,    D2041_LDO4_MCTL_REG   ),
	D2041_DEFINE_REGL(LDO_5,   D2041_LDO5_REG,    D2041_LDO5_MCTL_REG   ),
	D2041_DEFINE_REGL(LDO_6,   D2041_LDO6_REG,    D2041_LDO6_MCTL_REG   ),
	D2041_DEFINE_REGL(LDO_7,   D2041_LDO7_REG,    D2041_LDO7_MCTL_REG   ),
	D2041_DEFINE_REGL(LDO_8,   D2041_LDO8_REG,    D2041_LDO8_MCTL_REG   ),
	D2041_DEFINE_REGL(LDO_9,   D2041_LDO9_REG,    D2041_LDO9_MCTL_REG   ),
	D2041_DEFINE_REGL(LDO_10,  D2041_LDO10_REG,   D2041_LDO10_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_11,  D2041_LDO11_REG,   D2041_LDO11_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_12,  D2041_LDO12_REG,   D2041_LDO12_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_13,  D2041_LDO13_REG,   D2041_LDO13_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_14,  D2041_LDO14_REG,   D2041_LDO14_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_15,  D2041_LDO15_REG,   D2041_LDO15_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_16,  D2041_LDO16_REG,   D2041_LDO16_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_17,  D2041_LDO17_REG,   D2041_LDO17_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_18,  D2041_LDO18_REG,   D2041_LDO18_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_19,  D2041_LDO19_REG,   D2041_LDO19_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_20,  D2041_LDO20_REG,   D2041_LDO20_MCTL_REG  ),
	D2041_DEFINE_REGL(LDO_AUD, D2041_LDO_AUD_REG, D2041_LDO_AUD_MCTL_REG)
};

int d2041_i2c_read( unsigned int start_reg, unsigned char *dest,int bytes)
{
	unsigned char data;
	int ret = -1;

	if (start_reg >= D2041_MAX_REGISTER_CNT) {
		printf("ERROR : invalid register of d2041: %d", start_reg);
		return ret;
	}

	ret = i2c_read(D2041_I2C_ADDR, start_reg,1,dest,1);
	if (ret < 0) {
		serial_printf("ERROR : d2041 i2c read error, ret = %d", ret);
		return ret;
	}

	return 0;
}

/* Currently we allocate the write buffer on the stack; this is OK for
 * small writes - if we need to do large writes this will need to be
 * revised.
 */
int d2041_i2c_write(unsigned char start_reg, int bytes, void *src)
{
	int ret;

	ret = i2c_write(D2041_I2C_ADDR, start_reg,1,src, bytes);
	if (ret < 0) {
		serial_printf("ERROR : d2041 i2c write error,(ret < 0) ret = %d", ret);
		return ret;
	}

	return 0;
}

static int d2041_clear_bits(unsigned char reg, unsigned char mask)
{
	unsigned char data;
	int err;

	err = d2041_i2c_read(reg, &data,1);
	if (err < 0) {
		goto out;
	}

	data &= ~mask;

	err = d2041_i2c_write(reg, 1, &data);
	if (err < 0) {
		goto out;
	}
out:
	return err;
}

static int d2041_set_bits(unsigned char reg, unsigned char mask)
{
	unsigned char data;
	int err;

	err = d2041_i2c_read(reg, &data, 1);
	if (err < 0) {
		goto out;
	}

	data |= mask;

	err = d2041_i2c_write(reg, 1, &data);
	if (err < 0) {
		goto out;
	}
out:
	return err;

}

static int d2041_val_to_mv(unsigned int val, int regulator_id)
{
	int mvolts;

	switch (regulator_id) {
		case D2041_BUCK_1 :
		case D2041_BUCK_2 :
			mvolts = val * D2041_BUCK_VOLT_STEP + D2041_BUCK12_VOLT_LOWER;
			break;
		case D2041_BUCK_3 :
			mvolts = val * D2041_BUCK_VOLT_STEP + D2041_BUCK3_VOLT_LOWER;
			break;
		case D2041_BUCK_4 :
			mvolts = val * D2041_BUCK_VOLT_STEP + D2041_BUCK4_VOLT_LOWER;
			break;
		case D2041_LDO_1 ... D2041_LDO_AUD :
			mvolts = val * D2041_LDO_VOLT_STEP + D2041_LDO_VOLT_LOWER;
			break;
		default :
			mvolts = 0;
			serial_printf("ERROR: invalid regulator id %d\n", regulator_id);
			break;
	}

	return mvolts;
}

static unsigned int d2041_mv_to_val(unsigned int uv, unsigned int regulator_id)
{
	unsigned int val = 0;

	switch (regulator_id) {
		case D2041_BUCK_1 :
		case D2041_BUCK_2 :
			val = ((uv - D2041_BUCK12_VOLT_LOWER) / D2041_BUCK_VOLT_STEP);
			break;
		case D2041_BUCK_3 :
			val = ((uv - D2041_BUCK3_VOLT_LOWER) / D2041_BUCK_VOLT_STEP);
			break;
		case D2041_BUCK_4 :
			val = ((uv - D2041_BUCK4_VOLT_LOWER) / D2041_BUCK_VOLT_STEP);
			break;
		case D2041_LDO_1 ... D2041_LDO_AUD :
			val = ((uv - D2041_LDO_VOLT_LOWER) / D2041_LDO_VOLT_STEP);
			break;
		default :
			serial_printf("ERROR: invalid regulator id %d\n", regulator_id);
			break;
	}

	return val;
}

static int d2041_set_voltage(struct regulator* regulator,unsigned int min_uV, unsigned int max_uV)
{
	int ret;
	unsigned int uv_val;
	unsigned char val;
	unsigned int reg_num;

	/* before we do anything check the lock bit */
	ret = d2041_i2c_read(D2041_SUPPLY_REG, &val,1);

	if (val & D2041_SUPPLY_VLOCK)
		d2041_clear_bits(D2041_SUPPLY_REG, D2041_SUPPLY_VLOCK);

	uv_val =  d2041_mv_to_val(min_uV, regulator->id);

	/* Sanity check for maximum value */
	if (d2041_val_to_mv(uv_val, regulator->id) > max_uV)
		return -1;

	reg_num = get_regulator_reg(regulator->id);

	ret = d2041_i2c_read(reg_num, &val,1);

	if (regulator->id == D2041_BUCK_4) {
		val &= ~D2041_BUCK4_VSEL;
	} else {
		val &= ~D2041_MAX_VSEL;
	}

	val |= uv_val;

	d2041_i2c_write(reg_num, 1, &val);

	ret = d2041_i2c_read(reg_num, &val,1);
	/* For BUCKs enable the ramp */
	if (regulator->id <= D2041_BUCK_4)
		d2041_set_bits(D2041_SUPPLY_REG, (D2041_SUPPLY_VBUCK1GO << regulator->id));

	return ret;
}

static int is_mode_control_enabled(void)
{
	// 0 : mctl disable, 1 : mctl enable
#if defined(CONFIG_MFD_DA9024_USE_MCTL)
	return 1;
#else
	return 0;
#endif
}

static int d2041_regulator_enable(struct regulator *regulator)
{
	int ret = 0;
	unsigned char reg_val;
	unsigned int reg_num;
	if (regulator->id >= D2041_NUMBER_OF_REGULATORS)
		return -1;

	if (!is_mode_control_enabled()) {
		if (regulator->id == D2041_BUCK_4) {
			reg_num = D2041_SUPPLY_REG;
		} else {
			reg_num = get_regulator_reg(regulator->id);
		}

		d2041_i2c_read(reg_num, &reg_val,1);
		reg_val |= (1<<6);
		ret = d2041_i2c_write(reg_num, 1, &reg_val);
	} else {
		reg_num = get_regulator_mctl_reg(regulator->id);

		ret = d2041_i2c_read(reg_num, 1, &reg_val);
		if (ret < 0) {
			serial_printf("ERROR: I2C read error %d", ret);
			return ret;
		}

		reg_val &= ~(D2041_REGULATOR_MCTL1 | D2041_REGULATOR_MCTL3);   // Clear MCTL11 and MCTL01
		reg_val |= (D2041_REGULATOR_MCTL1_ON | D2041_REGULATOR_MCTL3_ON);
		switch (get_regulator_dsm_opmode(regulator->id)) {
			case D2041_REGULATOR_LPM_IN_DSM :
				reg_val &= ~(D2041_REGULATOR_MCTL0 | D2041_REGULATOR_MCTL2);
				reg_val |= (D2041_REGULATOR_MCTL0_SLEEP | D2041_REGULATOR_MCTL2_SLEEP);
				break;
			case D2041_REGULATOR_OFF_IN_DSM :
				reg_val &= ~(D2041_REGULATOR_MCTL0 | D2041_REGULATOR_MCTL2);
				break;
			case D2041_REGULATOR_ON_IN_DSM :
				reg_val &= ~(D2041_REGULATOR_MCTL0 | D2041_REGULATOR_MCTL2);
				reg_val |= (D2041_REGULATOR_MCTL0_ON | D2041_REGULATOR_MCTL2_ON);
				break;
		}
		ret |= d2041_i2c_write(reg_num, 1, &reg_val);
	}

	return ret;
}

static int d2041_regulator_disable(struct regulator* regulator)
{
	int ret = 0;
	unsigned char reg_val;
	unsigned int reg_num = 0;

	if (regulator->id >= D2041_NUMBER_OF_REGULATORS)
		return -1;

	if (!is_mode_control_enabled()) {
		if (regulator->id == D2041_BUCK_4) {
			reg_num = D2041_SUPPLY_REG;
		} else {
			reg_num = get_regulator_reg(regulator->id);
		}

		d2041_i2c_read(reg_num, 1, &reg_val);
		reg_val &= ~(1<<6);
		d2041_i2c_write(reg_num, 1, &reg_val);
	} else {

		reg_num = get_regulator_mctl_reg(regulator->id);
		/* 0x00 ==  D2041_REGULATOR_MCTL0_OFF | D2041_REGULATOR_MCTL1_OFF
		 *        | D2041_REGULATOR_MCTL2_OFF | D2041_REGULATOR_MCTL3_OFF
		 */
		reg_val = 0;

		ret = d2041_i2c_write(reg_num, 1, &reg_val);

	}

	return ret;
}


int d2041_shutdown(void)
{
    uint8_t dst;
    int ret;
    debug("d2041 shutdown-----!\n");

    d2041_clear_bits(D2041_CONTROLB_REG, D2041_CONTROLB_WRITEMODE | D2041_CONTROLB_I2C_SPEED);

    d2041_clear_bits(D2041_CONTROLB_REG,D2041_CONTROLB_OTPREADEN); //otp r

    d2041_clear_bits(D2041_POWERCONT_REG,D2041_POWERCONT_MCTRLEN); //mctl


    dst = 0x0;
    d2041_i2c_write(D2041_IRQMASKB_REG, 1, &dst); //onkey mask clear

    d2041_clear_bits(D2041_LDO5_REG,    D2041_REGULATOR_EN); //LDO 5 disable
    d2041_clear_bits(D2041_LDO6_REG,    D2041_REGULATOR_EN); //LDO 6 disable
    d2041_clear_bits(D2041_LDO7_REG,    D2041_REGULATOR_EN); //LDO 7 disable
    d2041_clear_bits(D2041_LDO8_REG,    D2041_REGULATOR_EN); //LDO 8 disable
    d2041_clear_bits(D2041_LDO9_REG,    D2041_REGULATOR_EN); //LDO 9 disable
    d2041_clear_bits(D2041_LDO10_REG,   D2041_REGULATOR_EN); //LDO 10 disable
    d2041_clear_bits(D2041_LDO11_REG,   D2041_REGULATOR_EN); //LDO 11 disable
    d2041_clear_bits(D2041_LDO12_REG,   D2041_REGULATOR_EN); //LDO 12 disable
    d2041_clear_bits(D2041_LDO13_REG,   D2041_REGULATOR_EN); //LDO 13 disable
    d2041_clear_bits(D2041_LDO14_REG,   D2041_REGULATOR_EN); //LDO 14 disable
    d2041_clear_bits(D2041_LDO15_REG,   D2041_REGULATOR_EN); //LDO 15 disable
    d2041_clear_bits(D2041_LDO16_REG,   D2041_REGULATOR_EN); //LDO 16 disable
    d2041_clear_bits(D2041_LDO17_REG,   D2041_REGULATOR_EN); //LDO 17 disable
    d2041_clear_bits(D2041_LDO18_REG,   D2041_REGULATOR_EN); //LDO 18 disable
    d2041_clear_bits(D2041_LDO19_REG,   D2041_REGULATOR_EN); //LDO 19 disable
    d2041_clear_bits(D2041_LDO20_REG,   D2041_REGULATOR_EN); //LDO 20 disable
    d2041_clear_bits(D2041_LDO_AUD_REG, D2041_REGULATOR_EN); //LDO_AUD disable

#if 0
    dst = 0x0;
    d2041_i2c_write( D2041_BUCK4_REG, 1, &dst); //BUCK 4
#endif

#if 0
  dst = 0x10;
  d2041_i2c_write(D2041_SUPPLY_REG, 1, &dst);

  dst = 0x0E;
  d2041_i2c_write(D2041_POWERCONT_REG, 1, &dst);
#endif
    dst = 0xef;
    d2041_i2c_write(D2041_PDDIS_REG, 1, &dst);

    dst = 0;
    ret = d2041_i2c_read(D2041_CONTROLB_REG, 1, &dst);

    dst |= D2041_CONTROLB_DEEPSLEEP/*|D2041_CONTROLB_SHUTDOWN*/;
    d2041_i2c_write(D2041_CONTROLB_REG, 1, &dst);

    return 0;
}

int d2041_regulator_init(void)
{
	int ret;
	int i;

	ret = i2c_probe(D2041_I2C_ADDR);
	if(ret) {
		printf("probe d2041 error, i2c addr ox%x\n", D2041_I2C_ADDR);
		return -1;
	}
	for (i = 0; i < ARRAY_SIZE(d2041_regulator); i++) {
		ret = regulator_register(&d2041_regulator[i], NULL);
		if(ret) {
			printf("%s regulator_register error\n",
					d2041_regulator[i].name);
			return ret;
		}
	}

	return 0;
}
