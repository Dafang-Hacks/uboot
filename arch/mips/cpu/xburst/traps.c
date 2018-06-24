
#include <config.h>
#include <common.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>

#include "traps.h"

extern void handle_exception(void);

void inline set_handler(unsigned long offset, void *addr, unsigned long size)
{
	memcpy((void *)(KSEG0 + offset), addr, size);
}

char *get_exception(int num)
{
	switch(num) {
		case 0  : return "Interrupt";
		case 1  : return "TLB modification exception (4KEc core)";
		case 2  : return "TLB exception (load or instruction fetch) (4KEc core)";
		case 3  : return "TLB exception (store) (4KEc core)";
		case 4  : return "Address error exception (load or instruction fetch)";
		case 5  : return "Address error exception (store)";
		case 6  : return "Bus error exception (instruction fetch)";
		case 7  : return "Bus error exception (data reference: load or store)";
		case 8  : return "Syscall exception";
		case 9  : return "Breakpoint exception";
		case 10 : return "Reserved instruction exception";
		case 11 : return "Coprocessor Unusable exception";
		case 12 : return "Arithmetic Overflow exception";
		case 13 : return "Trap exception";
		case 16 : return "Implementation-Specific Exception 1 (COP2)";
		case 17 : return "Implementation-Specific Exception 2(COP2)";
		case 18 : return "Coprocessor 2 exceptions";
		case 23 : return "Reference to WatchHi/WatchLo address";
		case 24 : return "Machine check";
	}
	return "UNKNOW";
}

void dump_task()
{
	int i,*p;
	struct task_info_regs *regs = *((unsigned int *)0x80000004);
	p = (int *)(*((volatile unsigned int *)(0x80000004)) + PT_LEN);
	
	printf("epc:%x\n",read_c0_epc());
	printf("status:%x\n",regs->status);
	printf("cause:%x\n",read_c0_cause());

	printf("\ndump regs:\n");
	printf("ra[%x] fp[%x] t9[%x] t8[%x]\n", regs->ra,regs->fp,regs->t9,regs->t8);
	printf("s7[%x] s6[%x] s5[%x] s4[%x]\n", regs->s7,regs->s6,regs->s5,regs->s4);
	printf("s3[%x] s2[%x] s1[%x] s0[%x]\n", regs->s3,regs->s2,regs->s1,regs->s0);
	printf("t7[%x] t6[%x] t5[%x] t4[%x]\n", regs->t7,regs->t6,regs->t5,regs->t4);
	printf("t3[%x] t2[%x] t1[%x] t0[%x]\n", regs->t3,regs->t2,regs->t1,regs->t0);
	printf("a3[%x] a2[%x] a1[%x] a0[%x]\n", regs->a3,regs->a2,regs->a1,regs->a0);
	printf("v0[%x] v1[%x] lo[%x] hi[%x]\n", regs->v0,regs->v1,regs->lo,regs->hi);
	printf("\nsp:%x",p);
	for(i=0;i<32;i++) {
		if(i%4 == 0) printf("\naddr %x:  ",&p[i]);
		printf("%x ",p[i]);
	}
	printf("\n\n=====================================================\n\n");
}

void __attribute__ ((noreturn)) print_exp(void) 
{
	printf("\nEXCEPTION:%s\n",get_exception((read_c0_cause() & 0x7c)>>2));
	dump_task();
	while(1)
		__asm__ __volatile__("wait\n\t");
}


int traps_init(void)
{
	*((volatile unsigned int *)(0x80000000)) = print_exp;
	set_handler(0x180, &handle_exception, 180);
	flush_cache_all();
	write_c0_cause(0x0 | 0x1<<23);
	write_c0_status(0x1000ff01);
	return 0;
}

