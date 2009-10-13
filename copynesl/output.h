/*
 * output.h - Functions related to data output.
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

#ifndef COPYNESL_OUTPUT_H
#define COPYNESL_OUTPUT_H

#include <copynes/copynes.h>

int write_to_files(copynes_packet_t* packets, int npackets, uint8_t copynes_mirroring_mask);
int dump_to_file(const char* filename, copynes_packet_t* packets, int npackets, int format_type);
int dump_to_nes_file(const char* filename, copynes_packet_t* packets, int npackets, uint8_t ines_mirrmask);
int dump_to_unif_file(const char* filename, copynes_packet_t* packets, int npackets);

#endif
