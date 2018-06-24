/*
 * d2041 register declarations.
 *
 * Copyright(c) 2011 Dialog Semiconductor Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
//TODO MW:
//unnessary comment at official version //James #warning UPDATE REGISTER MAP ACCORDING TO NEW DATASHEET

#ifndef __LINUX_D2041_REG_H
#define __LINUX_D2041_REG_H

/* Status / Config */
#define D2041_PAGECON0_REG          0x00    /* 0 */
#define D2041_STATUSA_REG           0x01    /* 1 */
#define D2041_STATUSB_REG           0x02    /* 2 */
#define D2041_STATUSC_REG           0x03    /* 3 */
#define D2041_STATUSD_REG           0x04    /* 4 */
#define D2041_EVENTA_REG            0x05    /* 5 */
#define D2041_EVENTB_REG            0x06    /* 6 */
#define D2041_EVENTC_REG            0x07    /* 7 */
#define D2041_EVENTD_REG            0x08    /* 8 */
#define D2041_FAULTLOG_REG          0x09    /* 9 */
#define D2041_IRQMASKA_REG          0x0a    /* 10 */
#define D2041_IRQMASKB_REG          0x0b    /* 11 */
#define D2041_IRQMASKC_REG          0x0c    /* 12 */
#define D2041_IRQMASKD_REG          0x0d    /* 13 */
#define D2041_CONTROLA_REG          0x0e    /* 14 */
#define D2041_CONTROLB_REG          0x0f    /* 15 */
#define D2041_CONTROLC_REG          0x10    /* 16 */
#define D2041_CONTROLD_REG          0x11    /* 17 */
#define D2041_PDDIS_REG             0x12    /* 18 */
#define D2041_INTERFACE_REG         0x13    /* 19 */
#define D2041_RESET_REG             0x14    /* 20 */

/* GPIO 0-1-2-3-4-5 */
#define D2041_GPIO0001_REG          0x15    /* 21 */
#define D2041_GPIO_TA_REG           0x16    /* 22 */
#define D2041_GPIO_NJIGON_REG       0x17    /* 23 */
#define D2041_RESERVED_0x18         0x18    /* 24 */
#define D2041_RESERVED_0x19         0x19    /* 24 */
#define D2041_RESERVED_0x1a         0x1a    /* 24 */
#define D2041_GPIO_REG              0x1b    /* 27 */
#define D2041_RESERVED_0x1c         0x1c    /* 28 */


/* Sequencer*/
#define D2041_ID01_REG              0x1d    /* 29 */
#define D2041_ID23_REG              0x1e    /* 30 */
#define D2041_ID45_REG              0x1f    /* 31 */
#define D2041_ID67_REG              0x20    /* 32 */
#define D2041_ID89_REG              0x21    /* 33 */
#define D2041_ID1011_REG            0x22    /* 34 */
#define D2041_ID1213_REG            0x23    /* 35 */
#define D2041_ID1415_REG            0x24    /* 36 */
#define D2041_ID1617_REG            0x25    /* 37 */
#define D2041_ID1819_REG            0x26    /* 38 */
#define D2041_ID2021_REG            0x27    /* 39 */
#define D2041_SEQSTATUS_REG         0x28    /* 40 */
#define D2041_SEQA_REG              0x29    /* 41 */
#define D2041_SEQB_REG              0x2a    /* 42 */
#define D2041_SEQTIMER_REG          0x2b    /* 43 */

/* Supplies */

/* Regulator Register set 1 */
#define D2041_BUCKA_REG             0x2c    /* 44 */
#define D2041_BUCKB_REG             0x2d    /* 45 */
    /* Sets of BUCK regulator */
#define D2041_BUCK1_REG             0x2e    /* 46 */
#define D2041_BUCK2_REG             0x2f    /* 47 */
#define D2041_BUCK3_REG             0x30    /* 48 */
#define D2041_BUCK4_REG             0x31    /* 49 */

    /* Sets of LDO regulator */
#define D2041_LDO1_REG              0x32    /* 50 */
#define D2041_LDO2_REG              0x33    /* 51 */
#define D2041_LDO3_REG              0x34    /* 52 */
#define D2041_LDO4_REG              0x35    /* 53 */
#define D2041_LDO5_REG              0x36    /* 54 */
#define D2041_LDO6_REG              0x37    /* 55 */
#define D2041_LDO7_REG              0x38    /* 56 */
#define D2041_LDO8_REG              0x39    /* 57 */
#define D2041_LDO9_REG              0x3a    /* 58 */
#define D2041_LDO10_REG             0x3b    /* 59 */
#define D2041_LDO11_REG             0x3c    /* 60 */
#define D2041_LDO12_REG             0x3d    /* 61 */
/* Regulator Register set 1 end */

#define D2041_PULLDOWN_REG_A        0x3e    /* 62 */
#define D2041_PULLDOWN_REG_B        0x3f    /* 63 */
#define D2041_PULLDOWN_REG_C        0x40    /* 64 */
#define D2041_PULLDOWN_REG_D        0x41    /* 65 */

