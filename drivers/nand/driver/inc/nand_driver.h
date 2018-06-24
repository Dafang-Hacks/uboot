#include <nand_chip.h>
#include <os_config.h>

#ifdef CONFIG_SOC_4780
#define CS_PER_NFI		4
#define NFI_MAX_RATE_LIMIT	(250 * 1000 * 1000)
#elif defined(CONFIG_SOC_4775)|| defined(CONFIG_SOC_M150)
#define CS_PER_NFI		2
#define NFI_MAX_RATE_LIMIT	(250 * 1000 * 1000)
#elif defined(CONFIG_SOC_M200) || defined(CONFIG_SOC_T15) || defined(CONFIG_SOC_T10)
#define CS_PER_NFI	    4
#define NFI_MAX_RATE_LIMIT	(250 * 1000 * 1000)
#endif

#define BCH_USE_NEMC_RATE

/* nand manager mode */
#define SPL_MANAGER  	0
#define DIRECT_MANAGER 	1
#define ZONE_MANAGER   	2
#define ONCE_MANAGER  	3

/* pt extend flags */
#define PT_USE_CACHE	0x01

/* CPU mode or DMA mode */
#define CPU_OPS		0x1
#define DMA_OPS		0x2

#define ONE_PLANE	1
#define TWO_PLANES	2
/* multi partitions */
#define MUL_PARTS 4

#define MAX_NAME_SIZE 32
/* driver strength, level 0 is weakest */
#define DRV_STRENGTH_DEFAULT	0
#define DRV_STRENGTH_LEVEL0	1
#define DRV_STRENGTH_LEVEL1	2
#define DRV_STRENGTH_LEVEL2	3
#define DRV_STRENGTH_LEVEL3	4

/* rb pulldown strength, level 0 is weakest */
#define RB_PULLDOWN_STRENGTH_DEFAULT	0
#define RB_PULLDOWN_STRENGTH_LEVEL0	1
#define RB_PULLDOWN_STRENGTH_LEVEL1	2
#define RB_PULLDOWN_STRENGTH_LEVEL2	3
#define RB_PULLDOWN_STRENGTH_LEVEL3	4

#define NAND_SHARING_PARMS_ADDR 0xB3425800
//#define NAND_SHARING_PARMS_ADDR 0xf4000800


#define PARTITION_RESERVE_BIT	(1 << 0)

#define PARTITION_NEED_RESERVE(n)	(n & PARTITION_RESERVE_BIT)


/* ####################################### */
/**
 * this is only used for kernel to get nand basic params
 **/
extern nand_sharing_params share_parms;

/**
 * nemc_base: nfi features for a specified product
 **/
typedef struct __nfi_base {
	void *gate;
	unsigned long rate;
	unsigned long cycle;
	unsigned int irq;
	void *iomem;
	void *cs_iomem[CS_PER_NFI];
	int (*readl)(int reg);
	void (*writel)(int reg, int val);
	int (*clk_enable)(void);
	void (*clk_disable)(void);

} nfi_base;

/**
 * bch_base: bch features for a specified product
 **/
typedef struct __bch_base {
	void *gate;
	void *clk;
	unsigned int irq;
	void *iomem;
	int (*readl)(int reg);
	void (*writel)(int reg, int val);
	int (*clk_enable)(void);
	void (*clk_disable)(void);
} bch_base;

/**
 * mcu_base: mcu features
 **/
typedef struct __pdma_base {
	void *iomem;
	unsigned int dma_channel;
} pdma_base;

/**
 * io_base: the collction of devices
 * features of a specified product
 *
 * @nfi: nfi_base
 * @bch: bch_base
 * @pdma: pdma_base
 **/
typedef struct __io_base {
	nfi_base nfi;
	bch_base bch;
	pdma_base pdma;
} io_base;

/* ############### ptinfo ################# */
/**
 * struct platform_nand_ex_partition
 * an element in platform_nand_partition
 * the member is as same as its
 */
typedef struct __plat_ex_partition {
	char name[MAX_NAME_SIZE];
	unsigned long long offset;
	unsigned long long size;
} plat_ex_partition;

/**
 * struct __plat_ndpartition:
 *
 * @name: the name of this partition
 * @offset: offset within the master MTD space
 * @size: partition size
 * @ops_mode: DMA_OPS || CPU_OPS
 * @nm_mode: partition manager mode, SPL_MANAGER || DIRECT_MANAGER || ZONE_MANAGER
 * @flags:
 **/
typedef struct __plat_ptitem {
	char name[MAX_NAME_SIZE];
	unsigned long long offset;
	unsigned long long size;
	unsigned char ops_mode;
	unsigned char nm_mode;
	unsigned int flags;
	unsigned int *pt_badblock_info;
	unsigned int attribute;
	plat_ex_partition ex_partition[MUL_PARTS];
} plat_ptitem;

typedef struct __plat_ptinfo {
	unsigned short ptcount;
	plat_ptitem *pt_table;
} plat_ptinfo;

/* ############### rbinfo ################# */
/**
 * struct __rb_item, rb gpio or irq info used for driver
 **/
typedef struct __rb_item {
	unsigned short id;
	unsigned short gpio;
	unsigned short irq;
	unsigned short pulldown_strength;
	void *irq_private;
} rb_item;

typedef struct __rb_info {
	unsigned short totalrbs;
	rb_item *rbinfo_table;
} rb_info;

/* ####################################### */
typedef struct __os_clib {
	void (*ndelay) (unsigned long nsecs);
	int (*div_s64_32)(long long dividend, int divisor);
	void* (*continue_alloc)(unsigned int size);
	void (*continue_free)(void *addr);
	int (*printf)(const char *fmt, ...);
	void* (*memcpy)(void *dst, const void *src, unsigned int count);
	void* (*memset)(void *s, int c, unsigned int count);
	int (*strcmp)(const char *cs, const char *ct);
	unsigned int (*get_vaddr)(unsigned int paddr);
	void (*dma_cache_wback)(unsigned long addr, unsigned long size);
	void (*dma_cache_inv)(unsigned long addr, unsigned long size);
	unsigned long long (*get_time_nsecs)(void);
} os_clib;

/**
 * struct nand_api_platdependent:
 *
 * @nandflash:
 * @rbinfo:
 * @platptinfo:
 * @gpio_wp:
 * @drv_strength:
 * @erasemode: there are four modes. 0: none, 1: normal-erase, 2, force-erase, 3: factory-erase
 * @update_errpt:
 **/
struct nand_api_platdependent {
	nand_flash *nandflash;
	rb_info *rbinfo;
	plat_ptinfo *platptinfo;
	unsigned short gpio_wp;
	unsigned char drv_strength;
	unsigned char erasemode;
};

/**
 * struct nand_api_osdependent:
 *
 * @base:
 * @clib:
 * @wp_enable:
 * @wp_disable:
 * @clear_rb_state:
 * @wait_rb_timeout:
 * @try_wait_rb:
 * @gpio_irq_request:
 * @ndd_gpio_request:
 **/
struct nand_api_osdependent {
	io_base *base;
	os_clib clib;
	void (*wp_enable) (int);
	void (*wp_disable) (int);
	void (*clear_rb_state)(rb_item *);
	int (*wait_rb_timeout) (rb_item *, int);
	int (*try_wait_rb) (rb_item *, int);
	int (*gpio_irq_request)(unsigned short gpio, unsigned short *irq, void **irq_private);
	int (*ndd_gpio_request)(unsigned int gpio, const char *lable);
};
