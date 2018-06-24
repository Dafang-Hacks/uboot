#ifndef _NAND_ERRNO_H_
#define _NAND_ERRNO_H_

#include "pagelist.h"

#define BLOCK_MOVE  	ND_ECC_TOOLARGE
#define SUCCESS         0
#define ENAND		ND_ERROR_ARGS
#define DMA_AR  	ND_ERROR_MEMORY
#define IO_ERROR      	ND_ERROR_IO
#define TIMEOUT       	ND_TIMEOUT
#define ECC_ERROR     	ND_ERROR_ECC
#define ALL_FF	 	ND_ERROR_NOWRITE
#define WRITE_PROTECT	-7

#endif /*_NAND_ERRNO_H_*/
