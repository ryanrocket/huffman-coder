#ifndef __WRITER__
#define __WRITER__

#include "huffman.h"
#include <stdio.h>

typedef struct {
	FILE * fptr;
	unsigned char buffer;
	int count;
} Writer;

void bwInit(Writer * bw, FILE * fptr);
void bwBit(Writer * bw, unsigned int bit);
void bwByte(Writer * bw, unsigned char byte);
void bwFlush(Writer * bw);
void writeTree(Writer * bw, TreeNode * root, int * length);
void encode(FILE * fInput, FILE * fOutput, Code table[128], TreeNode * root, int length);

#endif
