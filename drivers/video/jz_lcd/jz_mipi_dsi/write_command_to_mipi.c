#include <config.h>
#include <common.h>
#include <jz_lcd/jz_dsim.h>
#include <jz_lcd/jz_lcd_v1_2.h>
#include <jz_lcd/auo_x163.h>

static struct dsi_device *dsi = &jz_dsi;

static int write_to_mipi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int l_count = (argc - 1) % 256; /* parameter count low 8bits */
	int h_count = (argc - 1) / 256; /* parameter count high 8bits */
	int i;

	struct dsi_cmd_packet my_data;

	if(argc < 2) {
		debug("your parameter is too few\n");
		return -1;
	}
	if(argv[1][0] >= '0' && argv[1][0] <= '9') {
		my_data.packet_type = 0x39; /* sent long packet */
		my_data.cmd0_or_wc_lsb = l_count;
		my_data.cmd1_or_wc_msb = h_count;
		for(i = 0; i < argc - 1; i++)
			my_data.cmd_data[i] = simple_strtol(argv[i+1],'\0',10);
		write_command(dsi,my_data);

		debug(" write success \n ");
		return 0;
	}
	else {
		debug(" your command is wrong \n");
		return -1;
	}
}

U_BOOT_CMD( /* now just only can write command ,can't read*/
	mipi_command,	5,	1,	write_to_mipi, /* the max num of command line param argc be set 5 */
	"write long packet to AUO_X163",
	"--eg.  write_lcd 0x2a 0x00 0x05 0x00 0x0f (0x2a is the command to control the SC & EC, 0x00 is the param)\n"
);
