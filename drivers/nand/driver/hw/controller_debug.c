#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/interrupt.h>
//#include <include/linuxver.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/math64.h>
#include <linux/completion.h>

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include "nddata.h"
#include "nand_io.h"
#include "nand_bch.h"
#include "nand_hwfeature.h"
#include "nand_info.h"
#include "transadaptor.h"
#include "ref_info.h"

#define BASE 0xb0010000
int offset[]={0x10,0x14,0x18,0x20,0x24,0x28,0x30,0x34,0x38,0x40,0x44,0x48};
PipeNode *pipe = NULL;
static void set_function(int gpio, int group, int function)
{
	int i;
	for(i = 0; i < 4; i++){
		*(volatile unsigned int *)(BASE + offset[(3 - i) * 3 + 2 - ((function >> i) & 1)] + group*0x100) = 1 << gpio;
	}
}
static int get_function(int gpio, int group)
{
	int i;
	int func = 0;
	for(i = 0; i < 4; i++){
		func |= (((*(volatile unsigned int *)(BASE + offset[(3 - i) * 3] + group*0x100)) >> gpio) & 1) << i;
	}
	return func;
}
static void set_gpio_function(int gpio, int group, int function)
{
	set_function(gpio,group,function);
	//printk("gpio[%d] group[%d]:0x%x\n",gpio,group,get_function(gpio,group));
}
static void set_wp(int gpio, int group)
{
	set_function(gpio,group,5);
	//printk("gpio[%d] group[%d]:0x%x\n",gpio,group,get_function(gpio,group));
}

static int test_reflist(void)
{
	printk("11\n");
	add_ref_info(0,0,1);
	printk("22\n");
	add_ref_info(0,0,1);
	printk("33\n");
	reduce_ref_info(0,0,1);
	printk("44\n");
	add_ref_info(0,1,0);
	printk("55\n");
	add_ref_info(1,0,0);
	printk("66\n");
	add_ref_info(1,0,0);
	return 0;
}

static int test_read_id(nand_data *nddata, int io_id)
{
	int io_context;
	int ret = 0,i;
	unsigned char buf[8] = {1,1,1,1,1,1,1,1};
#if 0
	*(volatile unsigned int *)0xb3410014 = 0x3fffff00; //default timing
	*(volatile unsigned int *)0xb3410050 = 0x03; //chip_select 1
	msleep(10);
	*(volatile unsigned char *)0xbb400000 = 0xff; //send cmd reset
	msleep(10);
	*(volatile unsigned char *)0xbb400000 = 0x90; //send cmd read id
	*(volatile unsigned char *)0xbb800000 = 0x00; //send addr
	msleep(10);
	for(i=0;i<sizeof(buf);i++){
		buf[i] = *(volatile unsigned char *)0xbb000000; //read data
		printk("buf[%d]:0x%02x  ",i,buf[i]);
	}
#endif
	nand_io_setup_default(nddata,io_id);
	io_context = nand_io_open(nddata,0);
	nand_io_chip_select(io_context,0);
	ret = nand_io_send_cmd(io_context,0xff,300); // reset
	ret = nand_io_send_cmd(io_context,0x90,300); // read id :90 -- 00
	nand_io_send_spec_addr(io_context,0x00,1,300);
	nand_io_receive_data(io_context,buf,sizeof(buf));
	for(i=0; i<sizeof(buf); i++)
		printk("0x%02x  ",buf[i]);
	printk("\n");
	nand_io_chip_deselect(io_context,1);
	nand_io_close(io_context);
	printk("%s ok !\n",__func__);
	return ret;
}

