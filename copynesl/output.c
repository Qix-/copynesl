/*
 * output.c - Functions related to data output.
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

/*
#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
*/

/* required by copynes/copynes.h */
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <copynes/copynes.h>
#include <cartctl/nes.h>
#include <trk_log/trk_log.h>
#include <settings/settings.h>
#include "nes.h"
#include "output.h"
#include "options.h"
#include "unif.h"

/* local prototypes */
int 
write_to_files(struct cart_format_data* packets)
{
	int errorcode = 0;
	const char* cur_filename = NULL;
	struct cart_unif_data* unif_chunks = NULL;

	reset_string_setting("output-file");
	cur_filename = get_string_setting("output-file");
	while (cur_filename) {
		enum cart_format_type format_type = get_format_type(cur_filename);
		switch (format_type) {
			case FT_PRG:
			case FT_CHR:
			case FT_WRAM:
				cart_pmake_raw(cur_filename, packets, format_type);
				break;
			case FT_NES:
				{
					int ines_mirrmask = get_int_setting("ines_mirrmask");
					trk_log(TRK_DEBUG, "INESMIRRMASK getting %d", (uint8_t)ines_mirrmask);
					int mapper_no = (int)get_int_setting("mapper");
					cart_pmake_nes(cur_filename, packets, mapper_no, (uint8_t)ines_mirrmask);
				}
				break;
			case FT_UNIF:
				cart_pmake_unif(cur_filename, packets, add_unif_opts(unif_chunks));

				break;
			default:
				break;
		}
		cur_filename = get_string_setting("output-file");
	}
	return 0;
}


