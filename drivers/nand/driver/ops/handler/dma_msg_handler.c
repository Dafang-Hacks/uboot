/*
 * we default that nemc is only one.
 * if there are some nemcs,please modify mcu_init and dma_msg_handle_init
 */

#include "dma_msg_handler.h"
#include "jzdma.h"
#include "nand_ops.h"
#include "nddata.h"
#include "nand_debug.h"
#include "nand_info.h"
//#include "xdelay.h"
#include "os_clib.h"
#include "clib.h"

#define INIT_MSG          0
#define NONE_INIT_MSG     1
#define MCU_CONTROL       0xb3420030
#define DMA_MAILBOX_NAND (*(volatile unsigned int *)(0xb3422020))

#define MCU_TEST_INTER_NAND
#ifdef MCU_TEST_INTER_NAND
#define MCU_TEST_DATA_NAND 0xB34257C0 //PDMA_BANK7 - 0x40
#endif
#define __bch_cale_eccbytes(eccbit)  (14*(eccbit)>>3)

static int g_taskmsgret;
static nand_dma *g_nddma = NULL;
nand_dma m_nddma;
struct taskmsg_init m_msg_init;
struct msg_handler m_msg_handler;

extern unsigned int cpm_get_h2clk(void);

static void mcu_reset(void)
{
	int tmp = *(volatile unsigned int *)MCU_CONTROL;
	tmp |= 0x01;
	*(volatile unsigned int *)MCU_CONTROL = tmp;
	ndd_ndelay(100);
	tmp = *(volatile unsigned int *)MCU_CONTROL;
	tmp &= ~0x01;
	*(volatile unsigned int *)MCU_CONTROL = tmp;
}

static void mcu_complete(unsigned int *mail_box)
{
	unsigned int mailbox = *mail_box;

	if(mailbox & MB_MCU_DONE ){
		if(mailbox & MSG_RET_FAIL)
			g_taskmsgret = ECC_ERROR;
		else if(mailbox & MSG_RET_MOVE)
			g_taskmsgret = BLOCK_MOVE;
		else if(mailbox & MSG_RET_EMPTY)
			g_taskmsgret = ALL_FF;
		else if(mailbox & MSG_RET_WP)
			g_taskmsgret = ENAND;
		else
			g_taskmsgret = SUCCESS;
	}else
		g_taskmsgret = MB_MCU_ERROR;
	ndd_debug("%s g_taskmsgret = %d\n",__func__,g_taskmsgret);
}

static inline int wait_mcu_finish(unsigned int *mail_box)
{
	int timeout = 1000 * 1000;
	int dmint, icsr;

	while(timeout--){
		icsr = readl(INTC_IOBASE + ICSR); /*pdmam*/
#if 0
		ndd_debug("ICSR[%x]------->icsr[%x]\n",INTC_IOBASE + ICSR, icsr);
		ndd_debug("GPIO[A] flag----------> %x \n",*(volatile unsigned int*)0xb0010050);
		ndd_debug("GPIO[A] mask----------> %x \n",*(volatile unsigned int*)0xb0010020);
		ndd_debug("\nDMA----------->\n");
		ndd_debug("\n***** [%x] *******\n",*((volatile unsigned int *)(0xb3424000-0xc0) + 0));
		ndd_debug("\nTCSM----------->\n");
		ndd_debug("\n***** [%x] *******\n",*((volatile unsigned int *)MCU_TEST_DATA_DMA + 0));
#endif
		if((icsr >> INTC_PDMAM_BIT) & 0x1){
			/*check the mailbox if is normal mailbox*/
			dmint = readl(PDMA_IOBASE + DMINT);
			//ndd_debug("%s ------->DMINT[%x] DMNMB[%x]\n",__func__,dmint,*(volatile unsigned int*)0xb3421034);
			if((dmint >> NORMAL_MAILBOX_BIT) & 0x1){
				/*clear DMINT.N_IP  bit*/
				dmint &= ~(0x1 << NORMAL_MAILBOX_BIT);
				writel(dmint, PDMA_IOBASE + DMINT);
				if((readl(PDMA_IOBASE + DMNMB) >> 16) == MCU_MSG_NORMAL){
					/*get mcu return value*/
					*mail_box = DMA_MAILBOX_NAND;
					DMA_MAILBOX_NAND = 0;
					break;
				}else if((readl(PDMA_IOBASE + DMNMB) >> 16) == MCU_MSG_INTC){
					*(volatile unsigned int *)0xb0010058 = 0xffffffff;
					ndd_debug("%s clear gpioa intterupt.\n",__func__);
				}
			}

		}
	}

	return timeout;
}

