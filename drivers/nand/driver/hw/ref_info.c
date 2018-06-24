#include <linux/slab.h>

#include "ref_info.h"
#include <nand_debug.h>

struct list_head *top = NULL;

static ref_info *alloc_new_node(int dev_type, int dev_id, int init_flag){
	ref_info *info;

	info = ndd_alloc(sizeof(ref_info));
	if(info == NULL)
		RETURN_ERR(NULL, "Alloc memory error");

	info->dev_type = dev_type;
	info->dev_id = dev_id;
	info->refcount = 1;
	info->current_mode = -1;
	if(init_flag)
		info->clk = 1;
	ndd_debug("%s %d ref_info alloc a new node\n",__func__,__LINE__);
	return info;
}

/*
  @dev_type : BCH_TYPE or NEMC_TYPE
  @init_flag: 0 open parameter
              1 init parameter
 */
ref_info *add_ref_info(int dev_type, int dev_id, int init_flag)
{
	struct list_head *pos;
	ref_info *refinfo = NULL;

	if (top == NULL){
		top = ndd_alloc(sizeof(ref_info));
		INIT_LIST_HEAD(top);
		ndd_debug("reflist top is created\n");
	}
	if(top->prev == top && top->next == top){
new:
		refinfo = alloc_new_node(dev_type,dev_id,init_flag);
		list_add_tail(&refinfo->head,top);
		ndd_debug("add :%d refinfo=%p  \n",__LINE__,refinfo);
	}else{
		list_for_each(pos,top){
			refinfo = list_entry(pos,ref_info,head);
			if(refinfo->dev_type == dev_type && refinfo->dev_id == dev_id){
				refinfo->refcount++;
				if(init_flag)
					refinfo->clk++;
				break;
			}
			else if(list_is_last(pos,top)){
				goto new;
			}
		}
	}
#if 0
	list_for_each(pos,top){
		refinfo = list_entry(pos,ref_info,head);
		i++;
		ndd_debug("list %d : dev_id=%d dev_type=%d refcount=%d clk=%d \n",
			  i,refinfo->dev_id,refinfo->dev_type,refinfo->refcount,refinfo->clk);
	}
#endif

	return refinfo;
}

void reduce_ref_info(int dev_type, int dev_id, int init_flag)
{
	struct list_head *pos;
	ref_info *refinfo;

	//ndd_debug("%s start dev_id=%d dev_type=%d\n",__func__,dev_id,dev_type);
	list_for_each(pos,top){
		refinfo = list_entry(pos,ref_info,head);
		//ndd_debug("reduce: %d refinfo=%p  \n",__LINE__,refinfo);
		if(refinfo->dev_type == dev_type && refinfo->dev_id == dev_id){
			refinfo->refcount--;
			if(init_flag)
				refinfo->clk--;
			if(refinfo->refcount == 0 && refinfo->clk == 0){
				list_del(pos);
				ndd_free(refinfo);
				break;
			}
		}
	}
#if 0
	list_for_each(pos,top){
		refinfo = list_entry(pos,ref_info,head);
		ndd_debug("%d dev_id=%d dev_type=%d refcount=%d clk=%d \n",
			  __LINE__,refinfo->dev_id,refinfo->dev_type,refinfo->refcount,refinfo->clk);
	}
#endif
}
