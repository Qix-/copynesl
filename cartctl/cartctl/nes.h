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

#define CART_UNIF_REVISION 7

#define CART_HORIZONTAL_MIRRORING 0
#define CART_VERTICAL_MIRRORING 1
#define CART_HAS_BATTERY 2
#define CART_TRAINER 4
#define CART_FOUR_SCREEN_VROM 8

enum cart_format_type { FT_NONE = 0, FT_PRG, FT_CHR, FT_WRAM, FT_NES, FT_UNIF };
typedef struct cart_format_data
{
	unsigned int datasize;
	enum cart_format_type datatype; 
	uint8_t* data;
	struct cart_format_data* next;
} cart_format_data_t;

typedef struct cart_unif_data {
	struct cart_unif_chunk* chunk;
	struct cart_unif_data* next;
} cart_unif_data_t;

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

extern int cart_free_packets(struct cart_format_data** packets);

extern unsigned short cart_has_wram(struct cart_format_data* packets);

/* make a raw file (straight data dump of all format_type packets) to the specified file */
extern int cart_pmake_raw(const char* filename, struct cart_format_data* packets, enum cart_format_type format_type);
/* add a raw file to packets */
extern int cart_psplit_raw(const char* filename, struct cart_format_data** packets, enum cart_format_type format_type);

/* make a .nes file out of a prg and chr from memory */
extern int cart_make_nes(FILE* output, long prg_size_in_bytes, uint8_t* prg, long chr_size_in_bytes, uint8_t* chr, uint8_t mapper_no, uint8_t mirroring_mask);
/* make a .nes file out of a prg file and a chr file */
extern int cart_fmake_nes(FILE* output, FILE* prg, FILE* chr, uint8_t mapper_no, uint8_t mirroring_mask);
/* make a nes file out of a cart_format_data* */
extern int cart_pmake_nes(const char* filename, struct cart_format_data* packets, int mapper_no, uint8_t ines_mirr_mask);

/* split a .nes file into it's peices: prg, chr, mapper number and mirroring mask */
extern int cart_split_nes(FILE* nesfile, uint8_t** oprg, uint8_t** ochr, uint8_t* omapper, uint8_t* omirroring_mask);

extern int cart_psplit_nes(const char* filename, struct cart_format_data** packets, int* omapper_no, uint8_t* oines_mirr_mask);

/* make a .unif file out of a group of unif chunks from memory */
extern int cart_make_unif(FILE* output, struct cart_unif_data* chunks);
/* make a unif file out of a cart_format_data* */
extern int cart_pmake_unif(const char* filename, struct cart_format_data* packets, struct cart_unif_data* options);

/* Functions used to create unif chunks. */

/* Used to specify the unif boardname. */
struct cart_unif_data* cart_unif_add_boardname_chunk(struct cart_unif_data* chunks, const char* boardname);
/* Add a dumper's name to the file. */
struct cart_unif_data* cart_unif_add_dumpername_chunk(struct cart_unif_data* chunks, char* dumpername);
/* Add info about the dumper */
struct cart_unif_data* cart_unif_add_dumperinfo_chunk(struct cart_unif_data* chunks, struct cart_dumperinfo dumperinfo);
/* Add one chunk using this function for each prg chip */
struct cart_unif_data* cart_unif_add_prg_chunk(struct cart_unif_data* chunks, uint32_t size, uint8_t* prg_data, int chip_number);
/* Add one chunk using this function for each chr chip */
struct cart_unif_data* cart_unif_add_chr_chunk(struct cart_unif_data* chunks, uint32_t size, uint8_t* chr_data, int chip_number);
/* Generic Unif chunk function.  Use if no specific function is available for the desired chunk. */
struct cart_unif_data* cart_unif_add_chunk(struct cart_unif_data* chunks, char id[4], uint32_t data_size, const uint8_t* data);
#endif
