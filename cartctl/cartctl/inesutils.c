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
#include "nesutils.h"

const int PRG_BLOCKSIZE = 16384;
const int CHR_BLOCKSIZE = 8192;

static long get_filesize(FILE* file);

struct ines_header {
	uint8_t identification[4]; /* MUST be "NES^Z" */
	uint8_t num_prg_blocks; 
	uint8_t num_chr_blocks;
	uint8_t low_mapper_bits_and_mirroring;
	uint8_t high_mapper_bits;
        uint8_t expansion[8]; /* not yet used */
};
	

static long 
get_filesize(FILE* f)
{
	int errorcode = 0;
	long size;
	long original_offset = 0;

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
	struct ines_header header = 
	{
		"NES\x1A", 0, 0, 0, 0, "\0\0\0\0\0\0\0\0"
	};

	/* with the extended format, the low 4 bits of the mapper number go into the low 4 bits of byte 6
	 * the high 4 bits of mapper number go into the low 4 bits of byte 7.
	 */
	header.num_prg_blocks = prg_size_in_bytes / PRG_BLOCKSIZE;
	header.num_chr_blocks = chr_size_in_bytes / CHR_BLOCKSIZE;
	header.low_mapper_bits_and_mirroring = ((mapper & 0x0F) << 4) | (mirroring_mask);
	header.high_mapper_bits = (mapper & 0xF0);

	fwrite(&header, 1, sizeof(header), output);
	fwrite(prg, prg_size_in_bytes,1, output);
	fwrite(chr, chr_size_in_bytes,1, output);


	return errorcode;
}

int 
cart_pmake_nes(const char* filename, struct cart_format_data* packets, int mapper_no, uint8_t ines_mirr_mask)
{
	long prg_size = 0;
	uint8_t* prg = NULL;
	long chr_size = 0;
	uint8_t* chr = NULL;
	FILE* nes_outputfile = NULL;
/*	uint8_t ines_mirroring = 0;
 */
/*	int mapper_no = 0;
 */

	prg_size = get_data_size(packets, FT_PRG);
	prg = dump_data(packets, FT_PRG, prg_size);
	chr_size = get_data_size(packets, FT_CHR);
	chr = dump_data(packets, FT_CHR, chr_size);

/*	ines_mirroring = copynes_to_ines_mirrmask(copynes_mirroring_mask, has_wram(packets, npackets));
 */
	/* trk_log(TRK_DEBUG, "Outputing nes file. mapper %d, mirroring mask %x", mapper_no, ines_mirr_mask);
	 */
	nes_outputfile = fopen(filename, "w+b");
	cart_make_nes(nes_outputfile, prg_size, prg, chr_size, chr, (uint8_t) mapper_no, (uint8_t)ines_mirr_mask);
	free(prg);
	free(chr);
	fclose(nes_outputfile);

	return 0;
}

/* if you don't need the mapper_no and mirr_mask, just pass in NULL. */
int 
cart_psplit_nes(const char* filename, struct cart_format_data** packets, int* omapper_no, uint8_t* oines_mirr_mask)
{

	long prg_size = 0;
	uint8_t* prg = NULL;
	long chr_size = 0;
	uint8_t* chr = NULL;
	FILE* nes_outputfile = NULL;
/*	uint8_t ines_mirroring = 0;
 */
/*	int mapper_no = 0;
 */

	FILE* nesfile = NULL;
	long data_size = 0;
	uint8_t* prg_data = NULL;	
	uint8_t* chr_data = NULL;
	struct cart_format_data* cur = NULL;
	long readcount = 0;


	struct ines_header header;
	long prg_size_in_bytes = 0;
	long chr_size_in_bytes = 0;

	nesfile = fopen(filename, "r+b");
	if (nesfile == NULL) return -1;
	
	fread(&header, sizeof(uint8_t), sizeof(header), nesfile);
	
	if (omapper_no != NULL) {
		*omapper_no = ((header.low_mapper_bits_and_mirroring & 0xF0) >> 4);
		*omapper_no |= (header.high_mapper_bits & 0xF0);
	}
	if (oines_mirr_mask != NULL) {
		*oines_mirr_mask = (header.low_mapper_bits_and_mirroring & 0x0F);
	}

	prg_size_in_bytes = header.num_prg_blocks * PRG_BLOCKSIZE;
	chr_size_in_bytes = header.num_chr_blocks * CHR_BLOCKSIZE;
	
	if (*packets == NULL) {
		*packets =  (struct cart_format_data*)malloc(sizeof(struct cart_format_data));
		cur = *packets;
	} else {
		cur = *packets;
		while (cur->next) cur = (cur)->next;
		cur->next = (struct cart_format_data*)malloc(sizeof(struct cart_format_data));
		cur = cur->next;
	}

	cur->datasize = prg_size_in_bytes;
	cur->datatype = FT_PRG;
	cur->data = (uint8_t*)malloc(prg_size_in_bytes * sizeof(uint8_t));
	cur->next = NULL;
	readcount = fread(cur->data, sizeof(uint8_t), prg_size_in_bytes, nesfile);

	if (ferror(nesfile) || readcount < data_size) {
		clearerr(nesfile);
		return -1;
	}

	if (chr_size_in_bytes > 0) {
		cur->next = (struct cart_format_data*)malloc(sizeof(struct cart_format_data));
		cur = cur->next;
		cur->datasize = chr_size_in_bytes;
		cur->datatype = FT_CHR;
		cur->data = (uint8_t*)malloc(chr_size_in_bytes * sizeof(uint8_t));
		cur->next = NULL;
		readcount = fread(cur->data, sizeof(uint8_t), chr_size_in_bytes, nesfile);

		if (ferror(nesfile) || readcount < data_size) {
			clearerr(nesfile);
			return -1;
		}
	}
	
	fclose(nesfile);
	return 0;
}


/* *oprg and *ochr will be malloced, and must be fread by caller.
 * omapper and oxmirroring are expected to be able to hold one uint8_t each.
 */
int
cart_split_nes(FILE* nesfile, uint8_t** oprg, uint8_t** ochr, uint8_t* omapper, uint8_t* omirroring_mask)
{
	struct ines_header header;
	long prg_size_in_bytes = 0;
	long chr_size_in_bytes = 0;
	
	fread(&header, sizeof(uint8_t), sizeof(header), nesfile);
	prg_size_in_bytes = header.num_prg_blocks * PRG_BLOCKSIZE;
	chr_size_in_bytes = header.num_chr_blocks * CHR_BLOCKSIZE;
	if (oprg != NULL) {
		*oprg = (uint8_t*)malloc(sizeof(uint8_t) * prg_size_in_bytes);
	} 
	if (ochr != NULL) {
		*ochr = (uint8_t*)malloc(sizeof(uint8_t) * chr_size_in_bytes);
	}
	if (omapper != NULL) {
		*omapper = ((header.low_mapper_bits_and_mirroring & 0xF0) >> 4);
		*omapper |= (header.high_mapper_bits & 0xF0);
		*omirroring_mask = (header.low_mapper_bits_and_mirroring & 0x0F);
	}
	return 0;
}