static int test_erase_block(nand_data *nddata)
{
	int io_context;
	int ret = 1;
	int timeout = 3000;

	printk(" %s test begain ..............\n",__func__);
	nand_io_setup_default(nddata,0);
	io_context = nand_io_open(nddata,0);
	nand_io_chip_select(io_context,0);
	msleep(10);
#if 0
	while(1){
/*
	for(i=0;i<8;i++)
		set_gpio_function(i,0,0); // A
	set_gpio_function(18,0,0); //group A
	set_gpio_function(19,0,0); //group A
	set_gpio_function(21,0,0); //group A
	set_gpio_function(20,0,6);  //GPA(20) rb
	set_gpio_function(0,1,0);  //group B
	set_gpio_function(1,1,0);  //group B
*/

//		*(volatile unsigned int *)0xb3410050 = 0x03; //chip_select 1
//		set_gpio_function(20,0,10);  //GPA(20) rb
		//printk("rb function: %d  \n",get_function(20,0));
		ret = nand_io_send_cmd(io_context,0xff,0); // reset
		//printk("1111 %X ",*(volatile unsigned int *)0xb0010000);
		//while (((*(volatile unsigned int *)0xb0010000) & 0x00100000))printk("11211 %X ",*(volatile unsigned int *)0xb0010000);;
		//printk("2222 %X ",*(volatile unsigned int *)0xb0010000);
		//	while (!((*(volatile unsigned int *)0xb0010000) & 0x00100000))printk("12111 %X ",*(volatile unsigned int *)0xb0010000);;
		//printk("3333 %X ",*(volatile unsigned int *)0xb0010000);
		//if(timeout < 0){
		//	printk("wait rb timeout:%d \n",timeout);
		//	}
		//	timeout = 2000;
		msleep(10);
		printk("wait rb ok ...... \n");
	}
#endif
	ret = nand_io_send_cmd(io_context,0x60,0);
	nand_io_send_addr(io_context,-1,0,0);
	ret = nand_io_send_cmd(io_context,0xd0,10);
	while (((*(volatile unsigned int *)0xb0010000) & 0x00100000) && timeout--);
	while (!((*(volatile unsigned int *)0xb0010000) & 0x00100000));
	if(timeout < 0){
		printk("wait rb timeout:%d \n",timeout);
	}
	//msleep(100);
	ret = nand_io_send_cmd(io_context,0x70,300);
	if(ret != 0){
		printk("error: write data failed----------------ret=%d\n",ret);
		while(1);
	}
	nand_io_chip_deselect(io_context,1);
	nand_io_close(io_context);
	printk("%s ok ret=%d \n",__func__,ret);
	return ret;
}
static int test_write_data(nand_data *nddata)
{
	int io_context;
	int ret = -1;
	int timeout = 2000;
	//unsigned char buf[15] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14};
	unsigned char buf[4096] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb};
	io_context = nand_io_open(nddata,0);
	nand_io_chip_select(io_context,0);
	ret = nand_io_send_cmd(io_context,0x80,300);
	nand_io_send_addr(io_context,0,103,300);
	nand_io_send_data(io_context,buf,sizeof(buf));
	ret = nand_io_send_cmd(io_context,0x10,0);
        //wait rb
	while (((*(volatile unsigned int *)0xb0010000) & 0x00100000) && timeout--);
	while (!((*(volatile unsigned int *)0xb0010000) & 0x00100000));
	if(timeout < 0){
		printk("wait rb timeout:%d \n",timeout);
	}
	//msleep(10);
	ret = nand_io_send_cmd(io_context,0x70,300);
	if(ret != 0){
		printk("error: write data failed----------------ret=%d\n",ret);
		while(1);
	}
	nand_io_chip_deselect(io_context,1);
	nand_io_close(io_context);
	printk("%s ok ret=%d \n",__func__,ret);
	return ret;
}
static int test_read_data(nand_data *nddata)
{
	int io_context;
	int ret = 1,i;
	unsigned char buf[4096];
	int timeout = 2000;

	memset(buf,0x00,sizeof(buf));
	io_context = nand_io_open(nddata,0);
	nand_io_chip_select(io_context,0);
	ret = nand_io_send_cmd(io_context,0x00,0);
	nand_io_send_addr(io_context,0,103,300);
	ret = nand_io_send_cmd(io_context,0x30,100);
	// wait rb
	while (((*(volatile unsigned int *)0xb0010000) & 0x00100000) && timeout--);
	while (!((*(volatile unsigned int *)0xb0010000) & 0x00100000));
	if(timeout < 0){
		printk("wait rb timeout:%d \n",timeout);
	}
	//msleep(100);
	nand_io_receive_data(io_context,buf,sizeof(buf));
	printk("%s %d read data................\n",__func__,__LINE__);
	for(i=0; i<sizeof(buf); i++){
		if(i%8 == 0)
			printk("\n [%d] ",i);
		printk("0x%02x  ",buf[i]);
	}
	nand_io_chip_deselect(io_context,1);
	nand_io_close(io_context);

	return ret;
}

