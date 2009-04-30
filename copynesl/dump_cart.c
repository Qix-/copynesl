/*
 * dump_cart.c - Run plugin on CopyNES, retrieve and write out result.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <cartctl/nes.h>
#include <copynes/copynes.h>
#include <trk_log/trk_log.h>
#include <settings/settings.h>
#include "errorcodes.h"
#include "plugins.h"
#include "nes.h"

#define MAX_OUTPUT_FORMATS 5

struct cart_unif_data* add_unif_opts(struct cart_unif_data* unif_chunks);

int required_for_output(int packet_type)
{
	char* output_format = NULL;

	if (packet_type == PACKET_PRG_ROM) {
		
	}
	return 1;	
}

int do_input(copynes_t cn, copynes_packet_t** opackets, int* onpackets)
{
    copynes_packet_t packet = 0;
    copynes_packet_t* packets = NULL;
    struct timeval t = { 5L, 0L };
 
    const char* clear_plugin = get_string_setting("clear-plugin");
    const char* dump_plugin = get_string_setting("dump-plugin");
    uint8_t ines_mirroring_mask = 0;
    uint8_t copynes_mirroring_mask = 0;
/*    mirroring = 0;
 */
    int npackets = 0;
    int errorcode = 0;

    int i = 0;

	/* first flush the CopyNES */
	copynes_flush(cn);
	
	trk_log(TRK_DEBUG, "Running %s", clear_plugin);
	errorcode = run_plugin(cn, clear_plugin);
	if (errorcode) return errorcode;
	sleep(1);


	trk_log(TRK_DEBUG, "Running %s", dump_plugin);
	errorcode = run_plugin(cn, dump_plugin);
	if (errorcode) return errorcode;
	sleep(3);

	copynes_read(cn, &copynes_mirroring_mask, 1, &t);
	/* mirroring bit is the same */
	ines_mirroring_mask |= (copynes_mirroring_mask & 0x01);
	if (copynes_mirroring_mask & 0x02) {
		ines_mirroring_mask |= CART_FOUR_SCREEN_VROM;	
	}




    while(1)
    {
        packet = NULL;
	trk_log(TRK_VERBOSE, "reading packet...");
        /* read in a packet */
        if((errorcode = copynes_read_packet (cn, &packet, t)) < 0)
        {
	    trk_log(TRK_ERROR, "copynes_read_packet returned: %d. copynes error: %s", errorcode, copynes_error_string(cn));
            for(i = 0; i < npackets; i++)
            {
                free(packets[i]);
            }
            free(packets);
	    if (packet) free (packet);
            copynes_reset (cn, RESET_COPYMODE);
            return -1;
        }
	trk_log(TRK_VERBOSE, "packet type: %d", packet->type);
        
        if(packet->type == PACKET_EOD) {
	    trk_log(TRK_DEBUG, "PACKET_EOD read");
	    free (packet);
            break;
	}
        
        switch(packet->type)
        {
            case PACKET_PRG_ROM:
	    	trk_log(TRK_DEBUG, "PRG %d Kb\n", packet->size / 1024);
                break;
            case PACKET_CHR_ROM:
	    	trk_log(TRK_DEBUG, "CHR %d Kb\n", packet->size / 1024);
                break;
            case PACKET_WRAM:
	    	ines_mirroring_mask |= CART_HAS_BATTERY;
		trk_log(TRK_VERBOSE, "Cart has battery.  ines_mirroring_mask %d CART_HAS_BATTERY %d", ines_mirroring_mask, CART_HAS_BATTERY);
	    	trk_log(TRK_DEBUG, "SAV %d Kb\n", packet->size / 1024);
                break;
        }
        
	if (required_for_output(packet->type)) {
	        /* append the packet to the packet list */
        	packets = realloc(packets, (npackets + 1) * sizeof(*packet));
	        packets[npackets] = packet;
        	npackets++;
	} else {
		free (packet);
	}
    }
    trk_log(TRK_DEBUG, "ines mirroring mask byte: %x", ines_mirroring_mask);
    set_setting(INT_SETTING, "ines-mirroring-mask", (void*)(int)ines_mirroring_mask);
    trk_log(TRK_DEBUGVERBOSE, "npackets: %d", npackets);
    *opackets = packets;
    *onpackets = npackets;
    /* reset the CopyNES one last time */
    copynes_reset (cn, RESET_COPYMODE);
    return 0;
}

