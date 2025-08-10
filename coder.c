#include "coder.h"
#include <stdio.h>
#include <stdlib.h>

// Helper functions for writing bits
void bwInit(Writer * bw, FILE * fptr) {
	bw->fptr = fptr;
	bw->buffer = 0;
	bw->count = 0;
}
void bwBit(Writer * bw, unsigned int bit) {
	bw->buffer |= (bit & 1) << (7 - bw->count);
	bw->count++;
	if (bw->count == 8) {
		fputc(bw->buffer, bw->fptr);
		bw->buffer = 0;
		bw->count = 0;
	}
}
void bwByte(Writer * bw, unsigned char byte) {
	for (int i = 7; i >= 0; i--) {
		bwBit(bw, (byte >> i) & 1);
	}
}
void bwFlush(Writer * bw) {
	if (bw->count > 0) {
		fputc(bw->buffer, bw->fptr);
		bw->buffer = 0;
		bw->count = 0;
	}
}

// Helper function for encoding tree into header
void writeTree(Writer * bw, TreeNode * root, int * length) {
	if (root == NULL) return;

	// post-order traversal
	writeTree(bw, root->left, length);
	writeTree(bw, root->right, length);

	if (!root->left && !root->right) {
		// Leaf node, flag and write symbol
		bwBit(bw, 1);
		bwByte(bw, root->value);
		(*length) += 9;
	} else {
		// Internal node
		bwBit(bw, 0);
		(*length)++;
	}
}

void encode(FILE * fInput, FILE * fOutput, Code table[128], TreeNode * root, int length) {
	// HANDLING
	Writer * bw = malloc(sizeof(Writer));
	bwInit(bw, fOutput);

	// FILE HEADER
	// Write 4 bytes of zeroes to reserve space for header length
	for (int i = 0; i < 4; i++) bwByte(bw, 0);
	int bitLength = 0;
	writeTree(bw, root, &bitLength);
	bwFlush(bw);

	// Calculate bytes written, overwrite start of file, go back to EOF
	uint32_t bytes = (bitLength % 8 == 0) ? (bitLength / 8) : (bitLength / 8 + 1);
	fseek(fOutput, 0, SEEK_SET);
	fwrite(&bytes, sizeof(bytes), 1, fOutput);
	fflush(fOutput);
	fseek(fOutput, 0, SEEK_END);

	// Write original file length
	uint32_t input_length = (uint32_t)length;
	fwrite(&input_length, sizeof(input_length), 1, fOutput);

	// ENCODE RAW TEXT
	// Go to start of file, iterate over each char
	if (fseek(fInput, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Error resetting file read position.\n");
		return;
	}

	int charValue;
	unsigned char outbyte = 0;
	int bits_filled = 0;

	while ((charValue = fgetc(fInput)) != EOF) {
		unsigned int code = table[charValue].bits;
		int len = table[charValue].length;

		// Start with MSB, move back
		for (int i = len - 1; i >=0; i--) {
			// Shift bit into outbyte
			outbyte = (outbyte << 1) | ((code >> i) & 1);
			bits_filled++;

			// When outbyte is full, write and reset
			if (bits_filled == 8) {
				fputc(outbyte, fOutput);
				bits_filled = 0;
				outbyte = 0;
			}
		}
	}
	
	// Pad last byte with zeroes
	if (bits_filled > 0) {
		outbyte <<= (8 - bits_filled);
		fputc(outbyte, fOutput);
	}

	// Write terminator
	fwrite("\0", sizeof("\0"), 1, fOutput);

	free(bw);
}
