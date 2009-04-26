/*
 * test_dir.c - Test the directory assistance functionality.
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

void test_dir_setting(void)
{
	void* val = NULL;
	int errorcode = 0;
	char* filepath = NULL;
	FILE* fp = NULL;
	fp = get_program_file("test_array", "r");
	if (fp) { 
		fclose(fp);
	} else {
		trk_log(TRK_FATAL, "getting test-invalid-filepath. result: %d", errorcode);
	}

	filepath =  get_program_filepath("test_array", DATA);
	if (errorcode) trk_log(TRK_FATAL, "Error getting program filepath. %d", errorcode);
	if (strcmp("test_array", filepath)) {
		trk_log(TRK_FATAL, "Error getting program filepath.  Expected %s, got %s", "test_array", filepath);
	}
	free(filepath);

}



int main(int argc, char** argv)
{
	trk_set_program_name("test_dir");
#if INSPECT_ERRORS 
	trk_set_tracelevel(TRK_ERROR);
#else
	trk_set_tracelevel(TRK_NONE);
#endif

	test_dir_setting();
	return 0;
}
