/*
 * Ingenic burn command
 *
 * Copyright (c) 2013 cli <cli@ingenic.cn>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/errno.h>
#include <asm/arch/clk.h>

struct ddr_timing1 {
	unsigned int tWL:6;
	unsigned int Reserved:2;
	unsigned int tWR:6;
	unsigned int Reserved1:2;
	unsigned int tWTR:6;
	unsigned int Reserved2:2;
	unsigned int tRTP:6;
	unsigned int Reserved3:2;
};
struct ddr_timing2 {
	unsigned int tRL:6;
	unsigned int Reserved:2;
	unsigned int tRCD:6;
	unsigned int Reserved1:2;
	unsigned int tRAS:6;
	unsigned int Reserved2:2;
	unsigned int tCCD:6;
	unsigned int Reserved3:2;
};

struct ddr_timing3 {
	unsigned int tRC:6;
	unsigned int Reserved:2;
	unsigned int tRRD:6;
	unsigned int Reserved1:2;
	unsigned int tRP:6;
	unsigned int Reserved2:2;
	unsigned int tCKSRE:3;
	unsigned int tONUM:3;
	unsigned int Reserved3:2;
};

struct ddr_timing4 {
	unsigned int tMRD:2;
	unsigned int Reserved:2;
	unsigned int tXP:3;
	unsigned int Reserved1:1;
	unsigned int tMINSR:4;
	unsigned int Reserved2:4;
	unsigned int tCKE:3;
	unsigned int tRWCOV:2;
	unsigned int tEXTRW:3;
	unsigned int tRFC:6;
	unsigned int Reserved3:2;
};

struct ddr_timing5 {
	unsigned int tWDLAT:6;
	unsigned int Reserved:2;
	unsigned int tRDLAT:6;
	unsigned int Reserved1:2;
	unsigned int tRTW:6;
	unsigned int Reserved2:2;
	unsigned int tCTLUPD:8;
};

struct ddr_timing6 {
	unsigned int tCFGR:6;
	unsigned int Reserved:2;
	unsigned int tCFGW:6;
	unsigned int Reserved1:2;
	unsigned int tFAW:6;
	unsigned int Reserved2:2;
	unsigned int tXSRD:8;
};
struct ddr_parameters {
	struct ddr_timing1 *timing1;
	struct ddr_timing2 *timing2;
	struct ddr_timing3 *timing3;
	struct ddr_timing4 *timing4;
	struct ddr_timing5 *timing5;
	struct ddr_timing6 *timing6;
};



static unsigned long long clk2ps(unsigned int clk)
{
	unsigned long long ddr_rate;
	ddr_rate = (unsigned long long)clk_get_rate(DDR);
	return 1000*1000*1000*1000LL / ddr_rate * clk;


}
#define ddr_timing_print(c,x) \
	printf("t"#x":			%d		%lld(ps)\n",ddr_p->timing##c->t##x, clk2ps(ddr_p->timing##c->t##x))

static int print_ddr_param(struct ddr_parameters *ddr_p)
{
	printf("=============================DDR_PARAMS START=======================\n");
	printf("timing 1\n");

	ddr_timing_print(1, WL);
	ddr_timing_print(1, WR);
	ddr_timing_print(1, WTR);
	ddr_timing_print(1, RTP);

	printf("timing 2\n");
	ddr_timing_print(2, RL);
	ddr_timing_print(2, RCD);
	ddr_timing_print(2, RAS);
	ddr_timing_print(2, CCD);

	printf("timing 3\n");
	ddr_timing_print(3, RC);
	ddr_timing_print(3, RRD);
	ddr_timing_print(3, RP);
	ddr_timing_print(3, CKSRE);
	ddr_timing_print(3, ONUM);

	printf("timing 4\n");
	ddr_timing_print(4, MRD);
	ddr_timing_print(4, XP);
	ddr_timing_print(4, MINSR);
	ddr_timing_print(4, CKE);
	ddr_timing_print(4, RWCOV);
	ddr_timing_print(4, EXTRW);
	ddr_timing_print(4, RFC);

	printf("timing 5\n");
	ddr_timing_print(5, WDLAT);
	ddr_timing_print(5, RDLAT);
	ddr_timing_print(5, RTW);
	ddr_timing_print(5, CTLUPD);

	printf("timing 6\n");
	ddr_timing_print(6, CFGR);
	ddr_timing_print(6, CFGW);
	ddr_timing_print(6, FAW);
	ddr_timing_print(6, XSRD);

	printf("=============================DDR_PARAMS  END =======================\n");
}

int ddr_param_debug(void)
{
	struct ddr_parameters ddr_param;

	ddr_param.timing1 = (struct ddr_timing1 *)(0xb34f0060);
	ddr_param.timing2 = (struct ddr_timing2 *)(0xb34f0064);
	ddr_param.timing3 = (struct ddr_timing3 *)(0xb34f0068);
	ddr_param.timing4 = (struct ddr_timing4 *)(0xb34f006C);
	ddr_param.timing5 = (struct ddr_timing5 *)(0xb34f0070);
	ddr_param.timing6 = (struct ddr_timing6 *)(0xb34f0074);

	print_ddr_param(&ddr_param);

	return 0;
}
static int do_ddr_param_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *str_env;
	struct ddr_parameters ddr_param;

	printf("%s %d %d\n",__FILE__,__LINE__,argc);
	if (argc > 1)
		return CMD_RET_USAGE;

	ddr_param.timing1 = (struct ddr_timing1 *)(0xb34f0060);
	ddr_param.timing2 = (struct ddr_timing2 *)(0xb34f0064);
	ddr_param.timing3 = (struct ddr_timing3 *)(0xb34f0068);
	ddr_param.timing4 = (struct ddr_timing4 *)(0xb34f006C);
	ddr_param.timing5 = (struct ddr_timing5 *)(0xb34f0070);
	ddr_param.timing6 = (struct ddr_timing6 *)(0xb34f0074);

	print_ddr_param(&ddr_param);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(ddr_param_read, 1, 1, do_ddr_param_read,
	"Ingenic ddr paramaters read",
	"no param\n"
);
