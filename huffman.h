#ifndef __HUFFMAN__
#define __HUFFMAN__

#include <stdio.h>

typedef struct _TreeNode {
	struct _TreeNode * left;
	struct _TreeNode * right;
	int count;
	char value;
} TreeNode;

typedef struct _ListNode {
	struct _ListNode * next;
	union {
		TreeNode * node;
		int value;
	};
} ListNode;

TreeNode * createTreeNode(int count, char value);
ListNode * getIndex(ListNode * listHead, int index);
void swapNodes(ListNode * node1, ListNode * node2);
void quicksort(ListNode * listHead, int first, int last);
ListNode * treeHelper(ListNode * listHead, int length);
ListNode * countChars(FILE * fptr, int * length);
ListNode * refineCounts(ListNode * listHead, int * total);
void freeList(ListNode * listHead);
void freeTree(TreeNode * root);
void printChars(ListNode * listHead);

#endif
