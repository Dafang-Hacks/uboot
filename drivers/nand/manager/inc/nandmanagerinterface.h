#ifndef __NANDMANGER_H__
#define __NANDMANGER_H__

#include "sectorlist.h"
#include "lpartition.h"
#include "ppartition.h"
#include "blocklist.h"

enum nandmanager_cmd {
	NANDMANAGER_PREPARE_NEW_FLASH,
	NANDMANAGER_CHECK_USED_FLASH,
	NANDMANAGER_ERASE_FLASH,
	NANDMANAGER_UPDATE_ERRPT,
	NANDMANAGER_SET_XBOOT_OFFSET = 24, // can only used for xboot partition, other partition will ignore this cmd
};

/* init flags */
#define NM_FLAG_NO_ERROR	0x01

/*public*/
int NandManger_Init(void *heap, int heap_size, int flags);
void NandManger_DeInit(int handle);

int NandManger_getPartition(int handle, LPartition** pt);
int NandManger_ptOpen(int handle, const char* name, int mode);
int NandManger_ptRead(int context, SectorList* bl);
int NandManger_ptWrite(int context, SectorList* bl);
int NandManger_ptIoctrl(int context, int cmd, int args);
int NandManger_ptErase(int context);
int NandManger_ptClose(int context);

void NandManger_startNotify(int handle,void (*start)(int),int prdata);
void NandManger_regPtInstallFn(int handle, int data);
int NandManger_ptInstall(int handle, char *ptname);
int NandManger_Ioctrl(int handle, enum nandmanager_cmd cmd, int args);

/*---------- direct interface ---------------*/
/**
 * NandManger_getDirectPartition, get ppartition array
 **/
PPartArray* NandManger_getDirectPartition(int handle);

/**
 * NandManger_DirectRead, read data directory
 **/
int NandManger_DirectRead(int handle, PPartition *pt, int pageid, int off_t, int bytes, void *data);

/**
 * NandManger_DirectWrite, write data directory
 **/
int NandManger_DirectWrite(int handle, PPartition *pt, int pageid, int off_t, int bytes, void *data);

/**
 * NandManger_DirectWrite, direct erase blocks
 **/
int NandManger_DirectErase(int handle, PPartition *pt, BlockList* bl);

/**
 * NandManger_DirectIsBadBlock, direct show if block is bad block
 **/
int NandManger_DirectIsBadBlock(int handle, PPartition *pt, int blockid);

/**
 * NandManger_DirectMarkBadBlock, direct mark bad block
 **/
int NandManger_DirectMarkBadBlock(int handle, PPartition *pt, int blockid);
#endif
