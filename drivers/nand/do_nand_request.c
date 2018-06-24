#include <malloc.h>
#include <config.h>
#include <ingenic_nand_mgr/nand_param.h>
#include "nand_chip.h"
#include "lpartition.h"
#include "ppartition.h"
#include "vnandinfo.h"
#include "nandmanagerinterface.h"
#include "./driver/utils/rebuild_nand_spl.h"
#include "nandinterface.h"

#define SECTOR_SIZE 512
#define MAX_PARTITION_NUM 20
#define ZM_MEMORY_SIZE  (8*1024*1024)
struct Ghandle{
	int pagesize;
	int pageperblock;
	int nd_rbcnt;
	unsigned int  sectorid;
	int zm_handle;
	int  pphandle[MAX_PARTITION_NUM];
	char  pphandle_index;
	char  curpt_index;
	char  uperrorpt;
	char eraseall;
	PPartition *m_ppt;
	LPartition* lp;	
}g_handle = {
	.sectorid = 0,
	.pphandle_index = 0,
	.curpt_index = 0,
	.uperrorpt = 1,
};

static int g_pt_count = MAX_PARTITION_NUM;

unsigned int *erase_pt_startaddrs;
static int erase_pt_count;

nand_params ndparams;
extern int nd_raw_boundary;

int erase_partition_fill_pphandle(unsigned int startpage, int pt_index);
static int NM_PtDirecterase(PPartition *pt);

static int buf_compare(unsigned char *read_data,unsigned char *org_data,unsigned int len)
{
	unsigned int i;
	unsigned int *buf1 = (unsigned int *)org_data;
	unsigned int *buf2 = (unsigned int *)read_data;

	for(i = 0; i < len / 4; i++)
	{
		if(buf1[i] != buf2[i])
			printf("XXXXXXXXXX  compare error: org_data[%x] = 0x%08x read_data[%x] = 0x%08x len = %08x\n",i,buf1[i],i,buf2[i],len);
	}
}

static int erase_pt_fill_pphandle(unsigned int pt_startpage_offset)
{
	int pt_index,ret;
	unsigned int pt_startpage,pt_endpage;
	unsigned int startpage;
	PPartition *pt = NULL;

	startpage = pt_startpage_offset / ndparams.ndbaseinfo.pagesize;

	for(pt_index = 0; pt_index < g_pt_count; pt_index++){
		pt_startpage = g_handle.m_ppt[pt_index].startPage;
		pt_endpage = pt_startpage + g_handle.m_ppt[pt_index].totalblocks * g_handle.m_ppt[pt_index].pageperblock;
		if( pt_startpage ==startpage)
			break;
	}
	if(pt_index == g_pt_count) {
		printf("%s %d ERROR, write pos not in nand partition!\n",__func__,__LINE__);
		printf(" the pt_startaddr should be %d or  \n",g_handle.m_ppt[0].startPage * 4046);
		printf(" %d or ",g_handle.m_ppt[1].startPage * 4046);
		printf(" %d \n",g_handle.m_ppt[2].startPage * 4046);
		return -1;
	}

	pt = &(g_handle.m_ppt[pt_index]);
	//printf("----------------->> pt_name = %s \n",pt->name);
	NM_PtDirecterase(pt);
	ret = NandManger_Ioctrl(g_handle.zm_handle, NANDMANAGER_UPDATE_ERRPT, (int)pt);

	return ret;
}
static void start(int handle)
{
	int i;
	PPartArray *pptinfo;

	pptinfo = NandManger_getDirectPartition(g_handle.zm_handle);

	g_handle.m_ppt = pptinfo->ppt;
	g_pt_count = pptinfo->ptcount;

#if 1
		/* xhshen */
	switch (g_handle.eraseall) {
	/* due to the write time reason ,when eraseall == 0 ,the erase and fill pphandle operation execute in there */
	case 0:
		for(i = 0; i < erase_pt_count; i++)
			erase_pt_fill_pphandle(erase_pt_startaddrs[i]);
		break;
	case 1:
                /*if normal eraseall,then skip erase each partition before write*/
	case 2:
                /*if force eraseall,then skip erase each partition before write*/
	case 3:
                /*if factory eraseall,then skip erase each partition before write*/
		g_handle.uperrorpt = 0;
		break;
	default:
		break;
	}
//	printf("NANDMANAGER: Nand Manager scan bad blocks!\n");
//	NandManger_Ioctrl(g_handle.zm_handle, NANDMANAGER_SCAN_BADBLOCKS,0);
#endif
	/*update all the partition with CPU_OPS for read/write/erase*/
	NandManger_getPartition(g_handle.zm_handle,&g_handle.lp);

}

