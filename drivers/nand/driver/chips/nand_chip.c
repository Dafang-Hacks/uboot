/**
 * nand_ids.c
 **/

#include "nand_chip.h"
#include "nand_debug.h"

#define EXTID_MARK	0x00FFFFFF

#if 0
/* Samsung 2K page SLC nand flash */
optionalcmd optcmd0 =
{
    {0x00, 0x30},           //multi-plane read command
    {0x11, 0x81},           //multi-plane program command
    {0x00, 0x00, 0x35},     //multi-plane page copy-back read command
    {0x85, 0x11, 0x81},     //multi-plane page copy-back program command
    0x70,                   //multi-plane operation status read command
    0xf1,                   //inter-leave bank0 operation status read command
    0xf2,                   //inter-leave bank1 operation status read command
};

/* Samsung 4K page SLC nand flash */
optionalcmd optcmd1 =
{
    {0x60, 0x30},           //multi-plane read command
    {0x11, 0x81},           //multi-plane program command
    {0x60, 0x60, 0x35},     //multi-plane page copy-back read command
    {0x85, 0x11, 0x81},     //multi-plane page copy-back program command
    0x70,                   //multi-plane operation status read command
    0xf1,                   //inter-leave bank0 operation status read command
    0xf2,                   //inter-leave bank1 operation status read command
};

/* Samsung 2K page MLC nand flash */
optionalcmd optcmd2 =
{
    {0x00, 0x30},           //multi-plane read command
    {0x11, 0x81},           //multi-plane program command
    {0x00, 0x00, 0x35},     //multi-plane page copy-back read command
    {0x85, 0x11, 0x81},     //multi-plane page copy-back program command
    0x70,                   //multi-plane operation status read command
    0xf1,                   //inter-leave bank0 operation status read command
    0xf2,                   //inter-leave bank1 operation status read command
};

/* Samsung 4K page MLC nand flash */
optionalcmd optcmd3 =
{
    {0x60, 0x60},           //multi-plane read command
    {0x11, 0x81},           //multi-plane program command
    {0x60, 0x60, 0x35},     //multi-plane page copy-back read command
    {0x85, 0x11, 0x81},     //multi-plane page copy-back program command
    0x70,                   //multi-plane operation status read command
    0xf1,                   //inter-leave bank0 operation status read command
    0xf2,                   //inter-leave bank1 operation status read command
};

/* Micon nand flash */
optionalcmd optcmd4 =
{
    {0x00, 0x30},           //multi-plane read command
    {0x11, 0x80},           //multi-plane program command
    {0x00, 0x00, 0x35},     //multi-plane page copy-back read command
    {0x85, 0x11, 0x80},     //multi-plane page copy-back program command
    0x70,                   //multi-plane operation status read command
    0x78,                   //inter-leave bank0 operation status read command
    0x78,                   //inter-leave bank1 operation status read command
};

/* Toshiba SLC nand flash */
optionalcmd optcmd5 =
{
    {0x00, 0x30},           //multi-plane read command
    {0x11, 0x80},           //multi-plane program command
    {0x00, 0x00, 0x30},     //multi-plane page copy-back read command
    {0x8c, 0x11, 0x8c},     //multi-plane page copy-back program command
    0x71,                   //multi-plane operation status read command
    0x70,                   //inter-leave bank0 operation status read command
    0x70,                   //inter-leave bank1 operation status read command
};

/* Toshiba MLC nand flash which multi-plane offset is 1024 */
optionalcmd optcmd6 =
{
    {0x00, 0x30},           //multi-plane read command
    {0x11, 0x80},           //multi-plane program command
    {0x00, 0x00, 0x30},     //multi-plane page copy-back read command
    {0x8c, 0x11, 0x8c},     //multi-plane page copy-back program command
    0x71,                   //multi-plane operation status read command
    0x70,                   //inter-leave bank0 operation status read command
    0x70,                   //inter-leave bank1 operation status read command
};

/* Toshiba MLC nand flash which multi-plane offset is 2048 */
optionalcmd optcmd7 =
{
    {0x00, 0x30},           //multi-plane read command
    {0x11, 0x80},           //multi-plane program command
    {0x00, 0x00, 0x30},     //multi-plane page copy-back read command
    {0x8c, 0x11, 0x8c},     //multi-plane page copy-back program command
    0x71,                   //multi-plane operation status read command
    0x70,                   //inter-leave bank0 operation status read command
    0x70,                   //inter-leave bank1 operation status read command
};