/* Regulator Register set 2 */
#define D2041_LDO13_REG             0x42    /* 66 */
#define D2041_LDO14_REG             0x43    /* 67 */
#define D2041_LDO15_REG             0x44    /* 68 */
#define D2041_LDO16_REG             0x45    /* 69 */
#define D2041_LDO17_REG             0x46    /* 70 */
#define D2041_LDO18_REG             0x47    /* 71 */
#define D2041_LDO19_REG             0x48    /* 72 */
#define D2041_LDO20_REG             0x49    /* 73 */
#define D2041_LDO_AUD_REG           0x4a    /* 74 */
/* Regulator Register set 2 end */


#define D2041_SUPPLY_REG            0x4b    /* 75 */


/* Regulator MCTL Register */
#define D2041_LDO1_MCTL_REG         0x4c    /* 76 */
#define D2041_LDO2_MCTL_REG         0x4d    /* 77 */
#define D2041_LDO3_MCTL_REG         0x4e    /* 78 */
#define D2041_LDO4_MCTL_REG         0x4f    /* 79 */
#define D2041_LDO5_MCTL_REG         0x50    /* 80 */
#define D2041_LDO6_MCTL_REG         0x51    /* 81 */
#define D2041_LDO7_MCTL_REG         0x52    /* 82 */
#define D2041_LDO8_MCTL_REG         0x53    /* 83 */
#define D2041_LDO9_MCTL_REG         0x54    /* 84 */
#define D2041_LDO10_MCTL_REG        0x55    /* 85 */
#define D2041_LDO11_MCTL_REG        0x56    /* 86 */
#define D2041_LDO12_MCTL_REG        0x57    /* 87 */
#define D2041_LDO13_MCTL_REG        0x58    /* 88 */
#define D2041_LDO14_MCTL_REG        0x59    /* 89 */
#define D2041_LDO15_MCTL_REG        0x5a    /* 90 */
#define D2041_LDO16_MCTL_REG        0x5b    /* 91 */
#define D2041_LDO17_MCTL_REG        0x5c    /* 92 */
#define D2041_LDO18_MCTL_REG        0x5d    /* 93 */
#define D2041_LDO19_MCTL_REG        0x5e    /* 94 */
#define D2041_LDO20_MCTL_REG        0x5f    /* 95 */

#define D2041_LDO_AUD_MCTL_REG      0x60    /* 96 */

#define D2041_BUCK1_MCTL_REG        0x61    /* 97 */
#define D2041_BUCK2_MCTL_REG        0x62    /* 98 */
#define D2041_BUCK3_MCTL_REG        0x63    /* 99 */
#define D2041_BUCK4_MCTL_REG        0x64    /* 100 */

#define D2041_MISC_MCTL_REG         0x65    /* 101 */

#define D2041_BUCK1_RETENTION_REG   0x66    /* 102 */
#define D2041_BUCK1_TURBO_REG       0x67    /* 103 */

/* Control */
#define D2041_WAITCONT_REG          0x68    /* 104 */
#define D2041_ONKEYCONT1_REG        0x69    /* 105 */
#define D2041_ONKEYCONT2_REG        0x6a    /* 106 */
#define D2041_POWERCONT_REG         0x6b    /* 107 */
#define D2041_VDDFAULT_REG          0x6c    /* 108 */
//#define D2041_???_REG             0x6d    /* 109 */ // TODO MW: remove this line?
#define D2041_BBATCONT_REG          0x6e    /* 110 */

/* RTC */
#define D2041_COUNTS_REG            0x6f    /* 111 */
#define D2041_COUNTMI_REG           0x70    /* 112 */
#define D2041_COUNTH_REG            0x71    /* 113 */
#define D2041_COUNTD_REG            0x72    /* 114 */
#define D2041_COUNTMO_REG           0x73    /* 115 */
#define D2041_COUNTY_REG            0x74    /* 116 */
#define D2041_ALARMS_REG            0x75    /* 117 */
#define D2041_ALARMMI_REG           0x76    /* 118 */
#define D2041_ALARMH_REG            0x77    /* 119 */
#define D2041_ALARMD_REG            0x78    /* 120 */
#define D2041_ALARMMO_REG           0x79    /* 121 */
#define D2041_ALARMY_REG            0x7a    /* 122 */


/* OTP Config */
#define D2041_PAGE_CONT_P1          0x80    /* 128 */

#define D2041_CHIPID_REG            0x81    /* 129 */
#define D2041_CONFIGID_REG          0x82    /* 130 */
#define D2041_OTPCONT_REG           0x83    /* 131 */
#define D2041_OSCTRIM_REG           0x84    /* 132 */
#define D2041_GPID0_REG             0x85    /* 133 */
#define D2041_GPID1_REG             0x86    /* 134 */
#define D2041_GPID2_REG             0x87    /* 135 */
#define D2041_GPID3_REG             0x88    /* 136 */
#define D2041_GPID4_REG             0x89    /* 137 */
#define D2041_GPID5_REG             0x8a    /* 138 */
#define D2041_GPID6_REG             0x8b    /* 139 */
#define D2041_GPID7_REG             0x8c    /* 140 */
#define D2041_GPID8_REG             0x8d    /* 141 */
#define D2041_GPID9_REG             0x8e    /* 142 */

