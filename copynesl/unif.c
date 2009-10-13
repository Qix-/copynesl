/*
 * unif.c - Functions dealing with unif format.
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

#include <stdint.h>
#include <stdio.h>
#include <cartctl/nes.h>
#include <settings/settings.h>

struct 
cart_unif_data* add_unif_opts(struct cart_unif_data* unif_chunks)
{
	struct cart_unif_data* result = unif_chunks;
	const char* boardname = get_string_setting("boardname");
	if (boardname) {
		result = cart_unif_add_boardname_chunk(result, boardname);
	}
	return 0;
}

