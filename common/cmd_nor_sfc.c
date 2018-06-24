#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/sfc.h>


void sfc_nor_load_image(unsigned int src_addr, unsigned int count,unsigned int dst_addr)
{

	int i,j;
	unsigned char *data;
	unsigned int temp;


	writel(1<<7|1<<3,SFC_GLB);
	writel(0x07,SFC_DEV_CONF);
	writel(src_addr,SFC_DEV_ADR0);
	writel(count,SFC_TRAN_LEN);
	writel(3<<26|1<<24|1<<16|0x03,SFC_TRAN_CONF0);

	/********************* read_buf ***************************/

	__sfc_start();
	for(i=0;i<count/4;i++)
	{
		while(!(__sfc_rece_req()))
		{
			udelay(100);
		}
		writel(0x1f,SFC_SCR);
		temp = readl(SFC_DR);

		for(j=0;j<4;j++)
		{
			*((unsigned char *)(dst_addr) +j+i*4)=*((unsigned char*)(&temp) +j);
		}

	}
	while(!(__sfc_is_end()))
	{
		udelay(100);
	}
	writel(0x1f,SFC_SCR);

	return ;
}

static int do_sfcnor(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int src_addr,count,dst_addr;
	if(argc < 5){
		return CMD_RET_USAGE;
	}
	if(!strcmp(argv[1],"read")){
		src_addr = simple_strtoul(argv[2],NULL,16);
		count = simple_strtoul(argv[3],NULL,16);
		dst_addr = simple_strtoul(argv[4],NULL,16);
		printf("sfcnor load Image form 0x%x to  0x%x size is 0x%x ...\n",src_addr,dst_addr,count);
		sfc_nor_load_image(src_addr,count,dst_addr);
		printf("Image load ok!\n");
		return 0;
	}else{
		return CMD_RET_USAGE;
	}
usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	sfcnor, 5,	0,	do_sfcnor,
	"load Image from sfc nor",
	"sfcnor read [src:0x..] [bytes:0x..] [dst:0x..]"
);
