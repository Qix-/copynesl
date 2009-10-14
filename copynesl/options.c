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
#include <string.h>
#include <stdint.h>
#include <settings/settings.h>
#include <trk_log/trk_log.h>
#include <cartctl/nes.h>
#include "options.h"
#include "plugins.h"
#include "errorcodes.h"

int validate_plugin(void);
int validate_io_settings(char* setting, enum cart_format_type disallowed_mask);
enum cart_format_type get_format_type(const char* filename);

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
		{ "copynes-version", 0, NULL, 'v', NULL, BOOLEAN_SETTING, NULL, 
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
		cmd = CMD_DUMP_CART;
	} else if (get_bool_setting("copynes-version")) {
		cmd = CMD_PRINT_VERSION;
	} else if (get_bool_setting("play-mode")) {
		cmd = CMD_PLAY_MODE;
	} else if (get_bool_setting("convert")) {
		cmd = CMD_FORMAT_CONVERT;
	} else if (get_string_setting("list-plugins")) {
		cmd = CMD_LIST_PLUGINS;
	}
	return cmd;
}

int
validate_opts(enum commands cmd)
{
	switch (cmd) {
		case CMD_DUMP_CART:
			/* plugin and outputfile required.  input not allowed. */
			if (!(validate_plugin() == 0)) {
				trk_log(TRK_ERROR, "no valid plugins were found or provided.");
				return INVALID_OPTIONS;
			} else if (validate_io_settings("input-file", 0) == 0) {
				trk_log(TRK_ERROR, "dump-cart option cannot be used with inputs.");
				return INVALID_OPTIONS;
			} else if (!(validate_io_settings("output-file", 0) == 0)) {
				trk_log(TRK_ERROR, "dump-cart option requires at least one output option.");
				return INVALID_OPTIONS;
			}
		break;
		case CMD_PRINT_VERSION:
		break;
		case CMD_PLAY_MODE:
		break;
		case CMD_FORMAT_CONVERT:
		break;
		case CMD_LIST_PLUGINS:
		break;
		case CMD_NONE:
			return INVALID_OPTIONS;
	}
	return 0;
}

/* XXX TODO INCOMPLETE */
int 
required_for_output(int packet_type) 
{
	return 1;
}

int
validate_plugin(void)
{
	const char* plugin_dir = get_string_setting("plugin-dir");
	const char* clear_plugin = get_string_setting("clear-plugin");
	const char* requested_plugin = get_string_setting("dump-cart");
	char* plugin_path;
	char* clplugin_path;
	int errorcode = 0;

	if (!plugin_dir || !clear_plugin) {
		trk_log(TRK_FATAL, "the clear plugin could not be found. ensure that your plugin directory is correct.");
		return -1;
	}
	if (!requested_plugin) {
		trk_log(TRK_ERROR, "No Plugin Specified.");
		return INVALID_OPTIONS;
	}

	clplugin_path = get_plugin_path(plugin_dir, clear_plugin);
	if (!clplugin_path) {
		free(clplugin_path);
		return INVALID_OPTIONS;
	}
	trk_log(TRK_DEBUG, "Found %s", clplugin_path);
	set_setting(STRING_SETTING, "clear-plugin", clplugin_path);
	free(clplugin_path);

	trk_log(TRK_VERBOSE, "Looking for %s", requested_plugin);
	/* we need at least a plugin setting and either output format or output file settings. */
	plugin_path = get_plugin_path(plugin_dir, requested_plugin);
	if (errorcode || !plugin_path) {
		trk_log(TRK_ERROR, "Could not find %s at %s. Ensure plugin-dir and clear-plugin settings are correct.", get_string_setting("clear_plugin"), plugin_dir);
		free(plugin_path);
		return INVALID_OPTIONS;
	}
	trk_log(TRK_DEBUG, "Found %s", plugin_path);
	set_setting(STRING_SETTING, "dump-plugin", plugin_path);
	free(plugin_path);
	return 0;
}

/* if an input is provided that does not match the disallowed_mask,
 * returns 1, otherwise, returns 0;
 */
int
validate_io_settings(char* setting, enum cart_format_type disallowed_mask)
{
	const char* cur = NULL;
	reset_string_setting(setting);
	cur = get_string_setting(setting);
	while (cur) {
		if (!(get_format_type(cur) & disallowed_mask)) {
			return 0;
		}
		cur = get_string_setting(setting);
	}
	return 1;
}

enum cart_format_type
get_format_type(const char* filename)
{
	const char* ext = NULL;
	ext = strstr(filename, ".") + 1;
	trk_log(TRK_VERBOSE, "ext: %s", ext);
	if (!strcmp(ext, "prg") || !strcmp(ext, "PRG")) {
		return FT_PRG;
	} else if (   !strcmp(ext, "chr") || !strcmp(ext, "CHR")) {
		return FT_CHR;
	} else if (   !strcmp(ext, "wram") || !strcmp(ext, "WRAM") 
		   || !strcmp(ext, "wrm") || !strcmp(ext, "WRM") 
		   || !strcmp(ext, "sav") || !strcmp(ext, "SAV")) {
		return FT_WRAM;
	} else if (!strcmp(ext, "nes") || !strcmp(ext, "NES")) {
		return FT_NES;
	} else if (   !strcmp(ext, "unif") || !strcmp(ext, "UNIF") 
		   || !strcmp(ext, "unf") || !strcmp(ext, "UNF")) {
		return FT_UNIF;
		trk_log(TRK_VERBOSE, "setting unif");
	} else {
		trk_log(TRK_ERROR, "invalid extension in output file. %s", ext);
		return 0;
	}
}

