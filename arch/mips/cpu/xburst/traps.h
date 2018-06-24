#ifndef __INCLUDE__TASK_H__
#define __INCLUDE__TASK_H__

#ifndef __ASSEMBLY__

struct task_info_regs {
        unsigned int ra;
        unsigned int fp;
        unsigned int t9;
        unsigned int t8;
        unsigned int s7;
        unsigned int s6;
        unsigned int s5;
        unsigned int s4;
        unsigned int s3;
        unsigned int s2;
        unsigned int s1;
        unsigned int s0;
        unsigned int t7;
        unsigned int t6;
        unsigned int t5;
        unsigned int t4;
        unsigned int t3;
        unsigned int t2;
        unsigned int t1;
        unsigned int t0;
        unsigned int a3;
        unsigned int a2;
        unsigned int a1;
        unsigned int a0;
        unsigned int v1;
        unsigned int v0;
        unsigned int at;
        unsigned int lo;
        unsigned int hi;
        unsigned int status;
        unsigned int epc;
        unsigned int sp;
};

#endif

#define	GT_SP		(16)
#define REGS_OFFSET	0
#define	PT_RA		(REGS_OFFSET + 0x00)
#define	PT_FP		(REGS_OFFSET + 0x04)
#define	PT_T9		(REGS_OFFSET + 0x08)
#define	PT_T8		(REGS_OFFSET + 0x0c)
#define	PT_S7		(REGS_OFFSET + 0x10)
#define	PT_S6		(REGS_OFFSET + 0x14)
#define	PT_S5		(REGS_OFFSET + 0x18)
#define	PT_S4		(REGS_OFFSET + 0x1c)
#define	PT_S3		(REGS_OFFSET + 0x20)
#define	PT_S2		(REGS_OFFSET + 0x24)
#define	PT_S1		(REGS_OFFSET + 0x28)
#define	PT_S0		(REGS_OFFSET + 0x2c)
#define	PT_T7		(REGS_OFFSET + 0x30)
#define	PT_T6		(REGS_OFFSET + 0x34)
#define	PT_T5		(REGS_OFFSET + 0x38)
#define	PT_T4		(REGS_OFFSET + 0x3c)
#define	PT_T3		(REGS_OFFSET + 0x40)
#define	PT_T2		(REGS_OFFSET + 0x44)
#define	PT_T1		(REGS_OFFSET + 0x48)
#define	PT_T0		(REGS_OFFSET + 0x4c)
#define	PT_A3		(REGS_OFFSET + 0x50)
#define	PT_A2		(REGS_OFFSET + 0x54)
#define	PT_A1		(REGS_OFFSET + 0x58)
#define	PT_A0		(REGS_OFFSET + 0x5c)
#define	PT_V1		(REGS_OFFSET + 0x60)
#define	PT_V0		(REGS_OFFSET + 0x64)
#define	PT_AT		(REGS_OFFSET + 0x68)
#define	PT_LO		(REGS_OFFSET + 0x6c)
#define	PT_HI		(REGS_OFFSET + 0x70)
#define	PT_STATUS	(REGS_OFFSET + 0x74)
#define	PT_EPC		(REGS_OFFSET + 0x78)
#define	PT_SP		(REGS_OFFSET + 0x7c)
#define	PT_LEN		(PT_SP - PT_RA + 4)

#endif
