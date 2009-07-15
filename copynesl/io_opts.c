enum opt_masks {
	ipgr_required   = 0x0001,
	ichr_required   = 0x0002,
	iwram_required  = 0x0004,
	ines_required   = 0x0008,
	iunif_required  = 0x0010,

	ipgr_provided   = 0x0020,
	ichr_provided   = 0x0040,
	iwram_provided  = 0x0080,
	ines_provided   = 0x0100,
	iunif_provided  = 0x0200,

	opgr_requested  = 0x0400,
	ochr_requested  = 0x0800,
	owram_requested = 0x1000,
	ones_requested  = 0x2000,
	ounif_requested = 0x4000
}

typedef struct data_opt
{
	uint32_t datalen;
	FILE* file;
	uint8_t* data;
}

typedef struct nes_opt
{
	int mapper;
	uint32_t datalen;
	FILE* file;
	uint8_t* data;
}

typedef struct unif_opt
{
	int mapper;
	uint32_t datalen;
	FILE* file;
	uint8_t* data;
}

typedef struct file_io_opt
{
	uint8_t opt_mask;
	struct in {
		struct data_opt pgr;
		struct data_opt chr;
		struct data_opt wram;
		struct nes_opt nes;
		struct unif_opt unif;
	}
	struct out {
		struct data_opt pgr;
		struct data_opt chr;
		struct data_opt wram;
		struct nes_opt nes;
		struct unif_opt unif;
		struct data_opt prg;
	}
}



/* use the ext to select and set one of the output files.  
 * Leave the others alone.
 */
int 
set_out_file(IO_OPTS* opts, FILE* input, const char* ext, int* omapper)
{
	int mapper = get_int_setting("mapper");
	trk_log(TRK_VERBOSE, "ext: %s", ext);
	if (!strcmp(ext, "prg") || !strcmp(ext, "PRG")) {
		opt_mask &= pgr_requested;
		opts.out.pgr.file = input;
	} else if (   !strcmp(ext, "chr") || !strcmp(ext, "CHR")) {
		opt_mask &= chr_requested;
		opts.out.chr.file = input;
	} else if (   !strcmp(ext, "wram") || !strcmp(ext, "WRAM") 
		   || !strcmp(ext, "wrm") || !strcmp(ext, "WRM") 
		   || !strcmp(ext, "sav") || !strcmp(ext, "SAV")) {
		opt_mask &= wram_requested;
		opts.out.wram.file = input;
	} else if (!strcmp(ext, "nes") || !strcmp(ext, "NES")) {
		opt_mask &= nes_requested;
		opts->out.nes.file = input;
		opts->out.nes.mapper = mapper;
	} else if (   !strcmp(ext, "unif") || !strcmp(ext, "UNIF") 
		   || !strcmp(ext, "unf") || !strcmp(ext, "UNF")) {
		opt_mask &= unif_requested;
		trk_log(TRK_VERBOSE, "setting unif");
		opts->out.unif.file = input;
	} else {
		trk_log(TRK_ERROR, "invalid extension in output file. %s", ext);
		return -1;
	}
	return 0;
}

int 
get_output_options(IO_OPTS* opts)
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
		set_output_opt(opts, cur, ext, omapper);
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
					set_output_opt(oprg, ochr, owram, ones, ounif, cur, ext, omapper);
					trk_log(TRK_VERBOSE, "%d %d %d %d %d", *oprg, *ochr, *owram, *ones, *ounif);
				}
			}
		} while (cur_setting);
	}
	return 0;

}