long get_data_size(copynes_packet_t* packets, long npackets, int packet_type)
{
	long result_size = 0;
	int i = 0;

	trk_log(TRK_DEBUGVERBOSE, "data size: npackets: %d", npackets);

	for(i = 0; i < npackets; i++) {
		if (packets[i]->type == packet_type) result_size += packets[i]->size;
	}
	return result_size;
}

int set_one_file(FILE** oprg, FILE** ochr, FILE** owram, FILE** ones, FILE** ounif, FILE* input, const char* ext, int* omapper)
{
	int mapper = get_int_setting("mapper");
	trk_log(TRK_VERBOSE, "ext: %s", ext);
	if (!strcmp(ext, "prg") || !strcmp(ext, "PRG")) {
		*oprg = input;
	} else if (!strcmp(ext, "chr") || !strcmp(ext, "CHR")) {
		*ochr = input;
	} else if (!strcmp(ext, "wram") || !strcmp(ext, "WRAM") ||
		   !strcmp(ext, "wrm") || !strcmp(ext, "WRM")) {
		*owram = input;
	} else if (!strcmp(ext, "nes") || !strcmp(ext, "NES")) {
		*ones = input;
		*omapper = mapper;
	} else if (!strcmp(ext, "unif") || !strcmp(ext, "UNIF") ||
		   !strcmp(ext, "unf") || !strcmp(ext, "UNF")) {
		   	trk_log(TRK_VERBOSE, "setting unif");
			*ounif = input;
	} else {
		trk_log(TRK_ERROR, "invalid extension in output file. %s", ext);
		return -1;
	}
	return 0;
}

int get_dumper_options(FILE** oprg, FILE** ochr, 
		       FILE** owram, FILE** ones, 
		       FILE** ounif, int* omapper)
{
	FILE* cur;
	const char* oformat_setting;
	const char* cur_setting;
	char* filename;
	const char* ext;

	trk_log(TRK_VERBOSE, "strset: %s", get_string_setting("output-file"));

	reset_string_setting("output-file");

	oformat_setting = get_string_setting("output-format");
	if (oformat_setting) {
		cur_setting = get_string_setting("output-file");
		if (get_string_setting("output-file")) { /* 2 outputfiles + format == error */
			trk_log(TRK_ERROR, "Error, found multiple output files when output format specified.");
			return INVALID_OPTIONS;
		}
		
		if (cur_setting) {
			cur = fopen(cur_setting, "w+b");
			if (!cur) {
				trk_log(TRK_ERROR, "Opening file at %s", cur_setting);
				return -1;
			} 
		} else {
			cur = stdout;
		}
		ext = strstr(cur_setting, ".") + 1;
		set_one_file(oprg, ochr, owram, ones, ounif, cur, ext, omapper);
	} else {
		do {
			cur_setting = get_string_setting("output-file");
			trk_log(TRK_VERBOSE, "strset: %s", cur_setting);
			if (cur_setting) {
				cur = fopen(cur_setting, "w+b");
				if (!cur) {
					trk_log(TRK_ERROR, "opening output file %s", cur_setting);
					continue;
				} else {
					char* ext = strstr(cur_setting, ".") + 1;
					set_one_file(oprg, ochr, owram, ones, ounif, cur, ext, omapper);
					trk_log(TRK_VERBOSE, "%d %d %d %d %d", *oprg, *ochr, *owram, *ones, *ounif);
				}
			}
		} while (cur_setting);
	}
	return 0;

}

