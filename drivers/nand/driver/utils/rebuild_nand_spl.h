#ifndef __REBUILD_NAND_SPL_H__
#define __REBUILD_NAND_SPL_H__

enum nand_spl_type {
	NAND_SPL_TYPE_COMMON = 0x00,
	NAND_SPL_TYPE_TOGGLE = 0x01,
	NAND_SPL_TYPE_ONFI = 0x02,
};

typedef struct __nand_basic_info {
	unsigned int id;
	unsigned int extid;
	unsigned short pagesize;
	unsigned short oobsize;
	unsigned int blocksize;
	unsigned int totalblocks;
	unsigned int maxvalidblocks;
	unsigned char eccbit;
	unsigned char planepdie;
	unsigned char diepchip;
	unsigned char chips;
	unsigned char buswidth;
	unsigned char realplanenum;
	unsigned short badblockpos;
	unsigned char rowcycles;
	unsigned char planeoffset;
	unsigned int options;
} nand_basic_info;

typedef struct __nand_params {
	unsigned int magic;
	nand_basic_info ndbaseinfo;
	unsigned int kernel_offset; //kernel partition position
	unsigned int nand_manager_version;
} nand_params; //total size 44Bytes

#define REBUILD_GET_NAND_TYPE(n)	(((n)>>14) & 0x3)
int rebuild_nand_spl_4775(nand_params *ndparams, void *spl_sbuf, void *spl_dbuf);
int rebuild_nand_spl_4780(nand_params *ndparams, void *spl_sbuf, void *spl_dbuf);
int rebuild_nand_spl_m200(nand_params *ndparams, void *spl_sbuf, void *spl_dbuf);

#endif
