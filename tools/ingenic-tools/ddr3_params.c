#include "ddr_params_creator.h"
static struct ddr_out_impedance out_impedance[]={
	{40,11},
	{34,13},
};
static struct ddr_out_impedance odt_out_impedance[]={
	{120,1},
	{60,5},
	{40,8},
};

static void fill_in_params_ddr3(struct ddr_params *ddr_params)
{
	struct ddr3_params *params = &ddr_params->private_params.ddr3_params;
	if(params->RL == -1  || params->WL == -1) {
		out_error("lpddr cann't surpport auto mode!\n");
		assert(1);
	}
	params->tMRD = DDR_tMRD;
	params->tXSDLL = DDR_tXSDLL;
	params->tMOD = DDR_tMOD;
	params->tXPDLL = DDR_tXPDLL;
	params->tCKESR = DDR_tCKESR;
	params->tCKSRE = DDR_tCKSRE;
	params->tXS = DDR_tXS;
	params->tRTP = DDR_tRTP;
	params->tCCD = DDR_tCCD;
	params->tFAW = DDR_tFAW;
	ddr_params->cl = DDR_CL;

}
static void ddrc_params_creator_ddr3(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	int tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tRTP,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing1.b.tRTP = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tCCD,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tCCD = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tCKSRE,8) / 8;
	ASSERT_MASK(tmp,3);
	ddrc->timing3.b.tCKSRE = tmp;


	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tMRD,1) - 1;
	ASSERT_MASK(tmp,2);
	ddrc->timing4.b.tMRD = tmp;

	ddrc->timing1.b.tWTR =
		ps2cycle_ceil(p->private_params.ddr3_params.WL,1) +
		+ p->bl / 2 + ps2cycle_ceil(p->private_params.ddr3_params.tWTR,1); // write to read for our controller
	ASSERT_MASK(ddrc->timing1.b.tWTR,6);

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.RL +
			    p->private_params.ddr3_params.tCCD -
			    p->private_params.ddr3_params.WL,1) + 2;

	ASSERT_MASK(tmp,6);
	ddrc->timing5.b.tRTW = tmp;

	ddrc->timing5.b.tWDLAT = ddrc->timing1.b.tWL - 1;
	ddrc->timing5.b.tRDLAT = ddrc->timing2.b.tRL - 2;

	tmp = MAX(ps2cycle_ceil(p->private_params.ddr3_params.tXS,4),
		ps2cycle_ceil(p->private_params.ddr3_params.tXSDLL,4));
	tmp = MAX(tmp,
		ps2cycle_ceil(p->private_params.ddr3_params.tXPDLL,4));
	tmp = tmp / 4;
	ASSERT_MASK(tmp,8);
	ddrc->timing6.b.tXSRD = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tCKESR,8) / 8 - 1 ;
	if(tmp < 0)
		tmp = 0;
	ASSERT_MASK(tmp,4);
	ddrc->timing4.b.tMINSR = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tFAW,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing6.b.tFAW = tmp;

}


static void ddrp_params_creator_ddr3(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	int tmp = 0;
	struct ddr_out_impedance *impedance;
	struct ddr_out_impedance *odt_impedance;

	/* MRn registers */
	/* BL: 1 is on the fly,???? */
	if(p->bl == 4)
		ddrp->mr0.ddr3.BL = 2;
	else if(p->bl == 8)
		ddrp->mr0.ddr3.BL = 0;
	else{
		out_error("DDR_BL(%d) error,only support 4 or 8\n",p->bl);
		assert(1);
	}
	ddrp->mr0.ddr3.BL = (8 - p->bl) / 2;

	BETWEEN(p->cl,5,11);
	ddrp->mr0.ddr3.CL_4_6 = p->cl - 4;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tWR, 1);
	switch(tmp)
	{
	case 5 ... 8:
		ddrp->mr0.ddr3.WR = tmp - 4;
		break;
	case 9 ... 12:
		ddrp->mr0.ddr3.WR = (tmp + 1) / 2;
		break;
	default:
		out_error("tWR(%d) is error, valid value is between from 5 to 12.\n",
		       p->private_params.ddr3_params.tWR);
		assert(1);
	}

#ifdef CONFIG_DDR_DLL_OFF
	ddrp->mr0.ddr3.PD = 0;
#else
	ddrp->mr0.ddr3.PD = 1;
#endif

	/* MR1 register. */
#ifdef CONFIG_DDR_DLL_OFF
	ddrp->mr1.ddr3.DE = 1; /* DLL disable. */
#else
	ddrp->mr1.ddr3.DE = 0; /* DLL enable. */
#endif

