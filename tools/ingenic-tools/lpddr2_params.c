#include "ddr_params_creator.h"
struct ddr_latency_table
{
	unsigned int freq;
	int latency;
};
static struct ddr_latency_table rl_LPDDR2[] = {
	{100000000,3},/*memclk xxM, RL*/
	{150000000,3},
	{200000000,4},
	{300000000,5},
	{400000000,6},
	{450000000,7},
	{500000000,8},
};

static struct ddr_latency_table wl_LPDDR2[]= {
	{100000000,1},/*memclk xxM, WL*/
	{150000000,1},
	{200000000,2},
	{300000000,2},
	{400000000,3},
	{450000000,4},
	{500000000,4},
};
static struct ddr_out_impedance out_impedance[]={
	{80000,5},
	{60000,7},
	{48000,9},
	{40000,11},
	{34000,13},
};
static int find_ddr_lattency(struct ddr_latency_table *table,int size,unsigned int freq)
{
	int i;
	for(i = 0;i < size / sizeof(struct ddr_latency_table);i++)
	{
		if(freq < table[i].freq) {
			return table[i].latency;
		}
	}
	return -1;
}
static void fill_in_params_lpddr2(struct ddr_params *ddr_params)
{
	int tmp;
	struct lpddr2_params *params = &ddr_params->private_params.lpddr2_params;


	params->tDQSCK = DDR_tDQSCK;
	params->tDQSCKMAX = DDR_tDQSCKMAX;
	params->tXSR = DDR_tXSR;
	params->tCKESR = DDR_tCKESR;
	params->tRTP = DDR_tRTP;
	params->tCCD = DDR_tCCD;
	params->tFAW = DDR_tFAW;
	if(params->RL == -1)
	{
		tmp = find_ddr_lattency(rl_LPDDR2,sizeof(rl_LPDDR2),ddr_params->freq);
		if(tmp == -1) {
			out_error("it cann't find RL latency,when ddr frequancy is %d.check %s %d\n",
				  ddr_params->freq,__FILE__,__LINE__);
			assert(1);
		}
		params->RL = tmp * __ps_per_tck;
	}
	if(params->WL == -1)
	{
		tmp = find_ddr_lattency(wl_LPDDR2,sizeof(wl_LPDDR2),ddr_params->freq);
		if(tmp == -1) {
			out_error("it cann't find WL latency,when ddr frequancy is %d. check %s %d\n",
				  ddr_params->freq,__FILE__,__LINE__);
			assert(1);
		}
		params->WL = tmp * __ps_per_tck;
	}

}

static void ddrc_params_creator_lpddr2(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	int tmp;
	ddrc->timing3.b.tCKSRE = 0; // lpddr2 not used.

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tRTP,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing1.b.tRTP = tmp;

	ddrc->timing1.b.tWTR =
		ps2cycle_ceil(p->private_params.lpddr2_params.WL,1) + 1 +
		+ p->bl / 2 + ps2cycle_ceil(p->private_params.lpddr2_params.tWTR,1); // write to read for our controller

	ASSERT_MASK(ddrc->timing1.b.tWTR,6);

	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.tCCD,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tCCD = tmp;


	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.tCKESR,8) / 8 - 1 ;
	if(tmp < 0)
		tmp = 0;
	ASSERT_MASK(tmp,4);
	ddrc->timing4.b.tMINSR = tmp;

	ddrc->timing4.b.tMRD = 0;

	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.RL +
			    p->private_params.lpddr2_params.tDQSCKMAX -
			    p->private_params.lpddr2_params.WL,1) + p->bl / 2;

	ASSERT_MASK(tmp,6);
	ddrc->timing5.b.tRTW = tmp;

	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.WL,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing5.b.tWDLAT = tmp;
	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.RL + p->private_params.lpddr2_params.tDQSCK,1);
	tmp = tmp - 2;
	ASSERT_MASK(tmp,6);

	ddrc->timing5.b.tRDLAT = tmp;

	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.tXSR,4) / 4;
	ASSERT_MASK(tmp,8);
	ddrc->timing6.b.tXSRD = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr_base_params.tFAW,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing6.b.tFAW = tmp;
}


