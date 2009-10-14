

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

int
convert(void)
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

	read_files(&packets, &npackets);
	write_to_files(packets, npackets);
	
	/* cleanup */
	for(i = 0; i < npackets; i++)
	{
        	free(packets[i]->data);
	        free(packets[i]);
	}
	free(packets);

}

int read_files(copynes_packet_t** opackets, int* onpackets)
{
	unsigned int nprg = 0;
	unsigned int nchr = 0;

	copynes_packet_t packet = 0;
	copynes_packet_t* packets = NULL;

	reset_string_setting("input-file");
	cur_filename = get_string_setting("input-file");
	while (cur_filename) {
		enum format_types format_type = get_format_type(cur_filename);
		switch (format_type) {
			case FT_PRG:
			case FT_CHR:
			case FT_WRAM:
				packets = realloc(packets, (npackets + 1) * sizeof(*packet));
				packets[npackets] = packet;
				npackets++;



				

		}
		if (format_type == FT_NES) {
			/* nes to prg / chr */
			
		}
	}

	/* The deal is:
	 *   all prg files in given order
	 *   followed by chr in order
	 *   followed by prg from all nes files in order
	 *   followed by chr from all nes files in order
	 *   followed by prg from all unif files

}