#ifdef CONFIG_DDR_DRIVER_OUT_STRENGTH
	/*   00 - RZQ/6,01 - RZQ / 7 */
	ddrp->mr1.ddr3.DIC1 = CONFIG_DDR_DRIVER_OUT_STRENGTH;
	BETWEEN(ddrp->mr1.ddr3.DIC1,0,1);
#else
	ddrp->mr1.ddr3.DIC1 = 1; /* Impedance=RZQ/7 */
#endif

#ifdef CONFIG_DDR_CHIP_ODT_VAL
	/**********************
	 * 000 - ODT disable. *
	 * 001 - RZQ/4.       *
	 * 010 - RZQ/2.       *
	 * 011 - RZQ/6.       *
	 * 100 - RZQ/12.      *
	 * 101 - RZQ/8.       *
	 **********************/
	ddrp->mr1.ddr3.RTT2 = CONFIG_DDR_CHIP_ODT_VAL; /* Effective resistance of ODT RZQ/4 */
	BETWEEN(ddrp->mr1.ddr3.RTT2,0,5);
#endif

	//////////////////////////////////////////////////////////
	tmp = -1;
	if(__ps_per_tck <= 2500)
		tmp = 5;
	else if(__ps_per_tck < 1875)
		tmp = 6;
	else if(__ps_per_tck < 1500)
		tmp = 7;
	else if(__ps_per_tck < 1250)
		tmp = 8;
	else {
		out_error("ddr frequancy too fast. %d\n",p->freq);
		out_error(". %d\n",__ps_per_tck);
		assert(1);
	}
	ddrp->mr2.ddr3.CWL = tmp - 5;

	ddrp->ptr1.b.tDINIT0 = ps2cycle_ceil(500*1000*1000, 1); /* DDR3 default 500us*/

	tmp = MAX(
		ps2cycle_ceil(p->private_params.ddr3_params.tRFC + 10*1000,1),  /* tRFC + 10ns */
		5);
	ASSERT_MASK(tmp,8);
	ddrp->ptr1.b.tDINIT1 = tmp;


	ddrp->ptr2.b.tDINIT2 = ps2cycle_ceil(200 * 1000 * 1000,1); /* DDR3 default 200us*/
	ddrp->ptr2.b.tDINIT3 = 512;

	/* DTPR_COMMON_SETTING(ddr3_params); */
/* DTPR0 registers */
	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tMRD,1);
	BETWEEN(tmp,4,7);
	ddrp->dtpr0.b.tMRD = tmp - 4;

	DDRP_TIMING_SET(0,ddr3_params,tRTP,3,2,6);

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tCCD,1);
	ASSERT_MASK(tmp - 4, 1);
	BETWEEN(tmp,4,5);
	ddrp->dtpr0.b.tCCD = tmp - 4;

	DDRP_TIMING_SET(1,ddr3_params,tFAW,6,2,31);

/* DTPR1 registers */
	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tMOD,1);
	BETWEEN(tmp,12,15);
	ddrp->dtpr1.b.tMOD = tmp - 12;
	DDRP_TIMING_SET(1,ddr3_params,tRFC,8,0,255);

#if CONFIG_DDR_CHIP_ODT_VAL
	ddrp->dtpr1.b.tRTODT = 1; // for low power consumption.
#endif

/* DTPR2 registers */
	tmp = MAX(
		ps2cycle_ceil(p->private_params.ddr3_params.tXSDLL,1),
		ps2cycle_ceil(p->private_params.ddr3_params.tXS,1)
		);
	BETWEEN(tmp, 2, 1023);
	ddrp->dtpr2.b.tXS = tmp;

	tmp = MAX(
		ps2cycle_ceil(p->private_params.ddr3_params.tXPDLL,1),
		ps2cycle_ceil(p->private_params.ddr3_params.tXP,1)
		);
	BETWEEN(tmp, 2, 31);
	ddrp->dtpr2.b.tXP = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tCKESR,1);
	if(tmp < ps2cycle_ceil(p->private_params.ddr3_params.tCKE,1))
	{
		out_error("tCKESR(%d) should be great or equal tCKE (%d).\n",
		       p->private_params.ddr3_params.tCKESR,
		       p->private_params.ddr3_params.tCKE);
		assert(1);
	}
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
static struct ddr_creator_ops ddr3_creator_ops = {
	.type = DDR3,
	.fill_in_params = fill_in_params_ddr3,
	.ddrc_params_creator = ddrc_params_creator_ddr3,
	.ddrp_params_creator = ddrp_params_creator_ddr3,

};
#ifdef CONFIG_DDR_TYPE_DDR3
void ddr_creator_init(void)
{
	register_ddr_creator(&ddr3_creator_ops);
}
#endif