static int wait_dma_finish(unsigned int *mailbox)
{
	volatile unsigned int test[10] ={0};
	int i;
	int timeout = 0,count = 0;

	jzdma_start_memcpy(g_nddma->channelid);
	do{
		timeout = wait_mcu_finish(mailbox);
		//			volatile unsigned int *ret = (volatile unsigned int *)(0xb3425600);
		//			ndd_debug("pdma_task ret[0] = 0x%08x  ret[1] = 0x%08x\n",ret[0],ret[1]);
		/*
		   for(i=0;i<10;i++)
		   test[i] = ((volatile unsigned int *)MCU_TEST_DATA_NAND)[i];
		   ndd_debug("\ncpu send msg       [0x%08x]\n",test[0]);
		   ndd_debug("mcu receive msg    [0x%08x]\n",test[1]);
		   ndd_debug("mcu send mailbox   [0x%08x]\n",test[2]);
		   ndd_debug("dma receive inte   [0x%08x]\n",test[3]);
		   ndd_debug("dma mcu_irq handle [0x%08x]\n",test[4]);
		   ndd_debug("pdma_task_handle   [0x%08x]\n",test[5]);
		   ndd_debug("pdma_task_handle   [0x%08x]\n",test[6]);
		   ndd_debug("pdma_task_handle   [0x%08x]\n",test[7]);
		   ndd_debug("pdma_task_handle   [0x%08x]\n",test[8]);
		   ndd_debug("pdma_task_handle   [0x%08x]\n",test[9]);
		   ndd_debug("bch status         [0x%08x]\n",*(volatile unsigned int *)(0xb34d0184));
		   ndd_debug("bch status         [0x%08x]\n",*(volatile unsigned int *)(0xb0010540));
		   ndd_dump_taskmsg((struct task_msg *)PDMA_MSG_TCSMVA, 4);
		 */
#ifdef MCU_TEST_INTER_NAND
		if(timeout < 0){

			for(i=0;i<7;i++)
				test[i] = ((volatile unsigned int *)MCU_TEST_DATA_NAND)[i];
			ndd_print(NDD_DEBUG, "cpu send msg       [%x]\n",test[0]);
			ndd_print(NDD_DEBUG, "mcu receive msg    [%x]\n",test[1]);
			ndd_print(NDD_DEBUG, "mcu send mailbox   [%x]\n",test[2]);
			ndd_print(NDD_DEBUG, "dma receive inte   [%x]\n",test[3]);
			ndd_print(NDD_DEBUG, "dma mcu_irq handle [%x]\n",test[4]);
			ndd_print(NDD_DEBUG, "pdma_task_handle   [%x]\n",test[5]);
			ndd_print(NDD_DEBUG, "pdma_task_handle   [%x]\n",test[6]);
			ndd_print(NDD_DEBUG, "channel 0 state 0x%x\n",*(volatile unsigned int *)(0xb3420010));
			ndd_print(NDD_DEBUG, "channel 0 count 0x%x\n",*(volatile unsigned int *)(0xb3420008));
			ndd_print(NDD_DEBUG, "channel 0 source 0x%x\n",*(volatile unsigned int *)(0xb3420000));
			ndd_print(NDD_DEBUG, "channel 0 target 0x%x\n",*(volatile unsigned int *)(0xb3420004));
			ndd_print(NDD_DEBUG, "channel 0 command 0x%x\n",*(volatile unsigned int *)(0xb3420014));
			ndd_print(NDD_DEBUG, "channel 1 state 0x%x\n",*(volatile unsigned int *)(0xb3420030));
			ndd_print(NDD_DEBUG, "channel 1 count 0x%x\n",*(volatile unsigned int *)(0xb3420028));
			ndd_print(NDD_DEBUG, "channel 1 source 0x%x\n",*(volatile unsigned int *)(0xb3420020));
			ndd_print(NDD_DEBUG, "channel 1 target 0x%x\n",*(volatile unsigned int *)(0xb3420024));
			ndd_print(NDD_DEBUG, "channel 1 command 0x%x\n",*(volatile unsigned int *)(0xb3420034));
			ndd_print(NDD_DEBUG, "channel 2 state 0x%x\n",*(volatile unsigned int *)(0xb3420050));
			ndd_print(NDD_DEBUG, "channel 2 count 0x%x\n",*(volatile unsigned int *)(0xb3420048));
			ndd_print(NDD_DEBUG, "channel 2 source 0x%x\n",*(volatile unsigned int *)(0xb3420040));
			ndd_print(NDD_DEBUG, "channel 2 target 0x%x\n",*(volatile unsigned int *)(0xb3420044));
			ndd_print(NDD_DEBUG, "channel 2 command 0x%x\n",*(volatile unsigned int *)(0xb3420054));
			ndd_print(NDD_DEBUG, "channel 3 state 0x%x\n",*(volatile unsigned int *)(0xb3420070));
			ndd_print(NDD_DEBUG, "channel 3 count 0x%x\n",*(volatile unsigned int *)(0xb3420068));
			ndd_print(NDD_DEBUG, "channel 3 source 0x%x\n",*(volatile unsigned int *)(0xb3420060));
			ndd_print(NDD_DEBUG, "channel 3 target 0x%x\n",*(volatile unsigned int *)(0xb3420064));
			ndd_print(NDD_DEBUG, "channel 3 command 0x%x\n",*(volatile unsigned int *)(0xb3420074));
			ndd_print(NDD_DEBUG, "pdma mask register 0x%x\n",*(volatile unsigned int *)(0xb342102c));
			ndd_print(NDD_DEBUG, "pdma program register 0x%x\n",*(volatile unsigned int *)(0xb342101c));
			ndd_print(NDD_DEBUG, "mcu status  0x%x\n",*(volatile unsigned int *)(0xb3421030));
		}
#endif
		count++;
	}while((timeout < 0) && (count <= 10));

	if(timeout < 0)
		RETURN_ERR(TIMEOUT, "dma wait for completion timeout");
	mcu_complete(mailbox);

	return 0;
}