/* Audio registers */
#define D2041_PREAMP_A_CTRL1_REG    0x8f    /* 143 */
#define D2041_PREAMP_A_CTRL2_REG    0x90    /* 144 */
#define D2041_PREAMP_B_CTRL1_REG    0x91    /* 145 */
#define D2041_PREAMP_B_CTRL2_REG    0x92    /* 146 */

#define D2041_MXHPR_CTRL_REG        0x93    /* 147 */
#define D2041_MXHPL_CTRL_REG        0x94    /* 148 */
#define D2041_MXSP_CTRL_REG         0x95    /* 149 */

#define D2041_SP_CTRL_REG           0x96    /* 150 */
#define D2041_SP_CFG1_REG           0x97    /* 151 */
#define D2041_SP_CFG2_REG           0x98    /* 152 */
#define D2041_SP_ATB_SEL_REG        0x99    /* 153 */
#define D2041_SP_STATUS_REG         0x9a    /* 154 */

#define D2041_HP_L_CTRL_REG         0x9b    /* 155 */
#define D2041_HP_L_GAIN_REG         0x9c    /* 156 */
#define D2041_HP_L_GAIN_STATUS_REG  0x9d    /* 157 */
#define D2041_HP_R_CTRL_REG         0x9e    /* 158 */
#define D2041_HP_R_GAIN_REG         0x9f    /* 159 */
#define D2041_HP_R_GAIN_STATUS_REG  0xa0    /* 160 */


#define D2041_HP_TEST_REG           0xA1    /* 161 */
#define D2041_CP_CTRL_REG           0xA2    /* 162 */
#define D2041_CP_DELAY_REG          0xA3    /* 163 */
#define D2041_CP_DETECTOR_REG       0xA4    /* 164 */
#define D2041_CP_VOL_THRESHOLD_REG  0xA5    /* 165 */
#define D2041_HP_NG1_REG            0xA6    /* 166 */
#define D2041_HP_NG2_REG            0xA7    /* 167 */
#define D2041_SP_NG1_REG            0xA8    /* 168 */
#define D2041_SP_NG2_REG            0xA9    /* 169 */
#define D2041_SP_NON_CLIP_ZC_REG    0xAA    /* 170 */
#define D2041_SP_NON_CLIP_REG       0xAB    /* 171 */
#define D2041_SP_PWR_REG            0xAC    /* 172 */
#define D2041_SV_CTRL_REG           0xAD    /* 173 */
#define D2041_BIAS_CTRL_REG         0xAE    /* 174 */

#define D2041_PAGE0_REG_START       D2041_STATUSA_REG
#define D2041_PAGE0_REG_END         D2041_ALARMY_REG

#define D2041_PAGE1_REG_START       D2041_CHIPID_REG
#define D2041_PAGE1_REG_END         D2041_BIAS_CTRL_REG/*D2041_GPID9_REG*/

/************************PAGE CONFIGURATION ***************************/

/* PAGE CONFIGURATION 128 REGISTER */
/* RESERVED */
/*
    #define D2041_PAGECON0_REGPAGE          (1<<7)
*/

/* PAGE CONFIGURATION 128 REGISTER */
#define D2041_PAGECON128_REGPAGE            (1<<7)


/************************SYSTEM REGISTER ***************************/

/* STATUS REGISTER A */
#define D2041_STATUSA_MCTL                  (3<<5)
#define D2041_STATUSA_MCTL_SHIFT            5
#define D2041_STATUSA_VDDMON                (1<<4)

/* STATUS REGISTER B */
#define D2041_STATUSB_SEQUENCING            (1<<6)
#define D2041_STATUSB_NONKEY                (1<<0)

/* STATUS REGISTER C */
#define D2041_STATUSC_NJIGON                (1<<4)
#define D2041_STATUSC_TA                    (1<<3)

/* STATUS REGISTER D */
#define D2041_STATUSD_GPI0                  (1<<5)

/* EVENT REGISTER A */
#define D2041_EVENTA_ETICK                  (1<<7)
#define D2041_EVENTA_ESEQRDY                (1<<6)
#define D2041_EVENTA_EALRAM                 (1<<5)
#define D2041_EVENTA_EVDDMON                (1<<4)

/* EVENT REGISTER B */
#define D2041_EVENTB_ENONKEY_HOLDOFF        (1<<3)
#define D2041_EVENTB_ENONKEY_HOLDON         (1<<2)
#define D2041_EVENTB_ENONKEY_HI             (1<<1)
#define D2041_EVENTB_ENONKEY_LO             (1<<0)

