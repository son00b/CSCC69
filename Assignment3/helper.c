#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

unsigned char *disk;
/**** The following array is used to keep track of directories ****/
int dirs[128];
int dirsin = 0;

unsigned char* saveImage(char *name) {
    
    // copied from readimage.c
    // except this time return the disk
    int fd = open(name, O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	perror("mmap");
	exit(1);
    } else{
        return disk;
    }
}

void init(){
    
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);

    // Index to the group descriptor, cast to the required struct
    struct   ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    // Get inode bitmap
    char *bmi = (char *) (disk + (bgd->bg_inode_bitmap * EXT2_BLOCK_SIZE));
    // Want to keep track of the used inodes in this array
    int inum[32];
    inum[0] = 1;    // Root inode is Inode number 2, index 1
    // current size of the array
    int inumc = 1;  // because we stored the first one
    // counter for shift
    int index2 = 0;
    for (int i = 0; i < sb->s_inodes_count; i++) {
        unsigned c = bmi[i / 8];                     // get the corresponding byte
        // If that bit was a 1, inode is used, store it into the array.
        // Note, this is the index number, NOT the inode number
        // inode number = index number + 1
        if ((c & (1 << index2)) > 0 && i > 10) {    // > 10 because first 11 not used
            inum[inumc++] = i;
        }
        if (++index2 == 8) (index2 = 0); // increment shift index, if > 8 reset.
    }
    struct ext2_inode* in = (struct ext2_inode*) (disk + bgd->bg_inode_table * EXT2_BLOCK_SIZE);
    // Go through all the used inodes stored in the array above
    for (int i = 0; i < inumc; i++) {
        // Remember array stores the index
        struct ext2_inode* curr = in + inum[i];
        char type = (S_ISDIR(curr->i_mode)) ? 'd' : ((S_ISREG(curr->i_mode)) ? 'f' : 's');
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
            arr++;
        }
    }

}

// print everything in a directory block
void ls_block(unsigned int inode, int aflag){
    for (int i = 0; i < dirsin; i++) {
        // Get the block number
        int blocknum = dirs[i];
        // Get the position in bytes and index to block
        unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
        struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
        if (inode == dir->inode && strcmp(dir->name, ".") == 0){
            do {
            // Get the length of the current block and type
            int cur_len = dir->rec_len;
            // if file is regula file, link, or directory then print
            if (dir->file_type == EXT2_FT_REG_FILE || dir->file_type == EXT2_FT_SYMLINK || dir->file_type == EXT2_FT_DIR){
                // print hidden files only if -a is specified
                if (strncmp(".", dir->name, strlen(".")) != 0 || aflag == 1){
                    printf("%.*s\n", dir->name_len, dir->name);
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
}

unsigned long find_file_pos(unsigned int inode, char* filename){
    for (int i = 0; i < dirsin; i++) {
        // Get the block number
        int blocknum = dirs[i];
        // Get the position in bytes and index to block
        unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
        struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
        do {
        // Get the length of the current block and type
        int cur_len = dir->rec_len;
        // if we found the next dir
        if (strncmp (filename, dir->name, dir->name_len) == 0 && strlen(filename) == dir->name_len){
            return pos;
        }
        // Update position and index into it
        pos = pos + cur_len;
        dir = (struct ext2_dir_entry_2 *) pos;
        }while (pos % EXT2_BLOCK_SIZE != 0);
    }
    return 0;
}

struct ext2_dir_entry_2 *find_dir_entry(unsigned int inode, char *filename){
    for (int i = 0; i < dirsin; i++) {
        // Get the block number
        int blocknum = dirs[i];
        // Get the position in bytes and index to block
        unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
        struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
        do {
        // Get the length of the current block and type
        int cur_len = dir->rec_len;
        // if we found the next dir
        if (strncmp (filename, dir->name, dir->name_len) == 0 && strlen(filename) == dir->name_len){
            return dir;
        }
        // Update position and index into it
        pos = pos + cur_len;
        dir = (struct ext2_dir_entry_2 *) pos;
        }while (pos % EXT2_BLOCK_SIZE != 0);
    }
    return NULL;
}

unsigned int traverse(unsigned int inode, char *cur, char *filename){
    for (int i = 0; i < dirsin; i++) {
        // Get the block number
        int blocknum = dirs[i];
        // Get the position in bytes and index to block
        unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
        struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
        if (inode == dir->inode && strcmp(dir->name, ".") == 0){
            cur = strtok(NULL, "/");
            do {
            // Get the length of the current block and type
            int cur_len = dir->rec_len;
            // if we found the next dir
            if (strncmp (cur, dir->name, dir->name_len) == 0 && strlen(cur) == dir->name_len){
                if (strncmp(dir->name, filename, dir->name_len) == 0 && strlen(filename) == dir->name_len){
                    return dir->inode;
                }else{
                    return traverse(dir->inode, cur, filename);
                }
            }
            
            
            // Update position and index into it
            pos = pos + cur_len;
            dir = (struct ext2_dir_entry_2 *) pos;
            }while (pos % EXT2_BLOCK_SIZE != 0);
        }
    }
    return 0;
}


