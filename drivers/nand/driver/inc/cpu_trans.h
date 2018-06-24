#ifndef _CPU_TRANS_H__
#define _CPU_TRANS_H__

#include "transadaptor.h"

typedef struct __CpuCopyInfo cpu_copy_info;
struct __CpuCopyInfo{
	unsigned char *src;
	unsigned char *dst;
	unsigned int len;
	void (*src_add_to_dst)(unsigned char *src, unsigned char *dst, unsigned int len);
	void (*src_to_dst_add)(unsigned char *src, unsigned char *dst, unsigned int len);
	void (*src_to_dst_both_add)(unsigned char *src, unsigned char *dst, unsigned int len);
	void (*copy_data)(unsigned char *src, unsigned char *dst, unsigned int len);
};

int cpu_move_init(transadaptor *trans);
void cpu_move_deinit(int context, transadaptor *trans);
int cpu_prepare_memcpy(int context, unsigned char *src, unsigned char *dst, unsigned int len, unsigned short flag);
int cpu_finish_memcpy(int context);

#endif
