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
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <settings/settings.h>
#include <trk_log/trk_log.h>
#include "errorcodes.h"

/* return 1 if needle is in haystack (case insensitive)
 */
static int 
contains(const char* haystack, const char* needle)
{
	char* haystack_lower = NULL;
	char* needle_lower = NULL;
	char* match = NULL;
	long haystack_len = 0;
	long needle_len = 0;
	long i = 0;

	haystack_len = strlen(haystack + 1);
	haystack_lower = (char*)malloc(haystack_len);
	for (i = 0; i < haystack_len; i++) haystack_lower[i] = tolower(haystack[i]);
	haystack_lower[haystack_len] = '\0';

	needle_len = strlen(needle);
	needle_lower = (char*)malloc(needle_len + 1);
	for (i = 0; i < needle_len; i++) needle_lower[i] = tolower(needle[i]);
	needle_lower[needle_len] = '\0';

	match = strstr(haystack_lower, needle_lower);
	free(haystack_lower);
	free(needle_lower);
	if (match) { 
		return 1;
	} else {
		return 0;
	}

}

int 
list_plugins(void)
{
	void* val;
	char* plugin_dir;
	enum settings_type type = 0;
	FILE* mappers_dat = NULL;
	char* mappers_dat_path = NULL;
	const char* filter = get_string_setting("list-plugins");
	long mappers_dat_size = 0;
	char* buffer = NULL;
	char** lines = NULL;
	long linecount = 0;
	long i = 0;
	long j = 0;
	long readcount = 0;
	int linelen = 0;
	char* outline = NULL;
	int printing_section = 0;
	char* header_printed = 0;

	type = get_setting("plugin-dir", &val);
	if (type == STRING_SETTING) {
		plugin_dir = (char*) val;
	} else {
		/* error getting plugin dir */
		trk_log(TRK_ERROR, "No plugin directory specified."); 
		return INVALID_OPTIONS;
	}
	
	trk_log(TRK_VERBOSE, "list-plugins filter: %s", filter);

	mappers_dat_path = get_program_filepath("mappers.dat", DATA);

	if (!mappers_dat_path) {
		trk_log(TRK_ERROR, "Could not find mappers.dat file. Ensure dataroot directory is correct.");
		free(mappers_dat_path);
		return -1;
	} 

	/* open mappers.dat */
	mappers_dat = fopen(mappers_dat_path, "r");
	mappers_dat_size = get_filesize(mappers_dat);
	if (mappers_dat_size < 0) {
		trk_log(TRK_ERROR, "invalid mappers.dat file! size returned: %d", mappers_dat_size);
	}
	trk_log(TRK_DEBUGVERBOSE, "mappers.dat filesize: %d", mappers_dat_size);

	buffer = (char*)malloc(mappers_dat_size); /* ensure no overflow by using file size */
	readcount = fread(buffer, mappers_dat_size, sizeof(char*), mappers_dat);
	for (i = 0; i < mappers_dat_size; i++) { 
		/* seperate into strings by line */
		if (buffer[i] == '\n') { 
			buffer[i] = '\0';
			linecount++;
		}
	}
	lines = (char**) malloc(sizeof(char*) * linecount);
	trk_log(TRK_DEBUGVERBOSE, "linecount %d", linecount);
	j = 0;

	if (strlen(filter) <= 0) filter = NULL;

	printf ("    boardname                  plugin       mapper  description\n");
	printf ("    ---------                  ------       ------  -----------\n");
	for (i = 0; i < linecount; i++) {
		lines[i] = (char*)&(buffer[j]);
		linelen = strlen(lines[i]);
		j += linelen + 1;
		if (i > 1) { /* skip first two lines of junk */
			if (lines[i][0] == '*') { /* header */
				if (outline) free(outline);
				header_printed = 0;
				outline = (char*) malloc(linelen);
				sscanf(lines[i], "* %*[^ ] %[^\n]", outline);
				if (!filter || contains(outline, filter)) {
					printing_section = 1;
				} else {
					printing_section = 0;
				}
			} else {
				if (!filter || printing_section || contains(lines[i], filter)) {
					if (! header_printed) {
							printf("\n%s\n", outline);
							header_printed = 1;
							free(outline);
							outline = NULL;
					}
					printf("    %s\n", lines[i]);
					 
				}
			
			}
			
		}
	}
	if (outline) free(outline);
	free(buffer);
	/* discart data until the first header. */


	fclose(mappers_dat);

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


