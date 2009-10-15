#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "nes.h"
#include "nesutils.h"

long get_filesize(FILE* f);


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

/* Dump the data from any file type that requires no manipulation
 * into the end of packets.
 */
int 
cart_psplit_raw(const char* filename, struct cart_format_data** packets, enum cart_format_type format_type)
{
	FILE* inputfile = NULL;
	long data_size = 0;
	uint8_t* data_dump = NULL;	
	struct cart_format_data* cur = NULL;
	long readcount = 0;

	data_dump = dump_data(*packets, format_type, data_size);
	inputfile = fopen(filename, "r+b");
	data_size = get_filesize(inputfile);
	
	if (*packets == NULL) {
		*packets =  (struct cart_format_data*)malloc(sizeof(struct cart_format_data));
		cur = *packets;
	} else {
		cur = *packets;
		while (cur->next) cur = (cur)->next;
		cur->next = (struct cart_format_data*)malloc(sizeof(struct cart_format_data));
		cur = cur->next;
	}

	(cur)->datasize = data_size;
	(cur)->datatype = format_type;
	(cur)->data = (uint8_t*)malloc(data_size * sizeof(uint8_t));
	(cur)->next = NULL;

	readcount = fread((cur)->data, sizeof(uint8_t), data_size, inputfile);

	if (ferror(inputfile) || readcount < data_size) {
		clearerr(inputfile);
		return -1;
	}
	fclose(inputfile);
	return 0;
}

       	
long 
get_filesize(FILE* f)
{
	int errorcode = 0;
	long size;
	long original_offset = 0;

	if (f)
	{
		original_offset = ftell(f);
		if (original_offset == -1) return -1;

		errorcode = fseek(f, 0, SEEK_END);
		if (errorcode < 0) return -1;

		size = ftell(f);

		errorcode = fseek(f, 0, original_offset);
		if (errorcode < 0) return -1;
		return size;
	}
	return -1;
}