/* EVENT REGISTER C */
#define D2041_EVENTC_ENJIGON                (1<<4)
#define D2041_EVENTC_ETA                    (1<<3)

/* EVENT REGISTER D */
#define D2041_EVENTC_EGPI0                  (1<<5)

/* FAULT LOG REGISTER */
#define D2041_FAULTLOG_WAITSHUT             (1<<7)
#define D2041_FAULTLOG_KEYSHUT              (1<<5)
#define D2041_FAULTLOG_TEMPOVER             (1<<3)
#define D2041_FAULTLOG_VDDSTART             (1<<2)
#define D2041_FAULTLOG_VDDFAULT             (1<<1)
#define D2041_FAULTLOG_TWDERROR             (1<<0)

/* IRQ_MASK REGISTER A */
#define D2041_IRQMASKA_MTICK                (1<<7)
#define D2041_IRQMASKA_MSEQRDY              (1<<6)
#define D2041_IRQMASKA_MALRAM               (1<<5)
#define D2041_IRQMASKA_MVDDMON              (1<<4)

/* IRQ_MASK REGISTER B */
#define D2041_IRQMASKB_MNONKEY_HOLDOFF      (1<<3)
#define D2041_IRQMASKB_MNONKEY_HOLDON       (1<<2)
#define D2041_IRQMASKB_MNONKEY_HI           (1<<1)
#define D2041_IRQMASKB_MNONKEY_LO           (1<<0)

/* IRQ_MASK REGISTER C */
#define D2041_IRQMASKC_MNJIGON              (1<<4)
#define D2041_IRQMASKC_MTA                  (1<<3)

/* IRQ_MASK REGISTER D */
#define D2041_IRQMASKD_MGPI0                (1<<5)

/* CONTROL REGISTER A */
#define D2041_CONTROLA_GPIV                 (1<<7)
#define D2041_CONTROLA_PMIV                 (1<<4)
#define D2041_CONTROLA_PMIFV                (1<<3)
#define D2041_CONTROLA_PWR1EN               (1<<2)
#define D2041_CONTROLA_PWREN                (1<<1)
#define D2041_CONTROLA_SYSEN                (1<<0)

/* CONTROL REGISTER B */
#define D2041_CONTROLB_SHUTDOWN             (1<<7)
#define D2041_CONTROLB_DEEPSLEEP            (1<<6)
#define D2041_CONTROLB_WRITEMODE            (1<<5)
#define D2041_CONTROLB_I2C_SPEED            (1<<4)
#define D2041_CONTROLB_OTPREADEN            (1<<3)
#define D2041_CONTROLB_AUTOBOOT             (1<<2)


/* CONTROL REGISTER C */
#define D2041_CONTROLC_DEBOUNCING           (7<<2)
#define D2041_CONTROLC_DEBOUNCING_SHIFT     2
#define D2041_CONTROLC_PMFB1PIN             (1<<0)

/* CONTROL REGISTER D */
#define D2041_CONTROLD_WATCHDOG             (1<<7)
#define D2041_CONTROLD_ONKEYAUTOBOOTEN      (1<<5)
#define D2041_CONTROLD_ONKEYSD              (1<<4)
#define D2041_CONTROLD_KEEPACTEN            (1<<3)
#define D2041_CONTROLD_TWDSCALE             (7<<0)
#define D2041_CONTROLD_TWDSCALE_SHIFT       0

/* POWER DOWN DISABLE REGISTER */
#define D2041_PDDIS_PMCONTPD                (1<<7)
#define D2041_PDDIS_OUT32KPD                (1<<6)
#define D2041_PDDIS_CHGBBATPD               (1<<5)
//#define D2041_PDDIS_HSIFPD                  (1<<3)
#define D2041_PDDIS_HS2WIREPD               (1<<3)
#define D2041_PDDIS_PMIFPD                  (1<<2)
#define D2041_PDDIS_GPIOPD                  (1<<0)

/* INTERFACE REGISTER */
#define D2041_INTERFACE_IFBASEADDR          (7<<5)
#define D2041_INTERFACE_IFBASEADDR_SHIFT    5

/* RESET REGISTER */
#define D2041_RESET_RESETEVENT              (3<<6)
#define D2041_RESET_RESETEVENT_SHIFT        6
#define D2041_RESET_RESETTIMER              (63<<0)


/************************GPIO REGISTERS***************************/

/* TA control register */
#define D2041_GPIO_TAMODE                   (1<<7)
#define D2041_GPIO_TATYPE                   (1<<6)
#define D2041_GPIO_TAPIN                    (3<<4)
#define D2041_GPIO_TAPIN_SHIFT              4


/* NJIGON control register */
#define D2041_GPIO_NJIGONMODE               (1<<3)
#define D2041_GPIO_NJIGONTYPE               (1<<2)
#define D2041_GPIO_NJIGONPIN                (3<<0)
#define D2041_GPIO_NJIGONPIN_SHIFT          0


/* GPIO control register */
#define D2041_GPIO_MODE                     (1<<7)
#define D2041_GPIO_TYPE                     (1<<6)
#define D2041_GPIO_PIN                      (3<<4)
#define D2041_GPIO_IN_SHIFT                 4


