/*
 * nes.h - Manipulation of iNES and related ROM formats.
 *
 * Copyright (C) Bjorn Hedin 2009 <cradelit@gmail.com>
 * Copyright (C) David Huseby 2009 <dave@linuxprogrammer.org>
 * 
 * This file is part of libcartctl.
 *
 * libcartctl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libcartctl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libcartctl.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CARTCTL_NES_H
#define CARTCTL_NES_H

#define UNIF_REVISION 7

#define HORIZONTAL_MIRRORING 0
#define VERTICAL_MIRRORING 1
#define HAS_BATTERY 2
#define TRAINER 4
#define FOUR_SCREEN_VROM 8

typedef struct cart_unif_chunk_header {
	char id[4];
	uint32_t size;
} cart_unif_chunk_header_t;

typedef struct cart_unif_chunk {
	struct cart_unif_chunk_header header;
	uint8_t* data;
} cart_unif_chunk_t;


typedef struct cart_dumperinfo
{
	char dumper_name[100]; /* NULL-terminated string containing the name
				  of the person who dumped the cart. */
	uint8_t day; /* Day of the month when cartridge was dumped */
	uint8_t month; /* Month of the year when cartridge was dumped */
	char year[4]; /* Year during which the cartridge was dumped */
	char dumper_agent[100]; /* NULL-terminated string containing the name of
	                                   the ROM-dumping means used */
} cart_dumperinfo_t;

/* make a .nes file out of a prg and chr from memory */
extern int cart_make_nes(FILE* output, long prg_size_in_bytes, uint8_t* prg, long chr_size_in_bytes, uint8_t* chr, uint8_t mapper_no, uint8_t mirroring_mask);
/* make a .nes file out of a prg file and a chr file */
extern int cart_fmake_nes(FILE* output, FILE* prg, FILE* chr, uint8_t mapper_no, uint8_t mirroring_mask);

/* make a .unif file out of a group of unif chunks from memory */
extern int cart_make_unif(FILE* output, struct cart_unif_chunk* chunks, int num_chunks);

/* Functions used to create unif chunks. */

/* Used to specify the unif boardname. */
struct cart_unif_chunk cart_unif_boardname_chunk(char* boardname);
/* Add a dumper's name to the file. */
struct cart_unif_chunk cart_unif_dumpername_chunk(char* dumpername);
/* Add info about the dumper */
struct cart_unif_chunk cart_unif_dumperinfo_chunk(struct cart_dumperinfo dumperinfo);
/* Add one chunk using this function for each prg chip */
struct cart_unif_chunk cart_unif_prg_chunk(uint32_t size, uint8_t* prg_data, int chip_number);
/* Add one chunk using this function for each chr chip */
struct cart_unif_chunk cart_unif_chr_chunk(uint32_t size, uint8_t* chr_data, int chip_number);
/* Generic Unif chunk function.  Use if no specific function is available for the desired chunk. */
struct cart_unif_chunk cart_unif_chunk(char id[4], uint32_t data_size, uint8_t* data);

#endif
