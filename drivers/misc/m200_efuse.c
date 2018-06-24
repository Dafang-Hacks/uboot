#include <common.h>
#include <malloc.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/errno.h>
#include <asm/arch/base.h>
#include <asm/arch/clk.h>
//#include <asm/arch/sc_rom.h>

//#define EFU_NO_REG_OPS

#ifdef EFU_NO_REG_OPS
#undef writel
#undef readl
#define writel(b, addr) {;}
#define readl(addr)	0
#endif

static int efuse_debug = 0;
static int efuse_gpio = -1;

#define EFUCTRL		0x00
#define EFUCFG		0x04
#define EFUSTATE	0x08
#define EFUDATA(x)	(0x0C + (x)*0x4)
#define EFUDATA_REG_NUM 8

#define EFU_ROM_BASE	0x200
#define EFU_ROM_END	0x3FF
#define EFU_CHIP_ID_BASE	0x200
#define EFU_CHIP_ID_END		0x20F
#define EFU_CHIP_NUM_BASE	0x210
#define EFU_CHIP_NUM_END	0x21F
#define EFU_COMS_ID_BASE	0x220
#define EFU_COMS_ID_END		0x22F
#define EFU_TRIM_DATA0_BASE	0x230
#define EFU_TRIM_DATA0_END	0x233
#define EFU_TRIM_DATA1_BASE	0x234
#define EFU_TRIM_DATA1_END	0x237
#define EFU_TRIM_DATA2_BASE	0x238
#define EFU_TRIM_DATA2_END	0x23B
#define EFU_TRIM_DATA3_BASE	0x23B
#define EFU_TRIM_DATA3_END	0x23D
#define EFU_PROT_BIT_BASE	0x23E
#define EFU_PROT_BIT_END	0x23F
#define EFU_ROOT_KEY_BASE	0x240
#define EFU_ROOT_KEY_END	0x24F
#define EFU_CHIP_KEY_BASE	0x250
#define EFU_CHIP_KEY_END	0x25F
#define EFU_USER_KEY_BASE	0x260
#define EFU_USER_KEY_EN		0x26F
#define EFU_MD5_BASE		0x270
#define EFU_MD5_END		0x27F
#define EFU_FIX_BT_BASE		0x280
#define EFU_FIX_BT_END		0x3FF

#define WR_ADJ_10TIME	65
#define WR_WR_STROBE_1TIME	10000
#define WR_WR_STROBE_1TIME_MAX	11000

int adjust_efuse(int is_wirte)
{
	uint32_t efucfg_reg = 0;
	int adj = 0, strobe = 0;
	int h2clk = clk_get_rate(H2CLK);
	int ret = 0;
	if (is_wirte) {
		adj = ((((WR_ADJ_10TIME*(h2clk/1000000))+10-1)/10)+1000-1)/(1000);
		adj = (adj != 0) ? (adj - 1) : adj;
		strobe = ((WR_WR_STROBE_1TIME/1000)*(h2clk/1000000));
		strobe = strobe - 1666 - adj;
		if (strobe < -2047) {
			strobe = ((WR_WR_STROBE_1TIME_MAX/1000)*(h2clk/1000000));
			strobe = strobe - 1666 - adj;
			if (strobe < -2047) {
				error("h2clk is too slow");
				ret = -EFAULT;
				goto out;
			}
		}
		if (adj > 0xf || strobe > 2047) {
			error("h2clk is too fast");
			ret = -EFAULT;
			goto out;
		}

		if (strobe < 0)
			strobe = (0x800|(strobe&0x7ff));

		efucfg_reg = (adj<<12)|(strobe<<0);
		writel(efucfg_reg,(EFUSE_BASE + EFUCFG));
	} else {
		efucfg_reg = (0xf<<20)|(0xf<<16);
		writel(efucfg_reg,(EFUSE_BASE+EFUCFG));
		efucfg_reg = readl((EFUSE_BASE+EFUCFG));
		if (((efucfg_reg>>16)&0xf) != 0x7) {
			efucfg_reg = (0xf<<20)|(0x7<<16);
			writel(efucfg_reg,(EFUSE_BASE + EFUCFG));
		}
	}
out:
	printf("h2clk is %d, efucfg_reg 0x%x\n",h2clk,readl((EFUSE_BASE+EFUCFG)));
	return ret;
}

