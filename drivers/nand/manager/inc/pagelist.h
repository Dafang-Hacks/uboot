#ifndef __PAGELIST_H__
#define __PAGELIST_H__

#include "singlelist.h"

typedef struct _PageList PageList;

struct _PageList {
	struct singlelist head;//must be the first member of the struct
	unsigned int startPageID;
	unsigned int _startPageID;
	unsigned short OffsetBytes;
	unsigned short Bytes;
	void* pData;
	int retVal;
};
#define ND_ECC_TOOLARGE   1

#define ND_ERROR_ARGS    -1
#define ND_ERROR_MEMORY  -2
#define ND_ERROR_IO      -3
#define ND_TIMEOUT       -4
#define ND_ERROR_ECC     -5
#define ND_ERROR_NOWRITE -6

#define ISERROR(x) ((x) < 0)
#define ISARGSERROR(x) ((x) == ND_ERROR_ARGS)
#define ISMEMORYERROR(x) ((x) == ND_ERROR_MEMORY)
#define ISIOERROR(x) ((x) == ND_ERROR_IO)
#define ISTIMEOUT(x) ((x) == ND_TIMEOUT)
#define ISECCERROR(x) ((x) == ND_ERROR_ECC)
#define ISNOWRITE(x) ((x) == ND_ERROR_NOWRITE)

#define ECCTOOLARGE(x) ((x) == ND_ECC_TOOLARGE)
#define ISDATAMOVE(x) ECCTOOLARGE(((x) >> 16))

#define DATALEN(x) ((x) & 0xffff)

#define NDSTRERROR(x) ({						\
	char *str = "no error";						\
	char *nanddriver_error_string[] = {"parameter error","memory error", \
					   "io error","timeout",	\
					   "ecc error","no writed"};	\
	if((x) < 0 && (x) >= ND_ERROR_NOWRITE)				\
		str = nanddriver_error_string[-(x)];			\
	str;})


#endif
