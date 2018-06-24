#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/sfc.h>
#include <asm/gpio.h>

#define RECEIVE	 1
#define TRANSFER 0

static uint32_t jz_sfc_readl(unsigned int offset)
{
	return readl(offset);
}

static void jz_sfc_writel(unsigned int value, unsigned int offset)
{
	writel(value,offset);
}

void sfc_init(void)
{
	unsigned long sfc_dev_conf_value;
	unsigned long sfc_global_value;
	sfc_dev_conf_value = 0x7;
	sfc_global_value = 1<<7;
	jz_sfc_writel(sfc_dev_conf_value,SFC_DEV_CONF);
	jz_sfc_writel(sfc_global_value,SFC_GLB);

}

void sfc_config(struct sfc_xfer xfer)
{
	int cmd,tran_dir,addr_width,data_en,poll_en,tran_len,page_addr;
	unsigned long reg;
	cmd = xfer.cmd;
	tran_dir = xfer.tran_dir;
	addr_width = xfer.addr_width;
	data_en = xfer.data_en;
	poll_en = xfer.poll_en;
	tran_len = xfer.tran_len;

	if(tran_dir)
	{
		reg = jz_sfc_readl(SFC_GLB);
		reg |= 1<<13;
		jz_sfc_writel(reg,SFC_GLB);
	}
	jz_sfc_writel(addr_width <<26| poll_en <<25 |  CMD_EN<<24 | data_en <<16 | cmd, SFC_TRAN_CONF0);
	jz_sfc_writel(tran_len,SFC_TRAN_LEN);
	jz_sfc_writel(page_addr,SFC_DEV_ADR0);

	if(poll_en)
	{
		jz_sfc_writel(1<<26 | poll_en <<25 |1<<24 |1<<16 |0xf <<0,SFC_TRAN_CONF1);
		jz_sfc_writel(0xc0,SFC_DEV_ADR1);
		jz_sfc_writel(0x01,SFC_DEV_STA_MSK);
		jz_sfc_writel(0x00,SFC_STA_EXP);

	}

	return ;
}


void sfc_start(void)
{
	__sfc_start();
	return ;
}

void wait_sfc_requst(unsigned int tran_dir)
{
	if(tran_dir == TRANSFER)
	{
		while(!(__sfc_tran_req()))
		{
			udelay(100);
		}
	}
	else if(tran_dir == RECEIVE)
	{
		while(!(__sfc_rece_req()))
		{
			udelay(100);
		}
	}
	else
	{
		printf("tran_dir is error\n");
	}
	jz_sfc_writel(0x1f,SFC_SCR);
	return ;
}

void wait_sfc_end(void)
{
	while(!(__sfc_is_end()))
	{
		udelay(100);
	}
	jz_sfc_writel(0x1f,SFC_SCR);
	return ;
}

void write_byte(unsigned char *buf)
{
	int i;
	unsigned int data=0;
	unsigned int offset = 0;
			*(unsigned char *)(&data + i) = *buf;
		jz_sfc_writel(data,SFC_DR);
	return ;
}

void read_byte(unsigned char *data,int count)
{
	unsigned int offset = 0;
	unsigned int temp;
		temp = jz_sfc_readl(SFC_DR);
		*data= (unsigned char)temp;
}



void sfc_load(unsigned int src_addr, unsigned int count,unsigned int dst_addr)
{

	src_addr = ((src_addr & 0xFF)<<16) | (src_addr & 0x0000FF00) | ((src_addr >>16) &0xFF);

	/*** page_read_to_cache ***/
	writel(1<<THRESHOLD  | 1 <<PHASE_NUM,SFC_GLB) ;
	writel(1<<CE_DL | 1<<HOLD_DL | 1<<WP_DL,SFC_DEV_CONF);
	writel(3<<ADDR_WIDTH  | 1 <<CMD_EN |  0x13 <<TRAN_CMD,SFC_TRAN_CONF0);
	writel(0x0,SFC_TRAN_LEN);

	writel(0x0,SFC_DEV_ADR0);
	writel(1 <<26 | 1 <<25 | 1<<24 | 1<<16 |0xf <<TRAN_CMD,SFC_TRAN_CONF1);

	writel(0xc0,SFC_DEV_ADR1);
	writel(0x1,SFC_DEV_STA_MSK);
	writel(0x0,SFC_STA_EXP);
	sfc_start();
	wait_sfc_end();
	/*** page_read_from_cache ***/
	writel(1<<THRESHOLD  | 1 <<PHASE_NUM,SFC_GLB);
	writel(1<<CE_DL | 1<<HOLD_DL | 1<<WP_DL,SFC_DEV_CONF);
	writel(3 <<ADDR_WIDTH | 1 <<CMD_EN |1<< DATA_EN |  0x3 <<TRAN_CMD,SFC_TRAN_CONF0);
	writel(count,SFC_TRAN_LEN);
	writel((unsigned char *)&src_addr,SFC_DEV_ADR0);

	/*** read_buf ***/
	int i,j;
	unsigned char *data;
	unsigned int temp;
	sfc_start();
	for(i=0;i<count/4;i++)
	{
		while(!(__sfc_rece_req()))
		{
			printf("%x.wait read buf\n",i);
			udelay(100);
		}
		writel(0x1f,SFC_SCR);
		temp = readl(SFC_DR);
		for(j=0;j<4;j++)
		{
			*((unsigned char *)(&dst_addr) +j+i*4)=*((unsigned char*)(&temp) +j);
		}
	}
	wait_sfc_end();
	return ;

}

#ifdef CONFIG_SPL_SFC_SUPPORT
void spl_sfc_load_image(void)
{
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	spl_parse_image_header(header);

	sfc_load(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN,CONFIG_SYS_TEXT_BASE);
}
#endif