/*****************POWER SEQUENCER REGISTER*********************/

/* SEQ control register for ID 0 and 1 */
#define D2041_ID01_LDO1STEP                 (15<<4)
#define D2041_ID01_WAITIDALWAYS             (1<<3)
#define D2041_ID01_SYSPRE                   (1<<2)
#define D2041_ID01_DEFSUPPLY                (1<<1)
#define D2041_ID01_NRESMODE                 (1<<0)

/* SEQ control register for ID 2 and 3 */
#define D2041_ID23_LDO3STEP                 (15<<4)
#define D2041_ID23_LDO2STEP                 (15<<0)

/* SEQ control register for ID 4 and 5 */
#define D2041_ID45_LDO5STEP                 (15<<4)
#define D2041_ID45_LDO4STEP                 (15<<0)

/* SEQ control register for ID 6 and 7 */
#define D2041_ID67_LDO7STEP                 (15<<4)
#define D2041_ID67_LDO6STEP                 (15<<0)

/* SEQ control register for ID 8 and 9 */
#define D2041_ID89_LDO9STEP                 (15<<4)
#define D2041_ID89_LDO8STEP                 (15<<0)

/* SEQ control register for ID 10 and 11 */
//#define D2041_ID1011_PDDISSTEP              (15<<4)
#define D2041_ID1011_LDO11STEP              (15<<4)
#define D2041_ID1011_LDO10STEP              (15<<0)

/* SEQ control register for ID 12 and 13 */
//#define D2041_ID1213_LDO13STEP              (15<<4)
#define D2041_ID1213_PDDISSTEP              (15<<4)
#define D2041_ID1213_LDO12STEP              (15<<0)

/* SEQ control register for ID 14 and 15 */
#define D2041_ID1415_BUCK2                  (15<<4)
#define D2041_ID1415_BUCK1                  (15<<0)

/* SEQ control register for ID 16 and 17 */
#define D2041_ID1617_BUCK4                  (15<<4)
#define D2041_ID1617_BUCK3                  (15<<0)

/* Power SEQ Status register */
#define D2041_SEQSTATUS_SEQPOINTER          (15<<4)
#define D2041_SEQSTATUS_WAITSTEP            (15<<0)

/* Power SEQ A register */
#define D2041_SEQA_POWEREND                 (15<<4)
#define D2041_SEQA_SYSTEMEND                (15<<0)

/* Power SEQ B register */
#define D2041_SEQB_PARTDOWN                 (15<<4)
#define D2041_SEQB_MAXCOUNT                 (15<<0)

/* Power SEQ TIMER register */
#define D2041_SEQTIMER_SEQDUMMY             (15<<4)
#define D2041_SEQTIMER_SEQTIME              (15<<0)


/***************** REGULATOR REGISTER*********************/

/* BUCK REGISTER A */
#define D2041_BUCKA_BUCK2ILIM               (3<<6)
#define D2041_BUCKA_BUCK2ILIM_SHIFT         6
#define D2041_BUCKA_BUCK2MODE               (3<<4)
#define D2041_BUCKA_BUCK2MODE_SHIFT         4
#define D2041_BUCKA_BUCK1ILIM               (3<<2)
#define D2041_BUCKA_BUCK1ILIM_SHIFT         2
#define D2041_BUCKA_BUCK1MODE               (3<<0)
#define D2041_BUCKA_BUCK1MODE_SHIFT         0

/* BUCK REGISTER B */
#define D2041_BUCKB_BUCK4ILIM               (3<<6)
#define D2041_BUCKB_BUCK4ILIM_SHIFT         6
#define D2041_BUCKB_BUCK4IMODE              (3<<4)
#define D2041_BUCKB_BUCK4IMODE_SHIFT        4
#define D2041_BUCKB_BUCK3ILIM               (3<<2)
#define D2041_BUCKB_BUCK3ILIM_SHIFT         2
#define D2041_BUCKB_BUCK3MODE               (3<<0)
#define D2041_BUCKB_BUCK3MODE_SHIFT         0


/* PULLDOWN REGISTER A */
#define D2041_PULLDOWN_A_LDO4PDDIS          (1<<7)
#define D2041_PULLDOWN_A_LDO3PDDIS          (1<<6)
#define D2041_PULLDOWN_A_LDO2PDDIS          (1<<5)
#define D2041_PULLDOWN_A_LDO1PDDIS          (1<<4)
#define D2041_PULLDOWN_A_BUCK4PDDIS         (1<<3)
#define D2041_PULLDOWN_A_BUCK3PDDIS         (1<<2)
#define D2041_PULLDOWN_A_BUCK2PDDIS         (1<<1)
#define D2041_PULLDOWN_A_BUCK1PDDIS         (1<<0)


