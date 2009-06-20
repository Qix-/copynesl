/*
 * options.h - Specify which options are available and defaults.
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

#ifndef COPYNESL_OPTIONS_H
#define COPYNESL_OPTIONS_H

enum commands {NONE = 0, DUMP_CART, PRINT_VERSION, PLAY_MODE, FORMAT_CONVERT, LIST_PLUGINS };

int init_options(int argc, char** argv);
int print_invalid_options(char* program_name);
void free_options(void);
enum commands get_command(void);

#endif
