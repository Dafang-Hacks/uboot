#ifndef __LPDDR2_CONFIG_H
#define __LPDDR2_CONFIG_H

/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * LPDDR2 info
 */
/* LPDDR2 paramters */
#define DDR_ROW 	13 /* ROW : 12 to 14 row address */
#define DDR_ROW1 	13 /* ROW : 12 to 14 row address */
#define DDR_COL 	9  /* COL :  8 to 10 column address */
#define DDR_COL1 	9  /* COL :  8 to 10 column address */
#define DDR_BANK8 	1  /* Banks each chip: 0-4bank, 1-8bank ,0 for falcon fpga, 1 for develop board */
#define DDR_CL  	5  /* CAS latency: , LPDDR no */

/*
 * LPDDR2 controller timing1 register
 */
#define DDR_tRAS 	42 /*tRAS: ACTIVE to PRECHARGE command period to the same bank. ns*/
#define DDR_tRTP 	MAX(2, 7500)  /* 7.5ns READ to PRECHARGE command period. ???*/
#define DDR_tRP 	MAX(3, 21 * 1000) /* tRP: PRECHARGE command period to the same bank */
#define DDR_tRCD 	MAX(3, 18 * 1000) /* ACTIVE to READ or WRITE command period to the same bank. */
#define DDR_tRC 	(DDR_tRAS + DDR_tRP) /* ACTIVE to ACTIVE command period to the same bank.*/
#define DDR_tRRD 	MAX(2, 10 * 1000) /* ACTIVE bank A to ACTIVE bank B command period. */
#define DDR_tWR 	MAX(3, 15 * 1000) /* WRITE Recovery Time defined by register MR of DDR2 memory , ns*/
#define DDR_tWTR 	MAX(2, 7500)  /* WRITE to READ command delay. */
/*
 * LPDDR2 controller timing2 register
*/
//#define DDR_tRFC 	210 /* ns,  AUTO-REFRESH command period. */
#define DDR_tRFC 	130 /* ns,  AUTO-REFRESH command period. */
#define DDR_tMINSR 	57  /* Minimum Self-Refresh / Deep-Power-Down , tCK, no */
#define DDR_tXP 	MAX(2, 7500)   /* EXIT-POWER-DOWN to next valid command period. ns */
#define DDR_tMRD 	5	/* unit: tCK Load-Mode-Register to next valid command period, tck, tMRW */
#define DDR_tMRR	2	/* Mode Register Read command period */
/* new add */
#define DDR_tCKSRE  	MAX(3, 1500) 	/* LPDDR2 no: Valid Clock Requirement after Self Refresh Entry or Power-Down Entry */
#define DDR_tDLLLOCK	512
#define DDR_tXSRD 	MAX(2, (DDR_tRFC + 10) * 1000)	/* LPDDR2 : Exit self-refresh to next valid command , ns, tXSR */
#define DDR_tXS 	DDR_tXSRD	/* LPDDR2 : Exit self-refresh to next valid command , ns , tXSR */

#define DDR_tDQSCK    	3	/* LPDDR2 only: DQS output access from ck_t/ck_c, 2.5ns */
#define DDR_tDQSCKMAX 	6	/* LPDDR2 only: MAX DQS output access from ck_t/ck_c, 5.5ns */

#define DDR_BL	 	8	/* LPDDR2 Burst length: 3 - 8 burst, 2 - 4 burst , 4 - 16 burst*/
//#define DDR_tAL  	0	/* Additive Latency, tCK*/
#define DDR_tRL  	MATCH(CONFIG_SYS_MEM_FREQ,0)/*8*/	/* LPDDR2: Read Latency  - 3 4 5 6 7 8 , tck*/
#define DDR_tWL		MATCH(CONFIG_SYS_MEM_FREQ,1)/*4*/	/* LPDDR2: Write Latency - 1 2 2 3 4 4 , tck*/
//#define DDR_tRL  	3	/* LPDDR2: Read Latency  - 3 4 5 6 7 8 , tck*/
//#define DDR_tWL		1	/* LPDDR2: Write Latency - 1 2 2 3 4 4 , tck*/
#define DDR_tCCD 	2	/* CAS# to CAS# command delay , tCK*/
#define DDR_tRTW        (DDR_tRL + (((DDR_tDQSCKMAX * 1000) + tck_g.ps -1)/tck_g.ps) \
							 + DDR_BL / 2 + 1 - DDR_tWL)
#define DDR_tFAW 	MAX(8, 60 * 1000)	/* Four bank activate period, ns */
#define DDR_tCKE	3		/* CKE minimum pulse width, tCK */
#define DDR_tRDLAT	(DDR_tRL - 1)
#define DDR_tWDLAT	(DDR_tWL - 0)

/*
 * LPDDR2 controller refcnt register
 */
#define DDR_tREFI	7800	/* Refresh period: 4096 refresh cycles/64ms , line / 64ms */

#define DDR_CLK_DIV 	1   /* Clock Divider. auto refresh *cnt_clk = memclk/(16*(2^DDR_CLK_DIV))*/

#endif /* __LPDDR2_CONFIG_H */