static int send_msg_to_mcu(nand_dma *nd_dma, dma_addr_t src_addr, int len)
{
	int ret = 0;

	jzdma_prep_memcpy(g_nddma->channelid, src_addr, PDMA_MSG_TCSMPA, len);

#ifdef MCU_TEST_INTER_NAND
	(*(unsigned long long *)MCU_TEST_DATA_NAND)++;
#endif
	ret = wait_dma_finish(&(nd_dma->mailbox));
	if(ret < 0)
		RETURN_ERR(ret, "wait dma finish failed");
	else
		ret = g_taskmsgret;

	return ret;
}

#if 0
static void dump_initmsg(struct taskmsg_init *msg)
{
	ndd_debug("dump taskmsg_init start-------->\n");
	ndd_debug("msg->ops.flag = %d\n",msg->ops.flag);
	ndd_debug("msg->info.pagesize = %d\n",msg->info.pagesize);
	ndd_debug("msg->info.oobsize = %d\n",msg->info.oobsize);
	ndd_debug("msg->info.eccsize = %d\n",msg->info.eccsize);
	ndd_debug("msg->info.eccbytes = %d\n",msg->info.eccbytes);
	ndd_debug("msg->info.ecclevel = %d\n",msg->info.ecclevel);
	ndd_debug("msg->info.twhr2 = %d\n",msg->info.twhr2);
	ndd_debug("msg->info.tcwaw = %d\n",msg->info.tcwaw);
	ndd_debug("msg->info.tadl = %d\n",msg->info.tadl);
	ndd_debug("msg->info.tcs = %d\n",msg->info.tcs);
	ndd_debug("msg->info.tclh = %d\n",msg->info.tclh);
}
#endif

