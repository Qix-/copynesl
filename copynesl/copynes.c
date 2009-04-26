/*
 * copynes.c - Interface with CopyNES hardware.
 *
 * Copyright (C) Bjorn Hedin 2009 <cradelit@gmail.com>
 * Copyright (C) David Huseby 2009 <dave@linuxprogrammer.org>
 * 
 * This file is part of copynesl.
 *
 * copynesl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * copynesl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with copynesl.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* need to find a better solution, but this is required by copynes.h */
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <copynes/copynes.h>
#include <settings/settings.h>
#include <trk_log/trk_log.h>
#include "nes.h"
#include "errorcodes.h"
#include "plugins.h"


int print_version()
{
	int errorcode = 0;
	char version_str[255];
	copynes_t cn = copynes_new();
	errorcode = copynes_up(cn);
	if (errorcode) return errorcode;
/*	copynes_flush(cn);
 */
	errorcode = copynes_get_version(cn, &version_str, 255);
	if (errorcode < 0) return errorcode;
	printf("%s\n", version_str);
	copynes_free(cn);
	return errorcode;
}

int
run_plugin (copynes_t cn, const char* filepath)
{
    
	/* get the CopyNES ready for loading a plugin */
	copynes_reset(cn, RESET_COPYMODE);
    
	/* load the "clear" plugin to clean the 6502 RAM */
	copynes_load_plugin(cn, filepath);
    
	/* run it */
	copynes_run_plugin (cn);
	return 0;
}

int enter_playmode(void)
{
	int errorcode = 0;
	char version_str[255];
	copynes_t cn = copynes_new();
	errorcode = copynes_up(cn);
	if (errorcode) return errorcode;
	copynes_flush(cn);

	copynes_reset(cn, RESET_PLAYMODE);
	printf("Play testing.  Press a key to Exit.\n");
	fgetc(stdin);
	copynes_free(cn);
	return errorcode;

}

int 
copynes_up(copynes_t cn)
{
	copynes_open(cn, get_string_setting("data-port"), get_string_setting("control-port"));
	if (!copynes_nes_on(cn)) { 
		copynes_free(cn);
		return COPYNES_OFF;
	}
	return 0;
}