unsigned int jz_strlen(const char *s)
{
    int len = 0;
    while(*s++) len++;
    return len;
}

char * jz_strncpy(char *dest,const char *src, unsigned int count)
{

	while(count){
		if((*dest = *src) != 0)
			src++;
		dest++;
		count--;
	}
	return dest;
}

int  get_nand_manager_version()
{
	nm_version version;
	Get_NandManagerVersion(&version);

	return version.major * 100 + version.minor * 10 + version.revision;
}

int burn_nandmanager_init(PartitionInfo *pinfo,int eraseall,unsigned int *ops_pt_startaddrs,int erase_pt_cnt)
{
	void *heap = (void*)malloc(ZM_MEMORY_SIZE);
	if(!heap){
		printf("%s %d malloc heap error!\n",__func__,__LINE__);
		return -1;
	}
		/* init global structure g_handle*/
	g_handle.zm_handle = NandManger_Init(heap,ZM_MEMORY_SIZE,0);
	//printf("----------------------------------->>>>> g_handle.zm_handle = 0x%08x\n",g_handle.zm_handle);
	g_handle.eraseall = eraseall;
	g_handle.nd_rbcnt = pinfo->rbcount;
	g_handle.pagesize = ndparams.ndbaseinfo.pagesize;
	g_handle.pageperblock = ndparams.ndbaseinfo.blocksize / ndparams.ndbaseinfo.pagesize;
	memset(g_handle.pphandle, 0xff, MAX_PARTITION_NUM);

	erase_pt_startaddrs = ops_pt_startaddrs;
	erase_pt_count = erase_pt_cnt;

	NandManger_startNotify(g_handle.zm_handle, start, g_handle.zm_handle);

	return g_handle.zm_handle;
}
extern unsigned int get_nandflash_maxvalidblocks(void);
void fill_nand_basic_info(nand_flash_param *nand_info) {
	ndparams.ndbaseinfo.id			= nand_info->id;
	ndparams.ndbaseinfo.extid		= nand_info->extid;
	ndparams.ndbaseinfo.pagesize		= nand_info->pagesize;
	ndparams.ndbaseinfo.oobsize		= nand_info->oobsize;
	ndparams.ndbaseinfo.blocksize		= nand_info->blocksize;
	ndparams.ndbaseinfo.totalblocks		= nand_info->totalblocks;
	ndparams.ndbaseinfo.eccbit		= nand_info->eccbit;
	ndparams.ndbaseinfo.planepdie		= nand_info->planepdie;
	ndparams.ndbaseinfo.diepchip		= nand_info->diepchip;
	ndparams.ndbaseinfo.chips		= nand_info->chips;
	ndparams.ndbaseinfo.buswidth		= nand_info->buswidth;
	ndparams.ndbaseinfo.realplanenum	= nand_info->realplanenum;
	ndparams.ndbaseinfo.badblockpos		= nand_info->badblockpos;
	ndparams.ndbaseinfo.rowcycles		= nand_info->rowcycles;
	ndparams.ndbaseinfo.planeoffset		= nand_info->planeoffset;
	ndparams.ndbaseinfo.options		= nand_info->options;

}
#ifdef CONFIG_NAND_NFI
#define NAND_SPL_SIZE	(32 * 1024)
#else
#define NAND_SPL_SIZE	(16 * 1024)
#endif //CONFIG_NAND_NFI
#define NAND_PARAMS_OFFSET	NAND_SPL_SIZE
#define REBUILD_SPL_SIZE	(NAND_SPL_SIZE + (1 * 1024))	// 16K nand_spl.bin + 1K nand basic params
// (16 * 1024) is max support pagesize, used to write a full page data to nand
#define SPL_BUF_SIZE		(NAND_SPL_SIZE + 16 * 1024)

