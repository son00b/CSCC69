#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int lru_evict() {
	// get the last node (containing the least recent page)
	Node *last = coremap->head;
	while (last->nxt){
		last = last->nxt;
	}

	// evict the content of least recent frame
	int evict = last->index;

	// unlink last node and free its memory
	// ensure the second last node point to NULL (become new last node)
	Node *secondLast = last->pre;
	free(last);
	if (secondLast) {
		secondLast->nxt = NULL;
	}

	return evict;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	// head node contains the most recent page
	// last node contains the least recent page

	Node *old;
	Node *cur = coremap->head;
	int done = 0;
	while (done == 0 && cur) {
		// if the frame is referenced before then
		if (cur->index == (p->frame >> PAGE_SHIFT)) {
			// delete (unlink) the old position in the linked list
			old = cur;
			if (cur->pre) {
				cur->pre->nxt = cur->nxt;
			}
			if (cur->nxt) {
				cur->nxt->pre = cur->pre;
			}
			// free memory of the old nod
			free(old);
			done = 1; // exit loop
		}
		// update current node if desired node is not found yet
		if (done == 0) {
			cur = cur->nxt;
		}
	}

	// make p new head of linked list
	Node *n = malloc(sizeof(Node));
	n->pre = NULL;
	n->nxt = coremap->head;
	n->index = p->frame >> PAGE_SHIFT;
	if (coremap->head) {
		coremap->head->pre = n;
	}
	coremap->head = n;
	return;
}


/* Initialize any data structures needed for this
 * replacement algorithm
 */
void lru_init() {
	// using a linked list to keep track of pages
	coremap->head = NULL;
}
