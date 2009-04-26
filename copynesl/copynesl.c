/*
 * copynesl - Interface and utilities for CopyNES hardware. 
 *
 * copynesl.c - Determine and execute the appropriate command.
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

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <trk_log/trk_log.h>
#include <copynes/copynes.h>
#include "options.h"
#include "plugins.h"
#include "errorcodes.h"
#include "nes.h"

int main(int argc, char** argv)
{
	int errorcode = 0;
	enum commands cmd = 0;
	init_options(argc, argv);
	trk_set_program_name(argv[0]);
	
	cmd = get_command();
	switch(cmd) {
#if HAVE_LIBCOPYNES
		case DUMP_CART:
			errorcode = dump_cart();
			break;
		case PRINT_VERSION:
			errorcode = print_version();
			break;
		case PLAY_MODE:
			errorcode = enter_playmode();
			if (!errorcode) printf("Playmode successfully entered.\n");
			break;
#endif
		case LIST_PLUGINS:
			errorcode = list_plugins();
			break;
		default:
			errorcode = INVALID_OPTIONS;
			break;
	}
	if (errorcode == INVALID_OPTIONS) {
		print_invalid_options(argv[0]); 
	} else if (errorcode == COPYNES_OFF) {
		trk_log(TRK_ERROR, "CopyNES is OFF.  This option requires the copynes to be on.");
		return COPYNES_OFF;
	}
	/*}
	 */
	free_options();
	return 0;
}


