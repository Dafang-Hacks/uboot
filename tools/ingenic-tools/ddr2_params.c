#include "ddr_params_creator.h"
static struct ddr_out_impedance out_impedance[]={
	{40000,11},
};
static struct ddr_out_impedance odt_out_impedance[]={
	{150000,1},
	{75000,4},
	{50000,6},
};
static void fill_in_params_ddr2(struct ddr_params *ddr_params)
{
	struct ddr2_params *params = &ddr_params->private_params.ddr2_params;
	if(params->RL == -1  || params->WL == -1) {
		out_error("lpddr cann't surpport auto mode!\n");
		assert(1);
	}
	params->tMRD = DDR_tMRD;
	params->tXSNR = DDR_tXSNR;
	params->tXARD = DDR_tXARD;
	params->tXARDS = DDR_tXARDS;
	params->tXSRD = DDR_tXSRD;
	ddr_params->cl = DDR_CL;
	params->tCKESR = DDR_tCKESR;
	params->tCCD = DDR_tCCD;
	params->tFAW = DDR_tFAW;
	params->tRTP = DDR_tRTP;
}
static void ddrc_params_creator_ddr2(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	unsigned int tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tRTP,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing1.b.tRTP = tmp;

	ddrc->timing1.b.tWTR = p->cl + p->bl / 2 +
		ps2cycle_ceil(p->private_params.ddr2_params.tWTR,1) - 1;// write to read for our controller
	ASSERT_MASK(ddrc->timing1.b.tWTR,6);

	if(p->bl == 4)
		ddrc->timing5.b.tRTW = 4;
	else if(p->bl == 8)
		ddrc->timing5.b.tRTW = 6;
	else {
		out_error("DDR_BL(%d) error,only support 4 or 8\n",p->bl);
		assert(1);
	}

	if(ddrc->timing1.b.tWL > 0)
		ddrc->timing5.b.tWDLAT = ddrc->timing1.b.tWL - 1;
	else{
		out_error("DDR_WL too small! check %s %d\n",__FILE__,__LINE__);
		assert(1);
	}
	if(ddrc->timing2.b.tRL > 1)
		ddrc->timing5.b.tRDLAT = ddrc->timing2.b.tRL - 2;
	else{
		out_error("DDR_RL too small! check %s %d\n",__FILE__,__LINE__);
		assert(1);
	}

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tCCD,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tCCD = tmp;

	ddrc->timing3.b.tCKSRE = 0;
	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tMRD,1) - 1;
	ASSERT_MASK(tmp,2);
	ddrc->timing4.b.tMRD = tmp;

	tmp = MAX(ps2cycle_ceil(p->private_params.ddr2_params.tXSNR,4),
		ps2cycle_ceil(p->private_params.ddr2_params.tXSRD,4));

	tmp = tmp / 4;
	ASSERT_MASK(tmp,8);
	ddrc->timing6.b.tXSRD = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tCKESR,8) / 8 - 1 ;
	if(tmp < 0)
		tmp = 0;
	ASSERT_MASK(tmp,4);
	ddrc->timing4.b.tMINSR = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tFAW,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing6.b.tFAW = tmp;
}
static void ddrp_params_creator_ddr2(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	unsigned int tmp = 0;
	struct ddr_out_impedance *impedance;
	struct ddr_out_impedance *odt_impedance;
	/* MRn registers */
	if(p->bl == 4)
		ddrp->mr0.ddr2.BL = 2;
	else if(p->bl == 8)
		ddrp->mr0.ddr2.BL = 3;
	else{
		out_error("DDR_BL(%d) error,only support 4 or 8.check %s,%d\n",p->bl,__FILE__,__LINE__);
		assert(1);
	}
	if(p->cl >= 2 && p->cl <= 7) // debug default 6.
		ddrp->mr0.ddr2.CL = p->cl;
	else{
		out_error("DDR_CL(%d) error,it should be between 2 and 6. check %s,%d\n",p->cl,__FILE__,__LINE__);
		assert(1);
	}

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tWR, 1);
	BETWEEN(tmp,2,9);  // debug, BETWEEN(tmp,2,6)
	ddrp->mr0.ddr2.WR = tmp - 1;

