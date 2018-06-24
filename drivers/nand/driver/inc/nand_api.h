#ifndef _NAND_API_H_
#define _NAND_API_H_

#include <nand_driver.h>

#define NAND_NORMAL_ERASE_MODE	1
#define NAND_FORCE_ERASE_MODE	2
#define NAND_FACTORY_ERASE_MODE	3

#define REDUN_PT_NUM		1 //ndvirtual
#define HW_SECTOR		512

#define CPU_COPY_MODE		1
#define DMA_COPY_MODE		2
#define DEFAULT_COPY_MODE 	CPU_COPY_MODE
#define DEFAULT_OPS_MODE 	DMA_OPS

#define DEFAULT_ECCSIZE		1024
#define SPL_ECCSIZE		256

#define ZONE_COUNT_LIMIT	32 //min ndpartition zone count limit
#define ERR_PT_TOTALBLOCKS	4 //virtual blocks

#define MAX_NAME_SIZE		32

#define MAX_RB_COUNT		CS_PER_NFI
#define MAX_PT_COUNT		16
#define NAND_MAGIC		0x646e616e // ASCII of "nand"

int nand_api_init(struct nand_api_osdependent *osdep);
int nand_api_reinit(struct nand_api_platdependent *platdep);
int nand_api_get_nandflash_id(nand_flash_id *id);
int nand_api_suspend(void);
int nand_api_resume(void);

#endif /*_NAND_API_H_*/
