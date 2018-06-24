#ifndef __NAND_BASIC_H__
#define __NAND_BASIC_H__


#define NAND_PARAMS_LEN     256

#ifdef CONFIG_NAND_NFI
#define SPL_SIZE    (32*1024)
#else
#define SPL_SIZE    (16*1024)
#endif //endif CONFIG_NAND_NFI 
#define NAND_PARAMS_LEN     256
#define SPL_BACKUP_NUM      8

#define SPL_ECC_SIZE  256
#define SPL_BCH_BIT   64

#if defined(CONFIG_JZ4775) || defined(CONFIG_M150)

#define BUSWIDTH_FLAG_OFFSET    0               /* [0 : 63] */
#define NANDTYPE_FLAG_OFFSET    (BUSWIDTH_FLAG_OFFSET + 64) /* [64 : 127] */
#define ROWCYCLE_FLAG_OFFSET    (NANDTYPE_FLAG_OFFSET + 64) /* [128 : 159] */
#define PAGESIZE_FLAG2_OFFSET   (ROWCYCLE_FLAG_OFFSET + 32) /* [160 : 191] */
#define PAGESIZE_FLAG1_OFFSET   (ROWCYCLE_FLAG_OFFSET + 32 * 2) /* [192 : 223] */
#define PAGESIZE_FLAG0_OFFSET   (ROWCYCLE_FLAG_OFFSET + 32 * 3) /* [224 : 255] */

#endif //endif CONFIG_JZ4775

#ifdef CONFIG_JZ4780

#define NANDTYPE_FLAG_OFFSET    0               /* [0 : 63] */
#define ROWCYCLE_FLAG_OFFSET    (NANDTYPE_FLAG_OFFSET + 64) /* [64 : 95] */
#define PAGESIZE_FLAG2_OFFSET   (ROWCYCLE_FLAG_OFFSET + 32) /* [96 : 127] */
#define PAGESIZE_FLAG1_OFFSET   (ROWCYCLE_FLAG_OFFSET + 32 * 2) /* [128 : 159] */
#define PAGESIZE_FLAG0_OFFSET   (ROWCYCLE_FLAG_OFFSET + 32 * 3) /* [160 : 191] */

#endif //endif CONFIG_JZ4780

#if defined(CONFIG_M200) || defined(CONFIG_T15) || defined(CONFIG_T10)

#define BUSWIDTH_FLAG_OFFSET    0               /* [0 : 63] */
#define NANDTYPE_FLAG_OFFSET    (BUSWIDTH_FLAG_OFFSET + 64) /* [64 : 127] */
#define NAND_OTHER_FLAG_OFFSET  (NANDTYPE_FLAG_OFFSET + 64) /* [128 : 255] */

#endif //endif CONFIG_M200
#define FLAG_BUSWIDTH_8BIT  0x55
#define FLAG_BUSWIDTH_16BIT 0xaa
#define FLAG_NANDTYPE_COMMON    0x55
#define FLAG_NANDTYPE_TOGGLE    0xaa
#define FLAG_ROWCYCLE_2     0x55
#define FLAG_ROWCYCLE_3     0xaa
#define FLAG_PAGESIZE_512   0x555555
#define FLAG_PAGESIZE_2K    0x55aa55
#define FLAG_PAGESIZE_4K    0xaa5555
#define FLAG_PAGESIZE_8K    0xaaaa55
#define FLAG_PAGESIZE_16K   0xaaaaaa


struct spl_basic_param{
	unsigned char buswidth;
	unsigned char nandtype;
	unsigned char rowcycles;
	unsigned int pagesize;
};

/* the maxsize of struct nand_otherflag is 32bytes */
struct nand_otherflag{
	unsigned int splflag;   /* the string is "SPL!" = 0x21 4c 50 53 */
	unsigned int rowcycle;
	unsigned int pagesize;
	unsigned int space[20];
};


#endif //__NAND_BASIC_H__
