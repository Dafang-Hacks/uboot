#ifndef _NAND_OPS_TIMING_H_
#define _NAND_OPS_TIMING_H_

typedef struct __nand_ops_timing{
	const void *io_timing;
	const void *io_etiming;
        unsigned int tRP;	/* ... duration/width/time */
        unsigned int tWP;	/* ... duration/width/time */
        unsigned int tWHR;	/* ... duration/width/time */
	unsigned int tWHR2;	/* ... duration/width/time */
	unsigned int tRR;	/* ... duration/width/time */
	unsigned int tWB;	/* ... duration/width/time */
	unsigned int tADL;	/* ... duration/width/time */
	unsigned int tCWAW;	/* ... duration/width/time */
        unsigned int tCS;	/* ... duration/width/time */
	unsigned int tCLH;	/* ... duration/width/time */
	unsigned int tWC;
}nand_ops_timing;

#endif /* _NAND_OPS_TIMING_H_ */