static int mcu_init(nand_dma *nd_dma,Nand_Task *nandtask,int id)
{
	nand_data *nd_data = nd_dma->data;
	struct taskmsg_init *msg = nd_dma->msg_init;
	chip_info *ndinfo = nd_data->cinfo;
	nand_ops_timing *ndtime = &ndinfo->ops_timing;
	unsigned int h2clk = cpm_get_h2clk();
	unsigned int fcycle;
	int i;

	msg->ops.bits.type = MSG_MCU_INIT;
	msg->info.pagesize = ndinfo->pagesize;
	msg->info.oobsize  = ndinfo->oobsize;
	msg->info.eccsize  = nd_data->eccsize;
	/* we default that ecclevel equals the eccbit of first ndpartition;
	 *  the eccbit will be change,when pdma handles every taskmsg_prepare.
	 */
	msg->info.eccbytes = __bch_cale_eccbytes(24);
	msg->info.ecclevel = 24;
	/* calc firmware cycle in ps
	 *  firware has the same's clock as nemc's clock,which is AHB2.
	 */
	fcycle = 1000000000 / (h2clk / 1000); // unit: ps
	ndd_print(NDD_INFO,"^^^^^^^^^ h2clk=%d fcycle=%d ^^^^^^^^^^\n",h2clk,fcycle);
	msg->info.twhr2 = ((ndtime->tWHR2 * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.tcwaw = ((ndtime->tCWAW * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.tadl = ndtime->tADL;
	msg->info.tcs = ((ndtime->tCS * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.tclh= ((ndtime->tCLH * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.tsync = ((ndtime->tWC * 64 * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.fcycle = fcycle;

//	msg->info.eccpos = ndinfo->eccpos;
	msg->info.buswidth = ndinfo->buswidth;
	msg->info.rowcycle = ndinfo->rowcycles;
	/*
	 * we default that nemc is only one, which has no more than two rb.
	 */
	for(i = 0; i < nd_data->csinfo->totalchips; i++)
		msg->info.cs[i] = nd_data->csinfo->csinfo_table[i].id + 1;
	msg->info.rb0 = nd_data->rbinfo->rbinfo_table[0].gpio;
	if(nd_data->rbinfo->totalrbs > 1)
		msg->info.rb1 = nd_data->rbinfo->rbinfo_table[1].gpio;
	else
		msg->info.rb1 = 0xff;

	ndd_debug("*****  id = %d, rb0 = %d, rb1 = %d  *****\n",id,msg->info.rb0,msg->info.rb1);
	msg->info.taskmsgaddr = nandtask->msg_phyaddr;
	//dump_initmsg(msg);
	ndd_dma_cache_wback((unsigned int)nd_dma->msg_init, sizeof(struct taskmsg_init));
	ndd_debug("########  msg_phyaddr = 0x%x # PDMA_MSG_TCSMPA = 0x%x ########\n",msg->info.taskmsgaddr,PDMA_MSG_TCSMPA);
	return send_msg_to_mcu(nd_dma,nd_dma->msginit_phyaddr,sizeof(struct taskmsg_init));
}
extern void ndd_dump_taskmsg(struct task_msg *msg, int num);
static int dma_handle_msg(int context, Nand_Task *nt)
{
	nand_dma *nd_dma = (nand_dma *)context;
	int ret = 0, flag;
	//volatile unsigned int *retvalue = (volatile unsigned int *)(0xb3425600);

	if(nt->msg == NULL)
		RETURN_ERR(ENAND, "data buffer is NULL!");
	//ndd_dump_taskmsg(nt->msg, nt->msg_index);
	ndd_dma_cache_wback((unsigned int)nt->msg, sizeof(struct task_msg) * nt->msg_index);
	ret = send_msg_to_mcu(nd_dma, nt->msg_phyaddr, sizeof(struct task_msg) * MSG_NUMBER);
#ifdef DEBUG_ERR_TSKMSG
	if (ret == TIMEOUT)
		ndd_dump_taskmsg(nt->msg,nt->msg_index);
#endif
	if(ret == MB_MCU_ERROR){
		ndd_print(NDD_ERROR, "%s %d :send msg to mcu failed ! ret = %d \n",__func__,__LINE__,ret);
		mcu_reset();
		udelay(5);
		flag = send_msg_to_mcu(nd_dma,nd_dma->msginit_phyaddr,sizeof(struct taskmsg_init));
		if(flag != SUCCESS){
			ndd_print(NDD_FATE_ERROR, "%s %d :mcu_init is failed again! flag = %d \n",
					__func__,__LINE__,flag);
			while(1);
		}
	}
	//for(j = 0; j < ((nt->msg_index + 7)/ 8); j++)
	//	ndd_debug("pdma_task retvalue[%d] = %x\n", j, retvalue[j]);
	return ret;
}
static nand_dma *dma_ops_init(nand_data *data, Nand_Task *nandtask,int id)
{
	/* we default only pdma's unit */
	int ret = 0;
	if(g_nddma == NULL){
		g_nddma = &m_nddma;
	        ndd_memset(g_nddma,0,sizeof(nand_dma));
		/*alloc msg memory*/
		g_nddma->msg_init = &m_msg_init;

		g_nddma->msginit_phyaddr = nd_get_phyaddr((void *)g_nddma->msg_init);

		ndd_debug("***** msginit_vaddr = %x msginit_phyaddr = %x ********** \n",g_nddma->msg_init, g_nddma->msginit_phyaddr);

		g_nddma->data = data;
		g_nddma->channelid = 3;
		g_nddma->suspend_flag = 0;
		g_nddma->resume_flag = 0;

		ret = mcu_init(g_nddma,nandtask,id);
		if(ret != SUCCESS)
			goto err;

		ndd_print(NDD_INFO, "INFO: Nand DMA ops init success!\n");
	}
	return g_nddma;
err:
	return NULL;
}

int dma_msg_handle_suspend(int msghandler)
{
	if (g_nddma->suspend_flag)
		return 0;

	g_nddma->suspend_flag = 1;
	g_nddma->resume_flag = 0;

	return 0;
}

int dma_msg_handle_resume(int msghandler, Nand_Task *nandtask,int id)
{
	if (g_nddma->resume_flag)
		return 0;

	g_nddma->resume_flag = 1;
	g_nddma->suspend_flag = 0;

	//msleep(5);
	udelay(5);
	return mcu_init(g_nddma, nandtask, id);
}

#if 0
static void test_dma(void)
{
	unsigned char src_data[9] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
	unsigned char dst_data[12] = {0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}; //{0xcc}
	unsigned int src_addr = (unsigned int)src_data - 0x80000000;
	unsigned int dst_addr = (unsigned int)dst_data - 0x80000000;
	int i;

	ndd_dma_cache_wback((unsigned int)src_data,sizeof(src_data));
	ndd_dma_cache_wback((unsigned int)dst_data,sizeof(dst_data));
	jzdma_prep_memcpy(3,src_addr,dst_addr,sizeof(src_data));
	jzdma_start_memcpy(3);
	while(readl(0xb3420068));
	for(i=0; i < 12; i++){
		if(i%4==0)
			ndd_debug("\n[%d]",i);
		ndd_debug(" %x ",dst_data[i]);
	}
	ndd_debug("\n");
	ndd_debug("channel 3 state 0x%x\n",*(volatile unsigned int *)(0xb3420070));
	ndd_debug("channel 3 count 0x%x\n",*(volatile unsigned int *)(0xb3420068));
	ndd_debug("channel 3 source 0x%x\n",*(volatile unsigned int *)(0xb3420060));
	ndd_debug("channel 3 target 0x%x\n",*(volatile unsigned int *)(0xb3420064));
	ndd_debug("channel 3 command 0x%x\n",*(volatile unsigned int *)(0xb3420074));
	while(1);
}
#endif 

int dma_msg_handle_init(nand_data *data, Nand_Task *nandtask, int id)
{
	/* we default only pdma's unit */
	struct msg_handler *handler;
	nand_dma *nd_dma;

	jzdma_init();
	//test_dma();
	handler = &m_msg_handler;
	nd_dma = dma_ops_init(data,nandtask,id);
	handler->context = (int)nd_dma;
	handler->handler = dma_handle_msg;
	ndd_print(NDD_INFO, "INFO: Nand DMA ops init success!\n");
	return (int)handler;
}

void dma_msg_handle_deinit(int msghandler)
{

}
