/*
 * Ingenic mensa boot mode select
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Martin <czhu@ingenic.cn>
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

#include <common.h>
#include <config.h>
#include <boot_img.h>
#include <asm/arch/cpm.h>
#include <asm/gpio.h>
#include <asm/io.h>

#define FAST_BOOT                       0
#define NORMAL_BOOT                     1
#define RECOVERY_BOOT                   2
#define FASTBOOT_RECOVERY_BOOT          3

/*
 *KEY_UNPRESS:  key is not pressed
 *KEY_PRESS:	key is pressed
*/
enum {
	KEY_UNPRESS = 0,
	KEY_PRESS,
};
/*
 *Get the status of gpio.
 *Ret: 0 or 1
 */
static int get_key_level(unsigned pin)
{
#define GPIO_DEBOUNCE  20
	int cnt = GPIO_DEBOUNCE,v = 0, t = 0;

	while (cnt--) {
		t = !!gpio_get_value(pin);
		if (v != t) {
			cnt = GPIO_DEBOUNCE;
			mdelay(1);
		}
		v = t;
	}

	return v;
}

static int get_key_status(unsigned int pin, int en_level)
{
	gpio_direction_input(pin);
	gpio_disable_pull(pin);

	return en_level == get_key_level(pin) ? KEY_PRESS : KEY_UNPRESS;
}

/*
 * Get recovery signature and reset it.
 */
static int get_recovery_signature(void)
{
	unsigned int flag = cpm_get_scrpad();

	if ((flag & 0xffff) == RECOVERY_SIGNATURE) {
		/*
	 	* Clear the signature,
	 	* reset the signature to force into normal boot after factory reset
	 	*/
		cpm_set_scrpad(flag & ~(0xffff));
		return KEY_PRESS;
	} else {
		return KEY_UNPRESS;
	}
}

/*
 * Get boot keys.
 * ret: 0: USB boot  1: normal boot  2: recovery boot
 */
static int get_boot_sel(void)
{
	/* Fast boot keys */
#if defined(CONFIG_CMD_FASTBOOT)
	if (get_key_status(CONFIG_GPIO_FASTBOOT, CONFIG_GPIO_FASTBOOT_ENLEVEL)
#ifdef CONFIG_GPIO_USB_DETECT
	    && get_key_status(CONFIG_GPIO_USB_DETECT, CONFIG_GPIO_USB_DETECT_ENLEVEL)
#endif
	) {
		return FASTBOOT_RECOVERY_BOOT;
	}
#endif
	/* Recovery signature */
	if (get_recovery_signature()) {
		return RECOVERY_BOOT;
	}

	/* Recovery boot keys */
	if (get_key_status(CONFIG_GPIO_RECOVERY, CONFIG_GPIO_RECOVERY_ENLEVEL)) {
		return RECOVERY_BOOT;
	}
	return NORMAL_BOOT;
}

/* Select boot mode */
void boot_mode_select(void)
{
	int boot_select, rc;

	/* First, handle boot keys. */
	boot_select = get_boot_sel();
	switch (boot_select) {
#if defined(CONFIG_CMD_FASTBOOT)
	case FASTBOOT_RECOVERY_BOOT:
		printf("Mod:   Fast boot mode.\n");
		rc = run_command("fastboot", 0);
		if (rc < 0) {
			printf("fastboot:command run error!");
		}
		return;
#endif
	case RECOVERY_BOOT:
		printf("Mod:   Recovery boot mode.\n");
		setenv("bootcmd", CONFIG_RECOVERY_BOOT);
		return;
	case NORMAL_BOOT:
	default:
		printf("Mod:   Normal boot mode.\n");
		setenv("bootcmd", CONFIG_NORMAL_BOOT);
	};
}
