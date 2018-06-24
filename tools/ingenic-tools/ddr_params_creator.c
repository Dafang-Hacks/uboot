/*
 * Jz4775 ddr parameters creator.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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
#include "ddr_params_creator.h"

unsigned int __ps_per_tck = -1;
/**
 * ps2cycle:   translate timing(ps) to clk count
 *       ps:   timing(ps)
 *  div_clk:   divider for register.
 *			   eg:
 *                    regcount = ps2cycle(x,div_tck) / div_tck.
 *                    div_tck is divider.
 */

int ps2cycle_ceil(int ps,int div_tck)
{
	return (ps + div_tck * __ps_per_tck - 1) / __ps_per_tck;
}

int ps2cycle_floor(int ps)
{
	return ps / __ps_per_tck;
}
struct ddr_out_impedance* find_nearby_impedance(struct ddr_out_impedance *table,int table_size,int r_ohm)
{
	int i;
	if(r_ohm >= table[0].r)
		return &table[0];
	for(i = 1;i < table_size/sizeof(struct ddr_out_impedance);i++){
		int diff = (table[i].r + table[i-1].r) / 2;
		if(r_ohm > diff)
			return &table[i - 1];
	}
	return &table[i - 1];
}
static unsigned int sdram_size(int cs, struct ddr_params *p)
{
	unsigned int dw;
	unsigned int banks;
	unsigned int size = 0;
	unsigned int row, col;

	switch (cs) {
	case 0:
		if (p->cs0 == 1) {
			row = p->row;
			col = p->col;
			break;
		} else
			return 0;
	case 1:
		if (p->cs1 == 1) {
			row = p->row1;
			col = p->col1;
			break;
		} else
			return 0;
	default:
		return 0;
	}

	banks = p->bank8 ? 8 : 4;
	dw = p->dw32 ? 4 : 2;
	size = (1 << (row + col)) * dw * banks;

	return size;
}

