#include <sys/file.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <cartctl/nes.h>

int main(void)
{
	FILE* prg;
	FILE* chr;
	FILE* nes;

	nes = fopen("output.unif","wb");
	prg = fopen("input.prg","rb");
	chr = fopen("input.chr","rb");

	
	funif_sample(nes, prg, chr);
	if (prg) fclose(prg);
	if (chr) fclose(chr);
	if (nes) fclose(nes);
}

int funif_sample(FILE* output, FILE* prg, FILE* chr)
{
	struct stat sb;
	uint32_t prg_size_in_bytes = 0;
	uint32_t chr_size_in_bytes = 0;
	int errorcode = 0;
	int num_chunks = 0;
	unif_chunk_t chunks[2];

	if (!prg) return -1;
	errorcode = fstat(fileno(prg), &sb);
	if (errorcode) return errorcode;
	prg_size_in_bytes = sb.st_size;
		
	if (chr) {
		errorcode = fstat(fileno(chr), &sb);
		if (errorcode) return errorcode;
		chr_size_in_bytes = sb.st_size;
	}
	{
		uint8_t prg_mem[prg_size_in_bytes];
		uint8_t chr_mem[chr_size_in_bytes];
		fread(prg_mem,prg_size_in_bytes,1, prg);
		chunks[num_chunks++] = unif_prg_chunk(prg_size_in_bytes, prg_mem, 0);
		if (chr) {
			fread(chr_mem,chr_size_in_bytes,1, chr);
			chunks[num_chunks++] = unif_chr_chunk(chr_size_in_bytes, chr_mem, 0);
		}
		make_unif(output, chunks, num_chunks);
	}
	return 0;
}
