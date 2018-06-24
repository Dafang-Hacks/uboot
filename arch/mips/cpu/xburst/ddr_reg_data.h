#ifndef DDR_REG_DATA_H
#define DDR_REG_DATA_H
struct ddr_registers
{
	uint32_t ddrc_cfg;
	uint32_t ddrc_ctrl;
	uint32_t ddrc_mmap[2];
	uint32_t ddrc_refcnt;
	uint32_t ddrc_timing1;
	uint32_t ddrc_timing2;
	uint32_t ddrc_timing3;
	uint32_t ddrc_timing4;
	uint32_t ddrc_timing5;
	uint32_t ddrc_timing6;
	uint32_t ddrc_autosr;
	uint32_t ddrp_dcr;
	uint32_t ddrp_mr0;
	uint32_t ddrp_mr1;
	uint32_t ddrp_mr2;
	uint32_t ddrp_mr3;
	uint32_t ddrp_ptr0;
	uint32_t ddrp_ptr1;
	uint32_t ddrp_ptr2;
	uint32_t ddrp_dtpr0;
	uint32_t ddrp_dtpr1;
	uint32_t ddrp_dtpr2;
	uint32_t ddrp_pgcr;
	uint32_t ddrp_odtcr;
	uint32_t ddrp_dxngct[4];
	uint32_t ddrp_zqncr1;
	uint32_t ddrp_impedance[2];
	uint32_t ddrp_odt_impedance[2];
	unsigned char ddrp_rzq[32];
	uint32_t ddr_chip0_size;
	uint32_t ddr_chip1_size;
	unsigned int remap_array[5];
};
extern struct ddr_registers *g_ddr_param;
#define DDRC_CFG_VALUE			g_ddr_param->ddrc_cfg
#define DDRC_CTRL_VALUE			g_ddr_param->ddrc_ctrl
#define DDRC_MMAP0_VALUE		g_ddr_param->ddrc_mmap[0]
#define DDRC_MMAP1_VALUE		g_ddr_param->ddrc_mmap[1]
#define DDRC_REFCNT_VALUE		g_ddr_param->ddrc_refcnt
#define DDRC_TIMING1_VALUE		g_ddr_param->ddrc_timing1
#define DDRC_TIMING2_VALUE		g_ddr_param->ddrc_timing2
#define DDRC_TIMING3_VALUE		g_ddr_param->ddrc_timing3
#define DDRC_TIMING4_VALUE		g_ddr_param->ddrc_timing4
#define DDRC_TIMING5_VALUE		g_ddr_param->ddrc_timing5
#define DDRC_TIMING6_VALUE		g_ddr_param->ddrc_timing6
#define DDRC_AUTOSR_EN_VALUE		g_ddr_param->ddrc_autosr
#define DDRP_DCR_VALUE			g_ddr_param->ddrp_dcr
#define	DDRP_MR0_VALUE			g_ddr_param->ddrp_mr0
#define	DDRP_MR1_VALUE			g_ddr_param->ddrp_mr1
#define	DDRP_MR2_VALUE			g_ddr_param->ddrp_mr2
#define	DDRP_MR3_VALUE			g_ddr_param->ddrp_mr3
#define	DDRP_PTR0_VALUE			g_ddr_param->ddrp_ptr0
#define	DDRP_PTR1_VALUE			g_ddr_param->ddrp_ptr1
#define	DDRP_PTR2_VALUE			g_ddr_param->ddrp_ptr2
#define	DDRP_DTPR0_VALUE		g_ddr_param->ddrp_dtpr0
#define	DDRP_DTPR1_VALUE		g_ddr_param->ddrp_dtpr1
#define	DDRP_DTPR2_VALUE		g_ddr_param->ddrp_dtpr2
#define	DDRP_PGCR_VALUE			g_ddr_param->ddrp_pgcr
#define DDRP_ODTCR_VALUE                g_ddr_param->ddrp_odtcr
#define DDRP_DX0GCR_VALUE               g_ddr_param->ddrp_dxngct[0]
#define DDRP_DX1GCR_VALUE               g_ddr_param->ddrp_dxngct[1]
#define DDRP_DX2GCR_VALUE               g_ddr_param->ddrp_dxngct[2]
#define DDRP_DX3GCR_VALUE               g_ddr_param->ddrp_dxngct[3]
#define DDRP_ZQNCR1_VALUE               g_ddr_param->ddrp_zqncr1
#define DDRP_IMPANDCE_ARRAY             g_ddr_param->ddrp_impedance //0-cal_value 1-req_value
#define DDRP_ODT_IMPANDCE_ARRAY         g_ddr_param->ddrp_odt_impedance  //0-cal_value 1-req_value
#define DDRP_RZQ_TABLE                  g_ddr_param->ddrp_rzq
#define	DDR_CHIP_0_SIZE			g_ddr_param->ddr_chip0_size
#define	DDR_CHIP_1_SIZE			g_ddr_param->ddr_chip1_size
#define REMMAP_ARRAY                    g_ddr_param->remap_array

#endif /* DDR_REG_DATA_H */
