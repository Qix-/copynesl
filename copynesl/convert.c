#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cartctl/nes.h>
#include <trk_log/trk_log.h>
#include <settings/settings.h>
#include "output.h"
#include "nes.h"
#include "options.h"

/* rules for format conversion:
 * do not allow both .nes and .unif input
 * if there is a .nes or .unif input file, it must be the only source for .prg / .chr data.
 *  (if user wants to combine prg / chr from nes / other sources, they must use an intermediary step).
 * 
 *  if not .net or .unif in, any number of prg and / or chr inputs are accepted pgr / chr inputs from copynes are also accepted.
 * 
 * 1 and only 1 wram file may be provided as input, either from copynes, or from os.
 * 
 * 1 and only 1 wram file may be requested as output, either to copynes, or to os.
 * if output to copynes is selected, it must be the only output specified.  Input not valid for copynes plugin will be ignored.
 * 
 * if output to copynes not specified, output follows extensions.
 *   prg: write out n-1 inputs 1-1.  The last prg output gets all remaining inputs concatenated together.
 *   chr: same as prg
 *   nes: all input concatenated, all input chr concatenated.
 * 
 *   unif: each input prg is a seperate prg packet, each input chr is a seperate chr packet.
 *  
 */

int read_files(struct cart_format_data** opackets);
int read_settings();
uint8_t parse_mirroring(const char* formatstring);

int
format_convert(void)
{
	int errorcode = 0;
	struct cart_format_data* packets = NULL;
	read_files(&packets);
	read_settings();
	write_to_files(packets);
	cart_free_packets(&packets);	
	return errorcode;
}

int 
read_settings()
{
	int mapper = 0;
	const char* formatstr = NULL;
	int mirroring = 0;
	formatstr = get_string_setting("format-string");
	if (formatstr) {
		mirroring = (int)parse_mirroring(formatstr);
		set_setting(INT_SETTING, "ines_mirrmask", (void*)mirroring);	
	}
	return 0;
}

uint8_t 
parse_mirroring(const char* formatstring)
{
	int len = strlen(formatstring);
	uint8_t mirrmask = 0;
	int i = 0;
	for (i = 0; i < len; i++) {
		switch (formatstring[i]) {
			case 'h':
			case 'H':
				/* horizontal mirroring is default. */
				mirrmask |= CART_HORIZONTAL_MIRRORING;
				break;
			case 'v':
			case 'V':
				mirrmask |= CART_VERTICAL_MIRRORING;
				break;
			case 't':
			case 'T':
				mirrmask |= CART_TRAINER;
				break;
			case '4':
				mirrmask |= CART_FOUR_SCREEN_VROM;
				break;
			case 'b':
			case 'B':
				mirrmask |= CART_HAS_BATTERY;
				break;
			default:
				break;
		}
	}
	return mirrmask;
}

int 
read_files(struct cart_format_data** opackets)
{
	int errorcode = 0;
	struct cart_format_data* packet = 0;
	struct cart_format_data* packets = NULL;
	const char* cur_filename = NULL;

	reset_string_setting("input-file");
	cur_filename = get_string_setting("input-file");
	while (cur_filename) {
		enum cart_format_type format_type = get_format_type(cur_filename);
		switch (format_type) {
			case FT_PRG:
			case FT_CHR:
			case FT_WRAM:
				errorcode = cart_psplit_raw(cur_filename, &packets, format_type);
				if (errorcode) {
					trk_log(TRK_ERROR, "dumping %s failed.", cur_filename);
				}
				trk_log(TRK_DEBUG, "data_size: %d", packets->datasize);
				if (format_type == FT_CHR) {
					trk_log(TRK_DEBUG, "data_size: %d", packets->next->datasize);
				}
				break;
			case FT_NES:
				errorcode = cart_psplit_nes(cur_filename, &packets, NULL, NULL);
				if (errorcode) {
					trk_log(TRK_ERROR, "dumping %s failed.", cur_filename);
				}
				trk_log(TRK_DEBUG, "data_size: %d", packets->datasize);

			default:
				break;
		}
		cur_filename = get_string_setting("input-file");
	}
	*opackets = packets;

	/* The deal is:
	 *   all prg files in given order
	 *   followed by chr in order
	 *   followed by prg from all nes files in order
	 *   followed by chr from all nes files in order
	 *   followed by prg from all unif files
*/
	return errorcode;
}
