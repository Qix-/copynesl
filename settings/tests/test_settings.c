/*
 * test_dir.c - Test the storing and retrieving settings functionality.
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

void init()
{
	int option_index = 0;
	int errorcode = 0;
	char optchr = 0;
	static struct settings_init long_options[11] = {
		{ "bool-default", 0, NULL, 'b', NULL, BOOLEAN_SETTING, (void*)1, NULL },
		{ "int-default", 1, NULL, 'i', NULL, INT_SETTING, (void*)1, NULL },
		{ "string-default", 1, NULL, 's', NULL, STRING_SETTING, (void*)"default", NULL },
		{ "bool-cmdline", 0, NULL, 'B', NULL, BOOLEAN_SETTING, (void*)0, NULL },
		{ "int-cmdline", 1, NULL, 'I', NULL, INT_SETTING, (void*)1, NULL },
		{ "string-cmdline", 1, NULL, 'S', NULL, STRING_SETTING, (void*)"default", NULL },
		{ "bool-wierd", 1, NULL, 'X', NULL, BOOLEAN_SETTING, NULL, NULL },
		{ "int-wierd", 0, NULL, 'Y', NULL, INT_SETTING, NULL, NULL },
		{ "string-wierd", 0, NULL, 'Z', NULL, STRING_SETTING, NULL, NULL },
		{ "string-non-option", 1, NULL, 1, NULL, STRING_SETTING, NULL, NULL },
		{ NULL, 0, NULL, 0,NULL,0,NULL,NULL } 
	};
	int test_argc = 11;
	char* test_argv[12] = {"test_settings", "-B", "-I", "2", "-S", "commandline", "-X", "2", "-Y", "-Z", "nonoption", NULL};

	errorcode = load_settings(test_argc, test_argv, long_options);
	if (errorcode) trk_log(TRK_ERROR, "initializing settings: %d", errorcode);
	return;
}

void test_defaults(void)
{
	void* val = NULL;
	enum settings_type type = 0;
	type = get_setting("bool-default", &val);
	if (type != BOOLEAN_SETTING) {
		trk_log(TRK_ERROR, "getting bool-default, Invalid type. Expected %d, got %d", BOOLEAN_SETTING, type);
		exit(1);
	}
	if ((int)val != 1) {
		trk_log(TRK_ERROR, "getting bool-default. Expected %d, got %d", 1, (int)val);
		exit(1);
	}

	type = get_setting("int-default", &val);
	if (type != INT_SETTING) {
		trk_log(TRK_ERROR, "getting int-default, Invalid type. Expected %d, got %d", INT_SETTING, type);
		exit(1);
	}
	if ((int)val != 1) {
		trk_log(TRK_ERROR, "getting int-default. Expected %d, got %d", 1, (int)val);
		exit(1);
	}

	type = get_setting("string-default", &val);
	if (type != STRING_SETTING) {
		trk_log(TRK_ERROR, "getting string-default, Invalid type. Expected %d, got %d", STRING_SETTING, type);
		exit(1);
	}
	if (strcmp((char*)val, "default")) {
		trk_log(TRK_ERROR, "getting string-default. Expected %s, got %s", "default", (char*)val);
		exit(1);
	}

}

void test_cmdline(void)
{
	void* val = NULL;
	enum settings_type type = 0;
	type = get_setting("bool-cmdline", &val);
	if (type != BOOLEAN_SETTING) {
		trk_log(TRK_ERROR, "getting bool-cmdline, Invalid type. Expected %d, got %d", BOOLEAN_SETTING, type);
		exit(1);
	}
	if ((int)val != 1) {
		trk_log(TRK_ERROR, "getting bool-cmdline. Expected %d, got %d", 1, (int)val);
		exit(1);
	}

	type = get_setting("int-cmdline", &val);
	if (type != INT_SETTING) {
		trk_log(TRK_ERROR, "getting int-cmdline, Invalid type. Expected %d, got %d", INT_SETTING, type);
		exit(1);
	}
	if ((int)val != 2) {
		trk_log(TRK_ERROR, "getting int-cmdline. Expected %d, got %d", 2, (int)val);
		exit(1);
	}

	type = get_setting("string-cmdline", &val);
	if (type != STRING_SETTING) {
		trk_log(TRK_ERROR, "getting string-cmdline, Invalid type. Expected %d, got %d", STRING_SETTING, type);
		exit(1);
	}
	if (strcmp((char*)val, "commandline")) {
		trk_log(TRK_ERROR, "getting string-cmdline. Expected %s, got %s", "commandline", (char*)val);
		exit(1);
	}

}

/* defined behavior:
 *   bool with an argument: atoi(argument)
 *   int with no argument: -1
 *   string with no argument: ""
 */
