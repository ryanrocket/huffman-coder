#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"
#include "coder.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s [input file] [output file]\n", argv[0]);
		return EXIT_FAILURE;
	}

	FILE * fInput = fopen(argv[1], "r");
	if (fInput == NULL) {
		fprintf(stderr, "Failed to open file %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	FILE * fOutput = fopen(argv[2], "w");
	if (fOutput == NULL) {
		fprintf(stderr, "Failed to open file %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	// Attempt to decompress
	decode(fInput, fOutput);

	fclose(fInput);
	fclose(fOutput);

	return EXIT_SUCCESS;
}
