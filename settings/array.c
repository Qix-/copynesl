/*
 * array.c - Handle string array settings.
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

#include "array.h"

int
add_to_array(struct val_array** array, const char* value)
{
	struct val_array* cur;
	if (!*array) {
		*array = (struct val_array*)malloc(sizeof(struct val_array));	
		cur = *array;
	} else {
		cur = *array;
		while (cur->next) cur = cur->next;
		cur->next = (struct val_array*)malloc(sizeof(struct val_array));
		cur = cur->next;
	}
	cur->returned = 0;
  	cur->value = (char*)malloc(strlen(value) + 1);
	strcpy((char*)cur->value, value);
	cur->next = NULL;
	return 0;
}

const char* 
get_str_from_array(struct val_array* array)
{
	struct val_array* cur;
	cur = array;
	while (cur && cur->returned) cur = cur->next;
	if (cur) {
		cur->returned = 1;
		return cur->value;
	} else {
		return NULL;
	}
}

void 
reset_array(struct val_array* array)
{
	struct val_array* cur;
	cur = array;
	while (cur) {
		cur->returned = 0;
		cur = cur->next;
	}
}

void
free_array(struct val_array* array)
{
	struct val_array* cur;
	cur = array;
	while (cur) {
		cur->returned = 0;
		if (cur->value) free(cur->value);
		cur = cur->next;
	}
}
