#ifndef __TRANSADAPTOR_H__
#define __TRANSADAPTOR_H__

#define SRCADD  0
#define DSTADD  1
#define SRC_AND_DST_ADD    2

typedef struct __TransAdaptor transadaptor;
struct __TransAdaptor{
	int (*prepare_memcpy) (int context, unsigned char *src, unsigned char *dst, unsigned int len, unsigned short flag);
	int (*finish_memcpy) (int context);
};

#endif
