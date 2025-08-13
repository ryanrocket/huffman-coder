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
		bw->buffer <<= (8 - bw->count);
		fputc(bw->buffer, bw->fptr);
		bw->buffer = 0;
		bw->count = 0;
	}
}

// Helper function for encoding tree into header
void writeTree(Writer * bw, TreeNode * root, uint32_t * length) {
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

// Helper function to recover a Huffman coding tree from header buffer
TreeNode * recoverTree(unsigned char * buffer, int bitLength) {
    int position = 0;
    TreeNode * stack[1024];
	int top = 0;

    while (position < bitLength) {
        int byteIndex = position / 8;
        int bitIndex  = 7 - (position % 8);   // writer writes MSB first in each byte
        unsigned int bit = (buffer[byteIndex] >> bitIndex) & 1;
        position++;

        if (bit) {
            // leaf: read exactly 8 following bits as the byte
            if (position + 8 > bitLength) {
                fprintf(stderr, "Malformed tree header: truncated leaf byte\n");
                // cleanup partial stack nodes
                for (int i = 0; i < top; ++i) free(stack[i]);
                return NULL;
            }

            unsigned char value = 0;
            for (int i = 0; i < 8; ++i) {
                byteIndex = position / 8;
                bitIndex  = 7 - (position % 8);
                value = (value << 1) | ((buffer[byteIndex] >> bitIndex) & 1);
                position++;
            }

            TreeNode * node = malloc(sizeof(TreeNode));
            if (!node) { perror("malloc"); exit(1); }
            node->value = value;
            node->left = node->right = NULL;
            stack[top++] = node;
        } else {
            // internal node: need two children on stack
            if (top < 2) {
                fprintf(stderr, "Malformed tree header: not enough nodes for internal (top=%d, pos=%d/%d)\n",
                        top, position, bitLength);
                for (int i = 0; i < top; ++i) free(stack[i]);
                return NULL;
            }

            TreeNode * rightNode = stack[--top];
            TreeNode * leftNode  = stack[--top];
            TreeNode * node = malloc(sizeof(TreeNode));
            if (!node) { perror("malloc"); exit(1); }
            node->left = leftNode;
            node->right = rightNode;
            stack[top++] = node;
        }
    }

    if (top != 1) {
        fprintf(stderr, "Malformed tree header: after parsing top=%d (expected 1)\n", top);
        for (int i = 0; i < top; ++i) free(stack[i]);
        return NULL;
    }

    return stack[0];
}

void encode(FILE * fInput, FILE * fOutput, Code table[128], TreeNode * root, int length) {
	// HANDLING
	Writer * bw = malloc(sizeof(Writer));
	bwInit(bw, fOutput);

	// FILE HEADER
	// Write 4 bytes of zeroes to reserve space for header length
	for (int i = 0; i < 4; i++) bwByte(bw, 0);
	uint32_t bitLength = 0;
	writeTree(bw, root, &bitLength);
	bwFlush(bw);
	fseek(fOutput, 0, SEEK_SET);
	fwrite(&bitLength, sizeof(bitLength), 1, fOutput);
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

	// Write null terminator
	fputc('\0', fOutput);

	free(bw);
}

void decode(FILE * fInput, FILE * fOutput) {
	// Get header size
	int headerLength = 0;
	fseek(fInput, 0, SEEK_SET);
	if (fread(&headerLength, sizeof(uint32_t), 1, fInput) != 1) {
		fprintf(stderr, "Could not recover header length from compressed file\n");
		return;
	}

	// Go to start of header, read into buffer
	int bufSize = ((headerLength % 8) != 0) ? (headerLength / 8) + 1 : headerLength / 8;
	unsigned char * buffer = malloc(sizeof(unsigned char) * bufSize);
	fseek(fInput, sizeof(uint32_t), SEEK_SET);
	if (fread(buffer, sizeof(unsigned char), bufSize, fInput) != bufSize) {
		fprintf(stderr, "Failed to read header into buffer\n");
		return;
	}

	// Recover coding tree from header buffer
	TreeNode * root = recoverTree(buffer, headerLength);

	// Read file size
	uint32_t fileSize = 0;
	if (fread(&fileSize, sizeof(uint32_t), 1, fInput) != 1) {
		fprintf(stderr, "Failed to read file size from header\n");
		return;
	}

	// Begin decoding stream
	// Ingest directional bits until a leaf is reached, continue
	TreeNode * curNode = root;
	int charValue = 0;
	int position = 0;
	while ((charValue = fgetc(fInput)) != EOF) {
		// charValue is one byte, process each bit
		for (position = 0; position < 8; position++) {
			unsigned int bit = (charValue >> (7 - position)) & 1;
			curNode = bit ? curNode->right : curNode->left;
			if (curNode->left == NULL && curNode->right == NULL) {
				// Reached a leaf node!
				fputc(curNode->value, fOutput);
				curNode = root;
			}
		}
	}
	fputc('\0', fOutput);

	free(buffer);
	freeTree(root);
}