/* PULLDOWN REGISTER B */
#define D2041_PULLDOWN_B_LDO12PDDIS         (1<<7)
#define D2041_PULLDOWN_B_LDO11PDDIS         (1<<6)
#define D2041_PULLDOWN_B_LDO10PDDIS         (1<<5)
#define D2041_PULLDOWN_B_LDO9PDDIS          (1<<4)
#define D2041_PULLDOWN_B_LDO8PDDIS          (1<<3)
#define D2041_PULLDOWN_B_LDO7PDDIS          (1<<2)
#define D2041_PULLDOWN_B_LDO6PDDIS          (1<<1)
#define D2041_PULLDOWN_B_LDO5PDDIS          (1<<0)



/* PULLDOWN REGISTER C */
#define D2041_PULLDOWN_C_LDO20PDDIS         (1<<7)
#define D2041_PULLDOWN_C_LDO19PDDIS         (1<<6)
#define D2041_PULLDOWN_C_LDO18PDDIS         (1<<5)
#define D2041_PULLDOWN_C_LDO17PDDIS         (1<<4)
#define D2041_PULLDOWN_C_LDO16PDDIS         (1<<3)
#define D2041_PULLDOWN_C_LDO15PDDIS         (1<<2)
#define D2041_PULLDOWN_C_LDO14PDDIS         (1<<1)
#define D2041_PULLDOWN_C_LDO13PDDIS         (1<<0)


/* PULLDOWN REGISTER D */

#define D2041_PULLDOWN_D_LDOAUDPDDIS        (1<<0)



/* SUPPLY REGISTER */
#define D2041_SUPPLY_VLOCK                  (1<<7)
#define D2041_SUPPLY_BUCK4EN                (1<<6)
//#define D2041_SUPPLY_LDOAUDEN               (1<<5)
#define D2041_SUPPLY_BBCHGEN                (1<<4)
#define D2041_SUPPLY_VBUCK4GO               (1<<3)
#define D2041_SUPPLY_VBUCK3GO               (1<<2)
#define D2041_SUPPLY_VBUCK2GO               (1<<1)
#define D2041_SUPPLY_VBUCK1GO               (1<<0)

/* MISC MCTL */
#define D2041_MISC_MCTL3_DIGICLK            (1<<7)
#define D2041_MISC_MCTL2_DIGICLK            (1<<6)
#define D2041_MISC_MCTL1_DIGICLK            (1<<5)
#define D2041_MISC_MCTL0_DIGICLK            (1<<4)
#define D2041_MISC_MCTL3_BBAT               (1<<3)
#define D2041_MISC_MCTL2_BBAT               (1<<2)
#define D2041_MISC_MCTL1_BBAT               (1<<1)
#define D2041_MISC_MCTL0_BBAT               (1<<0)


/* BUCK 1 Retention register */
#define D2041_BUCK1_VRETENTION              (63<<0)

/* Buck 1 Turbo register */
#define D2041_BUCK1_VTURBO                  (63<<0)


/* WAIT CONTROL REGISTER */
#define D2041_WAITCONT_WAITDIR              (1<<7)
#define D2041_WAITCONT_RTCCLOCK             (1<<6)
#define D2041_WAITCONT_WAITMODE             (1<<5)
#define D2041_WAITCONT_EN32KOUT             (1<<4)
#define D2041_WAITCONT_DELAYTIME            (15<<0)

/* ONKEY CONTROL REGISTER */
#define D2041_ONKEYCONT_DEB                 (15<<4)
#define D2041_ONKEYCONT_PRESSTIME           (15<<0)

/* ONKEY CONTROL REGISTER */
#define D2041_ONKEYCONT_HOLDOFFDEB          (7<<4)
#define D2041_ONKEYCONT_HOLDONDEB           (7<<0)

/* POWER CONTROL REGISTER */
#define D2041_POWERCONT_NJIGMCTRLWAKEDIS    (1<<7)
#define D2041_POWERCONT_RTCAUTOEN           (1<<6)
#define D2041_POWERCONT_BBATLIMIGNORE       (1<<3)
//#define D2041_POWERCONT_BBATAUTOOFF         (1<<1)  //TODO as Data sheet under review?
#define D2041_POWERCONT_MCTRLEN             (1<<0)  //TODO as Data sheet under review?

/* VDD FAULT REGISTER */
#define D2041_VDD_FAULT_ADJ                 (15<<2)
#define D2041_VDD_HYST_ADJ                  (3<<0)


/***************** BAT CHARGER REGISTER *********************/

/* BACKUP BATTERY CONTROL REGISTER */
#define D2041_BBATCONT_BCHARGERISET         (15<<4)
#define D2041_BBATCONT_BCHARGERVSET         (15<<0)



/*****************RTC REGISTER*********************/

/* RTC TIMER SECONDS REGISTER */
#define D2041_COUNTS_COUNTSEC               (63<<0)

/* RTC TIMER MINUTES REGISTER */
#define D2041_COUNTMI_COUNTMIN              (63<<0)

/* RTC TIMER HOUR REGISTER */
#define D2041_COUNTH_COUNTHOUR              (31<<0)

