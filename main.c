#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"

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
		return EXIT_FAILURE;
	}
	int numOfChars = 0;
	ListNode * refinedCounts = refineCounts(counts, &numOfChars);
	free(counts);

	TreeNode * root = treeHelper(refinedCounts, numOfChars)->node;
	printf("Root count of %d versus file length of %d\n", root->count, fileLength);
	
	freeList(refinedCounts);
	fclose(fptr);

	return EXIT_SUCCESS;
}