void boost_vddq(int gpio)
{
	int val;
	printf("boost vddq\n");
	gpio_direction_output(gpio , CONFIG_EFUSE_LEVEL);
	do {
		val = gpio_get_value(gpio);
		printf("gpio %d level %d\n",gpio,val);
	} while (val);
}

void reduce_vddq(int gpio)
{
	int val;
	printf("reduce vddq\n");
	gpio_direction_output(gpio, !CONFIG_EFUSE_LEVEL);
	do {
		val = gpio_get_value(gpio);
		printf("gpio %d level %d\n",gpio,val);
	} while (!val);
}

int efuse_read_data(void *buf, int length, off_t offset)
{
	int i = 0, ret = 0;
	int len = length;
	int xlen = 0;
	int off = offset;
	char *tmp_buf = NULL;
	int32_t *ptmp_buf = NULL;
	char * pbuf =buf;
	uint32_t tmp_reg = 0;

	debug_cond(efuse_debug, "efuse read length %d from offset 0x%x\n",length,(int)offset);
	/*step 1 : Set config register*/
	ret = adjust_efuse(0);
	if (ret)
		return ret;

	if (efuse_gpio >= 0)
		reduce_vddq(efuse_gpio);

	tmp_buf = malloc((len+3/4)* sizeof(int32_t));
	if (!tmp_buf)
		return -ENOMEM;
	memset(tmp_buf , 0 , (len+3/4) * sizeof(int32_t));
	ptmp_buf = (int32_t *)tmp_buf;

	while (len > 0) {
		writel(0,(EFUSE_BASE+EFUSTATE));
		/*step 2 : Set control register to indicate what to read data address, read data numbers and read enable.*/
		xlen  = len > (EFUDATA_REG_NUM * 4) ? EFUDATA_REG_NUM * 4 : len;
		tmp_reg = (off&0x1ff)<<21| (xlen - 1) << 16|(1 << 0);
		debug_cond(efuse_debug,"EFUCTRL(0x%x):0x%x\n",(EFUSE_BASE+EFUCTRL),tmp_reg);
		writel(tmp_reg ,(EFUSE_BASE+EFUCTRL));
		off += xlen;
		len -= xlen;
		/*step 3 : Wait status register RD_DONE set to 1 or EFUSE interrupted*/
#ifndef EFU_NO_REG_OPS
		while(!(readl((EFUSE_BASE+EFUSTATE))&(1 << 0))) {
			debug_cond(efuse_debug, "EFUSTATE %x\n",readl(EFUSE_BASE+EFUSTATE));
		}
#endif
		/*step 4 : Software read EFUSE data buffer 0 ? 8 registers*/
		for (i = 0 ; i < (xlen + 3)/4 ; i++) {
			*ptmp_buf =  readl((EFUSE_BASE+EFUDATA(i)));
			debug_cond(efuse_debug,"EFUDATA[0x%x]:0x%x\n",(EFUSE_BASE+EFUDATA(i)),
					*ptmp_buf);
			ptmp_buf++;
		}
	}

	for (i = 0 ; i < length ; i++)
		pbuf[i] = tmp_buf[i];

	if (efuse_debug) {
		int i = 0;
		printf("====read data infomation====\n");
		for (i = 0; i < length; i++) {
			printf("0x%03x:0x%02x\n", i, pbuf[i]);
		}
		printf("============================\n");
	}

	return ret;
}

