/*
 * Copyright 2013 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/processor.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>
#include <asm/io.h>
#include <asm/fsl_law.h>

DECLARE_GLOBAL_DATA_PTR;

/* CONFIG_SYS_DDR_RAW_TIMING */
/*
 * Hynix H5TQ1G83TFR-H9C
 */
dimm_params_t ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 536870912u,
	.capacity = 536870912u,
	.primary_sdram_width = 32,
	.ec_sdram_width = 0,
	.registered_dimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 14,
	.n_col_addr = 10,
	.n_banks_per_sdram_device = 8,
	.edc_config = 0,
	.burst_lengths_bitmask = 0x0c,

	.tCKmin_X_ps = 1875,
	.caslat_X = 0x1e << 4,	/* 5,6,7,8 */
	.tAA_ps = 13125,
	.tWR_ps = 18000,
	.tRCD_ps = 13125,
	.tRRD_ps = 7500,
	.tRP_ps = 13125,
	.tRAS_ps = 37500,
	.tRC_ps = 50625,
	.tRFC_ps = 160000,
	.tWTR_ps = 7500,
	.tRTP_ps = 7500,
	.refresh_rate_ps = 7800000,
	.tFAW_ps = 37500,
};

int fsl_ddr_get_dimm_params(dimm_params_t *pdimm,
		unsigned int controller_number,
		unsigned int dimm_number)
{
	const char dimm_model[] = "Fixed DDR on board";

	if ((controller_number == 0) && (dimm_number == 0)) {
		memcpy(pdimm, &ddr_raw_timing, sizeof(dimm_params_t));
		memset(pdimm->mpart, 0, sizeof(pdimm->mpart));
		memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);
	}

	return 0;
}

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	int i;
	popts->clk_adjust = 6;
	popts->cpo_override = 0x1f;
	popts->write_data_delay = 2;
	popts->half_strength_driver_enable = 1;
	/* Write leveling override */
	popts->wrlvl_en = 1;
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;
	popts->wrlvl_start = 0x8;
	popts->trwt_override = 1;
	popts->trwt = 0;

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		popts->cs_local_opts[i].odt_rd_cfg = FSL_DDR_ODT_NEVER;
		popts->cs_local_opts[i].odt_wr_cfg = FSL_DDR_ODT_CS;
	}
}

