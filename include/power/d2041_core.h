/*
 * core.h  --  Core Driver for Dialog Semiconductor D2041 PMIC
 *
 * Copyright 2011 Dialog Semiconductor Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#ifndef __LINUX_D2041_CORE_H_
#define __LINUX_D2041_CORE_H_

/*
 * Register values.
 */

#define I2C					1

#define D2041_I2C				"d2041"

//#define D2041_IRQ					        S3C_EINT(9)

/* Module specific error codes */
#define INVALID_REGISTER			2
#define INVALID_READ				3
#define INVALID_PAGE				4

/* Total number of registers in D2041 */
#define D2041_MAX_REGISTER_CNT			(D2041_PAGE1_REG_END+1)


#define D2041_I2C_DEVICE_NAME			"d2041_i2c"
#define D2041_I2C_ADDR				0x49


/*
 * DA1980 Number of Interrupts
 */

//enum d2041_irq_num { //TODO MW: change to enum
#define D2041_IRQ_EVDDMON			0
#define D2041_IRQ_EALRAM			1
#define D2041_IRQ_ESEQRDY			2
#define D2041_IRQ_ETICK				3

#define D2041_IRQ_ENONKEY_LO			4
#define D2041_IRQ_ENONKEY_HI			5
#define D2041_IRQ_ENONKEY_HOLDON		6
#define D2041_IRQ_ENONKEY_HOLDOFF		7

#define D2041_IRQ_ETA				8
#define D2041_IRQ_ENJIGON			9

#define D2041_IRQ_EGPI0				10

#define D2041_NUM_IRQ				11

//#ifdef CONFIG_BOARD_CORI
#define D2041_IOCTL_REGL_MAPPING_NUM		23
//#endif /* CONFIG_BOARD_CORI */

#define D2041_MCTL_MODE_INIT(_reg_id, _dsm_mode, _default_pm_mode) \
    [_reg_id] = { \
        .reg_id = _reg_id, \
        .dsm_opmode = _dsm_mode, \
        .default_pm_mode = _default_pm_mode, \
    }

/*
 * d2041 device IO
 */
//extern int d2041_regulator_init(void);
#endif