int efuse_read_sc_key(unsigned int offset)
{
	int start = offset + EFU_ROM_BASE;
	int ret = -EPERM;

	/*step 1 : Set config register*/
	ret = adjust_efuse(0);
	if (ret) goto out;

	/*Step 2 : Invoke SC-ROM controller to read corresponding map address*/
	switch (start) {
	case EFU_ROOT_KEY_BASE:
		break;
	case EFU_CHIP_KEY_BASE:
		break;
	case EFU_USER_KEY_BASE:
		break;
	case EFU_MD5_BASE:
		break;
	default :
		ret = -EPERM;
	}
out:
	return ret;
}

#define EFUSE_CHECK
#define EFUSE_W_TIMEOUT	(100*800)
int efuse_write_data(void *buf, int length, off_t offset)
{
	int i = 0, ret = 0;
	char *tmp_buf = NULL;
	int32_t *ptmp_buf = NULL;
	char *pbuf = buf;
	int off = offset;
	int xlen = 0;
	int len = length;
	uint32_t tmp_reg = 0;
	int timeout = EFUSE_W_TIMEOUT;		//vddq high is less than 1 sec

	if  (efuse_gpio < 0) {
		error("efuse gpio is not init");
		ret = -ENODEV;
		goto out;
	}
	/*step 1 : Set config register*/
	ret = adjust_efuse(1);
	if (ret) goto out;

	tmp_buf = (char *)malloc(len + sizeof(int32_t));
	if (!tmp_buf)
		return  -ENOMEM;
	memset(tmp_buf, 0, len + sizeof(int32_t));
	ptmp_buf = (int32_t *)tmp_buf;

	for (i = 0; i < length; i++)
		tmp_buf[i] = pbuf[i];

	while (len > 0 && timeout) {
		timeout = EFUSE_W_TIMEOUT;
		writel(0, (EFUSE_BASE+EFUSTATE));
		/*step 2 : Write want program data to EFUSE data buffer 0-7 registers*/	//FIXME
		debug_cond(efuse_debug, "off = %x\n",(int)off);
		xlen = len > EFUDATA_REG_NUM * 4 ? EFUDATA_REG_NUM * 4 : len;
		for (i = 0; i < (xlen + 3)/4 ; i++ ,ptmp_buf++) {
			if (i == 0)
				debug_cond(efuse_debug,"====write data to register====\n");
			debug_cond(efuse_debug,"%d(0x%x):0x%x\n",i,(EFUSE_BASE+EFUDATA(i)),*ptmp_buf);
			writel(*ptmp_buf ,(EFUSE_BASE+EFUDATA(i)));
		}
		/*step 3: Set control register, indicate want to program address, data length*/
		/*step 4: Write control register PG_EN bit to 1*/
		tmp_reg = ((off&0x1ff)<<21)|((xlen-1)<<16)|(1<<15);
		writel(tmp_reg,(EFUSE_BASE+EFUCTRL));
		len -= xlen;
		off += xlen;
		/*step 5: Connect VDDQ pin to 2.5V*/
		boost_vddq(efuse_gpio);
		/*step 6: Write control register WR_EN bit*/
		tmp_reg = readl((EFUSE_BASE+EFUCTRL));
		tmp_reg |= (1 << 1);
		debug_cond(efuse_debug,"EFUCTRL(0x%x):0x%x\n",(EFUSE_BASE+EFUCTRL),tmp_reg);
		writel(tmp_reg ,(EFUSE_BASE+EFUCTRL));
		/*step 7:Wait status register WR_DONE set to 1.*/
		while(!(readl((EFUSE_BASE+EFUSTATE))&(1 << 1)) && --timeout) {
			debug_cond(efuse_debug, "EFUSTATE %x\n",readl(EFUSE_BASE+EFUSTATE));
			udelay(10);
		}
		/*step 8:Disconnect VDDQ pin from 2.5V.*/
		reduce_vddq(efuse_gpio);
		/*step 9:Write control register PG_EN bit to 0.*/
		writel(0,(EFUSE_BASE+EFUCTRL));
	}
#ifdef EFUSE_CHECK
	timeout = 0;
#endif
	if (!timeout) {
		ptmp_buf = (int32_t *)tmp_buf;
		memset(ptmp_buf, 0, (length + sizeof(int32_t)));
		ret = efuse_read_data(ptmp_buf, length , offset);
		if (ret)
			goto out;
		if (memcmp(ptmp_buf, buf, length)) {
			error("write efuse failed");
			ret = -EFAULT;
			goto out;
		}
	}
out:
	free(tmp_buf);
	return ret;
}

