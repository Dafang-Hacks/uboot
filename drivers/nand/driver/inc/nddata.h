#ifndef __NAND_DATA_H__
#define __NAND_DATA_H__

#include "nand_info.h"
#include "nand_api.h"
#include "ppartition.h"

/* ############### ptinfo ################# */
#define IS_RAW_PT(npt)	(((npt)->nm_mode == SPL_MANAGER) || ((npt)->nm_mode == DIRECT_MANAGER))
#define IS_ZONE_PT(npt)	((npt)->nm_mode == ZONE_MANAGER)
#define IS_ERR_PT(npt)	((npt)->nm_mode == ONCE_MANAGER)
/**
 * struct __ndpartition, partition used for driver
 **/
typedef struct __ndpartition {
	const char *name;
	unsigned int pagesize;		/* physical pagesize */
	unsigned int pagepblock;	/* physical pages per block */
	unsigned int startblockid;	/* physical start block id */
	unsigned int totalblocks;	/* physical total blocks */
	unsigned char blockpvblock;	/* physical blocks per virtual block */
	unsigned char vblockpgroup;	/* virtual blocks per group */
	unsigned char groupspzone;	/* groups per zone */
	unsigned char eccbit;
	unsigned char planes;
	unsigned char ops_mode;
	unsigned char nm_mode;
	unsigned char copy_mode;
	unsigned char ndparts_num;
	mul_parts ndparts[MUL_PARTS];
	unsigned int *pt_badblock_info;
	unsigned int flags;
	int handler;
} ndpartition;

typedef struct __pt_info {
	unsigned short ptcount;
	unsigned int raw_boundary;
	ndpartition *pt;
} pt_info;

/* ############### csinfo ################# */
/**
 * struct __cs_item, cs rbinfo used for driver
 **/
typedef struct __cs_item {
	unsigned short id;
	rb_item *rbitem;
	void *iomem;
} cs_item;

typedef struct __cs_info {
	unsigned short totalchips;
	cs_item *csinfo_table;
} cs_info;

/* ############### nddata ################# */
/**
 * nand_data: the global struct contain
 * the data used for the whole driver
 *
 * @gpio_wp: write protect gpio
 * @eccsize: data len the bch process each time
 * @spl_eccsize: data len of spl that the bch
 * 	process each time
 * @pt: ndpartition array
 * @ptcount: ndpartition count
 * @base: devices hardware feacher
 * @cinfo: nand chip info
 * @totalchips: total chips
 * @ioinfo: io info
 * @iocount: io count
 * @nemc_cnt: soc nemc count
 * @cspnemc: soc cs count of nemc
 * @bch_cnt: soc bch count
 * @pdma_cnt: soc pdma count
 * @ops_context: the context of nand_ops
 **/
typedef struct __nand_data {
	unsigned short eccsize;
	unsigned short spl_eccsize;
	chip_info *cinfo;
	io_base *base;
	rb_info *rbinfo;
	cs_info *csinfo;
	pt_info *ptinfo;
	int ops_context;
	int (*clear_rb) (int cs);
	int (*wait_rb) (int cs, int time);
	unsigned short gpio_wp;
	struct nand_api_platdependent *platdep;
} nand_data;

#endif /*__NAND_DATA_H__*/