/* RTC TIMER DAYS REGISTER */
#define D2041_COUNTD_COUNTDAY               (31<<0)

/* RTC TIMER MONTHS REGISTER */
#define D2041_COUNTMO_COUNTMONTH            (15<<0)

/* RTC TIMER YEARS REGISTER */
#define D2041_COUNTY_MONITOR                (1<<6)
#define D2041_COUNTY_COUNTYEAR              (63<<0)

/* RTC ALARM SECONDS REGISTER */
#define D2041_ALARMMI_COUNTSEC              (63<<0)

/* RTC ALARM MINUTES REGISTER */
#define D2041_ALARMMI_TICKTYPE              (1<<7)
#define D2041_ALARMMI_ALARMMIN              (63<<0)

/* RTC ALARM HOURS REGISTER */
#define D2041_ALARMH_ALARMHOUR              (31<<0)

/* RTC ALARM DAYS REGISTER */
#define D2041_ALARMD_ALARMDAY               (31<<0)

/* RTC ALARM MONTHS REGISTER */
#define D2041_ALARMMO_ALARMMONTH            (15<<0)

/* RTC ALARM YEARS REGISTER */
#define D2041_ALARMY_TICKON                 (1<<7)
#define D2041_ALARMY_ALARMON                (1<<6)
#define D2041_ALARMY_ALARMYEAR              (63<<0)


/*****************OTP REGISTER*********************/
#define D2041_PAGE_CONT_P1_BIT              (1<<7)

/* CHIP IDENTIFICATION REGISTER */
#define D2041_CHIPID_MRC                    (15<<4)
#define D2041_CHIPID_TRC                    (15<<0)

/* CONFIGURATION IDENTIFICATION REGISTER */
#define D2041_CONFIGID_CONFID               (7<<0)
#define D2041_CONFIGID_CONFID_SHIFT         0

/* OTP CONTROL REGISTER */
#define D2041_OTPCONT_GPWRITEDIS            (1<<7)
#define D2041_OTPCONT_OTPCONFLOCK           (1<<6)
#define D2041_OTPCONT_OTPGPLOCK             (1<<5)
#define D2041_OTPCONT_OTPCONFG              (1<<3)
#define D2041_OTPCONT_OTPGP                 (1<<2)
#define D2041_OTPCONT_OTPRP                 (1<<1)
#define D2041_OTPCONT_OTPTRANSFER           (1<<0)

/* RTC OSCILLATOR TRIM REGISTER */
#define D2041_OSCTRIM_TRIM32K               (255<<0)

/* GP ID REGISTER 0 */
#define D2041_GPID0_GP0                     (255<<0)

/* GP ID REGISTER 1 */
#define D2041_GPID1_GP1                     (255<<0)

/* GP ID REGISTER 2 */
#define D2041_GPID2_GP2                     (255<<0)

/* GP ID REGISTER 3 */
#define D2041_GPID3_GP3                     (255<<0)

/* GP ID REGISTER 4 */
#define D2041_GPID4_GP4                     (255<<0)

/* GP ID REGISTER 5 */
#define D2041_GPID5_GP5                     (255<<0)

/* GP ID REGISTER 6 */
#define D2041_GPID6_GP6                     (255<<0)

/* GP ID REGISTER 7 */
#define D2041_GPID7_GP7                     (255<<0)

/* GP ID REGISTER 8 */
#define D2041_GPID8_GP8                     (255<<0)

/* GP ID REGISTER 9 */
#define D2041_GPID9_GP9                     (255<<0)

/***************** AUDIO CONF REGISTERS********************/

/* Preamp CTRL1 regs (A and B) */
#define D2041_PREAMP_ZC_EN                  (1<<7)
#define D2041_PREAMP_VOL                    (31<<2)
#define D2041_PREAMP_VOL_SHIFT              2
#define D2041_PREAMP_MUTE                   (1<<1)
#define D2041_PREAMP_EN                     (1<<0)
/* Preamp CTRL2 regs (A and B) */
#define D2041_PREAMP_CFG                    (3<<0)

/* Mixer control registers (HPR, HPL and SP) */
#define D2041_MX_VOL                        (3<<5)
#define D2041_MX_VOL_SHIFT                  5
#define D2041_MX_SEL                        (15<<1)
#define D2041_MX_SEL_SHIFT                  1
#define D2041_MX_EN                         (1<<0)

/* Speaker control register */
#define D2041_SP_VOL                        (63<<2)
#define D2041_SP_VOL_SHIFT                  2
#define D2041_SP_MUTE                       (1<<1)
#define D2041_SP_EN                         (1<<0)

/* Speaker CFG1 register */
#define D2041_SP_CFG1                       (255<<0)
/* Speaker CFG2 register */
#define D2041_SP_CFG2                       (255<<0)
/* Speaker ATB_SEL register */
#define D2041_SP_ATB_SEL                    (255<<0)
/* Speaker STATUS register */
#define D2041_SP_STATUS                     (255<<0)

