#ifndef __NAND_PARAM_H__
#define __NAND_PARAM_H__


#define MAX_NAME_SIZE 32
#define MAX_RB_COUNT 4
#define MAX_PART_NUM 16
#define MUL_PARTS    4

/* driver strength, level 0 is weakest */
#define DRV_STRENGTH_DEFAULT    0
#define DRV_STRENGTH_LEVEL0 1
#define DRV_STRENGTH_LEVEL1 2
#define DRV_STRENGTH_LEVEL2 3
#define DRV_STRENGTH_LEVEL3 4

/* rb pulldown strength, level 0 is weakest */
#define RB_PULLDOWN_STRENGTH_DEFAULT    0
#define RB_PULLDOWN_STRENGTH_LEVEL0 1
#define RB_PULLDOWN_STRENGTH_LEVEL1 2
#define RB_PULLDOWN_STRENGTH_LEVEL2 3
#define RB_PULLDOWN_STRENGTH_LEVEL3 4

typedef struct _PartitionInfo PartitionInfo;
typedef struct _Nandppt Nandppt;
typedef struct __ui_plat_ex_partition ui_plat_ex_partition;

struct __ui_plat_ex_partition {
	char name[MAX_NAME_SIZE];
	int  offset;
	int  size;
};

struct _Nandppt{
	char name[MAX_NAME_SIZE];		/* the name of patition*/
	int offset;						/* the offset of patition (M)*/
	int size;						/* the size of the patition (M)*/
	int managermode;				/* the managermode of the patition; 0:spl_manger, 1:direct_manger, 2:zone_manger, 3:once_manger*/
	int cache;						/* whether the data need the nandmanger to cache */	
	unsigned int attribute;			/* the patition attribute*/
	ui_plat_ex_partition ui_ex_partition[MUL_PARTS];	/* the patition info which is a embedded patition  */
};

struct _PartitionInfo{
	Nandppt ndppt[MAX_PART_NUM];	/* patition */
	int ptcount;					/* the number of patition */
	int rbcount;					/* the number of rb */
	int rb_gpio[MAX_RB_COUNT];		/* the gpio pin of rb */
	int gpio_wp;					/* the gpio pin of wp */
	unsigned char  rb_pulldown_strength[MAX_RB_COUNT];		/* the strength of the pin of rb */
	unsigned char  nand_driver_strength;					/* the strength of nand driver */
};


/**
 *  * struct __nand_timing - NAND Flash Device timing
 *   **/
typedef struct __nand_timing_param {
	unsigned int tALS;  /* ... duration/width/time */
	unsigned int tALH;  /* ... duration/width/time */
	unsigned int tRP;   /* ... duration/width/time */
	unsigned int tWP;   /* ... duration/width/time */
	unsigned int tRHW;  /* ... duration/width/time */
	unsigned int tWHR;  /* ... duration/width/time */
	unsigned int tWHR2; /* ... duration/width/time */
	unsigned int tRR;   /* ... duration/width/time */
	unsigned int tWB;   /* ... duration/width/time */
	unsigned int tADL;  /* ... duration/width/time */
	unsigned int tCWAW; /* ... duration/width/time */
	unsigned int tCS;   /* ... duration/width/time */
	unsigned int tCLH;  /* ... duration/width/time */
	unsigned int tWH;   /* ... duration/width/time */
	unsigned int tCH;   /* ... duration/width/time */
	unsigned int tDH;   /* ... duration/width/time */
	unsigned int tREH;  /* ... duration/width/time */

} nand_timing_param;

 
typedef struct __optionalcmd_param {
	unsigned char multiplaneread[2];        // the sequence is [0] -ADDR- [0] -ADDR- [1] - DATA
	unsigned char multiplanewrite[2];       // the sequence is 80 -ADDR- DATA - [0] - [1] -ADDR- DATA - 10/15
	unsigned char multiplanecopyread[3];    // the sequence is [0] -ADDR- [1] -ADDR- [2]
	unsigned char multiplanecopywrite[3];   // the sequence is [0] -ADDR- [1] - [2] -ADDR- 10
	unsigned char multiplanestatus;     // the command may be 0x70/0x71/0x78/...
	unsigned char interbnk0status;      // the command may be 0xf1/0x78/...
	unsigned char interbnk1status;      // the command may be 0xf2/0x78/...
} optionalcmd_param;

/**
 *  * struct __nand_flash - NAND Flash Device attr Structure
 *   **/
typedef struct __nand_flash_param {
	char name[32];
	unsigned int id;
	unsigned int extid;
	unsigned int pagesize;
	unsigned int blocksize;
	unsigned int oobsize;
	unsigned int totalblocks;
	unsigned int maxvalidblocks;
	unsigned int minvalidblocks;
	unsigned int pagenumber;
	unsigned int badblockpage;
	unsigned int bpc;
	unsigned char eccbit;
	unsigned char planepdie;
	unsigned char diepchip;
	unsigned char chips;
	unsigned char buswidth;
	unsigned char realplanenum;
	unsigned short badblockpos;
	unsigned char timingmod;
	unsigned char rowcycles;
	unsigned char pageaddresscycle;
	unsigned char planeoffset; //multi-plane block address offset
	unsigned int options;
	unsigned char nandtype;
	nand_timing_param timing;
	optionalcmd_param *optcmd;
} nand_flash_param;


int nand_probe_burner(PartitionInfo *pinfo, nand_flash_param *nand_info_ids,int nand_nm,int eraseall,unsigned int *pt_startadd_offset,int ops_pt_cnt);
unsigned int do_nand_request(unsigned int startaddr, void *data_buf, unsigned int ops_length,unsigned int offset);
#endif //__NAND_PARAM_H__
