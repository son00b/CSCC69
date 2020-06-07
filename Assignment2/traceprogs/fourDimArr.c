/* File:     4 dimensional array
 *
 * Purpose:  make a 4 dimensional array with N^4 entries
 *
 * Compile:  gcc -g -Wall [-DDEBUG] -o fourDimArr fourDimArr.c
 *           [-] optional argument
 *
 * Run:      ./fourDimArr
 *
 * Output:   Elapsed time for making and assigning values to a 4 dimensional array
 *           If the DEBUG flag is given, all entries in the array will be printed.
 *
 * Why we think this program has interesting memory reference behavior:
 *          This program has extremely high hit rate (over 99%) regardless of
 *          page replacement algorithm or memory size.
 *          Moreover, there is no evictions when memory size >= 100.
 *
 * Note:    the format used in this block comment comes from starter/traceprogs/simpleloop.c
 */

#include <stdio.h>
#include <stdlib.h>

// N is used to determine the size of the array
#define N 5

// arr is a 4D array with N^4 entries
int ****arr;

// a function to create a 4 dimensional array
void init_multi_array();


int main() {
    // --- below adapted from starter/traceprogs/simpleloop.c ---
	volatile char MARKER_START, MARKER_END;
	FILE* marker_fp = fopen("fourDimArr.marker","w");
	if (marker_fp == NULL ) {
		perror("Couldn't open marker file:");
		exit(1);
	}
	fprintf(marker_fp, "%p %p", &MARKER_START, &MARKER_END);
	fclose(marker_fp);
    MARKER_START = 33;
    // --- above adapted from starter/traceprogs/simpleloop.c ---

    init_multi_array();

    MARKER_END = 34; // from starter/traceprogs/simpleloop.c
    return 0;
}


// initialize a 4 dimensional array
void init_multi_array() {
    int i, j, k, l;

    arr = malloc(N * sizeof(int***));

    for (i = 0; i < N; i ++) {

        arr[i] =  malloc(N * sizeof(int**));

        for (j = 0; j < N; j ++) {

            arr[i][j] = malloc(N * sizeof(int*));

            for (k = 0; k < N; k ++) {

                arr[i][j][k] = malloc(N * sizeof(int));

                for (l = 0; l < N; l ++) {

                    arr[i][j][k][l] = i*j*k*l;
#  ifdef DEBUG
                    // print entry if DEBUG flag is on
                    printf("%d ", arr[i][j][k][l]);
#  endif
                }
            }
        }
    }
}
