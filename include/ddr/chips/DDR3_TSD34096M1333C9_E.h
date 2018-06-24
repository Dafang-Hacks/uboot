#ifndef __DDR3_CONFIG_H
#define __DDR3_CONFIG_H

//#include "ddr.h"
/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * DDR3-1333 info
 */
#define DLL_OFF

/* DDR3 paramters */
#define DDR_ROW     15  /* ROW : 12 to 14 row address */
#define DDR_ROW1     15  /* ROW : 12 to 14 row address */
#define DDR_COL     10  /* COL :  8 to 10 column address */
#define DDR_COL1     10  /* COL :  8 to 10 column address */
#define DDR_BANK8   1 	/* Banks each chip: 0-4bank, 1-8bank */

#ifdef DLL_OFF
#define DDR_CL      6   /* dll off */
#define DDR_tCWL    6	/* DDR3 dll off*/
#else
#define DDR_CL      7   /* CAS latency: 5 to 14 ,tCK*/
#define DDR_tCWL   (DDR_CL - 1)	/* DDR3 only: CAS Write Latency, 5 to 8 */
#endif

/*
 * DDR3 controller timing1 register
 */
#define DDR_tRAS 36  /* tRAS: ACTIVE to PRECHARGE command period to the same bank. ns*/
#define DDR_tRP  12  /* tRP: PRECHARGE command period to the same bank. ns*/
#define DDR_tRCD 12  /* ACTIVE to READ or WRITE command period to the same bank. ns*/
#define DDR_tRC  48  /* ACTIVE to ACTIVE command period to the same bank. ns*/
#define DDR_tWR  15  /* WRITE Recovery Time defined by register MR of DDR2 memory, ns*/
#define DDR_tRRD MAX(4, 7500) /* ACTIVE bank A to ACTIVE bank B command period. DDR3 - tCK*/
#define DDR_tRTP MAX(4, 7500) /* READ to PRECHARGE command period. DDR3 spec no. 7.5ns*/
#define DDR_tWTR MAX(4, 7500) /* WRITE to READ command delay. DDR3 spec no. 7.5 ns*/
	 
/*
 * DDR3 controller timing2 register
 */
#define DDR_tRFC   160 	/* AUTO-REFRESH command period. DDR3 - ns*/
#define DDR_tMINSR 57   /* Minimum Self-Refresh / Deep-Power-Down . DDR3 no*/
#define DDR_tXP    3	/* DDR3 only: Exit active power down to any valid command, ns*/
#define DDR_tMRD   4    /* unit: tCK. Load-Mode-Register to next valid command period: DDR3 rang 4 to 7 tCK. DDR3 spec no */

/* new add */
#define DDR_BL	   8   /* DDR3 Burst length: 0 - 8 burst, 2 - 4 burst , 1 - 4 or 8(on the fly)*/
#define DDR_tAL    0	/* Additive Latency, tCK*/
#define DDR_tCCD   4	/* CAS# to CAS# command delay , tCK. 4 or 5 */
#define DDR_tFAW   38	/* Four bank activate period, DDR3 - tCK */
#define DDR_tCKE   	MAX(3, 7500)	/* CKE minimum pulse width, DDR3 spec no, tCK */
#define DDR_tRL 	(DDR_tAL + DDR_CL)	/* DDR3: Read Latency = tAL + tCL */
#define DDR_tWL 	(DDR_tAL + DDR_tCWL)	/* DDR3: Write Latency = tAL + tCWL */
#define DDR_tRDLAT	(DDR_tRL - 2)	
#define DDR_tWDLAT	(DDR_tWL - 1)
#define DDR_tRTW 	(DDR_tRL + DDR_tCCD + 2 - DDR_tWL)	/* Read to Write delay */
#define DDR_tCKSRE 	MAX(5, 10000) /* Valid Clock Requirement after Self Refresh Entry or Power-Down Entry */

#define DDR_tDLLLOCK	512		/* DDR3 only: DLL LOCK, tck */
#define DDR_tXSDLL 	MAX(DDR_tDLLLOCK, 0)		/* DDR3 only: EXit self-refresh to command requiring a locked DLL, tck*/
#define DDR_tMOD   	MAX(12, 15 * 1000)	/* DDR3 only: Mode Register Set Command update delay*/
#define DDR_tXPDLL 	MAX(10, 24 * 1000)	 /* DDR3 only: Exit active power down to command requirint a locked DLL, ns*/
#define DDR_tXS    	MAX(5, (DDR_tRFC + 10) * 1000) /* DDR3 only: EXit self-refresh to command not requiring a locked DLL, ns*/
#define DDR_tXSRD  	10		/* DDR2 only: Exit self refresh to a read command, tck */

/*
 * DDR3 controller refcnt register
 */
#define DDR_tREFI   7800	/* Refresh period: 64ms / 32768 = 1.95 us , 2 ^ 15 = 32768 */  
#define DDR_CLK_DIV 1    	/* Clock Divider. auto refresh
			  *	cnt_clk = memclk/(16*(2^DDR_CLK_DIV))
			  */

#endif /* __DDR3_CONFIG_H */