#define DEBUG_PTWRITE
unsigned int do_nand_request(unsigned int startaddr, void *data_buf, unsigned int ops_length,unsigned int offset)
{
	int pHandle;
	void *databuf = data_buf;
	char spl_buf[SPL_BUF_SIZE];
#ifdef DEBUG_PTWRITE
	char readbuf[512*1024]={0};
#endif
	int totalbytes = ops_length;
	unsigned int wlen = 0;
	SectorList *sl;
	unsigned int rsectorid;
	int bl;
	int pt_index;
	int ret ;
	unsigned int pt_startpage,pt_endpage;
	unsigned int spl_align_sectorcount = 0;

	startaddr = startaddr/g_handle.pagesize;
	g_handle.sectorid = offset/512;

	/*handle two rb,find the correct pt_index*/
	if(g_handle.nd_rbcnt > 1 && startaddr != 0){
		if(startaddr*g_handle.pagesize < 64*1024*1024)
			startaddr *= 2;
		else
			startaddr += (nd_raw_boundary * g_handle.pageperblock);
	}
	for(pt_index = 0; pt_index < g_pt_count; pt_index++){
		pt_startpage = g_handle.m_ppt[pt_index].startPage;
		pt_endpage = pt_startpage + g_handle.m_ppt[pt_index].totalblocks * g_handle.m_ppt[pt_index].pageperblock;
		//printf("$$$$ pt_index=%d pt_startpage=%d pt_endpage=%d startaddr=%d  nd_raw_boundary=%d pt_name = %s $$$$$\n",pt_index,pt_startpage,pt_endpage,startaddr,nd_raw_boundary,g_handle.m_ppt[pt_index].name);
		if( pt_startpage <=startaddr && (startaddr + ops_length / g_handle.pagesize) < pt_endpage){
			break;
		}
	}
	if(pt_index == g_pt_count) {
		printf("%s ERROR, write pos not in nand partition!\n",__func__);
		//printf("pt[%d]:%s ops_length = %d pagesize=%d totalbytes=%d\n",
		//pt_index,__func__,ops_length,pagesize,totalbytes);
		return -1;
	}
	if (startaddr == 0) {
		if (totalbytes < NAND_SPL_SIZE) {
			printf("%s ERROR: nand_spl.bin not write at once!\n",__func__);
			return -1;
		} else {
			/**
			 * WARNING: for unknown reason, we call function here will change
			 * the data of data_buf, so code of here we do not package as
			 * a funciion or we do not call rebuild_nand_spl_4775() in
			 * "rebuild_nand_spl.h" temply.
			 **/
			nand_basic_info *ndinfo = &ndparams.ndbaseinfo;
			int pagesize_flag;
#if defined(CONFIG_JZ4775) || defined(CONFIG_M150)
			struct parm_buf {
				void *bw_buf;
				void *tp_buf;
				void *rc_buf;
				void *pf2_buf;
				void *pf1_buf;
				void *pf0_buf;
			} parm_buf = {spl_buf, spl_buf + 64, spl_buf + 128, spl_buf + 160, spl_buf + 192, spl_buf + 224};
#elif defined(CONFIG_JZ4780)
			struct parm_buf {
				void *tp_buf;
				void *rc_buf;
				void *pf2_buf;
				void *pf1_buf;
				void *pf0_buf;
			} parm_buf = {spl_buf, spl_buf + 64, spl_buf + 96, spl_buf + 128, spl_buf + 160};

#elif defined(CONFIG_M200) || defined(CONFIG_T15) || defined(CONFIG_T10)
			struct parm_buf {
				void *bw_buf;
				void *tp_buf;
				void *otherparm_buf;
				void *ext_buf;
			} parm_buf = {spl_buf, spl_buf + 64, spl_buf + 128, spl_buf + 256};

			/* the maxsize of struct nand_otherflag is 32bytes */
			struct nand_otherflag{
				unsigned int splflag;   /* the string is "SPL!" = 0x21 4c 50 53 */
				unsigned int rowcycle;
				unsigned int pagesize;
				unsigned int space[20];
			} nand_othflag;

			nand_othflag.splflag = 0x214c5053;
			nand_othflag.rowcycle = ndparams.ndbaseinfo.rowcycles;
			nand_othflag.pagesize = ndparams.ndbaseinfo.pagesize;
			nand_othflag.space[0] = 0;
#endif

			memset(spl_buf + NAND_PARAMS_OFFSET, 0xff, SPL_BUF_SIZE - NAND_SPL_SIZE);
			memcpy(spl_buf, databuf, NAND_SPL_SIZE);

			/* rebuild the first 256Bytes of nand_spl.bin */
			switch (ndinfo->pagesize) {
			case 512:
				pagesize_flag = 0;
				break;
			case 2048:
				pagesize_flag = 2;
				break;
			case 4096:
				pagesize_flag = 4;
				break;
			case 8192:
				pagesize_flag = 6;
				break;
			case 16384:
				pagesize_flag = 7;
				break;
			default:
				printf("%s ERROR: unsupport nand pagesize %d!\n", __func__, ndinfo->pagesize);
				return -1;
			}
#ifndef CONFIG_NAND_NFI  // no define nfi
#if defined(CONFIG_JZ4775) || defined(CONFIG_M150)
			memset(parm_buf.bw_buf, (ndinfo->buswidth == 16) ? 0xAA : 0x55, 64);
#endif
			memset(parm_buf.tp_buf, (REBUILD_GET_NAND_TYPE(ndinfo->options) == NAND_TYPE_TOGGLE) ? 0xAA : 0x55, 64);
			memset(parm_buf.rc_buf, (ndinfo->rowcycles == 3) ? 0xAA : 0x55, 32);
			memset(parm_buf.pf2_buf, (pagesize_flag >> 2 & 1) ? 0xAA : 0x55, 32);
			memset(parm_buf.pf1_buf, (pagesize_flag >> 1 & 1) ? 0xAA : 0x55, 32);
			memset(parm_buf.pf0_buf, (pagesize_flag >> 0 & 1) ? 0xAA : 0x55, 32);
#else /* nfi */

			memset(parm_buf.bw_buf, (ndinfo->buswidth == 16) ? 0xAA : 0x55, 64);
			memset(parm_buf.tp_buf, (REBUILD_GET_NAND_TYPE(ndinfo->options) == NAND_TYPE_TOGGLE) ? 0xAA : 0x55, 64);
			memcpy(parm_buf.otherparm_buf, &nand_othflag, sizeof(struct nand_otherflag));
			memset(parm_buf.otherparm_buf + (sizeof(struct nand_otherflag)), 0x00, 128 - sizeof(struct nand_otherflag));
			memset(parm_buf.ext_buf, 0x00, 256);

#endif //CONFIG_NAND_NFI

			/* patch nand basic params */
			ndparams.magic = 0x646e616e;	//nand
			ndparams.kernel_offset = g_handle.m_ppt[pt_index + 1].startPage / g_handle.m_ppt[pt_index + 1].groupperzone;
			printf("-------------->>> kernel_offset = %x\n",ndparams.kernel_offset);
			printf("-------------->>> nand_manager_version = %d\n",get_nand_manager_version());
			/* update maxvalidblocks after initing nand_driver successfully */
			ndparams.ndbaseinfo.maxvalidblocks = get_nandflash_maxvalidblocks();
			ndparams.nand_manager_version = get_nand_manager_version();
			memcpy(spl_buf + NAND_PARAMS_OFFSET, &ndparams, sizeof(nand_params));
		}
	}

	erase_partition_fill_pphandle(startaddr, pt_index);
	pHandle = g_handle.pphandle[pt_index];
	if(g_handle.curpt_index != pt_index){
		g_handle.sectorid = 0;
		g_handle.curpt_index = pt_index;
	}

	if(pHandle == -1){
		printf("the partition is not open,so the handle is -1,checkout the index!\n");
		return -1;
	}
	bl = BuffListManager_BuffList_Init();
	if(bl == 0){
		printf("BuffListManager Init failed!\n");
		return -1;
	}

	if (startaddr == 0) {
		spl_align_sectorcount = ((REBUILD_SPL_SIZE + g_handle.pagesize - 1) / g_handle.pagesize * g_handle.pagesize) / 512;
		NandManger_ptIoctrl(pHandle, NANDMANAGER_SET_XBOOT_OFFSET, spl_align_sectorcount * 512);
	}

	if(pt_index == 0 && startaddr != 0)
		g_handle.sectorid += (1 * 1024 + g_handle.pagesize - 1) / g_handle.pagesize * g_handle.pagesize / 512;

	while(totalbytes){
		if(totalbytes >= 256 *512)
			wlen = 256 * 512;
		else{
			wlen = totalbytes;
			if(wlen % 512 != 0)
				memset(databuf+wlen, 0xff, 512-wlen%512);
		}
		sl = BuffListManager_getTopNode(bl,sizeof(SectorList));
		if(sl == 0){
			printf("Bufferlist request sectorlist failed!\n");
			return -1;
		}
		rsectorid = g_handle.sectorid;
		sl->startSector = g_handle.sectorid;

		if (startaddr == 0) {
			wlen = NAND_SPL_SIZE;
			sl->pData = (void*)spl_buf;
			// write lengh align to pagesize
			sl->sectorCount = spl_align_sectorcount;
			g_handle.sectorid += spl_align_sectorcount;
		} else {
			sl->pData = (void*)databuf;
			sl->sectorCount = (wlen + 511)/ 512;
			g_handle.sectorid += (wlen + 511)/ 512;
		}

		//printf("%s write:sectorid:%d sectorCount:%d curpt_index=%d pt_index=%d pHandle=%x sl[%x]\n",__func__,sl->startSector,sl->sectorCount,g_handle.curpt_index,pt_index,pHandle,(int)sl);
rewrite:
	//printf("============  %s %d pHandle = 0x%08x pHandle_nmhandle = 0x%x \n",__func__,__LINE__,pHandle,*(unsigned int *)pHandle);
		if(NandManger_ptWrite(pHandle,sl) < 0){
			printf("NandManger_ptWrite failed, now rewrite!\n");
			goto rewrite;
		}
		if(sl->startSector == 0){
			if (startaddr == 0) {
				sl->sectorCount = spl_align_sectorcount;
				// only used to set startaddr != 0
				startaddr = (REBUILD_SPL_SIZE + g_handle.pagesize - 1) / g_handle.pagesize;
			} else
				sl->sectorCount = (wlen + 511)/ 512;
		}
#ifdef DEBUG_PTWRITE
		sl->startSector = rsectorid;
		sl->pData = (void*)readbuf;
		memset(readbuf,0,sl->sectorCount * 512);
		if(pt_index != 0){
			if(NandManger_ptRead(pHandle,sl) < 0){
				printf("%s %d  NandManger_ptRead failed, now rewrite!\n",__func__,__LINE__);
				goto rewrite;
			}

			buf_compare(readbuf,databuf,sl->sectorCount * 512);
		}
#endif
		databuf+=wlen;
		totalbytes-=wlen;
		BuffListManager_freeAllList(bl,(void **)&sl,sizeof(SectorList));
	}
	BuffListManager_BuffList_DeInit(bl);

	return ret;
}

