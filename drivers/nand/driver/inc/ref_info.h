#ifndef __REFINFO_H__
#define __REFINFO_H__

#include <linux/list.h>

/*device type*/
#define BCH_TYPE  0
#define NEMC_TYPE 1

/*init flag*/
#define OPEN_FLAG  0
#define CLOSE_FLAG OPEN_FLAG
#define INIT_FLAG  1
#define DEINIT_FLAG INIT_FLAG

typedef struct __RefInfo ref_info;
struct __RefInfo{
	struct list_head head;
	int dev_type;
	int dev_id;
	unsigned int refcount;
	unsigned int clk;
	int current_mode;
};

ref_info *add_ref_info(int dev_type, int dev_id, int init_flag);
void reduce_ref_info(int dev_type, int dev_id, int init_flag);
#endif