static int test_write_data_with_oob(nand_data *nddata)
{
	int io_context,bch_context;
	int ret = -1,i;
	int timeout = 2000;
	unsigned char *ptr;
	//unsigned char buf[15] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14};
	unsigned char buf[2048] = {0};//{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb};
        /*open io and bch*/
	io_context = nand_io_open(nddata,0);
	bch_context = nand_bch_open(nddata,0,24);
	if(pipe == NULL){
		pipe = kmalloc(sizeof(PipeNode),GFP_KERNEL);
		if(!pipe)
			printk("%s %d alloc mem fail !\n",__func__,__LINE__);
		pipe->pipe_data = buf;
		pipe->pipe_par = &buf[1024];
	}
	memset(pipe->pipe_data,0xcc,1024);
/*	printk("pipe data .............\n");
	for(i=0;i<1024;i++){
		if(i%8 == 0)
			printk("\n [%d] ",i);
		printk("0x%02x ",(pipe->pipe_data)[i]);
	}
*/
        /*chip select*/
	nand_io_chip_select(io_context,0);
        /*get parity data*/
	ret = nand_bch_encode_prepare(bch_context,pipe,1);
	ret = nand_bch_encode_complete(bch_context,pipe);
	printk("parity data .............\n");
	for(i=0;i<((nand_bch*)bch_context)->eccbytes;i++){
		if(i%8 == 0)
			printk("\n [%d] ",i);
		printk("0x%X ",(pipe->pipe_par)[i]);
	}

	if(ret < 0){
		printk("%s %d error!\n",__func__,__LINE__);
		while(1);
	}
        /*random write pipe_data and pipe_par*/
	ret = nand_io_send_cmd(io_context,0x80,300);
	nand_io_send_addr(io_context,0,100,300);
	nand_io_send_data(io_context,pipe->pipe_data,1024);
	ret = nand_io_send_cmd(io_context,0x85,0);
	nand_io_send_addr(io_context,4100,-1,300);
	nand_io_send_data(io_context,pipe->pipe_par,((nand_bch*)bch_context)->eccbytes);
	ret = nand_io_send_cmd(io_context,0x10,0);
        /*wait rb*/
	while (((*(volatile unsigned int *)0xb0010000) & 0x00100000) && timeout--);
	while (!((*(volatile unsigned int *)0xb0010000) & 0x00100000));
	if(timeout < 0){
		printk("wait rb timeout:%d \n",timeout);
	}
        /*read status*/
	ret = nand_io_send_cmd(io_context,0x70,300);
	if(ret != 0){
		printk("error: write data failed----------------ret=%d\n",ret);
		while(1);
	}
        /*chip deselect*/
	nand_io_chip_deselect(io_context,1);
	nand_io_close(io_context);
	nand_bch_close(bch_context);

	printk("%s ok ret=%d \n",__func__,ret);
	return ret;
}
static int test_read_data_with_oob(nand_data *nddata)
{
	int io_context,bch_context;
	int ret = 1,i,errcnt;
	unsigned char buf[2048];
	int timeout = 2000;

        /*open io and bch*/
	io_context = nand_io_open(nddata,0);
	bch_context = nand_bch_open(nddata,0,24);
        /*chip select*/
	nand_io_chip_select(io_context,0);
        /*random read data and parity*/
	ret = nand_io_send_cmd(io_context,0x00,0);
	nand_io_send_addr(io_context,0,100,300);
	ret = nand_io_send_cmd(io_context,0x30,100);
	/* wait rb */
	while (((*(volatile unsigned int *)0xb0010000) & 0x00100000) && timeout--);
	while (!((*(volatile unsigned int *)0xb0010000) & 0x00100000));
	if(timeout < 0){
		printk("wait rb timeout:%d \n",timeout);
	}
	nand_io_receive_data(io_context,pipe->pipe_data,1024);
	ret = nand_io_send_cmd(io_context,0x05,0);
	nand_io_send_addr(io_context,4100,-1,300);
	ret = nand_io_send_cmd(io_context,0xe0,20000); // 20ns
	nand_io_receive_data(io_context,pipe->pipe_par,((nand_bch*)bch_context)->eccbytes);
	ret = nand_bch_decode_prepare(bch_context,pipe,1);
	errcnt = nand_bch_decode_complete(bch_context,pipe);
	printk("bch errcnt = %d \n",errcnt);
	printk("%s %d read data................\n",__func__,__LINE__);
	for(i=0; i<(1024 + ((nand_bch*)bch_context)->eccbytes); i++){
		if(i%8 == 0)
			printk("\n [%d] ",i);
		printk("0x%02x  ",buf[i]);
	}
	nand_io_chip_deselect(io_context,1);
	nand_io_close(io_context);
	nand_bch_close(bch_context);
	return ret;
}

