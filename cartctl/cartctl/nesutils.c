/*
 * nesutils.c - Manipulation of iNES format ROMS.
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "nes.h"

const int PRG_BLOCKSIZE = 16384;
const int CHR_BLOCKSIZE = 8192;

static long get_filesize(FILE* file);


static long 
get_filesize(FILE* file)
{
	int errorcode = 0;
	long size;
	long original_offset = 0;

	FILE *f = fopen("filename", "rb");
	if (f)
	{
		original_offset = ftell(f);
		if (original_offset == -1) return -1;

		errorcode = fseek(f, 0, SEEK_END);
		if (errorcode) return -1;

		size = ftell(f);

		errorcode = fseek(f, 0, original_offset);
		if (errorcode) return -1;
		return size;
	}
	return -1;
}

int 
cart_fmake_nes(FILE* output, FILE* prg, FILE* chr, uint8_t mapper, uint8_t mirroring_mask)
{
	struct stat sb;
	long prg_size_in_bytes = 0;
	long chr_size_in_bytes = 0;
	int errorcode = 0;

	if (!prg) return -1;
	prg_size_in_bytes = get_filesize(prg);
	if (prg_size_in_bytes < 0) return -1;
		
	if (chr) {
		chr_size_in_bytes = get_filesize(chr);
		if (chr_size_in_bytes < 0) return -1;
	}
	{
		/*
		uint8_t prg_mem[prg_size_in_bytes];
		uint8_t chr_mem[chr_size_in_bytes];
		*/
		uint8_t* prg_mem = NULL;
		uint8_t* chr_mem = NULL;
		prg_mem = (uint8_t*)malloc(prg_size_in_bytes);
		if (!prg_mem) return -2;
		chr_mem = (uint8_t*)malloc(chr_size_in_bytes);
		if (!chr_mem) return -2;

		fread(prg_mem,prg_size_in_bytes,1, prg);
		if (chr) {
			fread(chr_mem,chr_size_in_bytes,1, chr);
		}		
		cart_make_nes(output, prg_size_in_bytes, prg_mem, chr_size_in_bytes, chr_mem, mapper, mirroring_mask);
	}
	return 0;
}

/* NES FORMAT:
 * (0-2) (8 bit char) "NES"
 * (3) (8 bit char) 0x1A
 * (4) (uint8_t) prg size
 * (5) (uint8_t) chr size
 * (6) (uint8_t) 4bits - low 4 bits of mapper number (0-15) 4 bits - mirroring mask
 * (7) (uint8_t) 4bits - high 4 bits of mapper number / 4 bits - always 0.
 * (8-15) reserved - must be zero
 * (16-end) prg then chr
 * If you want, you can add a title on to the end of the rom.  must be 128 max bytes with the remainder zeroed out.
 */
int 
cart_make_nes(FILE* output, long prg_size_in_bytes, uint8_t* prg, long chr_size_in_bytes, uint8_t* chr, uint8_t mapper, uint8_t mirroring_mask)
{
	int errorcode = 0;

	struct {
		uint8_t identification[4]; /* MUST be "NES^Z" */
		uint8_t num_prg_blocks; 
		uint8_t num_chr_blocks;
		uint8_t low_mapper_bits_and_mirroring;
		uint8_t high_mapper_bits;
	        uint8_t expansion[8]; /* not yet used */
	} ines_header = 
	{
		"NES\x1A", 0, 0, 0, 0, "\0\0\0\0\0\0\0\0"
	};

	/* with the extended format, the low 4 bits of the mapper number go into the low 4 bits of byte 6
	 * the high 4 bits of mapper number go into the low 4 bits of byte 7.
	 */
	ines_header.num_prg_blocks = prg_size_in_bytes / PRG_BLOCKSIZE;
	ines_header.num_chr_blocks = chr_size_in_bytes / CHR_BLOCKSIZE;
	ines_header.low_mapper_bits_and_mirroring = ((mapper & 0x0F) << 4) | (mirroring_mask);
	ines_header.high_mapper_bits = (mapper & 0xF0);

	fwrite(&ines_header, 1, sizeof(ines_header), output);
	fwrite(prg, prg_size_in_bytes,1, output);
	fwrite(chr, chr_size_in_bytes,1, output);


	return errorcode;
}


