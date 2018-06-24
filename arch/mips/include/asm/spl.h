#ifndef	_ASM_SPL_H_
#define	_ASM_SPL_H_

#include <config.h>

/* Platform-specific defines */
//#include <asm/arch/spl.h>

#define BOOT_DEVICE_MMC1	0
#define BOOT_DEVICE_MMC2	1
#define BOOT_DEVICE_MMC2_2	2
#define BOOT_DEVICE_NAND	3
#define BOOT_DEVICE_SPI		4
#define BOOT_DEVICE_NOR		5
#define BOOT_DEVICE_RAM		6
#define BOOT_DEVICE_SFC_NOR	7
#define BOOT_DEVICE_SFC_NAND	8

extern char __bss_start[];
extern ulong __bss_end;

static inline u32 spl_boot_device(void)
{
#ifdef CONFIG_SPL_RAM_DEVICE
	return BOOT_DEVICE_RAM;
#endif
#if defined(CONFIG_SPL_NAND_SUPPORT) || defined(CONFIG_JZ_NAND_MGR)
	return BOOT_DEVICE_NAND;
#endif
#ifdef CONFIG_SPL_MMC_SUPPORT
	return BOOT_DEVICE_MMC1;
#endif
#ifdef CONFIG_SPL_SPI_SUPPORT
	return BOOT_DEVICE_SPI;
#endif
#ifdef CONFIG_SPL_SFC_SUPPORT
#ifdef CONFIG_SFC_NOR
	return BOOT_DEVICE_SFC_NOR;
#endif
#ifdef CONFIG_SFC_NAND
	return BOOT_DEVICE_SFC_NAND;
#endif
#endif
#ifdef CONFIG_SPL_NOR_SUPPORT
	return BOOT_DEVICE_NOR;
#endif
}

static inline u32 spl_boot_mode(void)
{
#ifdef CONFIG_SPL_FAT_SUPPORT
	return 2; /* MMCSD_MODE_FAT */
#else
	return 1; /* MMCSD_MODE_RAW */
#endif
}

#endif
