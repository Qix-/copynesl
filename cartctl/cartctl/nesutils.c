#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "nes.h"
#include "nesutils.h"



unsigned short
cart_has_wram(struct cart_format_data* packets)
{
	struct cart_format_data* cur = NULL;
	cur = packets;
	while (cur) {
		if (cur->datatype == FT_WRAM) return 1;
		cur = cur->next;
	}
	return 0;
}

long 
get_data_size(struct cart_format_data* packets, enum cart_format_type packet_type)
{
	long result_size = 0;
	int i = 0;
	struct cart_format_data* cur = NULL;

	cur = packets;
	while (cur) {
		if (cur->datatype == packet_type) result_size += cur->datasize;
		cur = cur->next;
	}
	return result_size;
}

uint8_t* 
dump_data(struct cart_format_data* packets, enum cart_format_type type, long size) 
{
	int i = 0;
	long size_completed = 0;
	uint8_t* start = NULL;
	uint8_t* cur_data = NULL;
	struct cart_format_data* cur = NULL;

	/* trk_log(TRK_DEBUGVERBOSE, "dump_data started");
	 */
	start = (uint8_t*)malloc(size * sizeof(uint8_t));
	cur_data = start;
	cur = packets;
	while (cur) {
		if (cur->datatype == type) {
			
			/* trk_log(TRK_DEBUGVERBOSE, "dump_data type %d size %d", cur->datatype, cur->datasize);
			 */
			cur_data = memcpy(cur_data, cur->data, cur->datasize);
			if (!cur_data) return NULL;
			cur_data += cur->datasize;
		}
		cur = cur->next;
	}
	return start;
}


int 
cart_pmake_raw(const char* filename, struct cart_format_data* packets, enum cart_format_type format_type)
{
	long written = 0;
	FILE* outputfile = NULL;
	int errorcode = 0;
	long data_size = 0;
	uint8_t* data_dump = NULL;

	data_size = get_data_size(packets, format_type);
	data_dump = dump_data(packets, format_type, data_size);
	outputfile = fopen(filename, "w+b");
	/* trk_log(TRK_VERBOSE, "Outputting %d k of data ", data_size / 1000);
	 */
	written = fwrite(data_dump, sizeof(uint8_t), data_size, outputfile);
	if (ferror(outputfile) || written < data_size) {
		/* trk_log(TRK_ERROR, "error writing to wram_outputfile. ");
		 */
		clearerr(outputfile);
		return -1;
	}
	fclose(outputfile);

	return 0;
}

int
cart_free_packets(struct cart_format_data** packets)
{
	struct cart_format_data* cur = NULL;

	cur = *packets;
	while (cur != NULL) {
		*packets = (*packets)->next;
		free(cur->data);
		free(cur);
		cur = *packets;
	}

	return 0;
}

