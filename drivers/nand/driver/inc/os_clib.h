#ifndef _NAND_OS_CLIB_H_
#define _NAND_OS_CLIB_H_

#ifndef NULL
#define NULL (void *)0
#endif

extern void (*ndd_ndelay) (unsigned long nsecs);
extern int (*ndd_div_s64_32)(long long dividend, int divisor);
/**
 * nand driver continue memory alloc
 **/
extern void* (*ndd_alloc)(unsigned int size);
/**
 * nand driver continue memory free
 **/
extern void (*ndd_free)(void *);
extern void* (*ndd_memcpy)(void *dst, const void *src, unsigned int count);
extern void* (*ndd_memset)(void *s, int c, unsigned int count);
extern int (*ndd_strcmp)(const char *cs, const char *ct);
extern unsigned int (*get_vaddr)(unsigned int paddr);
extern void (*ndd_dma_cache_wback)(unsigned long addr, unsigned long size);
extern void (*ndd_dma_cache_inv)(unsigned long addr, unsigned long size);
extern unsigned long long (*ndd_get_time_nsecs)(void);

#endif /* _NAND_OS_CLIB_H_ */
