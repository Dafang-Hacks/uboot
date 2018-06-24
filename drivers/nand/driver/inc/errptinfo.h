#ifndef __ERRPTINFO_H_
#define __ERRPTINFO_H_

#include "nddata.h"
#include "nand_api.h"

/* ======================  ATTENTION ===================== */
/*if you want to call nand_update_errpt after erase partition,
  please call ioctl, thank you for cooperation*/
/************************************************************/
int nand_errpt_init(nand_flash *ndflash);
void nand_errpt_deinit(void);
int nand_write_errpt(nand_data *nddata, plat_ptinfo *ptinfo, nand_flash *ndflash, int erasemode);
int get_errpt_head(nand_data *nddata, struct nand_api_platdependent *platdep);
int get_errpt_ppainfo(nand_data *nddata, struct nand_api_platdependent *platdep);
int nand_update_errpt(nand_data *nddata, nand_flash *ndflash, PPartition *pt);

#endif
