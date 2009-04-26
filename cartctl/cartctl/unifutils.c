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
#include <stdint.h>
#include <string.h>
#include "nes.h"

struct cart_unif_chunk
cart_unif_chunk(char id[4], uint32_t data_size, uint8_t* data)
{
	struct cart_unif_chunk result;
	result.header.id[0] = id[0];
	result.header.id[1] = id[1];
	result.header.id[2] = id[2];
	result.header.id[3] = id[3];
	
	result.header.size = data_size;
	result.data = data;
	return result;
}

/* MAPR Contains the unif boardname
 */
struct cart_unif_chunk
cart_unif_boardname_chunk(char* boardname)
{
	return cart_unif_chunk("MAPR", (uint32_t)strlen(boardname), (uint8_t*)boardname);
}

/* NAME - Contains a null terminated char* with the full name of the game.
 */
struct cart_unif_chunk
vart_unif_dumpername_chunk(char* dumpername)
{
	return cart_unif_chunk("NAME", (uint32_t)strlen(dumpername), (uint8_t*)dumpername);
}

struct cart_unif_chunk
cart_unif_dumperinfo_chunk(struct cart_dumperinfo dumperinfo)
{
	return cart_unif_chunk("DINF", (uint32_t)sizeof(dumperinfo), (uint8_t*)&dumperinfo);
}

/* PRG0 - PRGF
 * dump of PRG data.  Use 1-F if cartridge has more than 1 chip.
 */
struct cart_unif_chunk 
cart_unif_prg_chunk(uint32_t size, uint8_t* prg_data, int chip_number)
{
	return cart_unif_chunk("PRG0", size, prg_data);
}

/* CHR0 - CHRF
 * dump of chr data
 */
struct cart_unif_chunk
cart_unif_chr_chunk(uint32_t size, uint8_t* chr_data, int chip_number)
{
	return cart_unif_chunk("CHR0", size, chr_data);
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
cart_make_unif(FILE* output, struct cart_unif_chunk* chunks, int num_chunks)
{
	int i = 0;
	struct cart_unif_chunk cur_chunk;

	struct {
		uint8_t identification[4]; /* MUST be "UNIF" */
		uint32_t revision; /* Revision number */
	        uint8_t expansion[24]; /* not yet used */
	} unif_header = { "UNIF", UNIF_REVISION, {0} };

	memset (&(unif_header.expansion), 0, 24);
	fwrite(&unif_header, sizeof(unif_header), 1, output);

	for (i = 0; i < num_chunks; i++) {
		cur_chunk = chunks[i];
		fwrite(&(cur_chunk.header), sizeof(cur_chunk.header), 1, output);
		fwrite(cur_chunk.data, cur_chunk.header.size, 1, output);
	}
	return 0;
}