static int NM_PtDirecterase(PPartition *pt)
{
	int blockid,markcnt;
	int ret;
	BlockList bl;

	//printf("%s info: pt[%s] totalblocks= %d pt_startblockid = %d \n",__func__,pt->name,pt->totalblocks,pt->startblockID);
	for(blockid = 0; blockid < pt->totalblocks; blockid++){
		bl.startBlock = blockid;
		bl.BlockCount = 1;
		bl.head.next = NULL;
/*		if(NandManger_DirectIsBadBlock(0, pt, &bl)){
			printf("xxxxxxxxxxx WARNING xxxxxxxxxx\n");
			printf("%s pt[%s]:blockid=%d is badblock!\n",__func__,pt->name,blockid);
			printf("xxxxxxxxxxx ENDING xxxxxxxxxx\n");
			continue;
		}
*/
		ret = NandManger_DirectErase(0, pt, &bl);
		if(ret){
			ret = NandManger_DirectMarkBadBlock(0, pt, blockid);
			if(ret){
				printf("============== ERROR ATENTION ================\n");
				printf("%s DirectMarkBadBlock Fail. pt[%s],blockid=%d,ret=%d\n",__func__,pt->name,blockid,ret);
				printf("=============== MARK FAILED ==================\n");
			}
			markcnt++;
		}
	}
	return markcnt;
}

