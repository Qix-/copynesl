/*
 * dir.c - Facilitate getting program config and data files.
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

/*
#include <unistd.h>
#include <stdarg.h>
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <trk_log/trk_log.h>

#include "settings.h"
#include "dir.h"

static FILE* get_file(const char* test_filename, const char* file_opts);
static char* get_filepath (const char* test_filename);

/*
 * return the full path to the first location filename is found.
 * checks:
 *  if setting is absolute,
 *  if it is in user dir (either conf dir or sys dir depending on FILETYPE)
 *  if it is in system dir (either conf dir or sys dir depending on FILETYPE)
 *
 *  note this function can return 0 if settings_val is already 
 *  the correct path.  This is so that specifying absolute paths
 *  will still work despite being over max_len since they
 *  don't need the buffer.
 *
 *  returns 0 is setting_val is fine as is
 *  returns 1 if result_buffer is set and is fine
 *  returns negative on error.
 */
FILE* get_program_file(const char* filename, const char* file_opts)
{
	int errorcode = 0;
/*	const char* home = getenv("HOME");
	int used_len = 0;
	int testdir_len = 0;
	int testfile_len = strlen(test_filename);
	int setting_len = strlen(setting_val);
	int home_len = strlen(home);
	char* testdir;
*/

	int filename_len = 0;
	const char* userconfdir = NULL;
	int userconfdirlen = 0;
	const char* sysconfdir = NULL;
	char* testfile = NULL;
	int testfilelen = 0;
	FILE* output = NULL;

	filename_len = strlen(filename);
	userconfdir = get_string_setting("userconf-dir");
	sysconfdir = get_string_setting("sysconf-dir");

	testfilelen = filename_len;

	output = get_file(filename, file_opts);
	if (output) return output;
     	
	if (userconfdir) {
	        testfilelen = strlen(userconfdir) + 1 + filename_len;
		testfile = (char*) malloc(testfilelen);
		sprintf(testfile, "%s%c%s", userconfdir, DIR_SEPARATOR_CHAR,filename);
		output = get_file(testfile, file_opts);
		free(testfile);
		if (output) return output;
	}
	if (sysconfdir) {
	        testfilelen = strlen(sysconfdir) + 1 + filename_len;
		testfile = (char*) malloc(testfilelen);
		sprintf(testfile, "%s%c%s", sysconfdir, DIR_SEPARATOR_CHAR,filename);
		output = get_file(testfile, file_opts);
		free(testfile);
		if (output) return output;
	}	
	
	return NULL;
}

static FILE* 
get_file(const char* test_filename, const char* file_opts)
{
	FILE* fp = NULL;
	trk_log(TRK_DEBUG, "Checking for %s", test_filename);
	fp = fopen(test_filename, "r");
	return fp;
}

FILE*
get_program_file_d(const char* directory, const char* filename, const char* file_opts)
{
	char* filepath = NULL;
	int errorcode = 0;
	FILE* output = NULL;

	filepath = (char*)malloc(strlen(directory) + 1 + strlen(filename) + 1);
	if (!filepath) return NULL;

	errorcode = sprintf(filepath, "%s%c%s", directory, DIR_SEPARATOR_CHAR, filename);
	if (errorcode) return NULL;

	output = get_program_file(filepath, file_opts);
	free(filepath);
	return output;
}


char*
get_program_filepath(const char* filename, enum srcfile_type type)
{
	int errorcode = 0;
/*	const char* home = getenv("HOME");
	int used_len = 0;
	int testdir_len = 0;
	int testfile_len = strlen(test_filename);
	int setting_len = strlen(setting_val);
	int home_len = strlen(home);
	char* testdir;
*/

	int filename_len = 0;
	const char* userdir = NULL;
	int userconfdirlen = 0;
	const char* sysdir = NULL;
	char* testfile = NULL;
	int testfilelen = 0;
	char* output = NULL;

	filename_len = strlen(filename);
	if (type == CONFIG) {
		userdir = get_string_setting("userconf-dir");
		sysdir = get_string_setting("sysconf-dir");
	} else if (type == DATA) {
		userdir = get_string_setting("userdata-dir");
		sysdir = get_string_setting("sysdata-dir");
	} else {
		userdir = NULL;
		sysdir = NULL;
	}

	testfilelen = filename_len;
	
	output = get_filepath(filename);
	if (output) return output;
	     	
	if (userdir) {
	        testfilelen = strlen(userdir) + 1 + filename_len;
		testfile = (char*) malloc(testfilelen);
		sprintf(testfile, "%s%c%s", userdir, DIR_SEPARATOR_CHAR,filename);
		trk_log(TRK_VERBOSE, "Checking path: %s", testfile);
		output = get_filepath(testfile);
		free(testfile);
		if (output) return output;
	}
	if (sysdir) {
	        testfilelen = strlen(sysdir) + 1 + filename_len;
		testfile = (char*) malloc(testfilelen);
		sprintf(testfile, "%s%c%s", sysdir, DIR_SEPARATOR_CHAR,filename);
		trk_log(TRK_VERBOSE, "Checking path: %s", testfile);
		output = get_filepath(testfile);
		free(testfile);
		if (output) return output;
	}	
	
	return NULL;
}

static char* 
get_filepath (const char* test_filename)
{
	FILE* fp = NULL;
	char* filepath = NULL;
	char* output = NULL;

	trk_log(TRK_DEBUG, "Checking for %s", test_filename);
	fp = fopen(test_filename, "r");
	if (fp) {
		output = (char*)malloc(strlen(test_filename) + 1);
		output = strcpy(output, test_filename);
		trk_log(TRK_VERBOSE, "%s found.", output);
         	fclose(fp);
		return output;
        } else {
		return NULL;
	}
}

char* 
get_program_filepath_d(const char* directory, const char* filename, enum srcfile_type type)
{
	char* filepath = NULL;
	int errorcode = 0;
	int dirlen = 0;
	char* output = NULL;

	trk_log(TRK_VERBOSE, "here");
	trk_log(TRK_VERBOSE, "directory %s filename %s", directory, filename);
	if (!filename) return NULL;
	
	if (directory) {
		dirlen = strlen(directory) + 1;
	}

	filepath = (char*)malloc(dirlen + strlen(filename) + 1);
	if (!filepath) return NULL;
	filepath = strcpy(filepath, filename);
	if (!filepath) return NULL;
	
	trk_log(TRK_VERBOSE, "checking for direct path %s", filename);
	output = get_program_filepath(filepath, type);
	if (output) {
		free(filepath);
		return output;
	} 
	
	errorcode = sprintf(filepath, "%s%c%s", directory, DIR_SEPARATOR_CHAR, filename);
	if (errorcode <= 0) return NULL;

	trk_log(TRK_VERBOSE, "checking for direct path %s", filename);
	output = get_program_filepath(filepath, type);
	free(filepath);
	return output;
}
