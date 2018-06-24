#include <os_clib.h>
#include <nand_debug.h>
#include "cpu_trans.h"
#include "transadaptor.h"

static void src_add_to_dst(unsigned char *src, unsigned char *dst, unsigned int len)
{
	int i;
	if(!(len & (sizeof(int) - 1))){
		for(i = 0; i < len / sizeof(int); i++)
			*(volatile unsigned int *)dst = ((unsigned int *)src)[i];
	}else if(!(len & (sizeof(short) - 1))){
		for(i = 0; i < len / sizeof(short); i++)
			*(volatile unsigned short *)dst = ((unsigned short *)src)[i];
	}else{
		for(i=0; i<len; i++)
			*(volatile unsigned char *)dst = src[i];
	}
}

static void src_to_dst_add(unsigned char *src, unsigned char *dst, unsigned int len)
{
	int i;
	if(!(len & (sizeof(int) - 1))){
		for(i = 0; i < len / sizeof(int); i++)
			((unsigned int *)dst)[i] = *(volatile unsigned int *)src;
	}else if(!(len & (sizeof(short) - 1))){
		for(i = 0; i < len / sizeof(short); i++)
			((unsigned short *)dst)[i] = *(volatile unsigned short *)src;
	}else{
		for(i=0; i<len; i++)
			dst[i] = *(volatile unsigned char *)src;
	}
}

static void src_to_dst_both_add(unsigned char *src, unsigned char *dst, unsigned int len)
{
	int i;
	if(!(len & (sizeof(int) - 1))){
		for(i = 0; i < len / sizeof(int); i++)
			((unsigned int *)dst)[i] = ((volatile unsigned int *)src)[i];
	}else if(!(len & (sizeof(short) - 1))){
		for(i = 0; i < len / sizeof(short); i++)
			((unsigned short *)dst)[i] = ((volatile unsigned short *)src)[i];
	}else{
		for(i=0; i<len; i++)
			dst[i] = *(volatile unsigned char *)src++;
	}
}

int cpu_prepare_memcpy(int context, unsigned char *src, unsigned char *dst, unsigned int len, unsigned short flag)
{
	cpu_copy_info *info = (cpu_copy_info *)context;

	if(src == NULL || dst == NULL || len == 0)
		RETURN_ERR(ENAND, "memcpy failed, src = %p dst = %p len = %d", src, dst, len);

	info->src = src;
	info->dst = dst;
	info->len = len;
	if(flag == SRCADD){
		info->copy_data = src_add_to_dst;
	}else if(flag == DSTADD){
		info->copy_data = src_to_dst_add;
	}else if(flag == SRC_AND_DST_ADD){
		info->copy_data = src_to_dst_both_add;
	}else
		RETURN_ERR(ENAND, "memcpy flag is invalid, flag = %d", flag);
	return 0;
}

int cpu_finish_memcpy(int context)
{
	cpu_copy_info *info = (cpu_copy_info *)context;

	info->copy_data(info->src,info->dst,info->len);
	return 0;
}

int cpu_move_init(transadaptor *trans)
{
	cpu_copy_info *copy_info;

	copy_info = ndd_alloc(sizeof(cpu_copy_info));
	if(!copy_info)
		RETURN_ERR(ENAND, "alloc memory error");
	trans->prepare_memcpy = cpu_prepare_memcpy;
	trans->finish_memcpy = cpu_finish_memcpy;
	return (int)copy_info;
}

void cpu_move_deinit(int context, transadaptor *trans)
{
	cpu_copy_info *copy_info = (cpu_copy_info *)context;

	ndd_free(copy_info);
	trans->prepare_memcpy = NULL;
	trans->finish_memcpy = NULL;
}