int erase_partition_fill_pphandle(unsigned int startpage, int pt_index)
{
	PPartition *pt = &(g_handle.m_ppt[pt_index]);
	LPartition *lpentry;
	struct singlelist *it;
	int ret = 0;
	//printf("===========> startpage = %d pt_startblock[%d] pt_pageperblock[%d] pt_startpage[%d] pt_index = %d pt_name = %s\n",startpage,pt->startblockID,pt->pageperblock,pt->startPage,pt_index,pt->name);
	if(startpage == pt->startPage){
		singlelist_for_each(it,&(g_handle.lp->head)){
			lpentry = singlelist_entry(it,LPartition,head);
			if(strcmp(lpentry->name,pt->name) == 0){
				g_handle.pphandle[g_handle.pphandle_index] =
					NandManger_ptOpen(g_handle.zm_handle,lpentry->name,lpentry->mode);
				//printf("====================>  lpentry->name = %s pt->name = %s g_handle.pphandle_index = %d pphandle = 0x%08x nmhanle = 0x%x\n",lpentry->name,pt->name,g_handle.pphandle_index,g_handle.pphandle[g_handle.pphandle_index],*(unsigned int *)g_handle.pphandle[g_handle.pphandle_index]);
				g_handle.pphandle_index = 0;
				break;
			}
			g_handle.pphandle_index++;
		}
		printf("%s %d erase and open pt[%s] ok!\n",__func__,__LINE__,pt->name);
	}

	return ret;
}