optionalcmd optcmd8 =
{
    {0x00, 0x30},           //multi-plane read command
    {0x11, 0x80},           //multi-plane program command
    {0x00, 0x00, 0x30},     //multi-plane page copy-back read command
    {0x8c, 0x11, 0x8c},     //multi-plane page copy-back program command
    0x71,                   //multi-plane operation status read command
    0x70,                   //inter-leave bank0 operation status read command
    0x70,                   //inter-leave bank1 operation status read command
};

optionalcmd optcmd9 =
{
    {0x00, 0x30},           //multi-plane read command
    {0x11, 0x81},           //multi-plane program command
    {0x00, 0x00, 0x30},     //multi-plane page copy-back read command
    {0x8c, 0x11, 0x8c},     //multi-plane page copy-back program command
    0x71,                   //multi-plane operation status read command
    0x70,                   //inter-leave bank0 operation status read command
    0x70,                   //inter-leave bank1 operation status read command
};

optionalcmd Defualtoptcmd =
{
    {0x00, 0x30},           //multi-plane read command
    {0x11, 0x81},           //multi-plane program command
    {0x00, 0x00, 0x35},     //multi-plane page copy-back read command
    {0x85, 0x11, 0x81},     //multi-plane page copy-back program command
    0x70,                   //multi-plane operation status read command
    0xf1,                   //inter-leave bank0 operation status read command
    0xf2,                   //inter-leave bank1 operation status read command
};
#endif

/**
 * nand_chip_table: contain the nand chip attrs
 **/