void test_wierdos(void)
{
	void* val = NULL;
	enum settings_type type = 0;
	type = get_setting("bool-wierd", &val);
	if (type != BOOLEAN_SETTING) {
		trk_log(TRK_ERROR, "getting bool-wierd, Invalid type. Expected %d, got %d", BOOLEAN_SETTING, type);
		exit(1);
	}
	if ((int)val != 2) {
		trk_log(TRK_ERROR, "getting bool-wierd. Expected %d, got %d", 1, (int)val);
		exit(1);
	}

	type = get_setting("int-wierd", &val);
	if (type != INT_SETTING) {
		trk_log(TRK_ERROR, "getting int-wierd, Invalid type. Expected %d, got %d", INT_SETTING, type);
		exit(1);
	}
	if ((int)val != -1) {
		trk_log(TRK_ERROR, "getting int-wierd. Expected %d, got %d", -1, (int)val);
		exit(1);
	}

	type = get_setting("string-wierd", &val);
	if (type != STRING_SETTING) {
		trk_log(TRK_ERROR, "getting string-wierd, Invalid type. Expected %d, got %d", STRING_SETTING, type);
		exit(1);
	}
	if (strcmp((char*)val, "")) {
		trk_log(TRK_ERROR, "getting string-wierd. Expected %s, got %s", "true", (char*)val);
		exit(1);
	}

	type = get_setting("string-non-option", &val);
	if (type != STRING_SETTING) {
		trk_log(TRK_ERROR, "getting string-non-option, Invalid type. Expected %d, got %d", STRING_SETTING, type);
		exit(1);
	}
	if (strcmp((char*)val, "nonoption")) {
		trk_log(TRK_ERROR, "getting string-wierd. Expected %s, got %s", "true", (char*)val);
		exit(1);
	}


	type = get_setting("string-invalid", &val);
	if (type != EMPTY_SETTING) {
		trk_log(TRK_ERROR, "getting string-invalid, Invalid type. Expected %d, got %d", EMPTY_SETTING, type);
		exit(1);
	}

}

void test_set()
{
	int testval = 0;
	int returnval = 0;
	set_setting(INT_SETTING, "test-set", (void*)testval);
	returnval = (int)get_int_setting("test-set");
	if (testval != returnval) trk_log(TRK_FATAL, "failed test_set().  testval: %x, returnval %x", testval, returnval);
	trk_log(TRK_DEBUG, "set val %d matches get val %d", testval, returnval);
	testval = 2;
	set_setting(INT_SETTING, "test-set2", (void*)testval);
	returnval = (int)get_int_setting("test-set2");
	if (testval != returnval) trk_log(TRK_FATAL, "failed test_set().  testval: %x, returnval %x", testval, returnval);
	trk_log(TRK_DEBUG, "set val %d matches get val %d", testval, returnval);
}

int main(int argc, char** argv)
{
	trk_set_program_name("test_settings");
#if INSPECT_ERRORS 
	trk_set_tracelevel(TRK_ERROR);
#else
	trk_set_tracelevel(TRK_NONE);
#endif

	init();	
	test_defaults();
 	test_cmdline();
	test_wierdos();
	test_set();
	return 0;
}
