#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int clock_evict() {
	int evict = coremap->index;
	// if the page is referenced, make it not referenced
	if(coremap[evict].pte->frame & PG_REF){
		coremap[evict].pte->frame &= ~PG_REF;
		// cannot evict this candidate so we have to
		// increase index to look for another candidate to evict
		coremap->index++;
		if(coremap->index >= memsize){
			coremap->index = 0;
		}
		// get the evict candidate
		evict = clock_evict();

	}
	// increase index to prepare for next evict
	coremap->index++;
	// reset index if end of page frames
	if(coremap->index >= memsize){
		coremap->index = 0;
	}
	return evict;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	// set reference bit on
	coremap[coremap->index].pte->frame |= PG_REF;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm.
 */
void clock_init() {
	coremap->index = 0;
}
