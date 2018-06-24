#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <search.h>
#include <errno.h>
#include <../drivers/nand/manager/inc/lpartition.h>
#include <../drivers/nand/manager/inc/sectorlist.h>

#if defined(CONFIG_CMD_SAVEENV) && defined(CONFIG_CMD_NAND)
#define CMD_SAVEENV
#endif

#if defined(ENV_IS_EMBEDDED)
env_t *env_ptr = &environment;
#elif defined(CONFIG_NAND_ENV_DST)
env_t *env_ptr = (env_t *)CONFIG_NAND_ENV_DST;
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr;
#endif /* ENV_IS_EMBEDDED */

DECLARE_GLOBAL_DATA_PTR;


struct Ghandle{
	int zm_handle;
	unsigned int sectorid;
	int  pphandle;
	LPartition *lp;
};
struct Ghandle g_handle;

/*
 * This is called before nand_init() so we can't read NAND to
 * validate env data.
 *
 * Mark it OK for now. env_relocate() in env_common.c will call our
 * relocate function which does the real validation.
 *
 * When using a NAND boot image (like sequoia_nand), the environment
 * can be embedded or attached to the U-Boot image in NAND flash.
 * This way the SPL loads not only the U-Boot image from NAND but
 * also the environment.
 */
int env_init(void)
{
#if defined(ENV_IS_EMBEDDED) || defined(CONFIG_NAND_ENV_DST)
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1;

	tmp_env1 = env_ptr;
	crc1_ok = crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc;

	if (!crc1_ok && !crc2_ok) {
		gd->env_addr	= 0;
		gd->env_valid	= 0;

		return 0;
	} else if (crc1_ok && !crc2_ok) {
		gd->env_valid = 1;
	}
	if (gd->env_valid == 1)
		env_ptr = tmp_env1;

	gd->env_addr = (ulong)env_ptr->data;

#else /* ENV_IS_EMBEDDED || CONFIG_NAND_ENV_DST */
	gd->env_addr	= (ulong)&default_environment[0];
	gd->env_valid	= 1;
#endif /* ENV_IS_EMBEDDED || CONFIG_NAND_ENV_DST */

	return 0;
}

static int get_pt_handle(char *pt_name)
{
	struct singlelist *it;
	LPartition *lpentry;

	singlelist_for_each(it,&(g_handle.lp->head)){
		lpentry = singlelist_entry(it,LPartition,head);
		if(strcmp(lpentry->name,pt_name) == 0){
			g_handle.pphandle = NandManger_ptOpen(g_handle.zm_handle,lpentry->name,lpentry->mode);
			break;
		}
	}

	if(!g_handle.pphandle){
		printf("ERROR: the partition %s is not open ,pphandle is -1 ,please check the partition name ,if not have the partition,please add it\n",pt_name);
		return CMD_RET_FAILURE;
	}
}

/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
int writeenv(size_t offset, u_char *buf)
{

	int bl;
	unsigned int len;
	unsigned int write_len;
	SectorList *sl;
	unsigned char *databuf = buf;

	len = CONFIG_ENV_SIZE;
	//len = min(blocksize, CONFIG_ENV_SIZE);

	g_handle.sectorid = offset / 512;

	bl = BuffListManager_BuffList_Init();
	if(bl == 0){
		printf("BuffListManager Init failed!\n");
		return CMD_RET_FAILURE;
	}
	while(len){
		if(len >= 256 * 512)
			write_len = 256 * 512;
		else{
			write_len = len;
			if(write_len % 512 != 0)
				memset(databuf+write_len, 0xff, 512 - write_len%512);
		}
		sl = BuffListManager_getTopNode(bl,sizeof(SectorList));
		if(sl == 0){
			printf("Bufferlist request sectorlist failed!\n");
			return CMD_RET_FAILURE;
		}
		sl->startSector = g_handle.sectorid;
		sl->pData = (void*)databuf;
		sl->sectorCount = (write_len + 511)/ 512;
		g_handle.sectorid += (write_len + 511)/ 512;

		if(NandManger_ptWrite(g_handle.pphandle,sl) < 0){
			printf("NandManger_ptRead failed, now rewrite!\n");
		}
		databuf += write_len;
		len -= write_len;
	}

	return 0;
}

int saveenv(void)
{
	int	ret = 0;
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	ssize_t	len;
	char	*res;


	res = (char *)&env_new->data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new->crc = crc32(0, env_new->data, ENV_SIZE);

	puts("Writing to Nand... ");
	if (ret = writeenv(CONFIG_ENV_OFFSET, (u_char *)env_new)) {
		puts("FAILED!\n");
		return 1;
	}

	puts("done\n");
	return ret;
}

int readenv(size_t offset, u_char *buf)
{

	int bl;
	unsigned int len;
	unsigned int read_len;
	SectorList *sl;
	unsigned char *databuf = buf;

	len = CONFIG_ENV_SIZE;
	//len = min(blocksize, CONFIG_ENV_SIZE);

	g_handle.sectorid = offset / 512;

	bl = BuffListManager_BuffList_Init();
	if(bl == 0){
		printf("BuffListManager Init failed!\n");
		return CMD_RET_FAILURE;
	}
	while(len){
		if(len >= 256 * 512)
			read_len = 256 * 512;
		else{
			read_len = len;
			if(read_len % 512 != 0)
				memset(databuf+read_len, 0xff, 512 - read_len%512);
		}
		sl = BuffListManager_getTopNode(bl,sizeof(SectorList));
		if(sl == 0){
			printf("Bufferlist request sectorlist failed!\n");
			return CMD_RET_FAILURE;
		}
		sl->startSector = g_handle.sectorid;
		sl->pData = (void*)databuf;
		sl->sectorCount = (read_len + 511)/ 512;
		g_handle.sectorid += (read_len + 511)/ 512;

		if(NandManger_ptRead(g_handle.pphandle,sl) < 0){
			printf("NandManger_ptRead failed, now rewrite!\n");
		}
		databuf += read_len;
		len -= read_len;
	}

	return 0;
}

/*
 * The legacy NAND code saved the environment in the first NAND
 * device i.e., nand_dev_desc + 0. This is also the behaviour using
 * the new NAND code.
 */
void env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	int ret = 0;
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);

	get_pt_handle(CMDLINE_PARTITION);

	ret = readenv(CONFIG_ENV_OFFSET, (u_char *)buf);
	if (ret) {
		set_default_env("!readenv() failed");
		return;
	}

	env_import(buf, 1);
#endif /* ! ENV_IS_EMBEDDED */
}
