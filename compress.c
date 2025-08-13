#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"
#include "coder.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s [input file] [output file]\n", argv[0]);
		return EXIT_FAILURE;
	}

	FILE * fptr = fopen(argv[1], "r");
	if (fptr == NULL) {
		fprintf(stderr, "Failed to open file %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	// Count the characters in the file
	int fileLength = 0;
	ListNode * counts = countChars(fptr, &fileLength);
	if (fileLength == 0) {
		fprintf(stderr, "File has length zero.\n");
		fclose(fptr);
		return EXIT_FAILURE;
	}
	int numOfChars = 0;
	ListNode * refinedCounts = refineCounts(counts, &numOfChars);
	free(counts);

	// Convert from character count array to encoding tree
	ListNode * root = treeHelper(refinedCounts, numOfChars);

	// Use tree to make a coding table
	Code * table = calloc(128, sizeof(Code));
	makeCodeTable(root->node, table, (unsigned int) 0, 0);

	// Write to output file
	FILE * fptrOut = fopen(argv[2], "wb");
	if (fptrOut == NULL) {
		fprintf(stderr, "Failed to open output file for writing\n.");
		return EXIT_FAILURE;
	}
	encode(fptr, fptrOut, table, root->node, fileLength);

	freeTree(root->node);
	free(root);
	free(table);
	fclose(fptr);
	fclose(fptrOut);

	return EXIT_SUCCESS;
}
