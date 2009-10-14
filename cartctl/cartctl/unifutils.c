/*
 * unifutils.c - Manipulation of unif format ROMS.
 *
 * Copyright (C) Bjorn Hedin 2009 <cradelit@gmail.com>
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "nes.h"
#include "nesutils.h"

struct cart_unif_data*
cart_unif_add_chunk(struct cart_unif_data* chunks, char id[4], uint32_t data_size, const uint8_t* data)
{
	struct cart_unif_data* cur = NULL;
	struct cart_unif_chunk result;
	if (!chunks) {
		chunks = (struct cart_unif_data*)malloc(sizeof(struct cart_unif_data));
		cur = chunks;
	} else {
		cur = chunks;
		while (cur->next) cur = cur->next;
		cur->next = (struct cart_unif_data*)malloc(sizeof(struct cart_unif_data));
		cur = cur->next;
	}
	cur->next = NULL;
	cur->chunk = (struct cart_unif_chunk*)malloc(sizeof(struct cart_unif_chunk));
	cur->chunk->header.id[0] = id[0];
	cur->chunk->header.id[1] = id[1];
	cur->chunk->header.id[2] = id[2];
	cur->chunk->header.id[3] = id[3];
	
	cur->chunk->header.size = data_size;
	cur->chunk->data = (uint8_t*)malloc(data_size);
	cur->chunk->data = memcpy(cur->chunk->data, data, data_size);
	if (!cur->chunk->data) return NULL;

	return chunks;
}

/* MAPR Contains the unif boardname
 */
struct cart_unif_data*
cart_unif_add_boardname_chunk(struct cart_unif_data* chunks, const char* boardname)
{
	return cart_unif_add_chunk(chunks, "MAPR", (uint32_t)strlen(boardname) + 1, (const uint8_t*)boardname);
}

/* NAME - Contains a null terminated char* with the full name of the game.
 */
struct cart_unif_data*
cart_unif_add_dumpername_chunk(struct cart_unif_data* chunks, char* dumpername)
{
	return cart_unif_add_chunk(chunks, "NAME", (uint32_t)strlen(dumpername), (uint8_t*)dumpername);
}

struct cart_unif_data*
cart_unif_add_dumperinfo_chunk(struct cart_unif_data* chunks, struct cart_dumperinfo dumperinfo)
{
	return cart_unif_add_chunk(chunks, "DINF", (uint32_t)sizeof(dumperinfo), (uint8_t*)&dumperinfo);
}

/* PRG0 - PRGF
 * dump of PRG data.  Use 1-F if cartridge has more than 1 chip.
 */
struct cart_unif_data*
cart_unif_add_prg_chunk(struct cart_unif_data* chunks, uint32_t size, uint8_t* prg_data, int chip_number)
{
	return cart_unif_add_chunk(chunks, "PRG0", size, prg_data);
}

/* CHR0 - CHRF
 * dump of chr data
 */
struct cart_unif_data*
cart_unif_add_chr_chunk(struct cart_unif_data* chunks, uint32_t size, uint8_t* chr_data, int chip_number)
{
	return cart_unif_add_chunk(chunks, "CHR0", size, chr_data);
}

/* XXX TODO unimplemented unif functions */

/* READ - Contains extra information useful to the user.
 */

/* TVCI - 1 BYTE containing 
 * 	  0- Originally NTSC cartridge
 * 	  1- Originally PAL cartridge
 * 	  2- Does not matter
 */

/* DINF
 * Dumper information block - 204 bits long
 */

/* CTRL
 * The controllers used by this cart
 * Bit 0: Regular Joypad
 * Bit 1: Zapper
 * Bit 2: R.O.B
 * Bit 3: Arkanoid Controller
 * Bit 4: Power Pad
 * Bit 5: Four-Score adapter
 * Bit 6: Expansion (Do not touch)
 * Bit 7: Expansion (Do not touch)
 */

/* PCK0 - PCKF
 * a 32 bit crc checksum for PRG0 - PRGF
 */

/* CCK0 - CCK
 * a 32 bit crc checksum for CHR0 - CHRF
 */

/* BATR
 * 1 byte long.  If this block is present, the board contains a battery.
 */

/* MIRR
 * Mirroring
 * $00 - Horizontal Mirroring (Hard Wired)          
 * $01 - Vertical Mirroring (Hard Wired)            
 * $02 - Mirror All Pages From $2000 (Hard Wired)   
 * $03 - Mirror All Pages From $2400 (Hard Wired)  
 * $04 - Four Screens of VRAM (Hard Wired)              
 * $05 - Mirroring Controlled By Mapper Hardware   
 */


/*
 * 00h-1Fh : Header
 * 20h-EOF : Chunks
 */
int 
cart_make_unif(FILE* output, struct cart_unif_data* chunks)
{
	int i = 0;
	struct cart_unif_data* cur_chunk = NULL;
	struct cart_unif_data* tmp_chunk = NULL;

	struct {
		uint8_t identification[4]; /* MUST be "UNIF" */
		uint32_t revision; /* Revision number */
	        uint8_t expansion[24]; /* not yet used */
	} unif_header = { "UNIF", CART_UNIF_REVISION, {0} };

	memset (&(unif_header.expansion), 0, 24);
	fwrite(&unif_header, sizeof(unif_header), 1, output);

	cur_chunk = chunks;
	while (cur_chunk) {
		fwrite(&(cur_chunk->chunk->header), sizeof(cur_chunk->chunk->header), 1, output);
		fwrite(cur_chunk->chunk->data, cur_chunk->chunk->header.size, 1, output);
		tmp_chunk = cur_chunk;
		cur_chunk = cur_chunk->next;

		free(tmp_chunk->chunk->data);
		free(tmp_chunk->chunk);
		free(tmp_chunk);
	}

	return 0;
}

int 
cart_pmake_unif(const char* filename, struct cart_format_data* packets, struct cart_unif_data* options)
{
	struct cart_unif_data* unif_chunks = NULL;
	int errorcode = 0;
	
	int packet_type = 0;
	long prg_size = 0;
	uint8_t* prg = NULL;
	long chr_size = 0;
	uint8_t* chr = NULL;
	FILE* unif_outputfile = NULL;
	int prg_chipcount = 0;
	int chr_chipcount = 0;
	
	prg_size = get_data_size(packets, FT_PRG);
	prg = dump_data(packets, FT_PRG, prg_size);
	chr_size = get_data_size(packets, FT_CHR);
	chr = dump_data(packets, FT_CHR, chr_size);

	unif_chunks = options;
	unif_chunks = cart_unif_add_prg_chunk(unif_chunks, prg_size, prg, prg_chipcount++);
	unif_chunks = cart_unif_add_chr_chunk(unif_chunks, chr_size, chr, chr_chipcount++);

	/* trk_log(TRK_DEBUG, "Outputing unif file.");
	 */
	unif_outputfile = fopen(filename, "w+b");
	errorcode = cart_make_unif(unif_outputfile, unif_chunks);
	if (errorcode) {
		/* trk_log(TRK_ERROR, "error outputing unif file.");
		 */
		clearerr(unif_outputfile);
		errorcode = 0;
		return -1;
	}
	
	fclose(unif_outputfile);
	return 0;
}


