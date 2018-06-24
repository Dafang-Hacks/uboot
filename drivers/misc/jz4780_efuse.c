#include <common.h>
#include <malloc.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/errno.h>
#include <asm/arch/base.h>
#include <asm/arch/clk.h>

//#define EFU_NO_REG_OPS

#ifdef EFU_NO_REG_OPS
#undef writel
#undef readl
#define writel(b, addr) {;}
#define readl(addr)	0
#endif

static int efuse_debug = 0;
static int efuse_gpio = -1;

#define EFUCTRL		0xD0
#define EFUCFG		0xD4
#define EFUSTATE	0xD8
#define EFUDATA(x)	(0xDC + (x)*0x4)
#define EFU_ROM_BASE	0x200
#define EFU_ROM_END	0x5FF
#define EFU_RANDOM_ST	0x200
#define EFU_RANDOM_ED	0x207
#define EFU_SC_KEY_ST	0x500
#define EFU_SC_KEY_ED	0x5FF
#define EFUDATA_REG_NUM 8

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
		if (strobe < -1024) {
			strobe = ((WR_WR_STROBE_1TIME_MAX/1000)*(h2clk/1000000));
			strobe = strobe - 1666 - adj;
			if (strobe < -1024) {
				error("h2clk is too slow");
				ret = -EFAULT;
				goto out;
			}
		}
		if (adj > 0xf || strobe > 0x3ff) {
			error("h2clk is too fast");
			ret = -EFAULT;
			goto out;
		}

		if (strobe < 0)
			strobe = (0x400|(strobe&0x3ff));
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
	gpio_direction_output(gpio ,0);
	do {
		val = gpio_get_value(gpio);
		printf("gpio %d level %d\n",gpio,val);
	} while (val);
}

void reduce_vddq(int gpio)
{
	int val;
	printf("reduce vddq\n");
	gpio_direction_output(gpio ,1);
	do {
		val = gpio_get_value(gpio);
		printf("gpio %d level %d\n",gpio,val);
	} while (!val);
}

int efuse_read_data(void *buf, int length, off_t offset)
{
	int i = 0, ret = 0;
	int skip = offset%sizeof(int32_t);
	int len_wd = (length+skip+sizeof(int32_t) - 1)/sizeof(int32_t);
	int off = offset - skip;
	int xlen = 0;
	int h4k = 0;
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

	tmp_buf = malloc(len_wd * sizeof(int32_t));
	if (!tmp_buf)
		return -ENOMEM;
	ptmp_buf = tmp_buf;
	memset(ptmp_buf, 0 , len_wd * sizeof(int32_t));

	while (len_wd > 0) {
		writel(0,(EFUSE_BASE+EFUSTATE));
		/*step 2 : Set control register to indicate what to read data address, read data numbers and read enable.*/
		if (off >= 0x200)
			h4k = 1;
		xlen  = len_wd > EFUDATA_REG_NUM ? EFUDATA_REG_NUM : len_wd;
		if (!h4k && (off + xlen*sizeof(int32_t)) >= 0x200) {
			for (i = 0; i < xlen; i++)
				if ((offset+i*sizeof(int32_t)) >= 0x200)
					break;
			xlen = i;
		}
		tmp_reg = (off&0x3ff)<<21|((xlen*sizeof(int32_t)-1)<< 16)|(1 << 0);
		debug_cond(efuse_debug,"EFUCTRL(0x%x):0x%x\n",(EFUSE_BASE+EFUCTRL),tmp_reg);
		writel(tmp_reg ,(EFUSE_BASE+EFUCTRL));
		off += sizeof(int32_t) * xlen;
		len_wd -= xlen;
		/*step 3 : Wait status register RD_DONE set to 1 or EFUSE interrupted*/
#ifndef EFU_NO_REG_OPS
		while(!(readl((EFUSE_BASE+EFUSTATE))&(1 << 0))) {
			debug_cond(efuse_debug, "EFUSTATE %x\n",readl(EFUSE_BASE+EFUSTATE));
		}
#endif
		/*step 4 : Software read EFUSE data buffer 0 ? 8 registers*/
		for (i = 0 ; i < xlen ; i++) {
			*ptmp_buf =  readl((EFUSE_BASE+EFUDATA(i)));
			debug_cond(efuse_debug,"EFUDATA[0x%x]:0x%x\n",(EFUSE_BASE+EFUDATA(i)),
					*ptmp_buf);
			ptmp_buf++;
		}
	}


	for (i = 0 ; i < length ; i++)
		pbuf[i] = tmp_buf[i+skip];

	if (efuse_debug) {
		int i = 0;
		ptmp_buf = (int32_t *)buf;
		printf("====read data infomation====\n");
		for (i = 0; i < length/sizeof(int32_t); i++) {
			printf("%d:0x%x\n",i,ptmp_buf[i]);
		}
		printf("============================\n");
	}

	return ret;
}

