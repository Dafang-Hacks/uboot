#include <config.h>

#include <common.h>
#include <jz_lcd/jz_dsim.h>
#include <jz_lcd/jz_lcd_v1_2.h>
#include <jz_lcd/auo_x163.h>

static struct dsi_device *dsi = &jz_dsi;

typedef void (*MY_COMMON_FUN)(struct dsi_device *dsi);

enum my_common_name_count {
	SLEEP_ON = 0,
	SLEEP_OUT,
	DISPLAY_ON,
	DISPLAT_OUT,
	PIXELS_OFF,
	PIXELS_ON,

	COUNT_NUM,
};

static struct my_common {
	char name[20]; /* the common we enter in  the linux common line */
	MY_COMMON_FUN fun; /* the function we want to do when we enter the common*/
};

static struct my_common my_common_enter[COUNT_NUM] = {
	{ "sleep_in", auo_x163_sleep_in },
	{ "sleep_out", auo_x163_sleep_out},
	{ "display_on", auo_x163_display_on},
	{ "display_out", auo_x163_display_off},
	{ "pixels_off", auo_x163_set_pixel_off},
	{ "pixels_on", auo_x163_set_pixel_on},
};

static int write_to_auo_x163(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	if(argc < 2) {
		debug("your parameter is too few\n");
		return -1;
	}
	if(strcmp("brightness", argv[1]) == 0) { /* set brightness */
			auo_x163_set_brightness(dsi, simple_strtol(argv[2],'\0',10));
	}
	else {
		for(i = 0; i < COUNT_NUM; i++) {
			if(strcmp(my_common_enter[i].name, argv[1]) == 0) {
				/* match sleep display pixels_all_on or off */
				my_common_enter[i].fun(dsi);
				break;
			}
		}

		if(i == COUNT_NUM) { /* can't find anything */
			debug("\n your enter the common can't identy\n");
			return -1;
		}
	}
	return 0;
}

U_BOOT_CMD( /* now just only can write command ,can't read*/
	auo_command,	3,	1,	write_to_auo_x163, /* the max num of command line param argc be set 3 */
	"write command to AUO_X163",
	"eg1.  write_lcd pixels_off (the command is to off the pixels)\n"
	"eg2.  write_lcd brightness 50 ( the brightness is the command ,50 is its param\n"
);
