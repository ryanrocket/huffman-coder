#include "writer.h"
#include <stdio.h>

void writeTree(TreeNode * root, FILE * fptr) {
	if (root == NULL) return;
	writeTree(root->left, fptr);
	writeTree(root->right, fptr);
	if (!root->left && !root->right) fprintf(fptr, "1%03d", (int)root->value);
	else fprintf(fptr, "0");
}

void writeEncoding(TreeNode * root, FILE * fInput, FILE * fOutput) {
	// Go to start of the file, step and read characters
	if (fseek(fInput, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Error resetting file read positon.\n");
		return;
	}

	int charValue;
	while ((charValue = fgetc(fInput)) != EOF) {
		
	}
}
