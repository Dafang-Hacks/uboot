#ifndef __NANDDRIVER_H__
#define __NANDDRIVER_H__

#include "pagelist.h"
#include "blocklist.h"

enum ndd_cmd {
	NDD_UPDATE_ERRPT = 48,
};

typedef struct _NandInterface NandInterface;

struct _NandInterface {
	int (*iPageRead)(void *ppartition,int pageid, int offsetbyte, int bytecount, void * data );
	int (*iPageWrite)(void *ppartition,int pageid, int offsetbyte, int bytecount, void* data );
	int (*iPanicPageWrite)(void *ppartition,int pageid, int offsetbyte, int bytecount, void* data );
	int (*iMultiPageRead)(void *ppartition,PageList* pl );
	int (*iMultiPageWrite)(void *ppartition,PageList* pl );
	int (*iMultiBlockErase)(void *ppartition,BlockList* pl );
	int (*iIsBadBlock)(void *ppartition,int blockid );
	int (*iIsInherentBadBlock)(void *ppartition,int blockid);
	int (*iMarkBadBlock)(void *ppartition,int blockid);
	int (*iIoctl)(enum ndd_cmd cmd, int args);
	int (*iInitNand)(void * vNand);
	int (*iDeInitNand)(void * vNand);
};

typedef struct __nm_version {
	unsigned char major;
	unsigned char minor;
	unsigned char revision;
} nm_version;

void Get_NandManagerVersion(nm_version *version);
void Register_NandDriver(NandInterface *ni);

#endif