static int init_nand_data(nand_data *nand_data, int io_id)
{
	io_base *base;
	nand_info *ndinfo;

	base = kmalloc(sizeof(io_base),GFP_KERNEL);
	ndinfo = kmalloc(sizeof(nand_info),GFP_KERNEL);
	base->io[io_id].nemc_iomem = 0xb3410000;
	base->io[io_id].nemc_cs_iomem[0] = 0xbb000000;
	base->bch[io_id].bch_iomem = 0xb34d0000;
	ndinfo->rowcycles = 3;
	ndinfo->eccbit = 24;
	ndinfo->timing = NULL;
	nand_data->base = base;
	nand_data->ndinfo = ndinfo;

	return 0;
}

static int __init nand_init(void)
{
	nand_data *nddata;
	int ret = 0,i;
	struct clk *io_clk;
	struct clk *bch_clk;
	struct clk *bch_cgu_clk;
	printk("module init start ....................\n");
	nddata = kmalloc(sizeof(nand_data),GFP_KERNEL);
	if(!nddata){
		printk("alloc memory failed! line: %d\n",__LINE__);
		while(1);
	}
	init_nand_data(nddata,0);
	io_clk = clk_get(NULL,nand_io_get_clk_name());
	clk_enable(io_clk);
	bch_clk = clk_get(NULL,nand_bch_get_clk_name());
	bch_cgu_clk = clk_get(NULL,nand_bch_get_cgu_clk_name());
	clk_enable(bch_clk);
        clk_disable(bch_cgu_clk);
        clk_set_rate(bch_cgu_clk, clk_get_rate(bch_cgu_clk));
	clk_enable(bch_cgu_clk);
	printk("clkgate=0x%08x bchclk=%ld \n",*(volatile unsigned int *)0xb0000020,clk_get_rate(bch_cgu_clk));
	for(i=0;i<8;i++)
		set_gpio_function(i,0,0); // A
	set_gpio_function(18,0,0); //group A
	set_gpio_function(19,0,0); //group A
	set_gpio_function(21,0,0); //group A
	set_gpio_function(20,0,6);  //GPA(20) rb
	set_gpio_function(0,1,0);  //group B
	set_gpio_function(1,1,0);  //group B
	set_wp(22,5); // GPF(22) wp
#if 0
	printk("gpioa int =0x%x \n ",*(unsigned int *)0xb0010010);
	printk("gpioa mask =0x%x \n ",*(unsigned int *)0xb0010020);
	printk("gpioa pat1 =0x%x \n ",*(unsigned int *)0xb0010030);
	printk("gpioa pat0 =0x%x \n ",*(unsigned int *)0xb0010040);
	printk("gpiob int =0x%x \n ",*(unsigned int *)0xb0010110);
	printk("gpiob mask =0x%x \n ",*(unsigned int *)0xb0010120);
	printk("gpiob pat1 =0x%x \n ",*(unsigned int *)0xb0010130);
	printk("gpiob pat0 =0x%x \n ",*(unsigned int *)0xb0010140);
	printk("nemc nfcsr =0x%x \n ",*(unsigned int *)0xb3410050);
	printk("nemc smcr =0x%x \n ",*(unsigned int *)0xb3410014);
#endif
	printk("===================test begain ====================\n");
	ret = test_read_id(nddata,0);
	test_erase_block(nddata);
        /*test io*/
	//ret = test_read_data(nddata);
	//ret = test_write_data(nddata);
	//ret = test_read_data(nddata);
        /*test bch*/
	ret = test_write_data_with_oob(nddata);
	ret = test_read_data_with_oob(nddata);
	printk("===================test end ====================\n");
	return 0;
}

static int __exit nand_exit(void)
{
	return 0;
}

module_init(nand_init);
module_exit(nand_exit);
