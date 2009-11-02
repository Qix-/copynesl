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
#include <copynes.h>
#endif
#include <cartctl/nes.h>
#include <settings/settings.h>
#include <trk_log/trk_log.h>
#include "nes.h"
#include "errorcodes.h"
#include "plugins.h"
#include "options.h"
#include "output.h"
#include "input.h"

#define MAX_OUTPUT_FORMATS 5

int cnes_read(struct cart_format_data** opackets);
int cnes_write(struct cart_format_data* opackets);
int set_plugin_uservars(copynes_t cn);
uint8_t copynes_to_ines_mirrmask(uint8_t copynes_mirroring_mask, unsigned short has_battery);

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
	int errorcode = 0;
	struct cart_format_data* packets = NULL;
/*
    	copynes_packet_t* packets = NULL;
	int npackets = 0;
*/
    	/* read in the packets */
        errorcode = cnes_read(&packets);
	if (errorcode) return errorcode;
    
    	/* reading from copynes complete. */
	errorcode = write_to_files(packets);
	if (errorcode) return errorcode;
    	
	errorcode = cart_free_packets(&packets);	
	if (errorcode) return errorcode;

	return errorcode;
}

int 
write_cart(void)
{
	int errorcode = 0;
	struct cart_format_data* packets = NULL;

	trk_log(TRK_DEBUGVERBOSE, "readfiles"); 
	read_files(&packets);
	/* read_settings();
	 */
	trk_log(TRK_DEBUGVERBOSE, "cnes_write"); 
	cnes_write(packets);
	trk_log(TRK_DEBUGVERBOSE, "cart_free_pakets");
	cart_free_packets(&packets);	
	trk_log(TRK_DEBUGVERBOSE, "exit;");

	return 0;
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

int 
set_plugin_uservars(copynes_t cn)
{
	/* get uservalue */
	uint8_t enabled[4] = {0,0,0,0};
	uint8_t value[4] = {0,0,0,0};
	int i = 0;
	const char* cur_val;
	uint32_t value_holder = 0;

	reset_string_setting("plugin-uservar");
	cur_val = get_string_setting("plugin-uservar");
	printf("set plugin uservars\n");
	while (cur_val && i < 4) {
		printf("cur_val%s\n", cur_val);
		if (cur_val[0] == '0' && cur_val[1] == 'x') {
			/* user entered a hex value */
			const char* hexstr = &cur_val[2];
			sscanf(hexstr, "%X", (uint32_t*)&value_holder);
		} else {
			sscanf(cur_val, "%u", (uint32_t*)&value_holder);
		}
		if ( (value_holder > 0xFFFFFF) && (i <= 0) ) {
			enabled[i] = 1;
			value[i++] = (uint8_t)(value_holder >> 24);
		}
		if ( (value_holder > 0xFFFF) && (i <= 1) ) {
			enabled[i] = 1;
			value[i++] = (uint8_t)(value_holder >> 16);
		} 
		if ( (value_holder > 0xFF) && (i <= 2) ) {
			enabled[i] = 1;
			value[i++] = (uint8_t)(value_holder >> 8);
		}
		enabled[i] = 1;
		value[i++] = (uint8_t)value_holder;

		trk_log(TRK_DEBUGVERBOSE, "Uservars passed: u1:%d u2:%d u3:%d u4:%d \n", 
			value[0], value[1], value[2], value[3]);

		cur_val = get_string_setting("plugin-uservar");
	}
	if (i >= 4 && cur_val) {
		trk_log(TRK_WARNING, "Too many bytes passed in for plugin uservals!");
	}
	if (enabled[0] || enabled[1] || enabled[2] || enabled[3]) {
		/* copynes_set_uservars(cn, enabled, value);
		 */
	}
	
	return 0;
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
cnes_read(struct cart_format_data** opackets)
{
	copynes_t cn = NULL;

/*	copynes_packet_t packet = 0;
	copynes_packet_t* packets = NULL;
*/
	copynes_packet_t cnes_packet = 0;

	struct cart_format_data* cur_packet = NULL;
	struct cart_format_data* packets = NULL;
	struct timeval t = { 5L, 0L };
 
	const char* clear_plugin = get_string_setting("clear-plugin");
	const char* dump_plugin = get_string_setting("dump-cart");
	uint8_t copynes_mirroring_mask = 0;
	/*    mirroring = 0;
 	*/
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

	set_plugin_uservars(cn);
	


	trk_log(TRK_DEBUG, "Running %s", dump_plugin);
	errorcode = run_plugin(cn, dump_plugin);
	if (errorcode) return errorcode;
	sleep(3);

	copynes_read(cn, &copynes_mirroring_mask, 1, &t);

	while(1)
	{
		cnes_packet = NULL;
		trk_log(TRK_VERBOSE, "reading packet...");
		/* read in a packet */
		if((errorcode = copynes_read_packet (cn, &cnes_packet, t)) < 0) {
			trk_log(TRK_ERROR, "copynes_read_packet returned: %d. copynes error: %s", errorcode, copynes_error_string(cn));
			cart_free_packets(&packets);
			if (cnes_packet) free (cnes_packet);
			copynes_reset (cn, RESET_COPYMODE);
			copynes_free(cn);
			return -1;
		}
		trk_log(TRK_VERBOSE, "packet type: %d", cnes_packet->type);
        
		if(cnes_packet->type == PACKET_EOD) {
			trk_log(TRK_DEBUG, "PACKET_EOD read");
			free (cnes_packet);
			break;
		}
	
		switch(cnes_packet->type)
	        {
			case PACKET_PRG_ROM:
				trk_log(TRK_DEBUG, "PRG %d Kb\n", cnes_packet->size / 1024);
				break;
			case PACKET_CHR_ROM:
				trk_log(TRK_DEBUG, "CHR %d Kb\n", cnes_packet->size / 1024);
				break;
			case PACKET_WRAM:
				trk_log(TRK_DEBUG, "SAV %d Kb\n", cnes_packet->size / 1024);
				break;
		}
	
		if (required_for_output(packet_to_format_type(cnes_packet->type))) {
			/* append the packet to the packet list */
			struct cart_format_data* new_packet = NULL;
			new_packet = (struct cart_format_data*)malloc(sizeof(struct cart_format_data));
			new_packet->data = (uint8_t*)malloc(cnes_packet->size * sizeof(uint8_t));
			new_packet->data = (uint8_t*)memcpy(new_packet->data, cnes_packet->data, cnes_packet->size);
			new_packet->datasize = cnes_packet->size;
			new_packet->datatype = packet_to_format_type(cnes_packet->type);
			new_packet->next = NULL;
			if (packets == NULL) {
				packets = new_packet;
				cur_packet = new_packet;
			} else {
				cur_packet->next = new_packet;
				cur_packet = cur_packet->next;
			}
		}
		free (cnes_packet->data);
		free (cnes_packet);
	}
	trk_log(TRK_DEBUGVERBOSE, "all packets received.");
	if (packets != NULL) {
		int inesmirrmask = copynes_to_ines_mirrmask(copynes_mirroring_mask, cart_has_wram(packets));
		trk_log(TRK_DEBUG, "INESMIRRMASK setting %d", (uint8_t)inesmirrmask);
		set_setting(INT_SETTING, "ines_mirrmask", (void*)inesmirrmask);
	}
	*opackets = packets;
	/* reset the CopyNES one last time */
	copynes_reset (cn, RESET_COPYMODE);
	copynes_free(cn);
	return 0;
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
cnes_write(struct cart_format_data* opackets)
{
	copynes_t cn = NULL;

/*	copynes_packet_t packet = 0;
	copynes_packet_t* packets = NULL;
*/
	copynes_packet_t cnes_packet = 0;

	struct cart_format_data* cur_packet = NULL;
	struct cart_format_data* packets = NULL;
	struct timeval t = { 5L, 0L };
	 
	const char* write_plugin = get_string_setting("write-cart");
	struct cart_format_data* cur = opackets;
	/*    mirroring = 0;
 	*/
	int errorcode = 0;

	int i = 0;

	cn = copynes_new();
	errorcode = copynes_up(cn);
	if (errorcode) return errorcode;

	/* first flush the CopyNES */
	copynes_flush(cn);

	/* I don't *think* that writing requires a clear_plugin 
	 * call (but what do I know) 
	 *
	trk_log(TRK_DEBUG, "Running %s", clear_plugin);
	errorcode = run_plugin(cn, clear_plugin);
	if (errorcode) return errorcode;
	sleep(1);
	*/

	set_plugin_uservars(cn);
	

	trk_log(TRK_DEBUG, "Running %s", write_plugin);
	errorcode = run_plugin(cn, write_plugin);
	if (errorcode) return errorcode;
	sleep(3);
	trk_log(TRK_VERBOSE, "writing wram...");
	while (cur) {
		uint8_t cmdsize[1] = {0};
		cmdsize[0] = (uint8_t)((cur->datasize) >> 8);
		/* write size of data to be sent (shifted 8) */
		trk_log(TRK_DEBUGVERBOSE, "writing cmdsize: %d", cmdsize[0]);
		if ((errorcode = copynes_write(cn,cmdsize , sizeof(uint8_t))) < 0) { 
			trk_log(TRK_ERROR, "copynes_write_packet returned: %d. copynes error: %s", errorcode, copynes_error_string(cn));
			copynes_reset (cn, RESET_COPYMODE);
			copynes_free(cn);
			return errorcode;
		}
		
		trk_log(TRK_DEBUGVERBOSE, "cmdsize written: %x", cmdsize[0]);
		sleep(1);
		trk_log(TRK_DEBUGVERBOSE, "writting data: %x", cur->datasize);
		
		if ((errorcode = copynes_write(cn,cur->data , cur->datasize)) < 0) { 
			trk_log(TRK_ERROR, "copynes_write_packet returned: %d. copynes error: %s", errorcode, copynes_error_string(cn));
			copynes_reset (cn, RESET_COPYMODE);
			copynes_free(cn);
			return errorcode;
		}
		trk_log(TRK_DEBUGVERBOSE, "data written %d", cur->datasize);
		sleep(3);
		cur = cur->next;
	}

	copynes_reset (cn, RESET_COPYMODE);
	copynes_free(cn);
	return 0;
}

