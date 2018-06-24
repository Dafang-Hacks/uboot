/*
 * Synopsis DDR PHY common data structure.
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

#ifndef __DDRP_DWC_H__
#define __DDRP_DWC_H__

typedef union ddrp_mr0 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned BL:2;
		unsigned CL_2:1;
		unsigned BT:1;
		unsigned CL_4_6:3;
		unsigned TM:1;
		unsigned DR:1;
		unsigned WR:3;
		unsigned PD:1;
		unsigned RSVD:3;
		unsigned reserved16_31:16;
	} ddr3; /* MR0 */
	struct {
		unsigned BL:3;
		unsigned BT:1;
		unsigned CL:3;
		unsigned TM:1;
		unsigned RSVD8_11:4;
		unsigned RSVD12_15:4;
		unsigned reserved16_31:16;
	} lpddr; /* MR */
	struct {
		unsigned BL:3;
		unsigned BT:1;
		unsigned CL:3;
		unsigned TM:1;
		unsigned DR:1;
		unsigned WR:3;
		unsigned PD:1;
		unsigned RSVD13_15:3;
		unsigned reserved16_31:16;
	} ddr2; /* MR */
	struct {
		unsigned unsupported:31;
	} ddr; /* MR */
} ddrp_mr0_t;

typedef union ddrp_mr1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned DE:1;
		unsigned DIC1:1;
		unsigned RTT2:1;
		unsigned AL:2;
		unsigned DIC5:1;
		unsigned RTT6:1;
		unsigned LEVEL:1;
		unsigned RSVD8:1;
		unsigned RTT9:1;
		unsigned RSVD10:1;
		unsigned TDQS:1;
		unsigned QOFF:1;
		unsigned RSVD13_15:3;
		unsigned reserved16_31:16;
	} ddr3; /* MR1 */
	struct {
		unsigned BL:3;
		unsigned BT:1;
		unsigned WC:1;
		unsigned nWR:3;
		unsigned reserved8_31:24;
	} lpddr2; /* MR1 */
	struct {
		unsigned DE:1;
		unsigned DIC:1;
		unsigned RTT2:1;
		unsigned AL3_5:3;
		unsigned RTT6:1;
		unsigned OCD:3;
		unsigned DQS:1;
		unsigned RDQS:1;
		unsigned QOFF:1;
		unsigned RSVD13_15:3;
		unsigned reserved16_31:16;
	} ddr2; /* EMR */
	struct {
		unsigned unsupported:31;
	} ddr; /* EMR */
} ddrp_mr1_t;

typedef union ddrp_mr2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned PASR:3;
		unsigned CWL:3;
		unsigned ASR:1;
		unsigned SRT:1;
		unsigned RSVD8:1;
		unsigned RTTWR:2;
		unsigned RSVD11_15:5;
		unsigned reserved16_31:16;
	} ddr3; /* MR2 */
	struct {
		unsigned PASR:3;
		unsigned TCSR:2;
		unsigned DS:3;
		unsigned RSVD8_15:8;
		unsigned reserved16_31:16;
	} lpddr; /* EMR */
	struct {
		unsigned RL_WL:4;
		unsigned RSVD4_7:4;
		unsigned reserved8_31:24;
	} lpddr2; /* MR2 */
	struct {
		unsigned unsupported:31;
	} ddr2; /* EMR2 */
} ddrp_mr2_t;

typedef union ddrp_mr3 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned MPRLOC:2;
		unsigned MPR:1;
		unsigned RSVD3_15:13;
		unsigned reserved16_31:16;
	} ddr3; /* MR3 */
	struct {
		unsigned DS:4;
		unsigned RSVD4_7:4;
		unsigned reserved8_31:24;
	} lpddr2; /* MR2 */
	struct {
		unsigned unsupported:31;
	} ddr2; /* EMR3 */
} ddrp_mr3_t;

typedef union ddrp_ptr0 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tDLLSRST:6;
		unsigned tDLLLOCK:12;
		unsigned tITMSRST:4;
		unsigned reserved22_31:10;
	} b;
} ddrp_ptr0_t;

typedef union ddrp_ptr1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tDINIT0:19;
		unsigned tDINIT1:8;
		unsigned reserved27_31:5;
	} b;
} ddrp_ptr1_t;

typedef union ddrp_ptr2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tDINIT2:17;
		unsigned tDINIT3:10;
		unsigned reserved27_31:5;
	} b;
} ddrp_ptr2_t;

typedef union ddrp_dtpr0 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tMRD:2;
		unsigned tRTP:3;
		unsigned tWTR:3;
		unsigned tRP:4;
		unsigned tRCD:4;
		unsigned tRAS:5;
		unsigned tRRD:4;
		unsigned tRC:6;
		unsigned tCCD:1;
	} b;
} ddrp_dtpr0_t;

typedef union ddrp_dtpr1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tAOND_tAOFD:2;
		unsigned tRTW:1;
		unsigned tFAW:6;
		unsigned tMOD:2;
		unsigned tRTODT:1;
		unsigned reserved12_15:4;
		unsigned tRFC:8;
		unsigned tDQSCK:3;
		unsigned tDQSCKMAX:3;
		unsigned reserved30_31:2;
	} b;
} ddrp_dtpr1_t;

typedef union ddrp_dtpr2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tXS:10;
		unsigned tXP:5;
		unsigned tCKE:4;
		unsigned tDLLK:10;
		unsigned reserved29_31:3;
	} b;
} ddrp_dtpr2_t;

typedef union ddrp_odtcr {
	/** odtcr configure odt for read & write ctrl. */
	uint32_t d32;
	struct{
		unsigned int RDODT0:4;
		unsigned int RDODT1:4;
		unsigned int RDODT2:4;
		unsigned int RDODT3:4;
		unsigned int WDODT0:4;
		unsigned int WDODT1:4;
		unsigned int WDODT2:4;
		unsigned int WDODT3:4;
	}b;
}ddrp_odtcr_t;
typedef union ddrp_dxngcr{
	uint32_t d32;
	struct {
		unsigned int dxen:1;
		unsigned int dqsodt:1;
		unsigned int dqodt:1;
		unsigned int dxiom:1;
		unsigned int dxpdr:1;
		unsigned int dxpdd:1;
		unsigned int dqsrpd:1;
		unsigned int dsen:2;
		unsigned int dqsrtt:1;
		unsigned int dqrtt:1;
		unsigned int rttoh:2;
		unsigned int rttoal:1;
		unsigned int r0rvsl:4;
		unsigned int r1rvsl:4;
		unsigned int r2rvsl:4;
		unsigned int r3rvsl:4;
		unsigned int reserved:5;
	}b;
}ddrp_dxngcr_t;

struct ddrp_reg {
	uint32_t dcr;
	ddrp_mr0_t mr0;
	ddrp_mr1_t mr1;
	ddrp_mr2_t mr2;
	ddrp_mr3_t mr3;
	uint32_t pgcr;
	ddrp_ptr0_t ptr0;
	ddrp_ptr1_t ptr1;
	ddrp_ptr2_t ptr2;
	ddrp_dtpr0_t dtpr0;
	ddrp_dtpr1_t dtpr1;
	ddrp_dtpr2_t dtpr2;
	ddrp_odtcr_t odtcr;
	ddrp_dxngcr_t dxngcrt[4];
	unsigned int zqncr1;
	unsigned int impedance[2]; //0-cal_value 1-req_value
	unsigned int odt_impedance[2]; //0-cal_value 1-req_value
	unsigned char rzq_table[32];
};

#endif /* __DDRP_DWC_H__ */
