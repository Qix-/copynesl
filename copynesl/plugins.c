/*
 * plugins.c - Interact with plugin files and mappers.dat file.
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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <settings/settings.h>
#include <trk_log/trk_log.h>
#include "errorcodes.h"

int 
list_plugins(void)
{
	void* val;
	char* plugin_dir;
	enum settings_type type = 0;
	type = get_setting("plugin-dir", &val);
	if (type == STRING_SETTING) {
		plugin_dir = (char*) val;
	} else {
		/* error getting plugin dir */
		trk_log(TRK_ERROR, "No plugin directory specified."); 
		return INVALID_OPTIONS;
	}

	/* if plugin_dir is a relative dir,
	 * start with ./plugin_dir, then ~/.plugin-dir, then /etc/plugin-dir.
	 */
	/* XXX TODO - finish list_plugins */
	return 0;
}

static char*
lookup_plugin_by_number(const char* plugin_dir, int plugin_n)
{
	return NULL;
}

/* program accepts plugin setting as one of
 * - a number representing the numeric value
 *   from --list-plugins,
 * - a string represeting a full filepath.
 * - a string relative to plugin-dir setting.
 */
char* 
get_plugin_path(const char* plugin_dir, const char* plugin_setting)
{
	char* filepath = NULL;
	const char* plugin = NULL; 
	int errorcode = 0;
	int i = 0;
	int len = strlen(plugin_setting);
	char* output = NULL;

	for (i = 0; i < len; i++) {
		if (!isdigit(plugin_setting[i])) break;
	}
	if (i == len) {
		int plugin_n = atoi(plugin_setting);
		output = lookup_plugin_by_number(plugin_dir, plugin_n);
		return output;
	} else {
			output = get_program_filepath_d(plugin_dir, plugin_setting, DATA);
			trk_log(TRK_VERBOSE, "here %s", output);
			if (!output) {
				trk_log(TRK_ERROR, "Could not find %s at %s. Ensure plugin-dir and clear-plugin settings are correct.", plugin_setting, plugin_dir);
				free(output);
				return NULL;
			} else {
				return output;
			}
	}
	return NULL;
}


