/*
 * copynes.c - Interface with CopyNES hardware.
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

/* need to find a better solution, but this is required by copynes.h */
/* required by copynes/copynes.h */
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if HAVE_LIBCOPYNES
#include <copynes/copynes.h>
#endif
#include <cartctl/nes.h>
#include <settings/settings.h>
#include <trk_log/trk_log.h>
#include "nes.h"
#include "errorcodes.h"
#include "plugins.h"
#include "options.h"
#include "output.h"

#define MAX_OUTPUT_FORMATS 5

int cnes_read(copynes_packet_t** opackets, int* onpackets, uint8_t* omirrmask);
uint8_t copynes_to_ines_mirrmask(uint8_t copynes_mirroring_mask, unsigned short has_battery);
unsigned short has_wram(copynes_packet_t* packets, int npackets);

int print_version()
{
	int errorcode = 0;
	char version_str[255];
	copynes_t cn = copynes_new();
	errorcode = copynes_up(cn);
	if (errorcode) return errorcode;
/*	copynes_flush(cn);
 */
	errorcode = copynes_get_version(cn, &version_str, 255);
	if (errorcode < 0) return errorcode;
	printf("%s\n", version_str);
	copynes_free(cn);
	return errorcode;
}

int 
enter_playmode(void)
{
	int errorcode = 0;
	char version_str[255];
	copynes_t cn = copynes_new();
	errorcode = copynes_up(cn);
	if (errorcode) return errorcode;
	copynes_flush(cn);

	copynes_reset(cn, RESET_PLAYMODE);
	printf("Play testing.  Press a key to Exit.\n");
	fgetc(stdin);
	copynes_free(cn);
	return errorcode;

}




int
run_plugin (copynes_t cn, const char* filepath)
{
    
	/* get the CopyNES ready for loading a plugin */
	copynes_reset(cn, RESET_COPYMODE);
    
	/* load the "clear" plugin to clean the 6502 RAM */
	copynes_load_plugin(cn, filepath);
    
	/* run it */
	copynes_run_plugin (cn);
	return 0;
}

/*
 * prerequisits:
 * NES MUST BE VERIFIED TO BE ON
 * Options required for dumping cart (see man 1 copynesl)
 *  must be verified
 */
int
dump_cart(void)
{
	char* filepath = NULL;
	const char* plugin = NULL; 
	int errorcode = 0;
	uint8_t mirroring = 0;
	int output_formats[MAX_OUTPUT_FORMATS];
    	copynes_packet_t* packets = NULL;
	int npackets = 0;
	int i = 0;
	uint8_t copynes_mirroring_mask;
	unsigned short has_battery = 0;
	uint8_t** prg;
	uint8_t** chr;
	uint8_t** wram;



    	/* read in the packets */
        cnes_read(&packets, &npackets, &copynes_mirroring_mask);
    
    	/* reading from copynes complete. */
	write_to_files(packets, npackets, copynes_mirroring_mask);
    	
	/* write out .prg, .chr, and .sav files */
    
    	/* cleanup */
	for(i = 0; i < npackets; i++)
	{
        	free(packets[i]->data);
	        free(packets[i]);
	}
	free(packets);

	return errorcode;
}


/* Begin local only functions */

int 
copynes_up(copynes_t cn)
{
	copynes_open(cn, get_string_setting("data-port"), get_string_setting("control-port"));
	if (!copynes_nes_on(cn)) { 
		copynes_free(cn);
		return COPYNES_OFF;
	}
	return 0;
}

int
packet_to_format_type(int packet_type)
{
	switch (packet_type) {
		case PACKET_PRG_ROM:
			return FT_PRG;
			break;
		case PACKET_CHR_ROM:
			return FT_CHR;
			break;
		case PACKET_WRAM:
			return FT_WRAM;
			break;
		default:
			return 0;
			break;
	}
}

int
format_to_packet_type(int format_type)
{
	switch (format_type) {
		case FT_PRG:
			return PACKET_PRG_ROM;
			break;
		case FT_CHR:
			return PACKET_CHR_ROM;
			break;
		case FT_WRAM:
			return PACKET_WRAM;
			break;
		default:
			return 0;
			break;
	}
}



uint8_t 
copynes_to_ines_mirrmask(uint8_t copynes_mirroring_mask, unsigned short has_battery)
{
    	uint8_t ines_mirroring_mask = 0;
	/* mirroring bit is the same */
	ines_mirroring_mask |= (copynes_mirroring_mask & 0x01);
	if (copynes_mirroring_mask & 0x02) {
		ines_mirroring_mask |= CART_FOUR_SCREEN_VROM;	
	}
	if (has_battery) {
		ines_mirroring_mask |= CART_HAS_BATTERY;
	}
	
	return ines_mirroring_mask;
}


/* Pull data required for the dump from the copynes
 * and store is in opackets.  onpackets represents
 * the number of packets read.
 * data read by this function: 
 *  mirroring bit, prg, chr, wram
 * data that is not required for the output format
 *
 * Parameters: 
 *   cn - libcopynes connection to copynes
 *   opackets - store the data required for the chosen 
 *              output format
 *   onpackets - the number of packets in opackets
 *   omirrmask - the mirroring mask read from the copynes
 *
 * Options used:
 *   clear-plugin - normally clear.bin.  The plugin used
 *                  to clear the copynes memory.
 *   dump-plugin  - the plugin to use to pull the data
 *                  from the copynes.
 */
int 
cnes_read(copynes_packet_t** opackets, int* onpackets, uint8_t* omirrmask)
{
	copynes_t cn = NULL;

	copynes_packet_t packet = 0;
	copynes_packet_t* packets = NULL;
	struct timeval t = { 5L, 0L };
 
	const char* clear_plugin = get_string_setting("clear-plugin");
	const char* dump_plugin = get_string_setting("dump-plugin");
	uint8_t copynes_mirroring_mask = 0;
	/*    mirroring = 0;
 	*/
	int npackets = 0;
	int errorcode = 0;

	int i = 0;

	cn = copynes_new();
	errorcode = copynes_up(cn);
	if (errorcode) return errorcode;

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

	while(1)
	{
		packet = NULL;
		trk_log(TRK_VERBOSE, "reading packet...");
		/* read in a packet */
		if((errorcode = copynes_read_packet (cn, &packet, t)) < 0) {
			trk_log(TRK_ERROR, "copynes_read_packet returned: %d. copynes error: %s", errorcode, copynes_error_string(cn));
			for(i = 0; i < npackets; i++) {
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
	trk_log(TRK_DEBUGVERBOSE, "npackets: %d", npackets);
	if (npackets > 0) {
		uint8_t inesmirrmask = copynes_to_ines_mirrmask(copynes_mirroring_mask, has_wram(packets, npackets));
		set_setting(INT_SETTING, "ines_mirrmask", &inesmirrmask);
	}
	*opackets = packets;
	*onpackets = npackets;
	*omirrmask = copynes_mirroring_mask;
	/* reset the CopyNES one last time */
	copynes_reset (cn, RESET_COPYMODE);
	copynes_free(cn);
	return 0;
}

unsigned short
has_wram(copynes_packet_t* packets, int npackets)
{
	return 0;
}

