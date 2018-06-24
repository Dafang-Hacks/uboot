/*
 * JZ4780 RTC register definition
 * Copyright (C) 2010 Ingenic Semiconductor Co., Ltd.
 *
 * Author: cjwang@ingenic.cn
 *	   cwwang@ingenic.cn
 *
 */
#ifndef __MACH_JZRTC_H__
#define __MACH_JZRTC_H__

#define DEBUG 1
/*
 * RTC registers offset address definition
 */
#define BIT(nr)			(1UL << (nr))
#define RTC_RTCCR		(0x00)	/* rw, 32, 0x00000081 */
#define RTC_RTCSR		(0x04)	/* rw, 32, 0x???????? */
#define RTC_RTCSAR		(0x08)	/* rw, 32, 0x???????? */
#define RTC_RTCGR		(0x0c)	/* rw, 32, 0x0??????? */

#define RTC_HCR			(0x20)  /* rw, 32, 0x00000000 */
#define RTC_HWFCR		(0x24)  /* rw, 32, 0x0000???0 */
#define RTC_HRCR		(0x28)  /* rw, 32, 0x00000??0 */
#define RTC_HWCR		(0x2c)  /* rw, 32, 0x00000008 */
#define RTC_HWRSR		(0x30)  /* rw, 32, 0x00000000 */
#define RTC_HSPR		(0x34)  /* rw, 32, 0x???????? */
#define RTC_WENR		(0x3c)  /* rw, 32, 0x00000000 */
#define RTC_CKPCR		(0x40)  /* rw, 32, 0x00000010 */
#define RTC_OWIPCR		(0x44)  /* rw, 32, 0x00000010 */
#define RTC_PWRONCR		(0x48)  /* rw, 32, 0x???????? */

/*
 * RTC registers common define
 */

/* RTC control register(RTCCR) */
#define RTCCR_WRDY		BIT(7)
#define RTCCR_1HZ		BIT(6)
#define RTCCR_1HZIE		BIT(5)
#define RTCCR_AF		BIT(4)
#define RTCCR_AIE		BIT(3)
#define RTCCR_AE		BIT(2)
#define RTCCR_SELEXC		BIT(1)
#define RTCCR_RTCE		BIT(0)


/* Generate the bit field mask from msb to lsb */
#define BITS_H2L(msb, lsb)  ((0xFFFFFFFF >> (32-((msb)-(lsb)+1))) << (lsb))

/* RTC regulator register(RTCGR) */
#define RTCGR_LOCK		BIT(31)

#define RTCGR_ADJC_LSB		16
#define RTCGR_ADJC_MASK		BITS_H2L(25, RTCGR_ADJC_LSB)

#define RTCGR_NC1HZ_LSB		0
#define RTCGR_NC1HZ_MASK	BITS_H2L(15, RTCGR_NC1HZ_LSB)

/* Hibernate control register(HCR) */
#define HCR_PD			BIT(0)

/* Hibernate wakeup filter counter register(HWFCR) */
#define HWFCR_LSB		5
#define HWFCR_MASK		BITS_H2L(15, HWFCR_LSB)
#define HWFCR_WAIT_TIME(ms)	(((ms) << HWFCR_LSB) > HWFCR_MASK ? HWFCR_MASK : ((ms) << HWFCR_LSB))

/* Hibernate reset counter register(HRCR) */
#define HRCR_LSB		5
#define HRCR_MASK		BITS_H2L(11, HRCR_LSB)
#define HRCR_WAIT_TIME(ms)     (((ms) << HRCR_LSB) > HRCR_MASK ? HRCR_MASK : ((ms) << HRCR_LSB))


/* Hibernate wakeup control register(HWCR) */

/* Power detect default value; this value means didable, any other values means enable */
#define EPDET_DEFAULT           (0x1a55a5a5)
#define HWCR_EPDET		BIT(3)
#define HWCR_WKUPVL		BIT(2)
#define HWCR_EALM		BIT(0)


/* Hibernate wakeup status register(HWRSR) */
#define HWRSR_APD		BIT(8)
#define HWRSR_HR		BIT(5)
#define HWRSR_PPR		BIT(4)
#define HWRSR_PIN		BIT(1)
#define HWRSR_ALM		BIT(0)

/* write enable pattern register(WENR) */
#define WENR_WEN		BIT(31)

#define WENR_WENPAT_LSB		0
#define WENR_WENPAT_MASK	BITS_H2L(15, WENR_WENPAT_LSB)
#define WENR_WENPAT_WRITABLE	(0xa55a)

/* Hibernate scratch pattern register(HSPR) */
#define HSPR_RTCV               0x52544356      /* The value is 'RTCV', means rtc is valid */ 

/*power on initial procedure control bit(PWRONCR)*/
#define PWRON_EN                BIT(0)

#define ENABLE_CLK32K 0x00000006
#define DISABLE_CLK32K 0x00000010 

/* The divider is decided by the RTC clock frequency. */
#define RTC_FREQ_DIVIDER	(32768 - 1)

#define ms2clycle(x)  (((x) * RTC_FREQ_DIVIDER) / 1000)





#endif /* __MACH_JZRTC_H__ */
