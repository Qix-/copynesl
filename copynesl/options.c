/*
 * options.c - Specify which options are available and defaults.
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
#include <stdlib.h>
#include <settings/settings.h>
#include <trk_log/trk_log.h>
#include "options.h"
#include "errorcodes.h"

int  
init_options(int argc, char** argv)
{
	int option_index = 0;
	int errorcode = 0;
	char optchr = 0;
	static struct settings_init long_options[50] = {
		/* commands */
#if HAVE_LIBCOPYNES
		{ "dump-cart", 1, NULL, 'd', NULL, STRING_SETTING, NULL, 
			"  -d, --dump-cart=PLUGIN      Initiate a cartridge dump from the copynes.\n"},
		{ "play-mode", 0, NULL, 'p', NULL, BOOLEAN_SETTING, NULL, 
		        "  -p, --play-mode             Put copynes into play mode.\n"},
		{ "copynes-version", 0, NULL, 'n', NULL, BOOLEAN_SETTING, NULL, 
		        "  -v, --copynes-version       Print the copynes and bios version.\n"},
#endif
		{ "list-plugins", 2, NULL, 'l', NULL, STRING_SETTING, NULL, 
		        "  -l, --list-plugins=filter   List available plugin details.\n"},
		{ "convert", 0, NULL, 't', NULL, BOOLEAN_SETTING, NULL, 
		        "  -t, --convert               Convert the given input files to the provided\n"
			"                              output format.  Requires at least 1 --input-file\n"
			"                              option and either --output-file or\n"
		        "                              --output-format to be set.\n"},
		/* settings */
		{ "mapper", 1, NULL, 'm', NULL, INT_SETTING, NULL, 
		        "  -m, --mapper=MAPPER         Specify the mapper number to use in output ROMS.\n"},
		{ "boardname", 1, NULL, 'b', NULL, STRING_SETTING, NULL, 
			"  -b, --boardname=BOARD       Specify the boardname used for unif formats.\n"},
		{ "input-file", 1, NULL, 'i', NULL, STR_ARRAY_SETTING, NULL, 
			"  -i, --input-file=FILE       Specify a file to be used as input.  Extensions\n"
			"                              are used to determine file format, so only\n"
			"                              specific extensions are accepted.  This option\n"
			"                              can be repeated for multiple input files.\n"},
		{ "output-file", 1, NULL, 'o', NULL, STR_ARRAY_SETTING, NULL, 
			"  -o, --ouput-file=FILE       Specify a file to be used as ouput.  Extensions\n"
			"                              are used to determine file format, so only\n"
			"                              specific extensions are accepted.  This option\n"
			"                              can be repeated for multiple output files.\n"},
#if HAVE_LIBCOPYNES
		{ "data-port", 1, NULL, 'D', NULL, STRING_SETTING, "/dev/ttyUSB0", 
			"  -D, --data-port=DEVICE      Specify the copynes data port.\n"},
		{ "control-port", 1, NULL, 'T', NULL, STRING_SETTING, "/dev/ttyUSB1", 
			"  -T, --control-port=DEVICE   Specify the copynes control port.\n"},
#endif
		{ "debug", 0, NULL, 'g', NULL, BOOLEAN_SETTING, NULL,
			"  -d, --debug                 Print out debugging information.\n"}, 
		{ "verbose", 0, NULL, 'v', NULL, BOOLEAN_SETTING, NULL,
			"  -d, --debug                 Print out debugging information.\n"}, 
		{ "help", 0, NULL, 'h', NULL, BOOLEAN_SETTING, NULL, 
			"  -h, --help                  Print this text and exit.\n"}, 
		{ "version", 0, NULL, 'V', NULL, BOOLEAN_SETTING, NULL, 
			"  -V, --version               Print version and exit.\n"}, 
		/* this option is for non option command line arguments 
		 * (they will be treated as input files. */
		{ "input-file", 1, NULL, 1, NULL, STR_ARRAY_SETTING, NULL, NULL },
		/* Non command line options. Set only in config file or through env vars. */
		{ "sysconf-dir", -1, NULL, 0, NULL, STRING_SETTING, SYSCONFDIR "/" PACKAGE, NULL },
		{ "userconf-dir", -1, NULL, 0, NULL, STRING_SETTING, NULL, NULL },
		{ "sysdata-dir", -2, NULL, 0, NULL, STRING_SETTING, DATAROOTDIR "/" PACKAGE, NULL }, 
		{ "userdata-dir", -2, NULL, 0, NULL, STRING_SETTING, NULL, NULL }, 
		{ "plugin-dir", -1, NULL, 0, NULL, STRING_SETTING, "plugins", NULL },
		{ "clear-plugin", -1, NULL, 0, NULL, STRING_SETTING, "clear.bin", NULL },
		{ NULL, 0, NULL, 0, NULL, 0, NULL, NULL } 
	};

	errorcode = load_settings(argc, argv, long_options);
	if (errorcode != 0) return errorcode;

	if (get_bool_setting("verbose")) {
		trk_set_tracelevel(TRK_VERBOSE);
	}
	if (get_bool_setting("debug")) {
		if (get_bool_setting("verbose")) {
			trk_set_tracelevel(TRK_DEBUGVERBOSE);
		} else {			
			trk_set_tracelevel(TRK_DEBUG);
		}
	}
	return 0;
}

int  
print_invalid_options(char* program_name)
{
	int usage_argc=2;
	char* usage_argv[2];
	usage_argv[0] = program_name;
	usage_argv[1] = "-0"; /* invalid option */
	return init_options(usage_argc, usage_argv);
}

void free_options(void)
{
	free_settings();
}


enum commands 
get_command(void)
{
	enum commands cmd = 0;
	if (get_string_setting("dump-cart")) {
		cmd = DUMP_CART;
	} else if (get_bool_setting("copynes-version")) {
		cmd = PRINT_VERSION;
	} else if (get_bool_setting("play-mode")) {
		cmd = PLAY_MODE;
	} else if (get_string_setting("list-plugins")) {
		cmd = LIST_PLUGINS;
	}
	return cmd;
}


