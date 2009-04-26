/*
 * test_array.c - Test the array setting functionality.
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

#include <stdlib.h>
#include <string.h>

#include <trk_log/trk_log.h>
#include <settings/settings.h>

void init(int argc, char** argv)
{
	int option_index = 0;
	int errorcode = 0;
	char optchr = 0;
	static struct settings_init long_options[2] = {
		{ "array-test", 1, NULL, 'a', NULL, STR_ARRAY_SETTING, NULL, NULL },
		{ NULL, 0, NULL, 0,NULL,0,NULL,NULL } 
	};
	int test_argc = 7;
	char* test_argv[8] = {"test_array", "-a", "test1", "-a", "test2", "-a", "test3", NULL};

	errorcode = load_settings(test_argc, test_argv, long_options);
	if (errorcode) trk_log(TRK_ERROR, "initializing settings: %d", errorcode);
	return;
}

void test_array_setting(void)
{
	const char* test_string = NULL;

	test_string = get_string_setting("array-test");
	if (!test_string || strcmp(test_string, "test1")) {
		trk_log(TRK_ERROR, "getting array setting. Expected %s, got %s", "default", test_string);
		exit(1);
	}
	trk_log(TRK_DEBUG, "got array value %s", test_string);


	test_string = get_string_setting("array-test");
	if (!test_string || strcmp(test_string, "test2")) {
		trk_log(TRK_ERROR, "getting array setting. Expected %s, got %s", "default", test_string);
		exit(1);
	}
	trk_log(TRK_DEBUG, "got array value %s", test_string);


	test_string = get_string_setting("array-test");
	if (!test_string || strcmp(test_string, "test3")) {
		trk_log(TRK_ERROR, "getting array setting. Expected %s, got %s", "default", test_string);
		exit(1);
	}
	trk_log(TRK_DEBUG, "got array value %s", test_string);

}



int main(int argc, char** argv)
{
	trk_set_program_name("test_settings");
#if INSPECT_ERRORS 
	trk_set_tracelevel(TRK_ERROR);
#else
	trk_set_tracelevel(TRK_NONE);
#endif

	init(argc, argv);	
	trk_log(TRK_DEBUG, "initializing settings complete.");
	test_array_setting();
 
	return 0;
}
