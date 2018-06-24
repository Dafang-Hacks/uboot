/**
 * os_clib.c
 **/
#include <nand_debug.h>
#include <nand_api.h>
#include <os_clib.h>

void (*ndd_ndelay) (unsigned long nsecs);
int (*ndd_div_s64_32)(long long dividend, int divisor);
void* (*ndd_alloc)(unsigned int size);
void (*ndd_free)(void *addr);
int (*ndd_printf)(const char *fmt, ...);
void* (*ndd_memcpy)(void *dst, const void *src, unsigned int count);
void* (*ndd_memset)(void *s, int c, unsigned int count);
int (*ndd_strcmp)(const char *cs, const char *ct);
unsigned int (*get_vaddr)(unsigned int paddr);
void (*ndd_dma_cache_wback)(unsigned long addr, unsigned long size);
void (*ndd_dma_cache_inv)(unsigned long addr, unsigned long size);
unsigned long long (*ndd_get_time_nsecs)(void);

static void *__memcpy(void *dst, const void *src, unsigned int count)
{
	char *tmp = dst;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;

	return dst;
}

static int __strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

static void *__memset(void *s, int c, unsigned int count)
{
	char *xs = s;

	while (count--)
		*xs++ = c;
	return s;
}

int os_clib_init(os_clib *clib)
{
	/*#############################*/
	ndd_ndelay = clib->ndelay;
	if (!ndd_ndelay)
		RETURN_ERR(ENAND, "function ndd_ndelay() not defined!\n");

	ndd_div_s64_32 = clib->div_s64_32;
	if (!ndd_div_s64_32)
		RETURN_ERR(ENAND, "function ndd_div_s64_32() not defined!\n");

	ndd_alloc = clib->continue_alloc;
	if (!ndd_alloc)
		RETURN_ERR(ENAND, "function ndd_alloc() not defined!\n");

	ndd_free = clib->continue_free;
	if (!ndd_free)
		RETURN_ERR(ENAND, "function ndd_free() not defined!\n");

	/*#############################*/
	get_vaddr = clib->get_vaddr;
	if (!get_vaddr)
		RETURN_ERR(ENAND, "function get_vaddr() not defined!\n");

	ndd_dma_cache_wback = clib->dma_cache_wback;
	if (!ndd_dma_cache_wback)
		RETURN_ERR(ENAND, "function ndd_dma_cache_wback() not defined!\n");

	ndd_dma_cache_inv = clib->dma_cache_inv;
	if (!ndd_dma_cache_inv)
		RETURN_ERR(ENAND, "function ndd_dma_cache_inv() not defined!\n");

	/*#############################*/
	ndd_printf = clib->printf;
	if (!ndd_printf)
		RETURN_ERR(ENAND, "function ndd_printf() not defined!\n");

	ndd_get_time_nsecs = clib->get_time_nsecs;
	if (!ndd_get_time_nsecs)
		RETURN_ERR(ENAND, "function ndd_get_time_nsecs() not defined!\n");

	/*#############################*/
	ndd_memcpy = clib->memcpy;
	if (!ndd_memcpy)
		ndd_memcpy = __memcpy;

	ndd_memset = clib->memset;
	if(!ndd_memset)
		ndd_memset = __memset;

	ndd_strcmp = clib->strcmp;
	if (!ndd_strcmp)
		ndd_strcmp = __strcmp;

	return 0;
}
