/* from handout:
ext2_ls:
This program takes two command line arguments.
The first is the name of an ext2 formatted virtual disk.
The second is an absolute path on the ext2 formatted disk.

The program should work like ls -1 (that's number one "1", not lowercase letter "L"),
printing each directory entry on a separate line.

If the flag "-a" is specified (after the disk image argument),
your program should also print the . and .. entries. In other words,
it will print one line for every directory entry in the directory specified by the absolute path.

If the path does not exist,
print "No such file or directory",
and return an ENOENT.

Directories passed as the second argument may end in a "/"
- in such cases the contents of the last directory in the path (before the "/")
should be printed (as ls would do).

Additionally, the path (the last argument) may be a file or link.
In this case, your program should simply print the file/link name (if it exists) on a single line,
and refrain from printing the . and ..
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#include "ext2.h"
#include "helper.c"

int main(int argc, char *argv[]) {
    char *err_message = "USAGE: ./ext2_ls disk_name path_to_disk [-a]\n";
    
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }
    // name of disk
    char *disk_name = argv[1];
    // path of disk
    char *disk_path = argv[2];
    // the disk
    unsigned char *disk = saveImage(disk_name);
    fprintf(stderr, "%s", disk);

        struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);


    /**********************************************************************************/
    /************************************** TASK 1 ************************************/
    /**********************************************************************************/


    // Index to the group descriptor, cast to the required struct
    struct   ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);


    
    /**********************************************************************************/
    /************************************** TASK 2 ************************************/
    /**********************************************************************************/


    
    /******************************* Block Bitmap *************************************/

    // Get block bitmap
    char *bm = (char *) (disk + (bgd->bg_block_bitmap * EXT2_BLOCK_SIZE));
    // counter for shift
    printf("block bitmap: ");
    int index = 0;
    for (int i = 0; i < sb->s_blocks_count; i++) {
        unsigned c = bm[i / 8];                     // get the corresponding byte
        printf("%d", (c & (1 << index)) > 0);       // Print the correcponding bit
        if (++index == 8) (index = 0, printf(" ")); // increment shift index, if > 8 reset.
    }
    printf("\n");

    /******************************* Inode Bitmap *************************************/
    /************************* + store used inodes ************************************/

    // Get inode bitmap
    char *bmi = (char *) (disk + (bgd->bg_inode_bitmap * EXT2_BLOCK_SIZE));
    // Want to keep track of the used inodes in this array
    int inum[32];
    inum[0] = 1;    // Root inode is Inode number 2, index 1
    // current size of the array
    int inumc = 1;  // because we stored the first one
    // counter for shift
    printf("inode bitmap: ");
    int index2 = 0;
    for (int i = 0; i < sb->s_inodes_count; i++) {
        unsigned c = bmi[i / 8];                     // get the corresponding byte
        printf("%d", (c & (1 << index2)) > 0);       // Print the correcponding bit
        // If that bit was a 1, inode is used, store it into the array.
        // Note, this is the index number, NOT the inode number
        // inode number = index number + 1
        if ((c & (1 << index2)) > 0 && i > 10) {    // > 10 because first 11 not used
            inum[inumc++] = i;
        }
        if (++index2 == 8) (index2 = 0, printf(" ")); // increment shift index, if > 8 reset.
    }
    printf("\n\n");

    struct ext2_inode* in = (struct ext2_inode*) (disk + bgd->bg_inode_table * EXT2_BLOCK_SIZE);



    /******************************* Print Inodes *************************************/


    /**** The following array is used to keep track of directories ****/
    int dirs[128];
    int dirsin = 0;

    printf("Inodes:\n");

    // Go through all the used inodes stored in the array above
    for (int i = 0; i < inumc; i++) {
        // Remember array stores the index
        struct ext2_inode* curr = in + inum[i];
        int inodenum = inum[i] + 1;     // Number = index + 1
        char type = (S_ISDIR(curr->i_mode)) ? 'd' : ((S_ISREG(curr->i_mode)) ? 'f' : 's');
        printf("[%d] type: %c size: %d links: %d blocks: %d\n", inodenum, type, \
            curr->i_size, curr->i_links_count, curr->i_blocks);     // Print Inode info
        
        // Now to print all the blocks for the inode
        printf("[%d] Blocks: ", inodenum);  
        // Get the array of blocks from inode
        unsigned int *arr = curr->i_block;
        // Loop through and print all value till a 0 is seen in the array
        while(1) {
            if (*arr == 0) {
                break;
            }
            // If it's a directory, add to the array.
            if (type == 'd') {
                dirs[dirsin++] = *arr;
            }
            printf("%d ", *arr++);
        }
        printf("\n");
    }
    printf("\n");

    /**********************************************************************************/
    /************************************** TASK 3 ************************************/
    /**********************************************************************************/

    char *string = "Hello! /We are learning about strtok";
    char *cur;

    string = strdup(disk_path);
    printf("Original string: '%s'\n",string);
    // skip root directory
    cur = strsep(&string,"/");
    char *filename = basename(disk_path);
    while( (cur = strsep(&string,"/")) != NULL ){
        printf("Directory Blocks:\n");
        // For all the directory blocks
        for (int i = 0; i < dirsin; i++) {
            // Get the block number
            int blocknum = dirs[i];
            // Get the position in bytes and index to block
            unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
            struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;

            printf("    DIR BLOCK NUM: %d (for inode %d)\n", blocknum, dir->inode);

            do {
                char *name = dir->name;
                // Get the length of the current block and type
                int cur_len = dir->rec_len;
                if (strcmp(name, cur)){
                    printf("name: %s %s\n", cur, filename);
                    printf("%lu %lu", strlen(cur), strlen(filename));
                    printf("name: %d\n", strncmp(cur, filename, strlen(cur)));
                    if (strcmp(cur, filename)){
                        // Print the current directory entry
                        printf("name: %.*s\n", dir->name_len, dir->name);
                    }
                }
                
                
                // Update position and index into it
                pos = pos + cur_len;
                dir = (struct ext2_dir_entry_2 *) pos;

                // Last directory entry leads to the end of block. Check if 
                // Position is multiple of block size, means we have reached the end
            } while (pos % EXT2_BLOCK_SIZE != 0);
        }

    }
        


   
    
    
    return 0;
}
