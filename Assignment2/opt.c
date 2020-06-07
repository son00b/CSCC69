#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"
#include "sim.h"
// extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the optimal (aka MIN) algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	int evict = 0; // evict candidate
	int max = -1; // farthest time out of the times which the pages are referenced
	int found = 0; // variable to exit loop
	int k = 0; // variable to loop through indices
	// for every element in the coremap
	while (k < memsize && found == 0){
		// if the element never referenced in the future
		if (coremap->memory[k] == -1 && coremap[k].pte){
			// select this element to be the evicted candidate, and exit loop
			found = 1;
			evict = k;
		}
		// otherwise, if this element referenced at the farthest time
		else if (coremap->memory[k] > max) {
			// update farthest time, and let this be the evict candidate for now
			max = coremap->memory[k];
			evict = k;
		}
		k++;
	}
	return evict;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	// set the index
	coremap->cur->index = p->frame >> PAGE_SHIFT;
	// if element exists in the index
	if(coremap[p->frame >> PAGE_SHIFT].pte){
		// if this element is not the first element
		if(coremap->cur->pre){
			int j = coremap->cur->time; // variable to loop through time
			int done = 0; // variable to exit loop
			// find the address of the page that is referenced previous timeslice
			// find & update the page's next reference (if exists)
			while(j < coremap->lines && done == 0) {
				coremap->memory[coremap->cur->pre->index] = -1;
				if (coremap->cur->pre->addr == coremap->addrs[j]){
					coremap->memory[coremap->cur->pre->index] = j;
					done = 1;
				}
				j++;
			}
		}
	}
	// go to the next time slice
	TraceItem *old = coremap->cur->pre;
	coremap->cur->pre = NULL;
	free(old);
	coremap->cur = coremap->cur->nxt;
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	// calculate total number of lines in the tracefile
	// then store it in the struct
	coremap->lines = 0;
	char ch;
	FILE *file = fopen(tracefile, "r");
    if (file)
    {
		while((ch = fgetc(file)) != EOF){
			if(ch == '\n'){
				coremap->lines++;
			}
		}
		coremap->lines++;
		fclose(file);
    }
	// addrs is array of addresses such that addrs[i] is the address of the frame at time i
	coremap->addrs = malloc(sizeof(addr_t) * coremap->lines);

	// cur represents the current position in tracefile
	coremap->cur = malloc(sizeof(TraceItem));

	TraceItem *pre;
	TraceItem *cur;
	int i = 0;
	file = fopen(tracefile, "r");
	if (file)
    {
		// create a doubly linked list (coremap->cur) of TraceItems
		// TraceItem contains info about the entry in tracefile
		// the list is sorted in the order of tracefile
		while((ch = fgetc(file)) != EOF) {
			// create head of doubly linked list
			if (i == 0) {
				coremap->cur->time = i;
				char c;
				fscanf(file, "%c %x", &c, (unsigned int*)&(coremap->cur->addr));
				coremap->addrs[i] = coremap->cur->addr;
				pre = coremap->cur;
				coremap->cur->pre = NULL;
			} else {
				cur = malloc(sizeof(*cur));
				char c;
				fscanf(file, "%c %x", &c, (unsigned int*)&(cur->addr));
				coremap->addrs[i] = cur->addr;
				cur->time = i;
				cur->nxt = NULL;
				pre->nxt = cur;
				cur->pre = pre;
				pre = cur;
			}
			i++;
		}
		fclose(file);
    }
	// initialize array to store the next occurence of page at index in coremap
	coremap->memory = malloc(sizeof(int) * memsize);
	for (int m = 0; m < memsize; m++) {
		coremap->memory[m] = -1;
	}
}

