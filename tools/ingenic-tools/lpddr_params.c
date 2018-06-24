#include "ddr_params_creator.h"
static void fill_in_params_lpddr(struct ddr_params *ddr_params)
{
	struct lpddr_params *params = &ddr_params->private_params.lpddr_params;
	params->tXSR = DDR_tXSR;
	params->tMRD = DDR_tMRD;
	params->tDQSSMAX = DDR_tDQSSMAX;
	ddr_params->cl = DDR_CL;
}

static void ddrc_timing_creator_lpddr(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	int tmp;

	ddrc->timing3.b.tCKSRE = 0;
	ddrc->timing1.b.tWL = 1;
	ddrc->timing2.b.tRL = p->cl;

	tmp =  p->bl / 2;
	ASSERT_MASK(tmp,6);
	ddrc->timing1.b.tRTP = tmp;

	tmp = ps2cycle_ceil(p->private_params.lpddr_params.tMRD,1) - 1;
	ASSERT_MASK(tmp,2);
	ddrc->timing4.b.tMRD = tmp;

	tmp = p->bl / 2;
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tCCD = tmp;

	ddrc->timing1.b.tWTR =
		ps2cycle_ceil(p->private_params.lpddr_params.tWTR + p->private_params.lpddr_params.tDQSSMAX,1) +
		p->bl / 2; // write to read for our controller
	ASSERT_MASK(ddrc->timing1.b.tWTR,6);

	tmp = p->cl + p->bl / 2;
	ASSERT_MASK(tmp,6);
	ddrc->timing5.b.tRTW = tmp;

	if(ddrc->timing1.b.tWL > 0)
		ddrc->timing5.b.tWDLAT = ddrc->timing1.b.tWL - 1;
	else{
		out_error("DDR_tWL too small! check %s %d\n",__FILE__,__LINE__);
		assert(1);
	}
	if(ddrc->timing2.b.tRL > 1)
		ddrc->timing5.b.tRDLAT = ddrc->timing2.b.tRL - 2;
	else{
		out_error("DDR_tRL too small! check %s %d\n",__FILE__,__LINE__);
	}

	tmp = ps2cycle_ceil(p->private_params.lpddr_params.tXSR,4) / 4;
	ASSERT_MASK(tmp,8);
	ddrc->timing6.b.tXSRD = tmp;

	ddrc->timing6.b.tXSRD = 0x80;

	tmp = ps2cycle_ceil(p->private_params.lpddr_params.tRFC,8) / 8 - 1;
	if(tmp < 0)
		tmp = 0;

	ASSERT_MASK(tmp,4);
	ddrc->timing4.b.tMINSR = tmp;
//debug.
	ddrc->timing4.b.tMINSR = 0xf;
	ddrc->timing6.b.tFAW = 0;

}

static void ddrp_params_creator_lpddr(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	int tmp;
	int  count = 0;

	/* MRn registers */
	tmp = p->cl;
	ASSERT_MASK(tmp,3);
	BETWEEN(tmp, 2, 4);
	ddrp->mr0.lpddr.CL = tmp;

	tmp = p->bl;
	while (tmp >>= 1) count++;
	ASSERT_MASK(count,3);

	ddrp->mr0.lpddr.BL = count;

	ddrp->mr2.lpddr.PASR = 0;
	ddrp->mr2.lpddr.TCSR = 3;  //85 degree centigrade.
#ifdef DDR_DRIVER_STRENGTH
	ddrp->mr2.lpddr.DS = DDR_DRIVER_STRENGTH;
#endif

	ddrp->ptr1.b.tDINIT0 = ps2cycle_ceil(200000000, 1); /* LPDDR default 200us*/
	ddrp->ptr1.b.tDINIT1 = 192;  /* default 192,LPDDR don't used */

	ddrp->ptr2.b.tDINIT2 = 106610;  /* default 106610, LPDDR don't used */
	ddrp->ptr2.b.tDINIT3 = 534;     /* default 534,    LPDDR don't used */

		/* DTPR0 registers */
	DDRP_TIMING_SET(0,lpddr_params,tMRD,2,0,3);

	tmp = p->bl/2;
	if(tmp <= 2 || tmp >= 6)
		out_error("tRTP %d is Out of range and check %s %d!\n",tmp,__FILE__,__LINE__);
	ddrp->dtpr0.b.tRTP = tmp;

	ddrp->dtpr0.b.tCCD = 0; // default 0, lpddr don't used.

	ddrp->dtpr1.b.tFAW = 18;

	/* DTPR2 registers */
	tmp = ps2cycle_ceil(p->private_params.lpddr_params.tXSR, 1);  // the controller is same.
	ASSERT_MASK(tmp,10);
	BETWEEN(tmp, 2, 1023);
	ddrp->dtpr2.b.tXS = tmp;

	DDRP_TIMING_SET(2,lpddr_params,tXP,5,2,31);

	tmp = ps2cycle_ceil(p->private_params.lpddr_params.tCKE,1);
	BETWEEN(tmp, 2, 15);
	ddrp->dtpr2.b.tCKE = tmp;

	/* PGCR registers */
	ddrp->pgcr = DDRP_PGCR_ITMDMD | DDRP_PGCR_DQSCFG | 7 << DDRP_PGCR_CKEN_BIT
		| 2 << DDRP_PGCR_CKDV_BIT
		| (p->cs0 | p->cs1 << 1) << DDRP_PGCR_RANKEN_BIT
		| DDRP_PGCR_PDDISDX;
	ddrp->zqncr1 = 0x7b;   // default. lpddr do not used.
}

static struct ddr_creator_ops lpddr_creator_ops = {
	.type = LPDDR,
	.fill_in_params = fill_in_params_lpddr,
	.ddrc_params_creator = ddrc_timing_creator_lpddr,
	.ddrp_params_creator = ddrp_params_creator_lpddr,
};
#ifdef CONFIG_DDR_TYPE_LPDDR
void ddr_creator_init(void)
{
	register_ddr_creator(&lpddr_creator_ops);
}
#endif
