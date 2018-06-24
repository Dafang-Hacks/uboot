#ifndef __NAND_INFO_H__
#define __NAND_INFO_H__

#include <nand_chip.h>
#include <nand_api.h>
#include <nand_ops_timing.h>

/* chip_info->options */
#define SUPPROT_CACHE_READ(cinfo)	((cinfo)->options & NAND_CACHE_READ)
#define SUPPROT_CACHE_PROGRAM(cinfo)	((cinfo)->options & NAND_CACHE_PROGRAM)
#define SUPPROT_MULTI_READ(cinfo)	((cinfo)->options & NAND_MULTI_READ)
#define SUPPROT_MULTI_PROGRAM(cinfo)	((cinfo)->options & NAND_MULTI_PROGRAM)
#define SUPPROT_TIMING_MODE(cinfo)	((cinfo)->options & NAND_TIMING_MODE)
#define SUPPROT_DRIVER_STRENGTH(cinfo)	((cinfo)->options & NAND_DRIVER_STRENGTH)
#define SUPPROT_RB_PULL_DOWN_STRENGTH(cinfo)	((cinfo)->options & NAND_RB_PULL_DOWN_STRENGTH)
#define SUPPROT_READ_RETRY(cinfo)	((cinfo)->options & NAND_READ_RETRY)
#define READ_RETRY_MODE(cinfo)		(((cinfo)->options >> 16) & 0x0f)
#define TIMING_MODE(cinfo)		(((cinfo)->options >> 20) & 0x0f)
#define GET_NAND_TYPE(cinfo)	(((cinfo)->options >> 24) & 0x0f)

#define RETRY_DATA_SIZE		64

/**
 * struct __retry_parms
 **/
typedef struct __retry_parms {
	unsigned char mode;			/* mode */
	unsigned char cycle;			/* cycle */
	unsigned char regcnt;			/* regs count */
	unsigned int data[(RETRY_DATA_SIZE + 3) / 4];	/* retry level parm table */
} retry_parms;

/**
 * struct __nand_info - NAND Flash info privide to NandDriver
 **/
typedef struct __chip_info {
	unsigned int manuf;		/* manufacturers */
	unsigned int pagesize;		/* page size */
	unsigned int oobsize;		/* oob area size */
	unsigned int ppblock;		/* pages per block */
	unsigned int maxvalidblocks;	/* valid blocks per chip */
	unsigned int maxvalidpages;	/* valid pages per chip */
	unsigned char planepdie;	/* planes per die */
	unsigned char totaldies;	/* die per chip */
	unsigned short origbadblkpos;	/* original bad block position*/
	unsigned short badblkpos;	/* bad block position redifined */
	unsigned char eccpos;		/* parity position redifined */
	unsigned char buswidth;		/* chip bus width */
	unsigned char rowcycles;	/* rowcycles */
        unsigned char eccbit;		/* chip eccbit */
	unsigned char planenum;		/* chip plane number */
	unsigned char planeoffset;	/* multi-plane block address offset */
	unsigned int options;		/* options */
	retry_parms *retryparms;	/* retry parm */
	nand_ops_timing ops_timing;	/* chip timing */
	unsigned short drv_strength;	/* driver strength */
	void *flash;
} chip_info;

rb_item* get_rbitem(nfi_base *base, unsigned int cs_id, rb_info *rbinfo);
int early_nand_prepare(nfi_base *base, unsigned int cs_id);
int get_retry_parms(nfi_base *base,chip_info *cinfo, unsigned int cs_id, rb_info *rbinfo, retry_parms *retryparms);
int nand_set_features(nfi_base *base, unsigned int cs_id, rb_item *rbitem, chip_info *cinfo);
int set_retry_feature(int ndata, unsigned int cs_id, int cycle);
int get_nand_id(nfi_base *base, nand_flash_id *id, unsigned int cs_id);

#endif /*__NAND_INFO_H__*/