static void ddr_base_params_fill(struct ddr_params *ddr_params)
{
	struct ddr_params_common *params = &ddr_params->private_params.ddr_base_params;
	memset(&ddr_params->private_params, 0, sizeof(union private_params));
	DDR_PARAMS_FILL(params,tRAS);
	DDR_PARAMS_FILL(params,tRP);
	DDR_PARAMS_FILL(params,tRCD);
	DDR_PARAMS_FILL(params,tRC);
	DDR_PARAMS_FILL(params,tWR);
	DDR_PARAMS_FILL(params,tRRD);
	DDR_PARAMS_FILL(params,tWTR);
	DDR_PARAMS_FILL(params,tRFC);
	DDR_PARAMS_FILL(params,tXP);
	DDR_PARAMS_FILL(params,tCKE);
	DDR_PARAMS_FILL(params,tREFI);
	DDR_PARAMS_FILL(params,WL);
	DDR_PARAMS_FILL(params,RL);
}
static int ddr_refi_div(int reftck,int *div)
{
	int tmp,factor = 0;
	do{
		tmp = reftck;
		tmp = tmp / (16 << factor);
		factor++;
	}while(tmp > 255);
	*div = (factor - 1);
	return tmp;
}
static void ddrc_base_params_creator_common(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	int tmp;
	int div;
	/* tRP is differ in lpddr & lpddr2 & ddr2 & ddr3*/
	/* tWTR is differ in lpddr & lpddr2 & ddr2 & ddr3*/
	/* tWTR is differ in lpddr & lpddr2 & ddr2 & ddr3*/
	DDRC_TIMING_SET(1,ddr_base_params,tWR,6);

	tmp =ps2cycle_ceil(p->private_params.ddr_base_params.WL,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing1.b.tWL = tmp;
	/* tCCD is differ in lpddr & lpddr2 & ddr2 & ddr3*/
	DDRC_TIMING_SET(2,ddr_base_params,tRAS,6);
	DDRC_TIMING_SET(2,ddr_base_params,tRCD,6);

	tmp =ps2cycle_ceil(p->private_params.ddr_base_params.RL,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tRL = tmp;

	ddrc->timing3.b.ONUM = 4;  //version info.

	/* tCKSRE is differ in lpddr & lpddr2 & ddr2 & ddr3*/

	DDRC_TIMING_SET(3,ddr_base_params,tRP,6);
	DDRC_TIMING_SET(3,ddr_base_params,tRRD,6);
	DDRC_TIMING_SET(3,ddr_base_params,tRC,6);

	tmp = ps2cycle_ceil(p->private_params.ddr_base_params.tRFC,2) / 2 - 1 ;
	if(tmp < 0)
		tmp = 0;
	ASSERT_MASK(tmp,6);
	ddrc->timing4.b.tRFC = tmp;

	ddrc->timing4.b.tEXTRW = 1; /* default */
	ddrc->timing4.b.tRWCOV = 0;  /* default */
	tmp = ps2cycle_ceil(p->private_params.ddr_base_params.tCKE,1) + 1;
	ASSERT_MASK(tmp,3);
	ddrc->timing4.b.tCKE = tmp;

	DDRC_TIMING_SET(4,ddr_base_params,tXP,3);

	ddrc->timing5.b.tCTLUPD = 0xff; /* 0xff is the default value */

	/* tRTW is differ in lpddr & lpddr2 & ddr2 & ddr3 */
	/* tRDLAT is differ in lpddr & lpddr2 & ddr2 & ddr3 */
	/* tWDLAT is differ in lpddr & lpddr2 & ddr2 & ddr3 */

	/* tXSRD is diff in lpddr & lpddr2 & ddr2 & ddr3. */
	/* tXSRD is diff in lpddr & lpddr2 & ddr2 & ddr3. */

	ddrc->timing6.b.tCFGW = 5;  // default and controller don't used.
	ddrc->timing6.b.tCFGR = 5;  // default and controller don't used.

	tmp = ps2cycle_floor(p->private_params.ddr_base_params.tREFI);
	tmp -= 16; // controller is add 16 cycles.
	if(tmp < 0){
		out_error("tREFI[%d] is too small. check %s %d\n",
			  p->private_params.ddr_base_params.tREFI,
			  __FILE__,__LINE__);
	}
	tmp = ddr_refi_div(tmp,&div);
	if(div > 7){
		out_error("tREFI[%d] is too large. check %s %d\n",
			  p->private_params.ddr_base_params.tREFI,
			  __FILE__,__LINE__);
	}
	ASSERT_MASK(tmp,8);
	ddrc->refcnt = (tmp << DDRC_REFCNT_CON_BIT)
		| (div << DDRC_REFCNT_CLK_DIV_BIT)
		| DDRC_REFCNT_REF_EN;

	ddrc->autosr_en = 0;

#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
	ddrc->autosr_en = 1;
#endif
}

static void ddrc_config_creator(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	unsigned int  mem_base0 = 0, mem_base1 = 0, mem_mask0 = 0, mem_mask1 = 0;
	unsigned int memsize_cs0, memsize_cs1, memsize;
	/* CFG */
	ddrc->cfg.b.ROW1 = p->row1 - 12;
	ddrc->cfg.b.COL1 = p->col1 - 8;
	ddrc->cfg.b.BA1 = p->bank8;
	ddrc->cfg.b.IMBA = 1;
	ddrc->cfg.b.BSL = (p->bl == 8) ? 1 : 0;
#ifdef CONFIG_DDR_CHIP_ODT
	ddrc->cfg.b.ODTEN = 1;
#else
	ddrc->cfg.b.ODTEN = 0;
#endif
	ddrc->cfg.b.MISPE = 1;
	ddrc->cfg.b.ROW0 = p->row - 12;
	ddrc->cfg.b.COL0 = p->col - 8;

	ddrc->cfg.b.CS1EN = p->cs1;
	ddrc->cfg.b.CS0EN = p->cs0;

	ddrc->cfg.b.CL = 0; /* defalt and NOT used in this version */

	ddrc->cfg.b.BA0 = p->bank8;
	ddrc->cfg.b.DW = p->dw32;

	switch (p->type) {
#define _CASE(D, P)				\
		case D:				\
			ddrc->cfg.b.TYPE = P;	\
			break
		_CASE(DDR3, 6);		/* DDR3:0b110 */
		_CASE(LPDDR, 3);	/* LPDDR:0b011 */
		_CASE(LPDDR2, 5);	/* LPDDR2:0b101 */
		_CASE(DDR2, 4);	    /* DDR2:0b100 */
#undef _CASE
	default:
		out_error("don't support the ddr type.!");
		assert(1);
		break;
	}

	/* CTRL  */
	ddrc->ctrl = DDRC_CTRL_ACTPD | DDRC_CTRL_PDT_64 | DDRC_CTRL_ACTSTP
		| DDRC_CTRL_PRET_8 | 0 << 6 | DDRC_CTRL_UNALIGN
		| DDRC_CTRL_ALH | DDRC_CTRL_RDC | DDRC_CTRL_CKE;
	/* ddrc->ctrl = DDRC_CTRL_PRET_8 | 0 << 6 | DDRC_CTRL_UNALIGN */
	/* 	| DDRC_CTRL_ALH | DDRC_CTRL_RDC | DDRC_CTRL_CKE; */
#ifdef CONFIG_DDRC_CTRL_PDT
	ddrc->ctrl &= ~(DDRC_CTRL_PDT_MASK);
	ddrc->ctrl |= CONFIG_DDRC_CTRL_PDT;
#endif
	/* MMAP0,1 */
	memsize_cs0 = p->size.chip0;
	memsize_cs1 = p->size.chip1;
	memsize = memsize_cs0 + memsize_cs1;

	if (memsize > 0x20000000) {
		if (memsize_cs1) {
			mem_base0 = 0x0;
			mem_mask0 = (~((memsize_cs0 >> 24) - 1) & ~(memsize >> 24))
				& DDRC_MMAP_MASK_MASK;
			mem_base1 = (memsize_cs1 >> 24) & 0xff;
			mem_mask1 = (~((memsize_cs1 >> 24) - 1) & ~(memsize >> 24))
				& DDRC_MMAP_MASK_MASK;
		} else {
			mem_base0 = 0x0;
			mem_mask0 = ~(((memsize_cs0 * 2) >> 24) - 1) & DDRC_MMAP_MASK_MASK;
			mem_mask1 = 0;
			mem_base1 = 0xff;
		}
	} else {
		mem_base0 = (DDR_MEM_PHY_BASE >> 24) & 0xff;
		mem_mask0 = ~((memsize_cs0 >> 24) - 1) & DDRC_MMAP_MASK_MASK;
		mem_base1 = ((DDR_MEM_PHY_BASE + memsize_cs0) >> 24) & 0xff;
		mem_mask1 = ~((memsize_cs1 >> 24) - 1) & DDRC_MMAP_MASK_MASK;
	}
	ddrc->mmap[0] = mem_base0 << DDRC_MMAP_BASE_BIT | mem_mask0;
	ddrc->mmap[1] = mem_base1 << DDRC_MMAP_BASE_BIT | mem_mask1;
}

static void ddrp_base_params_creator_common(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	int tmp = 0;

	switch (p->type) {
#define _CASE(D, P)				\
		case D:				\
			tmp = P;		\
			break
		_CASE(DDR3, 3);		/* DDR3:0b110 */
		_CASE(LPDDR, 0);	/* LPDDR:0b011 */
		_CASE(LPDDR2, 4);	/* LPDDR2:0b101 */
		_CASE(DDR2, 2);	    /* DDR2:0b100 */
#undef _CASE
	default:
		break;
	}
	ddrp->dcr = tmp | (p->bank8 << 3);

	/* MR0'Register is differ in lpddr ddr2 lpddr2 ddr3 */

	/* DTPR0 registers */
	/* tMRD is differ in lpddr ddr2 lpddr2 ddr3 */
	/* tRTP is differ for ddr2 */
	DDRP_TIMING_SET(0,ddr_base_params,tWTR,3,1,15);
	DDRP_TIMING_SET(0,ddr_base_params,tRP,4,2,20);
	DDRP_TIMING_SET(0,ddr_base_params,tRCD,4,2,11);
	DDRP_TIMING_SET(0,ddr_base_params,tRAS,5,2,31);
	DDRP_TIMING_SET(0,ddr_base_params,tRRD,4,1,8);
	DDRP_TIMING_SET(0,ddr_base_params,tRC,6,2,42);
	/* tCCD is differ in lpddr ddr2 lpddr2 ddr3 */

	/* DTPR1 registers */
	/* tAOND is only used by DDR2. */
	/* tRTW is differ in lpddr ddr2 lpddr2 ddr3 */
	/* tFAW is differ in lpddr ddr2 lpddr2 ddr3 */

	//DDRP_TIMING_SET(1,ddr_base_params,tFAW,6,2,31);


	/* tMOD is used by ddr3 */
	/* tRTODT is used by ddr3 */
	DDRP_TIMING_SET(1,ddr_base_params,tRFC,8,0,255);
	/* tDQSCKmin is used by lpddr2 */
	/* tDQSCKmax is used by lpddr2 */

	/* DTPR2 registers */
	/* tXS is differ in lpddr2 ddr3 */
	/* tXP is differ in lpddr2 ddr3 */
	/* tCKE is differ in ddr3 */

	/* PTRn registers */
	tmp = ps2cycle_ceil(50 * 1000,1);   /* default 50 ns and min clk is 8 cycle */
	ASSERT_MASK(tmp,6);
	if(tmp < 8) tmp = 8;
	ddrp->ptr0.b.tDLLSRST = tmp;


	tmp = ps2cycle_ceil(5120*1000, 1); /* default 5.12 us*/
	ASSERT_MASK(tmp,12);
	ddrp->ptr0.b.tDLLLOCK = tmp;

	ddrp->ptr0.b.tITMSRST = 8;    /* default 8 cycle & more than 8 */

	//BETWEEN(tmp, 2, 1023);
	ddrp->dtpr2.b.tDLLK = 512;
	/* PGCR'Register is differ in lpddr ddr2 lpddr2 ddr3 */
}
void init_ddr_params_common(struct ddr_params *ddr_params,int type)
{
	ddr_params->type = type;
	ddr_params->freq = CONFIG_SYS_MEM_FREQ;
	ddr_params->cs0 = CONFIG_DDR_CS0;
	ddr_params->cs1 = CONFIG_DDR_CS1;
	ddr_params->dw32 = CONFIG_DDR_DW32;
	ddr_params->bl = DDR_BL;
	ddr_params->col = DDR_COL;
	ddr_params->row = DDR_ROW;

#ifdef DDR_COL1
	ddr_params->col1 = DDR_COL1;
#endif
#ifdef DDR_ROW1
	ddr_params->row1 = DDR_ROW1;
#endif
	ddr_params->bank8 = DDR_BANK8;
	ddr_params->size.chip0 = sdram_size(0, ddr_params);
	ddr_params->size.chip1 = sdram_size(1, ddr_params);
}



/* #define CONFIG_DDR_CHIP_IMPEDANCE */

static void ddrp_config_creator(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	int i;
	unsigned char rzq[]={
		0x00,0x01,0x02,0x03,0x06,0x07,0x04,0x05,
		0x0C,0x0D,0x0E,0x0F,0x0A,0x0B,0x08,0x09,
		0x18,0x19,0x1A,0x1B,0x1E,0x1F,0x1C,0x1D,
		0x14,0x15,0x16,0x17,0x12,0x13,0x10,0x11};  //from ddr multiphy page 158.
	ddrp->odtcr.d32 = 0;
#ifdef CONFIG_DDR_CHIP_ODT
	ddrp->odtcr.d32 = 0x84210000;   //power on default.
#endif
	/* DDRC registers assign */
	for(i = 0;i < 4;i++)
		ddrp->dxngcrt[i].d32 = 0x00090e80;
	i = 0;
#ifdef CONFIG_DDR_PHY_ODT
	for(i = 0;i < (CONFIG_DDR_DW32 + 1) * 2;i++){
		ddrp->dxngcrt[i].b.dqsrtt = 1;
		ddrp->dxngcrt[i].b.dqrtt = 1;
		ddrp->dxngcrt[i].b.dqsodt = 1;
		ddrp->dxngcrt[i].b.dqodt = 1;
	}
#else
	for(i = 0;i < (CONFIG_DDR_DW32 + 1) * 2;i++){
		ddrp->dxngcrt[i].b.dqsrtt = 0;
		ddrp->dxngcrt[i].b.dqrtt = 0;
		ddrp->dxngcrt[i].b.dxen = 1;
	}
#endif
	for(i = 0;i<sizeof(rzq);i++)
		    ddrp->rzq_table[i] = rzq[i];
}
static void params_print(struct ddrc_reg *ddrc, struct ddrp_reg *ddrp)
{
	int i;
	/* DDRC registers print */
	printf("#define DDRC_CFG_VALUE			0x%08x\n", ddrc->cfg.d32);
	printf("#define DDRC_CTRL_VALUE			0x%08x\n", ddrc->ctrl);
	printf("#define DDRC_MMAP0_VALUE		0x%08x\n", ddrc->mmap[0]);
	printf("#define DDRC_MMAP1_VALUE		0x%08x\n", ddrc->mmap[1]);
	printf("#define DDRC_REFCNT_VALUE		0x%08x\n", ddrc->refcnt);
	printf("#define DDRC_TIMING1_VALUE		0x%08x\n", ddrc->timing1.d32);
	printf("#define DDRC_TIMING2_VALUE		0x%08x\n", ddrc->timing2.d32);
	printf("#define DDRC_TIMING3_VALUE		0x%08x\n", ddrc->timing3.d32);
	printf("#define DDRC_TIMING4_VALUE		0x%08x\n", ddrc->timing4.d32);
	printf("#define DDRC_TIMING5_VALUE		0x%08x\n", ddrc->timing5.d32);
	printf("#define DDRC_TIMING6_VALUE		0x%08x\n", ddrc->timing6.d32);
	printf("#define DDRC_AUTOSR_EN_VALUE		0x%08x\n", ddrc->autosr_en);

	/* DDRP registers print */
	printf("#define DDRP_DCR_VALUE			0x%08x\n", ddrp->dcr);
	printf("#define	DDRP_MR0_VALUE			0x%08x\n", ddrp->mr0.d32);
	printf("#define	DDRP_MR1_VALUE			0x%08x\n", ddrp->mr1.d32);
	printf("#define	DDRP_MR2_VALUE			0x%08x\n", ddrp->mr2.d32);
	printf("#define	DDRP_MR3_VALUE			0x%08x\n", ddrp->mr3.d32);
	printf("#define	DDRP_PTR0_VALUE			0x%08x\n", ddrp->ptr0.d32);
	printf("#define	DDRP_PTR1_VALUE			0x%08x\n", ddrp->ptr1.d32);
	printf("#define	DDRP_PTR2_VALUE			0x%08x\n", ddrp->ptr2.d32);
	printf("#define	DDRP_DTPR0_VALUE		0x%08x\n", ddrp->dtpr0.d32);
	printf("#define	DDRP_DTPR1_VALUE		0x%08x\n", ddrp->dtpr1.d32);
	printf("#define	DDRP_DTPR2_VALUE		0x%08x\n", ddrp->dtpr2.d32);
	printf("#define	DDRP_PGCR_VALUE			0x%08x\n", ddrp->pgcr);
	printf("#define DDRP_ODTCR_VALUE                0x%08x\n", ddrp->odtcr.d32);
	for(i = 0;i < 4;i++){
		printf("#define DDRP_DX%dGCR_VALUE              0x%08x\n",i,ddrp->dxngcrt[i].d32);
	}
	printf("#define DDRP_ZQNCR1_VALUE               0x%08x\n", ddrp->zqncr1);
	printf("#define DDRP_IMPANDCE_ARRAY             {0x%08x,0x%08x} //0-cal_value 1-req_value\n", ddrp->impedance[0],ddrp->impedance[1]);
	printf("#define DDRP_ODT_IMPANDCE_ARRAY         {0x%08x,0x%08x} //0-cal_value 1-req_value\n", ddrp->odt_impedance[0],ddrp->odt_impedance[1]);
	printf("#define DDRP_RZQ_TABLE  {0x%02x",ddrp->rzq_table[0]);
	for(i = 1;i < sizeof(ddrp->rzq_table);i++){
		printf(",0x%02x",ddrp->rzq_table[i]);
	}
	printf("}\n");
}

static void sdram_size_print(struct ddr_params *p)
{
	printf("#define	DDR_CHIP_0_SIZE			%u\n", p->size.chip0);
	printf("#define	DDR_CHIP_1_SIZE			%u\n", p->size.chip1);
}
static unsigned int frandom(int max)
{
	return max;
}
#define swap_bytes(buf,b1,b2) do{                \
         unsigned char swap;                        \
         swap = buf[b1];                            \
         buf[b1] = buf[b2];                        \
         buf[b2] = swap;                            \
     }while(0)

static void mem_remap_print(struct ddr_params *p)
{
     int address_bits;
     int swap_bits;
     int bank_bits;
     int startA, startB;
     int bit_width;
     unsigned int remap_array[5];
     unsigned char *s;
     int i,width;
     s = (unsigned char *)remap_array;
     for(i = 0;i < sizeof(remap_array);i++)
         s[i] = i;

     if(p->size.chip1 && (p->size.chip0 != p->size.chip1))
         return;

     bank_bits = DDR_BANK8 == 1 ? 3 : 2;
     bit_width = CONFIG_DDR_DW32 == 1 ? 2 : 1;

     /*
      * count the chips's address space bits.
      */
     address_bits =  bit_width + DDR_COL + DDR_ROW + bank_bits +
(CONFIG_DDR_CS0 + CONFIG_DDR_CS1 - 1);
     /*
      * count address space bits for swap.
      */
     swap_bits = bank_bits + (CONFIG_DDR_CS0 + CONFIG_DDR_CS1 - 1);

     startA = bit_width + DDR_COL > 12 ? bit_width + DDR_COL : 12;

     startB = address_bits - swap_bits - startA;
     startA = startA - 12;
     /*
      * bank and cs swap with row.
      */
     for(i = startA;i < swap_bits;i++){
         swap_bytes(s,startA + i,startB + i);
         //swap_bytes(s,startA + i,startB + i,startB);
     }


     /*
      * random high address for securing.
      */
#if 0
     for(i = 0;i < swap_bits / 2;i++){
         int sw = frandom(swap_bits - 1 - i);
         swap_bytes(s,startA + i,startA + sw);
     }

     width = startB + startA;
     startA = startA + swap_bits;
     for(i = 0;i < width / 2;i++){
         int sw = frandom(width - 1 - i);
         swap_bytes(s,startA + i,startA + sw);
     }
#endif
     printf("#define REMMAP_ARRAY {\\\n");
     for(i = 0;i <sizeof(remap_array) / sizeof(remap_array[0]);i++)
     {
         printf("\t0x%08x,\\\n",remap_array[i]);
     }
     printf("};\n");
}
static void file_head_print(void)
{
	printf("/*\n");
	printf(" * DO NOT MODIFY.\n");
	printf(" *\n");
	printf(" * This file was generated by ddr_params_creator\n");
	printf(" *\n");
	printf(" */\n");
	printf("\n");

	printf("#ifndef __DDR_REG_VALUES_H__\n");
	printf("#define __DDR_REG_VALUES_H__\n\n");
}

static void file_end_print(void)
{
	printf("\n#endif /* __DDR_REG_VALUES_H__ */\n");
}

static struct ddr_creator_ops *p_ddr_creator = NULL;
static int ops_count = 0;
void register_ddr_creator(struct ddr_creator_ops *ops)
{
	if(ops_count++ == 0){
		p_ddr_creator = ops;
	}else{
		out_error("Error: DDR CREATEOR cann't register %d\n",ops->type);
	}
}
/**
 * ddr parameter prev setting :
 *    1.  the ddr chip parameter is filled.
 *    2.  the ddr controller parameter is generated.
 *    3.  the ddr phy paramerter is generated.
 *    4.  all parameter is outputted.
 */
int main(int argc, char *argv[])
{
	struct ddrc_reg ddrc;
	struct ddrp_reg ddrp;
	struct ddr_params ddr_params;
	__ps_per_tck = (1000000000 / (CONFIG_SYS_MEM_FREQ / 1000));
	memset(&ddrc, 0, sizeof(struct ddrc_reg));
	memset(&ddrp, 0, sizeof(struct ddrp_reg));
	memset(&ddr_params, 0, sizeof(struct ddr_params));
	ddr_creator_init();

	init_ddr_params_common(&ddr_params,p_ddr_creator->type);
	ddr_base_params_fill(&ddr_params);
	p_ddr_creator->fill_in_params(&ddr_params);

	ddrc_base_params_creator_common(&ddrc, &ddr_params);
	p_ddr_creator->ddrc_params_creator(&ddrc, &ddr_params);
	ddrc_config_creator(&ddrc,&ddr_params);

	ddrp_base_params_creator_common(&ddrp, &ddr_params);
	p_ddr_creator->ddrp_params_creator(&ddrp, &ddr_params);
	ddrp_config_creator(&ddrp,&ddr_params);
	file_head_print();
	params_print(&ddrc, &ddrp);
	sdram_size_print(&ddr_params);
	mem_remap_print(&ddr_params);
	file_end_print();

	return 0;
}
