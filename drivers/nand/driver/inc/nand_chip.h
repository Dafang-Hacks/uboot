#ifndef __NAND_CHIP_H__
#define __NAND_CHIP_H__

#include <nfi_nand_timing.h>
#include <emc_nand_timing.h>

/* NAND Flash Manufacturer ID Codes */
#define NAND_MFR_TOSHIBA	0x98	// Toshiba
#define NAND_MFR_SAMSUNG	0xec	// Samsung
#define NAND_MFR_FUJITSU	0x04	// Fujitsu
#define NAND_MFR_NATIONAL	0x8f	// National
#define NAND_MFR_RENESAS	0x07	// Renesas
#define NAND_MFR_STMICRO	0x20	// ST Micro
#define NAND_MFR_HYNIX		0xad	// Hynix
#define NAND_MFR_MICRON		0x2c	// Micron
#define NAND_MFR_AMD		0x01	// AMD/Spansion
#define NAND_MFR_MACRONIX	0xc2	// Macronix
#define NAND_MFR_EON		0x92	// Eon

/**
 * nand_flash->options bitmap,
 * low 16bit is support switch,
 * hight 16bit is value of support.
 **/
#define NAND_CACHE_READ         (1 << 0)	// support cache read operation
#define NAND_CACHE_PROGRAM      (1 << 1)	// support page cache program operation
#define NAND_MULTI_READ         (1 << 2)	// support multi-plane page read operation
#define NAND_MULTI_PROGRAM      (1 << 3)	// support multi-plane page program operation
#define NAND_TIMING_MODE	(1 << 4)	// support select timing mode
#define NAND_DRIVER_STRENGTH    (1 << 5)	// support select driver stength
#define NAND_RB_PULL_DOWN_STRENGTH    (1 << 6)	// support select rb pull_down strength
#define NAND_READ_RETRY	        (1 << 8)	// support read retry
#define NAND_MICRON_NORMAL      (1 << 9)	// the command of two-planes read is 00-32-00-30
#define NAND_MICRON_PARTICULAR  (1 << 10)	// the command of two-planes read is 00-00-30
#define NAND_TYPE(n)		((n & 0x03) << 14)	// common nand or toggle nand..., bit (14) ~bit (15)
#define NAND_READ_RETRY_MODE(n)	((n & 0x0f) << 16)	// read retry mode (bit(16) ~ bit(19): 0 <= (n) <= 16)
#define NAND_TIMING_MODE_V(n)	((n & 0x0f) << 20)	// timing mode (bit(20) ~ bit(23): 0 <= (n) <= 16)

enum hynix_retry_mode {
	HY_RR_F26_32G_MLC,
	HY_RR_F20_64G_MLC_A,
	HY_RR_F20_64G_MLC_B,
	HY_RR_F20_32G_MLC_C,
	HY_RR_F1Y_64G_MLC,
};

enum Micron_retry_mode{
	MT_RR_29F_32G_MLC_ADA,
};

enum micron_timing_mode {
	MR_TIMING_MODE0 = 0x00,
	MR_TIMING_MODE1 = 0x01,
	MR_TIMING_MODE2 = 0x02,
	MR_TIMING_MODE3 = 0x03,
	MR_TIMING_MODE4 = 0x04,
	MR_TIMING_MODE5 = 0x05,
};

enum micron_driver_strength {
	MR_DRIVER_STRENGTH_OVER2 = 0x00,
	MR_DRIVER_STRENGTH_OVER1 = 0x01,
	MR_DRIVER_STRENGTH_NORMAL = 0x02,
	MR_DRIVER_STRENGTH_UNDER = 0x03,
};

enum nand_type {
	NAND_TYPE_COMMON = 0x00,
	NAND_TYPE_TOGGLE = 0x01,
	NAND_TYPE_ONFI = 0x02,
};

/**
 * struct __nand_chip_id
 **/
typedef struct __nand_flash_id {
	unsigned short id;
	unsigned int extid;
} nand_flash_id;

#define is_id_null(fid) (((fid)->id == 0) && ((fid)->extid == 0))

#define copy_id(dst_fid, src_fid)			\
	do {						\
		(dst_fid)->id = (src_fid)->id;		\
		(dst_fid)->extid = (src_fid)->extid;	\
	} while (0)					\

#define cmp_id(fid1, fid2) (!(((fid1)->id == (fid2)->id) && ((fid1)->extid == (fid2)->extid)))

typedef struct __optionalcmd {
    unsigned char multiplaneread[2];		// the sequence is [0] -ADDR- [0] -ADDR- [1] - DATA
    unsigned char multiplanewrite[2];		// the sequence is 80 -ADDR- DATA - [0] - [1] -ADDR- DATA - 10/15
    unsigned char multiplanecopyread[3];	// the sequence is [0] -ADDR- [1] -ADDR- [2]
    unsigned char multiplanecopywrite[3];	// the sequence is [0] -ADDR- [1] - [2] -ADDR- 10
    unsigned char multiplanestatus;		// the command may be 0x70/0x71/0x78/...
    unsigned char interbnk0status;		// the command may be 0xf1/0x78/...
    unsigned char interbnk1status;		// the command may be 0xf2/0x78/...
} optionalcmd;

typedef struct __nand_timing_com {
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

} nand_timing_com;

typedef struct _nand_timing {
	    nand_timing_com timing;
} nand_timing;


typedef union __nand_extra_timing {
	nfi_toggle_timing nfi_toggle;
	nfi_onfi_timing   nfi_onfi;
	emc_toggle_timing emc_toggle;
} nand_extra_timing;

/**
 * struct __nand_flash - NAND Flash Device attr Structure
 **/
typedef struct __nand_flash {
        char name[32];
        unsigned int id;
        unsigned int extid;
        unsigned int pagesize;
	unsigned int blocksize;
        unsigned int oobsize;
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
	unsigned char planeoffset; //multi-plane block address offset
        unsigned int options;
	nand_timing timing;
	nand_extra_timing nand_extra;
	//optionalcmd *optcmd;
} nand_flash;

struct nand_basic_info {
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
};

typedef struct __nand_sharing_params {
	unsigned int magic;
	struct nand_basic_info nandinfo;
	unsigned int kernel_offset;
	unsigned int nand_manager_version;
} nand_sharing_params;
const nand_flash *get_nand_flash(nand_flash_id *fid);

#endif /*__NAND_CHIP_H__*/