uint8_t* dump_data(copynes_packet_t* packets, int npackets, int type, long size) {
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

int do_output(copynes_packet_t* packets, int npackets)
{
	uint8_t* prg_data = NULL;
	uint8_t* chr_data = NULL;
	uint8_t* wram_data = NULL;
	int mapper = 0;
	FILE* prg_outputfile = NULL;
	FILE* chr_outputfile = NULL;
	FILE* wram_outputfile = NULL;
	FILE* nes_outputfile = NULL;
	FILE* unif_outputfile = NULL;

	long prg_data_size = 0;
	long chr_data_size = 0;
	long wram_data_size = 0;
	int errorcode = 0;
	struct cart_unif_data* unif_chunks = NULL;

	errorcode = get_dumper_options(&prg_outputfile, &chr_outputfile, &wram_outputfile, &nes_outputfile, &unif_outputfile, &mapper);
	trk_log(TRK_VERBOSE, "%d %d %d %d %d mapper: %d", prg_outputfile, chr_outputfile, wram_outputfile, nes_outputfile, unif_outputfile, mapper);

	if (unif_outputfile) {
		unif_chunks = add_unif_opts(unif_chunks);
	}
	if (prg_outputfile || nes_outputfile || unif_outputfile) {
		prg_data_size = get_data_size(packets, npackets, PACKET_PRG_ROM);
		if (prg_data_size > 0) {
			prg_data = dump_data(packets, npackets, PACKET_PRG_ROM, prg_data_size);
			if (prg_outputfile) {
				trk_log(TRK_VERBOSE, "Outputting %d k of PRG data ", prg_data_size / 1000);
				errorcode = fwrite(prg_data, sizeof(uint8_t), prg_data_size, prg_outputfile);
				if (ferror(prg_outputfile)) {
					trk_log(TRK_ERROR, "error writing to prg_outputfile. ");
					clearerr(prg_outputfile);
				}
				fclose(prg_outputfile);
				trk_log(TRK_DEBUGVERBOSE, "done");
			}
			if (unif_outputfile) {
				static int prg_chipcount = 0;
				trk_log(TRK_VERBOSE, "Creating %d k PRG data unif chunk", prg_data_size / 1000);
				unif_chunks = cart_unif_add_prg_chunk(unif_chunks, prg_data_size, prg_data, prg_chipcount++);
			}
		}
	}

	if (chr_outputfile || nes_outputfile || unif_outputfile) {
		chr_data_size = get_data_size(packets, npackets, PACKET_CHR_ROM);
		if (chr_data_size > 0) {
			chr_data = dump_data(packets, npackets, PACKET_CHR_ROM, chr_data_size);
			if (chr_outputfile) { 
				trk_log(TRK_VERBOSE, "Outputting %d k of CHR data ", chr_data_size / 1000);
				errorcode = fwrite(chr_data, sizeof(uint8_t), chr_data_size, chr_outputfile);
				if (ferror(chr_outputfile)) {
					trk_log(TRK_ERROR, "error writing to chr outputfile. ");
					clearerr(chr_outputfile);
				}
				fclose(chr_outputfile);
			}
			if (unif_outputfile) {
				static int chr_chipcount = 0;
				trk_log(TRK_VERBOSE, "Creating %d k CHR data unif chunk", chr_data_size / 1000);
				unif_chunks = cart_unif_add_chr_chunk(unif_chunks, chr_data_size, chr_data, chr_chipcount++);
			}
		}
	}	

	if (wram_outputfile) {
		wram_data_size = get_data_size(packets, npackets, PACKET_WRAM);
		if (wram_data_size > 0) {
			wram_data = (uint8_t*)malloc(wram_data_size * sizeof(uint8_t));
			wram_data = dump_data(packets, npackets, PACKET_WRAM, wram_data_size);
			if (wram_outputfile) {
				errorcode = fwrite(wram_data, sizeof(uint8_t), wram_data_size, wram_outputfile);
				if (ferror(wram_outputfile)) {
					trk_log(TRK_ERROR, "error writing to wram_outputfile. ");
					clearerr(wram_outputfile);
				}
				fclose(wram_outputfile);
			}
		}
	}

	if (nes_outputfile && prg_data_size > 0) {
		int mirroring = (int)get_int_setting("ines-mirroring-mask");
		int mapper_no = (int)get_int_setting("mapper");
		trk_log(TRK_DEBUG, "Outputing nes file. mapper %d, mirroring mask %x", mapper, mirroring);
		 cart_make_nes(nes_outputfile, prg_data_size, prg_data, chr_data_size, chr_data, (uint8_t) mapper, (uint8_t)mirroring);
		 
		fclose(nes_outputfile);
	}

	if (unif_outputfile) {
		errorcode = cart_make_unif(unif_outputfile, unif_chunks);
		if (errorcode) {
			trk_log(TRK_ERROR, "error outputing unif file.");
			clearerr(unif_outputfile);
			errorcode = 0;
		}
		fclose(unif_outputfile);
	}

	if (prg_data) free(prg_data);
	if (chr_data) free(chr_data);
	if (wram_data) free(wram_data);

	trk_log(TRK_DEBUG, "End output");
	return 0;
}

/*
 * prerequisits:
 * NES MUST BE VERIFIED TO BE ON
 * Options required for dumping cart (see man 1 copynesl)
 *  must be verified
 */
int dump_cart(void)
{
	const char* plugin_dir = get_string_setting("plugin-dir");
	const char* clear_plugin = get_string_setting("clear-plugin");
	const char* requested_plugin = get_string_setting("dump-cart");
	char* plugin_path;
	char* clplugin_path;
	char* filepath = NULL;
	const char* plugin = NULL; 
	int errorcode = 0;
	uint8_t mirroring = 0;
	int output_formats[MAX_OUTPUT_FORMATS];
    	copynes_packet_t * packets = NULL;
	int npackets = 0;
	int i = 0;

	copynes_t cn = NULL;

	if (!plugin_dir || !clear_plugin) return -1;
	if (!requested_plugin) {
		trk_log(TRK_ERROR, "No Plugin Specified.");
		return INVALID_OPTIONS;
	}

	clplugin_path = get_plugin_path(plugin_dir, clear_plugin);
	if (!clplugin_path) {
		free(clplugin_path);
		return INVALID_OPTIONS;
	}
	trk_log(TRK_DEBUG, "Found %s", clplugin_path);
	set_setting(STRING_SETTING, "clear-plugin", clplugin_path);
	free(clplugin_path);

	trk_log(TRK_VERBOSE, "Looking for %s", requested_plugin);
	/* we need at least a plugin setting and either output format or output file settings. */
	plugin_path = get_plugin_path(plugin_dir, requested_plugin);
	if (errorcode || !plugin_path) {
		trk_log(TRK_ERROR, "Could not find %s at %s. Ensure plugin-dir and clear-plugin settings are correct.", get_string_setting("clear_plugin"), plugin_dir);
		free(plugin_path);
		return INVALID_OPTIONS;
	}
	trk_log(TRK_DEBUG, "Found %s", plugin_path);
	set_setting(STRING_SETTING, "dump-plugin", plugin_path);
	free(plugin_path);
	

	cn = copynes_new();
	errorcode = copynes_up(cn);
	if (errorcode) return errorcode;

    /* read in the packets */
       do_input(cn, &packets, &npackets);
    
    /* reading from copynes complete. */
	do_output(packets, npackets);
    /* write out .prg, .chr, and .sav files */
    
    /* cleanup */
    for(i = 0; i < npackets; i++)
    {
        free(packets[i]->data);
        free(packets[i]);
    }
    free(packets);

	copynes_free(cn);
	return errorcode;
}

struct 
cart_unif_data* add_unif_opts(struct cart_unif_data* unif_chunks)
{
	struct cart_unif_data* result = unif_chunks;
	const char* boardname = get_string_setting("boardname");
	if (boardname) {
		result = cart_unif_add_boardname_chunk(result, boardname);
	}
}
