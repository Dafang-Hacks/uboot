#ifndef __VNANDINFO_H__
#define __VNANDINFO_H__

#include "nandinterface.h"
#include "ppartition.h"

#define SECTOR_SIZE	512
#define L4INFOLEN_2K	1136
#define L4INFOLEN_512	372

/**
 * here pagesize can use physical
 * pagesize or virtual pagesize
 **/
#define L4INFOLEN(pagesize)		((pagesize) >= 2048 ? L4INFOLEN_2K : L4INFOLEN_512)
#define VNANDCACHESIZE(pagesize)	(L4INFOLEN(pagesize) / sizeof(unsigned int) * SECTOR_SIZE)

typedef struct _VNandInfo VNandInfo;
typedef struct _VNandManager VNandManager;
struct _VNandInfo {
	int startBlockID;
	int GroupPerZone;
	int PagePerGroup;
	int PagePerBlock;
	int BytePerPage;
	int TotalBlocks;
	int* prData;
	int MaxBadBlockCount;
	unsigned short hwSector;
	int mode;
	unsigned int *pt_availableblockid;
	struct virt2phy_page *v2pp;
#ifdef STATISTICS_DEBUG
	TimeByte *timebyte;
#endif
};

struct _VNandManager {
	VNandInfo info;
	PPartArray* pt;
};

#define CONV_PT_VN(pt,vn)						\
	do{								\
		(vn)->startBlockID = 0;					\
		(vn)->GroupPerZone = (pt)->groupperzone;		\
		if (pt->mode == ZONE_MANAGER) {				\
			(vn)->PagePerGroup = (pt)->pagespergroup * (pt)->v2pp->vpp; \
			(vn)->PagePerBlock = (pt)->pageperblock * (pt)->v2pp->vpp; \
			(vn)->BytePerPage = ((pt)->byteperpage >= 2048) ? 2048 : 512; \
			(vn)->TotalBlocks = (pt)->totalblocks - (pt)->badblockcount; \
		} else {						\
			(vn)->PagePerGroup = (pt)->pagespergroup;	\
			(vn)->PagePerBlock = (pt)->pageperblock;	\
			(vn)->BytePerPage = (pt)->byteperpage;		\
			(vn)->TotalBlocks = (pt)->totalblocks;		\
		}							\
		(vn)->MaxBadBlockCount = (vn)->TotalBlocks >> 1;	\
		(vn)->hwSector = (pt)->hwsector;			\
		(vn)->prData = (void*)(pt);				\
		(vn)->pt_availableblockid = (pt)->pt_availableblockid;	\
		(vn)->v2pp = (pt)->v2pp;				\
		(vn)->mode = (pt)->mode;				\
	} while(0)

#endif