const nand_flash nand_flash_table[] = {
	/*
	  {name,			id,		extid,
	  pagesize,	blocksize,	oobsize,	totalblocks,	maxvalidblocks,
	  eccbit,	planepdie,	diepchip,	chips,
	  buswidth,	realplanenum,	badblockpos,	rowcycles,	planeffset,
	  options,
	  {tALS,	tALH,	tRP,	tWP,	tRHW,	tWHR,	tWHR2,	tRR,	tWB,	tADL,	tCWAW,	tCS,	tCLH}}
	*/
	/****************************** SAMSUNG *********************************/
	{"SAMSUNG_K9GBG08U0A",          0xECD7,		0x43547A94,
	 8192,		1024*1024,	640,		4152,		0,
	 24,		2,		1,		1,
	 8,		2,		4,		3,		0,
	 (0),
	 .timing.timing =
	 {12,		5,	25,	25,	100,	120,	300,	20,	100,	300,	300,	0,	0}},

	{"SAMSUNG_K9GAG08U0D", 		0xECD5,		0x41342994,
	 4096,		512*1024,       218, 		4096,		0,
	 24,		2,		1,		1,
	 8,		2,		4,		3,		0,
	 (0),
	 .timing.timing =
	 {15,		5,	15,	15,	100,	60,	60,     20,     0,      100,     0,	0,	0}},

	{"SAMSUNG_K524G2ACI-B050",	0xECBC,		0x00545500,
	 2048,		(2048 * 64),	64,		(4096 + 80),	0,
	 8,		2,		1,		1,
	 16,		1,		4,		3,		0,
	 (0),
	 .timing.timing =
	 {21,		5,	21,	21,	100,	60,	200,	20,	100,	100,	0,	20,	0}},

	{"SAMSUNG_K9F1208U0C_PCB0",	0xEC76,		0x00743F5A,
	 512,		(512 * 32),	16,		(4096 + 128),		0,
	 4,		1,		1,		1,
	 8,		1,		4,		3,		0,
	 (0),
	 .timing.timing =
	 {21,		5,	21,	21,	100,	60,	200,	20,	100,	100,	0,	31,	5}},

	/****************************** MICRON *********************************/
	{"MICRON_MT29F32G08CBACA",      0x2C68,         0x00A94A04,
	 4096,		1024*1024,      224,		4096,		0,
	 24,		2,		1,		1,
	 8,		2,		4,		3,		0,
	 (NAND_MICRON_NORMAL),
	 .timing.timing =
	 {10,   	5,      15,	15,	100,    60,	200,	20,     100,    70,    	0,	25,	5}},

	{"MICRON_MT29F64G08CBAAAWP",	0x2C88,		0x00A94B04,
	 8192,		2048*1024,	448,		4096,		0,
	 24,		2,		1,		1,
	 8,		2,		0,		3,		0,
	 (NAND_MICRON_NORMAL),
	 .timing.timing =
	 {10,		5,	15,	15,	100,	60,	200,	20,	100,	70,	0,	20,	5}},

	{"MICRON_MT29F64G08CBABA",	0x2C64,		0x00A94B44,
	 8192,		2048*1024,	744,		4096,		0,
	 40,		2,		1,		1,
	 8,		2,		0,		3,		0,
	 (NAND_MICRON_NORMAL | NAND_TIMING_MODE | NAND_TIMING_MODE_V(MR_TIMING_MODE3) | NAND_DRIVER_STRENGTH),
	 .timing.timing =
	 {10,		5,	15,	15,	100,	60,	200,	20,	100,	70,	0,	20,	5}},

	{"MICRON_MT29F128G08CFABA",	0x2C64,		0x00A94B44,
	 8192,		2048*1024,	744,		8192,		0,
         40,		2,              1,	        2,
         8,	        2,	        4,	        3,		0,
	 (0),
	 .timing.timing =
	 {10,		5,	15,	15,     100,    60,     200,    20,     100,    70,      0,	20,	5}},

	/****************************** HYNIX *********************************/
	{"HYNIX_H27UBG8T2BTR",		0xADD7,		0xC374DA94,
	 8192,		2048*1024,	640,		2048,		0,
         40,		2,              1,	        1,
         8,	        2,	        4,	        3,		0,
	 (NAND_READ_RETRY | NAND_READ_RETRY_MODE(HY_RR_F26_32G_MLC)),
	 .timing.timing =
	 {10,		5,	15,	15,     100,    80,     200,    20,     100,    200,     0,	20,	5}},

	{"HYNIX_H27UCG8T2ATR",		0xADDE,		0xC474DA94,
	 8192,		2048*1024,	640,		4096,		0,
         40,		2,              1,	        1,
         8,	        2,	        4,	        3,		0,
	 (NAND_READ_RETRY | NAND_READ_RETRY_MODE(HY_RR_F20_64G_MLC_A)),
	 .timing.timing =
	 {6,		3,	8,	8,     100,    80,     200,    20,     100,    200,     0,	20,	3}},

	{"HYNIX_H9DA4GH2GJBMCR-4EM",	0xADBC,		0x00565590,
	 2048,		2048*64,	128,		4096,		0,
	 8,		2,		1,		1,
	 16,		1,		4,		3,		0,
	 (0),
	 .timing.timing =
	 {25,		10,	25,	25,	100,	60,	200,	20,	100,	100,	0,	20,	3}},

	/****************************** TOSHIBA *********************************/
	/******** we don't support operations of two-planes now, as a result of realplanenum = 1 ********/
	{"TOSHIBA_TC58TEG6DCJTA00",	0x98DE,		0x57729384,
	 16384,		4096*1024,	1280,		2092,		0,
         40,		2,              1,	        1,
         8,	        1,	        4,	        3,              0,
	 (NAND_READ_RETRY),
	 .timing.timing =
	 {10,		5,	15,	15,     30,    180,     300,    20,     100,    300,     0,	20,	5}},
};

/*
  struct nand_manufacturers nand_manuf_ids[] = {
  {NAND_MFR_TOSHIBA, "Toshiba"},
  {NAND_MFR_SAMSUNG, "Samsung"},
  {NAND_MFR_FUJITSU, "Fujitsu"},
  {NAND_MFR_NATIONAL, "National"},
  {NAND_MFR_RENESAS, "Renesas"},
  {NAND_MFR_STMICRO, "ST Micro"},
  {NAND_MFR_HYNIX, "Hynix"},
  {NAND_MFR_MICRON, "Micron"},
  {NAND_MFR_AMD, "AMD/Spansion"},
  {NAND_MFR_MACRONIX, "Macronix"},
  {NAND_MFR_EON, "Eon"},
  {0x0, "Unknown"}
  };
*/

/**
 * get_nand_flash: get nand flash attr from nand_chip_table by nand_id
 *
 * @nand_id: contain nand id nand extid
 **/
const nand_flash *get_nand_flash(nand_flash_id *fid)
{
	int index;
	int table_size = sizeof(nand_flash_table) / sizeof(nand_flash);

	for (index = 0; index < table_size; index++) {
		if ((fid->id == nand_flash_table[index].id) &&
		    ((fid->extid & EXTID_MARK) == (nand_flash_table[index].extid & EXTID_MARK)))
			return &nand_flash_table[index];
	}

	return NULL;
}
