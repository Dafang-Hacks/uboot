#ifndef __LPARTITION_H__
#define __LPARTITION_H__

#define LPARTITION(OBJ) ((LPartition*)OBJ)

#ifndef String
#define String char*
#endif

#include "singlelist.h"
typedef struct _LPartition LPartition;
#define MUL_PARTS 4
typedef struct _lmul_parts lmul_parts;
struct _lmul_parts{
    int startSector;
    int sectorCount;
    char *name;
};
struct _LPartition {
	struct singlelist head;
    int startSector;
    int sectorCount;
    const char* name;
    int mode;
    int pc; /*partcontext*/

    int hwsector;
    unsigned int segmentsize;
    lmul_parts lparts[MUL_PARTS];
    int parts_num;
};

#endif
