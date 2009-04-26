/*
 * nes.h - Interface with CopyNES hardware.
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

#ifndef COPYNESL_COPYNES_H
#define COPYNESL_COPYNES_H
#ifdef HAVE_LIBCOPYNES

int print_version(void);
int dump_cart(void);
int enter_playmode(void);
int copynes_up(copynes_t cn);
int run_plugin (copynes_t cn, const char* filepath);


#endif
#endif
