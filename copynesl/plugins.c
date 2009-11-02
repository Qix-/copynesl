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

char* get_plugin_path(const char* plugin_dir, const char* plugin_setting);
int show_plugin_info(const char* plugin_path);

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

	haystack_len = strlen(haystack);
	haystack_lower = (char*)malloc(haystack_len + 1);
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

/* return the number of lines */
long split_into_lines(char* istr, char*** lines)
{
	long len = 0;
	int max_line_len = 75;
	int linelen = 0;
	long linecount = 0;
	int i = 0;
	int j = 0;
	char* str = istr;


	len = strlen(str);	


	linecount = 1;
	for (i = 0; i < len; i++) { 
		if (str[i] == '\r') {
			str[i] = '\0';
		}
		/* seperate into strings by line */
		if (str[i] == '\n') { 
			str[i] = '\0';
			linecount++;
		}
	}

	(*lines) = (char**) malloc(sizeof(char*) * linecount);
	for (i = 0; i < linecount; i++) {
		(*lines)[i] = (char*)&(str[j]);
		linelen = strlen((*lines)[i]);
		trk_log(TRK_DEBUGVERBOSE, "linelen=%d\n", linelen);
		j += linelen + 1;
		if (str[j + 1] == '\0') {
			j++;
		} 
		j++;
		trk_log(TRK_DEBUGVERBOSE, "lines[i]=: %s", (*lines)[i]);
	}

	trk_log(TRK_DEBUGVERBOSE, "%d", linecount);

	return linecount;
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
	int header_printed = 0;
	const char* plugin_path;

	type = get_setting("plugin-dir", &val);
	if (type == STRING_SETTING) {
		plugin_dir = (char*) val;
	} else {
		/* error getting plugin dir */
		trk_log(TRK_ERROR, "No plugin directory specified."); 
		return INVALID_OPTIONS;
	}
	
	plugin_path = get_plugin_path(plugin_dir, filter);
	trk_log(TRK_VERBOSE, "list-plugins filter: %s", filter);

	mappers_dat_path = get_program_filepath("mappers.dat", DATA);

	if (!mappers_dat_path) {
		trk_log(TRK_ERROR, "Could not find mappers.dat file. Ensure dataroot directory is correct.");
		free(mappers_dat_path);
		return -1;
	} 

	/* open mappers.dat */
	mappers_dat = fopen(mappers_dat_path, "r");
	free(mappers_dat_path);
	mappers_dat_size = get_filesize(mappers_dat);
	if (mappers_dat_size < 0) {
		trk_log(TRK_ERROR, "invalid mappers.dat file! size returned: %d", mappers_dat_size);
	}
	trk_log(TRK_DEBUGVERBOSE, "mappers.dat filesize: %d", mappers_dat_size);

	buffer = (char*)malloc(mappers_dat_size * sizeof(char*) + 1); /* ensure no overflow by using file size */
	readcount = fread(buffer, sizeof(char*), mappers_dat_size, mappers_dat);
	buffer[mappers_dat_size] = '\0';

	linecount = split_into_lines(buffer, &lines);
	trk_log(TRK_DEBUGVERBOSE, "linescount=%d", linecount);

	if (strlen(filter) <= 0) filter = NULL;

	printf ("    boardname                  plugin       mapper  description\n");
	printf ("    ---------                  ------       ------  -----------\n");
	/* -1 is because file ends with an "end" header we want to skip. */
	for (i = 0; i < linecount - 1; i++) {
		if (i > 1) { /* skip first two lines of junk */
			if (lines[i][0] == '*') { /* header */
				if (outline) free(outline);
				header_printed = 0;
				outline = (char*) malloc(strlen(lines[i]));
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
	free(lines);
	free(buffer);
	/* discart data until the first header. */


	fclose(mappers_dat);

	show_plugin_info(plugin_path);
	
	

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
			trk_log(TRK_DEBUGVERBOSE, "get_plugin_path filepath %s", output);
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

int printf_hanging_indent(const char* indent, char* str)
{
	char** lines = NULL;
	long linecount = 0;
	int i = 0;
	linecount = split_into_lines(str, &lines);
	trk_log(TRK_DEBUGVERBOSE, "linecount: %d, lines[0] %s", linecount, lines[0]);
	 printf(lines[0]);
	 
	
	for (i = 1; i < linecount; i++) { 
		printf("%s%s\n",indent, lines[i]);
 
	}
	
	free (lines);

	return 0;
}

int
show_plugin_info(const char* plugin_path)
{
	typedef struct plugin_header {
		char description[96];
		char author[24] ;
		uint8_t prg_min_max[2];
		uint8_t chr_min_max[2];
		uint8_t wram_min_max[2];
		uint8_t plugin_ver[1];
		uint8_t flags[1];
	} plugin_header_t;

	typedef struct uservar_header {
		/* note, description is actually 14,
		 * enabled 1, value 1 but enabled and value are
		 * only used when plugin is running
		 * and description needs a space for a terminator.
		 */
		char description[16];
	} uservar_header_t;

	enum usrvar_flags {
		downloading = 0x01,
		uploading = 0x02,
		uservar1 = 0x04,
		uservar2 = 0x08,
		uservar3 = 0x16,
		uservar4 = 0x32
	};


	FILE* f = 0;
	int i;
	uint8_t* prg = 0;
	struct plugin_header header;
	struct uservar_header varhdr;

	if((f = fopen(plugin_path, "rb")) == 0)
	{
		return -1;
	}
	fread(&header, sizeof(header), 1, f); 

	/* if the string specified was a plugin name
	 * then print out extra plugin info about that plugin
	 */
	if (plugin_path) {
		int i = 0;
		int len = 0;
		len = 16 + strlen(plugin_path);
		printf("\n");
		for (i = 0; i < len; i++) printf("-");
		printf("\n");
		printf("Plugin info for %s\n", plugin_path);
		for (i = 0; i < len; i++) printf("-");
		printf("\n");
		printf("\n");
		printf("    description: ");
		printf_hanging_indent("                 ", header.description);
	 
		printf("\n");
		printf("    author:      %s\n", header.author);
		printf("    uservars: \n");
		
	}

	/* try to open the plugin file */

	/* send the command to store the plugin prg data at 0400h */
	/*
	if(copynes_write(cn, CMD_LOAD_PLUGIN, CMD_SIZE(CMD_LOAD_PLUGIN)) != CMD_SIZE(CMD_LOAD_PLUGIN))
	{
		fclose(f);
		cn->err = FAILED_COMMAND_SEND;
		return -cn->err;
	}
	*/

	/* seek to the plugin prg data */
	fseek(f, -64, SEEK_END);

	fread(&varhdr, sizeof(varhdr), 1, f); 
	if ((header.flags[0] & uservar1) && (varhdr.description[0] != -1)) {
		varhdr.description[15] = '\0';
		printf("       1:        %s\n", varhdr.description);
	}
	
	fread(&varhdr, sizeof(varhdr), 1, f); 
	if ((header.flags[0] & uservar2) && (varhdr.description[0] != -1)) {
		varhdr.description[15] = '\0';
		printf("       2:        %s\n", varhdr.description);
	}

	fread(&varhdr, sizeof(varhdr), 1, f); 
	if ((header.flags[0] & uservar3) && (varhdr.description[0] != -1)) {
		varhdr.description[15] = '\0';
		printf("       3:        %s\n", varhdr.description);
	}
	
	fread(&varhdr, sizeof(varhdr), 1, f); 
	if ((header.flags[0] & uservar4) && (varhdr.description[0] != -1)) {
		varhdr.description[15] = '\0';
		printf("       4:        %s\n", varhdr.description);
	}
	fclose(f);
	/*
	prg = calloc(KB(1), sizeof(uint8_t));
	*/
	/* read in the plugin prg data */	
	/*fread(prg, KB(1), sizeof(uint8_t), f);
	*/
	return 0;
}
