/**
 * clib.c
 **/
#include "clib.h"

unsigned int nm_sleep(unsigned int seconds)
{
	return 0;
#if 0
	return sleep(seconds);
#endif
}

long long nd_getcurrentsec_ns(void)
{
	return 0;
#if 0
	long long ns = -1LL;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ns = (long long)ts.tv_sec * 1000000000L	+ (long long)ts.tv_nsec;
	return ns;
#endif
}

unsigned int nd_get_timestamp(void) {
	return 0;
#if 0
	long long ns = -1LL;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ns = (long long)ts.tv_sec * 1000000000L	+ (long long)ts.tv_nsec;
	return ns / 1000000L;
#endif
}

int nm_print_message(enum nm_msg_type type, int arg)
{
	return 0;
}

unsigned int nd_get_phyaddr(void * addr)
{
	unsigned int address = (unsigned int)addr;
	if(address >= 0xa0000000)
		return (address - 0xa0000000);
	else
		return (address - 0x80000000);
}

unsigned int nd_get_pid(void)
{
	return 0;
}
