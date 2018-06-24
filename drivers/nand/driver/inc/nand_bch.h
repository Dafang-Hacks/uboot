#ifndef __NAND_BCH_H__
#define __NAND_BCH_H__

#include <nddata.h>
#include "transadaptor.h"

#define MAX_BCHSEL	64
#define BCHSEL_STEP	4

typedef struct __PipeNode {
	unsigned char *data;
	unsigned char *parity;
} PipeNode;

int get_parity_size(unsigned char eccbit);

int nand_bch_encode_prepare(int context, PipeNode* pipe, unsigned char eccbit);
int nand_bch_encode_complete(int context, PipeNode* pipe);
int nand_bch_decode_prepare(int context, PipeNode* pipe, unsigned char eccbit);
int nand_bch_decode_complete(int context, PipeNode* pipe);
int nand_bch_open(bch_base *base, int eccsize);
void nand_bch_close(int context);

int nand_bch_suspend(void);
int nand_bch_resume(void);

char *nand_bch_get_clk_name(void);
char *nand_bch_get_cgu_clk_name(void);
#endif
