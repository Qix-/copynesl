/*
 * settings.h - Store and restore settings for programs.
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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_
#include <stdio.h>

#define MAX_CONFIGFILE_PATH 255

enum srcfile_type {
  CONFIG = 1,
  DATA = 2
};

enum settings_type {
  EMPTY_SETTING = 0,
  BOOLEAN_SETTING,
  INT_SETTING,
  STRING_SETTING,
  STR_ARRAY_SETTING
};

struct settings_init {
        const char* long_opt;
	int         has_arg;
	int*        flag;
	int         short_opt;
	const char* env_var;
	int	    val_type;
	void* default_val;
	const char* usage_long;
};

/* load_settings returns negative on error, 0 on regular completion, 1 if help printed, 2 if version printed.
 */
extern int load_settings(int argc, char** argv, struct settings_init* init);
extern enum settings_type get_setting(const char* key, void** value);
extern int set_setting(enum settings_type type, const char* key, void* value);
extern void settings_usage(char* program_name, struct settings_init* init);
extern void free_settings();

/* convinience get functions */
extern int get_bool_setting(const char* setting);
extern int get_int_setting(const char* setting);
extern const char* get_string_setting(const char* setting);
extern int reset_string_setting(const char* setting);

/* check a bunch of directories for test_filename based on setting_val */
FILE* get_program_file(const char* filename, const char* file_opts);
FILE* get_program_file_d(const char* directory, const char* filename, const char* file_opts);
char* get_program_filepath(const char* filename, enum srcfile_type type);
char* get_program_filepath_d(const char* directory, const char* filename, enum srcfile_type type);
long get_filesize(FILE* file);

#endif /* _SETTINGS_H_ */
