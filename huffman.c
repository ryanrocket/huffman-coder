#include "huffman.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Create a new tree node
TreeNode * createTreeNode(int count, char value) {
	TreeNode * node = malloc(sizeof(TreeNode));
	node->left = NULL;
	node->right = NULL;
	node->count = count;
	node->value = value;
	return node;
}

// Quicksort helper functions
ListNode * getIndex(ListNode * listHead, int index) {
	// Yes, this is slow. No, I don't care.
	ListNode * curNode = listHead;
	for (int i = 0; i < index; i++) {
		curNode = curNode->next;
	}
	return curNode;
}
void swapNodes(ListNode * node1, ListNode * node2) {
	TreeNode * tmp = node1->node;
	node1->node = node2->node;
	node2->node = tmp;
}

// Quicksort implementation
void quicksort(ListNode * listHead, int first, int last) {
	if (first >= last) return;

	int pivot = getIndex(listHead, first)->node->count;
	int low = first + 1;
	int high = last;

	while (low <= high) {
		while ((low <= last) && (getIndex(listHead, low)->node->count <= pivot)) low++;
		while ((first < high) && (getIndex(listHead, high)->node->count > pivot)) high--;
		if (low < high) swapNodes(getIndex(listHead, low), getIndex(listHead, high));
	}
	swapNodes(getIndex(listHead, first), getIndex(listHead, high));
	quicksort(listHead, first, high - 1);
	quicksort(listHead, high + 1, last);
}

// Recursive function to convert from counts list to binary tree
ListNode * treeHelper(ListNode * listHead, int length) {
	// Check for exit condition
	if (listHead->next == NULL || length == 1) return listHead;

	// Sort the elements left in the array
	quicksort(listHead, 0, length - 1);

	// Combine last two elements into a new node
	TreeNode * newNode = malloc(sizeof(TreeNode));
	newNode->left = listHead->node;
	newNode->right = listHead->next->node;
	newNode->count = (newNode->left->count + newNode->right->count);

	// Insert new node into the list
	ListNode * insert = malloc(sizeof(ListNode));
	insert->node = newNode;
	insert->next = listHead->next->next;
	free(listHead->next);
	free(listHead);

	// Recurse further down the list
	return treeHelper(insert, length - 1);
}

// Create a list of character counts in a file
ListNode * countChars(FILE * fptr, int * length) {
	// Instantiate new list, node for each ASCII character
	ListNode * listHead = malloc(sizeof(ListNode));
	listHead->next = NULL;
	listHead->node = createTreeNode(0, 0);
	ListNode * prevNode = listHead;
	for (int i = 1; i < 128; i++) {
		ListNode * curNode = malloc(sizeof(ListNode));
		curNode->next = NULL;
		curNode->node = createTreeNode(0, i);
		prevNode->next = curNode;
		prevNode = curNode;
	}

	// Go to start of file, step and count characters
	if (fseek(fptr, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Error resetting file read position.\n");
		return NULL;
	}
	int charValue;
	ListNode * curNode = listHead;
	while ((charValue = fgetc(fptr)) != EOF) {
		for (int i = 0; i < charValue; i++) {
			curNode = curNode->next;
		}
		curNode->node->count++;
		(*length)++;
		curNode = listHead;
	}

	return listHead;
}

// Refine a list of character counts, removing unused chars
ListNode * refineCounts(ListNode * listHead, int * total) {
	ListNode * refineHead = NULL;
	ListNode * prevNode = refineHead;
	ListNode * curNode = listHead;

	while (curNode != NULL) {
		if (curNode->node->count > 0) {
			(*total)++;
			ListNode * newNode = malloc(sizeof(ListNode));
			newNode->next = NULL;
			newNode->node = createTreeNode(curNode->node->count, curNode->node->value);
			if (refineHead == NULL) {
				refineHead = newNode;
				prevNode = refineHead;
			} else {
				prevNode->next = newNode;
				prevNode = newNode;
			}
		}
		curNode = curNode->next;
	}

	return refineHead;
}

// Free a list of character counts
void freeList(ListNode * listHead) {
	ListNode * curNode = listHead;
	ListNode * nextNode;
	while (curNode != NULL) {
		free(curNode->node);
		nextNode = curNode->next;
		free(curNode);
		curNode = nextNode;
	}
}

// Free a tree of character counts
void freeTree(TreeNode * root) {

}

// Print a list of character counts
void printChars(ListNode * listHead) {
	ListNode * curNode = listHead;
	while (curNode != NULL) {
		printf("ASCII char %c val %d has count %d\n", 
				(char) curNode->node->value,
				curNode->node->value,
				curNode->node->count);
		curNode = curNode->next;
	}
}