int efuse_read_sc_key(unsigned int *sec_key)
{
	return 0;
}

int efuse_read_random(void)
{
	return 0;
}

#define EFUSE_CHECK
#define EFUSE_W_TIMEOUT	(100*800)
int efuse_write_data(void *buf, int length, off_t offset)
{
	int i = 0, ret = 0;
	char *tmp_buf = NULL;
	int32_t *ptmp_buf = NULL;
	char *pbuf = buf;
	int skip = offset%4;
	int len_wd = (length+skip+sizeof(int32_t)-1)/sizeof(int32_t);
	int off = offset - skip;
	int h4k = 0;
	uint32_t tmp_reg = 0;
	int timeout = EFUSE_W_TIMEOUT;		//vddq high is less than 1 sec

	if  (efuse_gpio < 0) {
		error("efuse gpio is not init");
		ret = -ENODEV;
		goto out;
	}
	/*step 1 : Set config register*/
	ret = adjust_efuse(1);
	if (ret)
		goto out;

	tmp_buf = (int32_t *)malloc(len_wd*sizeof(int32_t));
	if (!tmp_buf)
		return  -ENOMEM;
	ptmp_buf = tmp_buf;
	memset(ptmp_buf, 0, len_wd*sizeof(int32_t));
	for (i = 0; i < length; i++)
		tmp_buf[i+skip] = pbuf[i];

	while (len_wd > 0 && timeout) {
		timeout = EFUSE_W_TIMEOUT;
		writel(0, (EFUSE_BASE+EFUSTATE));
		/*step 2 : Write want program data to EFUSE data buffer 0-7 registers*/	//FIXME
		debug_cond(efuse_debug, "off = %x\n",(int)off);
		if (off >= 0x200) h4k = 1;
		for (i = 0; i < EFUDATA_REG_NUM && len_wd > 0; i++, len_wd--, ptmp_buf++) {
			if (!h4k && (off+i*4) == 0x200)
				break;
			if (i == 0)
				debug_cond(efuse_debug,"====write data to register====\n");
			debug_cond(efuse_debug,"%d(0x%x):0x%x\n",i,(EFUSE_BASE+EFUDATA(i)),*ptmp_buf);
			writel(*ptmp_buf ,(EFUSE_BASE+EFUDATA(i)));
		}
		/*step 3: Set control register, indicate want to program address, data length*/
		/*step 4: Write control register PG_EN bit to 1*/
		tmp_reg = ((off&0x3ff)<<21)|((i*4-1)<<16)|(1<<15);
		writel(tmp_reg,(EFUSE_BASE+EFUCTRL));
		off += sizeof(int32_t) * i;
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
		ptmp_buf = tmp_buf;
		memset(ptmp_buf, 0, len_wd * sizeof(int32_t));
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

int efuse_write_sc_key(unsigned int *sec_key)
{
	return 0;
}
int efuse_write_random(void)
{
	return 0;
}

int efuse_write(void *buf, int length, off_t offset)
{
	int sc_key = 0 , random = 0;
	int start = (offset+EFU_ROM_BASE);
	int end = (offset + length + EFU_ROM_BASE - 1);
	int ret = 0;

	if (end > EFU_ROM_END)
		return -EINVAL;

	/*SC_KET and RANDOM only write by mcu*/
	if (start > EFU_SC_KEY_ST) {
		if (start != EFU_SC_KEY_ST)
			return -EINVAL;
		sc_key = 1;
	}
	if (start <= EFU_RANDOM_ED) {
		if (start != EFU_RANDOM_ST)
			return -EINVAL;
		random = 1;
	}

	if (sc_key)
		ret =  efuse_write_sc_key((unsigned *)buf);
	else if (random)
		ret =  efuse_write_random();
	else
		ret =  efuse_write_data(buf,length,offset);

	return ret;
}

int efuse_read(void *buf, int length, off_t offset)
{
	int sc_key = 0, random = 0;
	int start = (offset+EFU_ROM_BASE);
	int end = (offset + length + EFU_ROM_BASE - 1);
	int ret = 0;

	/*SC_KET and RANDOM only write by mcu*/
	if (start > EFU_SC_KEY_ST) {
		if (start != EFU_SC_KEY_ST)
			return -EINVAL;
		sc_key = 1;
	}
	if (start <= EFU_RANDOM_ED) {
		if (start != EFU_RANDOM_ST)
			return -EINVAL;
		random = 1;
	}

	if (sc_key)
		ret = efuse_read_sc_key((unsigned *)buf);
	else if (random)
		ret = efuse_read_random();
	else
		ret = efuse_read_data(buf,length,offset);
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
