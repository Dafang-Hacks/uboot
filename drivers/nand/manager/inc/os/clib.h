#ifndef _CLIB_H_
#define _CLIB_H_

#ifdef  __KERNEL__
#include <linux/string.h>
#include <linux/math64.h>
#else
typedef unsigned int size_t;
#define  NULL (void*)0
extern void*  memcpy(void *, const void *, size_t);
extern void*  memset(void *, int, size_t);
extern size_t strlen(const char *);
extern int    strcmp(const char *, const char *);
extern char*  strcpy(char *, const char *);
//#include <string.h>
/*add for print*/
extern void serial_puts_msg (const char *s);
extern void dump_uint(unsigned int);
extern void serial_puts (const char *s);
extern void serial_put_hex(unsigned int  d);
#endif

unsigned int nm_sleep(unsigned int seconds);
long long nd_getcurrentsec_ns(void);
unsigned int nd_get_timestamp(void);
unsigned int nd_get_phyaddr(void * addr);
unsigned int nd_get_pid(void);

enum nm_msg_type {
	NM_MSG_ERASE_PERSENT,
};

int nm_print_message(enum nm_msg_type type, int arg);

#endif /*_CLIB_H_*/
