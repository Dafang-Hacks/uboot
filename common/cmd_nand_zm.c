/*
 * cmd_nand_zm.c
 *
 * NAND cmd for which nand support the way of zone manager;
 *
 * Copyright (c) 2005-2008 Ingenic Semiconductor Inc.
 *
 */
#include <common.h>
#include <command.h>
#include <../drivers/nand/manager/inc/lpartition.h>
#include <../drivers/nand/manager/inc/sectorlist.h>

#define ZM_MEMORY_SIZE  (8*1024*1024)

#define DEFAULT_LOAD_KERNEL_ADDR	0x80600000
#define DEFAULT_PARTITION_OFFS	0

struct Ghandle{
	int zm_handle;
	unsigned int sectorid;
	int  pphandle;
	LPartition *lp;
};

struct Ghandle g_handle;

static int get_pt_handle(char *pt_name)
{
	struct singlelist *it;
	LPartition *lpentry;

	singlelist_for_each(it,&(g_handle.lp->head)){
		lpentry = singlelist_entry(it,LPartition,head);
		if(strcmp(lpentry->name,pt_name) == 0){
			g_handle.pphandle = NandManger_ptOpen(g_handle.zm_handle,lpentry->name,lpentry->mode);
			break;
		}
	}

	if(!g_handle.pphandle){
		printf("ERROR: the partition %s is not open ,pphandle is -1 ,please check the partition name,if not have the partition,please add it \n",pt_name);
		return CMD_RET_FAILURE;
	}

	return lpentry->sectorCount * 512;
}

int do_nand_zm(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *cmd;
	int bl;
	char *pt_name;
	SectorList *sl;
	unsigned char *databuf;
	unsigned int dst_addr,offset,len;
	unsigned int read_len;
	unsigned int pt_size;

	cmd = argv[1];
	pt_name = argv[2];

	if(!(argc == 3 || argc == 6))
	{
		printf("ERROR: argv error,please check the param of cmd !!!\n");
		return CMD_RET_USAGE;
	}

	pt_size = get_pt_handle(pt_name);
	if(pt_size < 0){
		return CMD_RET_FAILURE;
	}

	if(argc == 3){
		offset = DEFAULT_PARTITION_OFFS;
		dst_addr = DEFAULT_LOAD_KERNEL_ADDR;
		len = pt_size;
	}else{
		offset = (unsigned int)simple_strtoul(argv[3], NULL, 16);
		len = (unsigned int)simple_strtoul(argv[4], NULL, 16);
		dst_addr = (unsigned int)simple_strtoul(argv[5], NULL, 16);
	}
	databuf = dst_addr;

	g_handle.sectorid = offset / 512;


	bl = BuffListManager_BuffList_Init();
	if(bl == 0){
		    printf("BuffListManager Init failed!\n");
			    return CMD_RET_FAILURE;
	}

	while(len){
		if(len >= 256 * 512)
			read_len = 256 * 512;
		else{
			read_len = len;
			if(read_len % 512 != 0)
				    memset(databuf+read_len, 0xff, 512 - read_len%512);
		}
		sl = BuffListManager_getTopNode(bl,sizeof(SectorList));
		if(sl == 0){
			    printf("Bufferlist request sectorlist failed!\n");
				    return CMD_RET_FAILURE;
		}

		sl->startSector = g_handle.sectorid;
		sl->pData = (void*)databuf;
		sl->sectorCount = (read_len + 511)/ 512;
		g_handle.sectorid += (read_len + 511)/ 512;

		if(NandManger_ptRead(g_handle.pphandle,sl) < 0){
			    printf("NandManger_ptRead failed, now rewrite!\n");
		}

		databuf += read_len;
		len -= read_len;
	}

	load_addr = dst_addr;

	return CMD_RET_SUCCESS;
}



U_BOOT_CMD(nand_zm, 6, 1, do_nand_zm,
		"nand_zm    - NAND sub-system\n",
		"nand_zm read partition offs size addr\n"
		);

static void nand_manager_init(void)
{
	void *heap = (void*)malloc(ZM_MEMORY_SIZE);
	if(!heap){
		printf("%s %d malloc heap error!\n",__func__,__LINE__);
		return CMD_RET_FAILURE;
	}
	/* init global structure g_handle*/
	g_handle.zm_handle = NandManger_Init(heap,ZM_MEMORY_SIZE,0);

}
void nand_zm_init(void)
{
	int ret;

	nand_manager_init();
	ret = nand_probe();
	if(ret < 0){
		printf(" nand probe fail ! ret = %d\n",ret);
		return CMD_RET_FAILURE;
	}

	NandManger_getPartition(g_handle.zm_handle,&g_handle.lp);
}