static void ddrp_params_creator_lpddr2(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	int tmp;
	int rl = 0,wl = 0;
	int  count = 0;
	struct ddr_out_impedance *impedance;
	/* MRn registers */
	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.tWR, 1);
	ASSERT_MASK(tmp,3);
	BETWEEN(tmp,3,8);
	ddrp->mr1.lpddr2.nWR = tmp - 2;
	if(!(p->bl == 4 || p->bl == 8 || p->bl == 16)) {
		out_error("BL(%d) should is 4 or 8 or 16\n", p->bl);
		assert(1);
	}
	tmp = p->bl;
	while (tmp >>= 1) count++;
	ddrp->mr1.lpddr2.BL = count;
	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.RL,1);
	if(tmp < 3 ||
	   tmp > 8)
	{
		out_error("the PHY don't support the RL(%d) \n",p->private_params.lpddr2_params.RL);
		assert(1);
	}

	rl = tmp;
	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.WL,1);
	if(tmp < 1 ||
	   tmp > 4)
	{
		out_error("the PHY don't support the WL(%d) \n",p->private_params.lpddr2_params.WL);
		assert(1);
	}
	wl = tmp;

	tmp = wl | (rl << 4);

	switch(tmp)
	{
	case 0x31:
		tmp = 1;
		break;
	case 0x42:
		tmp = 2;
		break;
	case 0x52:
		tmp = 3;
		break;
	case 0x63:
		tmp = 4;
		break;
	case 0x74:
		tmp = 5;
		break;
	case 0x84:
		tmp = 6;
		break;
	default:
		out_error("the PHY don't support the WL(%d) or RL(%d)\n",
			  p->private_params.lpddr2_params.WL,p->private_params.lpddr2_params.RL);
		assert(1);
	}
	ddrp->mr2.lpddr2.RL_WL = tmp;

#ifdef DDR_DRIVER_STRENGTH
	ddrp->mr3.lpddr2.DS = DDR_DRIVER_STRENGTH;
#else
	ddrp->mr3.lpddr2.DS = 2;
	out_warn("Warnning: Please set ddr driver strength.");
#endif

	ddrp->ptr1.b.tDINIT0 = ps2cycle_ceil(200000 * 1000, 1); /* LPDDR2 default 200us*/
	tmp = ps2cycle_ceil(100 * 1000, 1); /* LPDDR2 default 100 ns*/
	ddrp->ptr1.b.tDINIT1 = tmp;

	ddrp->ptr2.b.tDINIT2 = ps2cycle_ceil(11000 * 1000, 1); /* LPDDR2 default 11 us*/
	ddrp->ptr2.b.tDINIT3 = ps2cycle_ceil(1000 *1000, 1); /* LPDDR2 default 1 us*/

	/* DTPR0 registers */
	ddrp->dtpr0.b.tMRD = 0; /* LPDDR2 no use, don't care */
	DDRP_TIMING_SET(0,lpddr2_params,tRTP,3,2,6);
	ddrp->dtpr0.b.tCCD = 0; /* LPDDR2 no use, don't care */
	/* DTPR1 registers */
//	ddrp->dtpr1.b.tRTW = 1; /* add 1 tck for test */
	DDRP_TIMING_SET(1,lpddr2_params,tDQSCK,3,0,7);
	DDRP_TIMING_SET(1,lpddr2_params,tDQSCKMAX,3,1,7);
	DDRP_TIMING_SET(1,lpddr2_params,tFAW,6,2,31);

	/* DTPR2 registers */
	tmp = ps2cycle_ceil(p->private_params.lpddr2_params.tXSR, 1);  // the controller is same.
	ASSERT_MASK(tmp,10);
	BETWEEN(tmp, 2, 1023);
	ddrp->dtpr2.b.tXS = tmp;

	DDRP_TIMING_SET(2,lpddr2_params,tXP,5,2,31);
	tmp = MAX(
		ps2cycle_ceil(p->private_params.lpddr2_params.tCKESR,1),
		ps2cycle_ceil(p->private_params.lpddr2_params.tCKE,1)
		);

	BETWEEN(tmp, 2, 15);
	ddrp->dtpr2.b.tCKE = tmp;

	/* PGCR registers */
	ddrp->pgcr = DDRP_PGCR_DQSCFG | 7 << DDRP_PGCR_CKEN_BIT
		| 2 << DDRP_PGCR_CKDV_BIT
		| (p->cs0 | p->cs1 << 1) << DDRP_PGCR_RANKEN_BIT
		| DDRP_PGCR_ZCKSEL_32 | DDRP_PGCR_PDDISDX;


	impedance = find_nearby_impedance(out_impedance,sizeof(out_impedance),CONFIG_DDR_PHY_IMPEDANCE);
	ddrp->impedance[0] = impedance->r;
	ddrp->impedance[1] = CONFIG_DDR_PHY_IMPEDANCE;
	ddrp->odt_impedance[0] = 40000;
	ddrp->odt_impedance[1] = 40000;

	ddrp->zqncr1 = (0xb << 4) | impedance->index;//7 - is odt impedance default.
}
static struct ddr_creator_ops lpddr2_creator_ops = {
	.type = LPDDR2,
	.fill_in_params = fill_in_params_lpddr2,
	.ddrc_params_creator = ddrc_params_creator_lpddr2,
	.ddrp_params_creator = ddrp_params_creator_lpddr2,

};
#ifdef CONFIG_DDR_TYPE_LPDDR2
void ddr_creator_init(void)
{
	register_ddr_creator(&lpddr2_creator_ops);
}
#endif
