/*
 * output.c - Functions related to data output.
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

/*
#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
*/

/* required by copynes/copynes.h */
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <copynes/copynes.h>
#include <cartctl/nes.h>
#include <trk_log/trk_log.h>
#include <settings/settings.h>
#include "nes.h"
#include "output.h"
#include "options.h"
#include "unif.h"

/* local prototypes */
long get_data_size(copynes_packet_t* packets, long npackets, int packet_type);
uint8_t* dump_data(copynes_packet_t* packets, int npackets, int type, long size);

long 
get_data_size(copynes_packet_t* packets, long npackets, int packet_type)
{
	long result_size = 0;
	int i = 0;

	trk_log(TRK_DEBUGVERBOSE, "data size: npackets: %d", npackets);

	for(i = 0; i < npackets; i++) {
		if (packets[i]->type == packet_type) result_size += packets[i]->size;
	}
	return result_size;
}

uint8_t* 
dump_data(copynes_packet_t* packets, int npackets, int type, long size) 
{
	int i = 0;
	long size_completed = 0;
	uint8_t* start = NULL;
	uint8_t* cur = NULL;
	trk_log(TRK_DEBUGVERBOSE, "dump_data started");
	start = (uint8_t*)malloc(size * sizeof(uint8_t));
	cur = start;
	for (i = 0; i < npackets; i++) {
		if (packets[i]->type == type) {
			trk_log(TRK_DEBUGVERBOSE, "dump_data %i type %d size %d", i, packets[i]->type, packets[i]->size);
			cur = memcpy(cur, packets[i]->data, packets[i]->size);
			if (!cur) return NULL;
			cur += packets[i]->size;
		}
	}
	return start;
}



int 
write_to_files(copynes_packet_t* packets, int npackets, uint8_t copynes_mirroring_mask)
{
	int errorcode = 0;
	const char* cur_filename = NULL;

	reset_string_setting("output-file");
	cur_filename = get_string_setting("output-file");
	while (cur_filename) {
		enum format_types format_type = get_format_type(cur_filename);
		switch (format_type) {
			case FT_PRG:
			case FT_CHR:
			case FT_WRAM:
				dump_to_file(cur_filename, packets, npackets, format_type);
				break;
			case FT_NES:
				{
					uint8_t ines_mirrmask = get_int_setting("ines_mirrmask");
					dump_to_nes_file(cur_filename, packets, npackets, ines_mirrmask);
				}
				break;
			case FT_UNIF:
				dump_to_unif_file(cur_filename, packets, npackets);
				break;
			default:
				break;
		}
		cur_filename = get_string_setting("output-file");
	}
	return 0;
}



int 
dump_to_nes_file(const char* filename, copynes_packet_t* packets, int npackets, uint8_t ines_mirr_mask)
{
	int packet_type = 0;
	long prg_size = 0;
	uint8_t* prg = NULL;
	long chr_size = 0;
	uint8_t* chr = NULL;
	FILE* nes_outputfile = NULL;
/*	uint8_t ines_mirroring = 0;
 */
	int mapper_no = 0;

	packet_type = format_to_packet_type(FT_PRG);
	prg_size = get_data_size(packets, npackets, packet_type);
	prg = dump_data(packets, npackets, packet_type, prg_size);
	packet_type = format_to_packet_type(FT_CHR);
	chr_size = get_data_size(packets, npackets, packet_type);
	chr = dump_data(packets, npackets, packet_type, chr_size);

	mapper_no = (int)get_int_setting("mapper");
/*	ines_mirroring = copynes_to_ines_mirrmask(copynes_mirroring_mask, has_wram(packets, npackets));
 */
	trk_log(TRK_DEBUG, "Outputing nes file. mapper %d, mirroring mask %x", mapper_no, ines_mirr_mask);
	nes_outputfile = fopen(filename, "w+b");
	cart_make_nes(nes_outputfile, prg_size, prg, chr_size, chr, (uint8_t) mapper_no, (uint8_t)ines_mirr_mask); 
	fclose(nes_outputfile);

	return 0;
}


int 
dump_to_file(const char* filename, copynes_packet_t* packets, int npackets, int format_type)
{
	long written = 0;
	int packet_type = format_to_packet_type(format_type);
	FILE* outputfile = NULL;
	int errorcode = 0;
	long data_size = 0;
	uint8_t* data_dump = NULL;

	data_size = get_data_size(packets, npackets, packet_type);
	data_dump = dump_data(packets, npackets, packet_type, data_size);
	outputfile = fopen(filename, "w+b");
	trk_log(TRK_VERBOSE, "Outputting %d k of data ", data_size / 1000);
	written = fwrite(data_dump, sizeof(uint8_t), data_size, outputfile);
	if (ferror(outputfile) || written < data_size) {
		trk_log(TRK_ERROR, "error writing to wram_outputfile. ");
		clearerr(outputfile);
		return -1;
	}
	fclose(outputfile);

	return 0;
}


int 
dump_to_unif_file(const char* filename, copynes_packet_t* packets, int npackets)
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
/*	uint8_t ines_mirroring = 0;
 */
	int mapper_no = 0;

	packet_type = format_to_packet_type(FT_PRG);
	prg_size = get_data_size(packets, npackets, packet_type);
	prg = dump_data(packets, npackets, packet_type, prg_size);
	packet_type = format_to_packet_type(FT_CHR);
	chr_size = get_data_size(packets, npackets, packet_type);
	chr = dump_data(packets, npackets, packet_type, chr_size);


	unif_chunks = add_unif_opts(unif_chunks);		
	unif_chunks = cart_unif_add_prg_chunk(unif_chunks, prg_size, prg, prg_chipcount++);
	unif_chunks = cart_unif_add_chr_chunk(unif_chunks, chr_size, chr, chr_chipcount++);

	trk_log(TRK_DEBUG, "Outputing unif file.");
	unif_outputfile = fopen(filename, "w+b");
	errorcode = cart_make_unif(unif_outputfile, unif_chunks);
	if (errorcode) {
		trk_log(TRK_ERROR, "error outputing unif file.");
		clearerr(unif_outputfile);
		errorcode = 0;
	}
	
	fclose(unif_outputfile);
	return 0;
}


