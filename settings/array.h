/*
 * array.h - Handle string array settings.
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

#ifndef SETTINGS_ARRAY_H
#define SETTINGS_ARRAY_H 1

struct val_array {
  int returned;
  char* value;
  struct val_array* next;
};

int add_to_array(struct val_array** array, const char* value);
const char* get_str_from_array(struct val_array* array);
void reset_array(struct val_array* array);
void free_array(struct val_array* array);

#endif