int efuse_write_sc_key(unsigned int offset)
{
	int start = offset + EFU_ROM_BASE;
	int ret = -EPERM;
	unsigned tmp_reg = 0;

	/*step 1 : Set config register*/
	ret = adjust_efuse(1);
	if (ret) goto out;

	/*step 2: Write control register PG_EN bit to 1*/
	tmp_reg = (1<<15);
	writel(tmp_reg,(EFUSE_BASE+EFUCTRL));

	/*step 3: Connect VDDQ pin to 2.5V*/
	boost_vddq(efuse_gpio);
	switch (start) {
	case EFU_ROOT_KEY_BASE:
		break;
	case EFU_CHIP_KEY_BASE:
		break;
	case EFU_USER_KEY_BASE:
		break;
	case EFU_MD5_BASE:
		break;
	default :
		ret =  -EPERM;
	}

	/*Step 7: Disconnect VDDQ pin from 2.5V*/
	reduce_vddq(efuse_gpio);

	/*step 8:Write control register PG_EN bit to 0.*/
	writel(0,(EFUSE_BASE+EFUCTRL));
out:
	return ret;
}

int efuse_write(void *buf, int length, off_t offset)
{
	int start = (offset+EFU_ROM_BASE);
	int end = (offset + length + EFU_ROM_BASE - 1);
	int ret = -EPERM;

	if (end > EFU_ROM_END)
		return -EINVAL;
	printf("offset %x length %x start %x end %x\n", offset, length ,start, end);
	if ((start < EFU_ROOT_KEY_BASE && end <= EFU_ROOT_KEY_BASE) ||
			(start > EFU_MD5_BASE && end <= EFU_MD5_END) ||
			(start > EFU_FIX_BT_BASE && end <= EFU_FIX_BT_END) ||
			(start > EFU_PROT_BIT_BASE && end <= EFU_PROT_BIT_END))
		ret = efuse_write_data(buf,length,offset);
	else if (buf == NULL && length == 0) {
		ret = efuse_write_sc_key(offset);
	}
	return ret;
}

int efuse_read(void *buf, int length, off_t offset)
{
	int start = (offset + EFU_ROM_BASE);
	int end = (offset + length + EFU_ROM_BASE - 1);
	int ret = -EPERM;

	if (end > EFU_ROM_END)
		return -EINVAL;

	printf("offset %x length %x start %x end %x\n", offset, length ,start, end);
	if ((start < EFU_ROOT_KEY_BASE && end <= EFU_ROOT_KEY_BASE) ||
			(start > EFU_MD5_BASE && end <= EFU_MD5_END) ||
			(start > EFU_FIX_BT_BASE && end <= EFU_FIX_BT_END) ||
			(start > EFU_PROT_BIT_BASE && end <= EFU_PROT_BIT_END))
		ret = efuse_read_data(buf,length,offset);
	else if (buf == NULL && length == 0) {
		ret = efuse_read_sc_key(offset);
	}

	return ret;
}

int efuse_init(int gpio_pin)
{
	if (gpio_pin >= 0) {
		if (efuse_gpio >= 0) gpio_free(efuse_gpio);
		efuse_gpio = gpio_request(gpio_pin, "VDDQ");
		if (efuse_gpio < 0) return efuse_gpio;
	} else {
		efuse_gpio = -1;
	}
	return 0;
}

void efuse_deinit(void)
{
	if (efuse_gpio >= 0)
		gpio_free(efuse_gpio);
	efuse_gpio = -1;
	return;
}

void efuse_debug_enable(int enable)
{
	efuse_debug = !!enable;
	return;
}