#ifdef DDR2_CHIP_DRIVER_OUT_STRENGTH
	ddrp->mr1.ddr2.DIC = DDR2_CHIP_DRIVER_OUT_STRENGTH;
#else
	ddrp->mr1.ddr2.DIC = 1; /* Impedance=RZQ/7 */
#endif

#ifdef CONFIG_DDR_CHIP_ODT
	ddrp->mr1.ddr2.RTT2 = CONFIG_DDR_CHIP_ODT; /* Effective resistance of ODT RZQ/4 */
#endif

	ddrp->ptr1.b.tDINIT0 = ps2cycle_ceil(200*1000*1000, 1); /* DDR2 default 200us*/
	ddrp->ptr1.b.tDINIT1 = ps2cycle_ceil(400 * 1000, 1); /* DDR2 default 400ns*/

	/* DTPR0 registers */
	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tMRD,1);
	if(!((tmp == 2) || (tmp == 3))){
		out_error("DDR2_tMRD(%d) error, tMRD is only 2 or 3 for DDR2. check %s,%d\n",
			  p->private_params.ddr2_params.tMRD,__FILE__,__LINE__);
		assert(1);
	}
	ddrp->dtpr0.b.tMRD = tmp;

	/* AL = 0,other's cann't support by controller. */
	tmp = p->bl / 2 +
		MAX(ps2cycle_ceil(p->private_params.ddr2_params.tRTP,1),2) - 2;
	BETWEEN(tmp,2,7);
	ddrp->dtpr0.b.tRTP = tmp;
	ddrp->dtpr0.b.tCCD = 0;

	/* DTPR1 registers */
	ddrp->dtpr1.b.tAOND_tAOFD = 0;  //non-standard DDR2 will be setting to 1.

	DDRP_TIMING_SET(1,ddr2_params,tFAW,6,2,31);

	/* DTPR2 registers */
	tmp = MAX(
		ps2cycle_ceil(p->private_params.ddr2_params.tXSNR,1),
		ps2cycle_ceil(p->private_params.ddr2_params.tXSRD,1));
	BETWEEN(tmp,2,1023);
	ddrp->dtpr2.b.tXS = tmp;

	tmp = MAX(
		ps2cycle_ceil(p->private_params.ddr2_params.tXP,1),
		ps2cycle_ceil(p->private_params.ddr2_params.tXARD,1)
		);
	tmp = MAX(
		tmp,
		ps2cycle_ceil(p->private_params.ddr2_params.tXARDS,1)
		);
	BETWEEN(tmp, 2, 31);
	ddrp->dtpr2.b.tXP = tmp;

	tmp = MAX(
		ps2cycle_ceil(p->private_params.ddr2_params.tCKESR,1),
		ps2cycle_ceil(p->private_params.ddr2_params.tCKE,1)
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
	odt_impedance = find_nearby_impedance(odt_out_impedance,sizeof(odt_out_impedance),CONFIG_DDR_PHY_ODT_IMPEDANCE);
	ddrp->odt_impedance[0] = odt_impedance->r;
	ddrp->odt_impedance[1] = CONFIG_DDR_PHY_ODT_IMPEDANCE;
	ddrp->zqncr1 = (odt_impedance->index << 4) | impedance->index;

}
static struct ddr_creator_ops ddr2_creator_ops = {
	.type = DDR2,
	.fill_in_params = fill_in_params_ddr2,
	.ddrc_params_creator = ddrc_params_creator_ddr2,
	.ddrp_params_creator = ddrp_params_creator_ddr2,

};
#ifdef CONFIG_DDR_TYPE_DDR2
void ddr_creator_init(void)
{
	register_ddr_creator(&ddr2_creator_ops);
}
#endif