/* Headphones control registers (Left and Right) */
#define D2041_HP_AMP_EN                     (1<<7)
#define D2041_HP_AMP_MUTE_EN                (1<<6)
#define D2041_HP_AMP_RAMP_EN                (1<<5)
#define D2041_HP_AMP_ZC_EN                  (1<<4)
#define D2041_HP_AMP_OE                     (1<<3)
#define D2041_HP_AMP_MIN_GAIN_EN            (1<<2)
#define D2042_HP_AMP_BIAS                   (3<<0)

/* Headphones amplifier gain register (Left and Right) */
#define D2042_HP_AMP_GAIN                   (63<<0)

/* Headphones amplifier gain status register (Left and Right) */
#define D2042_HP_AMP_GAIN_STATUS            (63<<0)


/* D2041_HP_TEST_REG */
#define D2041_HP_AMP_EMS_EN                 (1<<0)

/* D2041_CP_CTRL_REG */
#define D2041_CP_EN                         (1<<7)
#define D2041_CP_SMALL_SWITCH_FREQ_EN       (1<<6)
#define D2041_CP_MCHANGE                    (3<<4)
#define D2041_CP_MCHANGE_SHIFT              4
#define D2041_CP_MOD                        (3<<2)
#define D2041_CP_MOD_SHIFT                  2
#define D2041_CP_ANALOGUE_LVL               (3<<0)
#define D2041_CP_ANALOGUE_LVL_SHIFT         0

/* D2041_DELAY_REG */
#define D2041_CP_ONOFF                      (3<<6)
#define D2041_CP_ONOFF_SHIFT                6
#define D2041_CP_TAU_DELAY                  (7<<3)
#define D2041_CP_TAU_DELAY_SHIFT            3
#define D2041_CP_FCONTROL                   (7<<0)
#define D2041_CP_FCONTROL_SHIFT             0

/* D2041_CP_DETECTOR_REG */
#define D2041_CP_DET_DROP                   (3<<0)
#define D2041_CP_DET_DROP_SHIFT             0

/* D2041_CP_VOL_THRESHOLD_REG */
#define D2041_CP_THRESH_VDD2                (63<<0)
#define D2041_CP_THRESH_VDD2_SHIFT          0

/* D2041_HP_NG1_REG */
#define D2041_HP_NG1_CFG                    (3<<6)
#define D2041_HP_NG1_CFG_SHIFT              6
#define D2041_HP_NG1_ATK                    (7<<3)
#define D2041_HP_NG1_ATK_SHIFT              3
#define D2041_HP_NG1_DEB                    (3<<1)
#define D2041_HP_NG1_DEB_SHIFT              1
#define D2041_HP_NG1_EN                     (1<<0)

/* D2041_HP_NG2_REG */
#define D2041_HP_NG2_RMS                    (3<<3)
#define D2041_HP_NG2_RMS_SHIFT              3
#define D2041_HP_NG2_REL                    (7<<0)
#define D2041_HP_NG2_REL_SHIFT              0

/* D2041_SP_NG1_REG */
#define D2041_SP_NG1_CFG                    (3<<6)
#define D2041_SP_NG1_CFG_SHIFT              6
#define D2041_SP_NG1_ATK                    (7<<3)
#define D2041_SP_NG1_ATK_SHIFT              3
#define D2041_SP_NG1_DEB                    (3<<1)
#define D2041_SP_NG1_DEB_SHIFT              1
#define D2041_SP_NG1_EN                     (1<<0)

/* D2041_SP_NG2_REG */
#define D2041_SP_NG2_RMS                    (3<<3)
#define D2041_SP_NG2_RMS_SHIFT              3
#define D2041_SP_NG2_REL                    (7<<0)
#define D2041_SP_NG2_REL_SHIFT              0

/* D2041_SP_NON_CLIP_ZC_REG */
#define D2041_SP_ZC_EN                      (1<<7)
#define D2041_SP_NON_CLIP_REL               (7<<4)
#define D2041_SP_NON_CLIP_REL_SHIFT         4
#define D2041_SP_NON_CLIP_ATK               (7<<1)
#define D2041_SP_NON_CLIP_ATK_SHIFT         1
#define D2041_SP_NON_CLIP_EN                (1<<0)

/* D2041_SP_NON_CLIP_REG */
#define D2041_SP_NON_CLIP_HLD               (3<<6)
#define D2041_SP_NON_CLIP_HLD_SHIFT         6
#define D2041_SP_NON_CLIP_THD               (63<<0)
#define D2041_SP_NON_CLIP_THD_SHIFT         0

/* D2041_SP_PWR_REG */
#define D2041_SP_PWR                        (63<<1)
#define D2041_SP_PWR_SHIFT                  1
#define D2041_SP_PWR_LIMIT_EN               (1<<0)

/* D2041_SV_CTRL_REG */

/* D2041_BIAS_CTRL_REG */
#define D2041_BIAS_CTRL                     (255<<0)


#endif /* __LINUX_D2041_REG_H */

