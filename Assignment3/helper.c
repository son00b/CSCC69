#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

unsigned char *disk;



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

// print everything in a directory block
void ls_block(unsigned int inode, int dirsin, int dirs[128]){
    for (int i = 0; i < dirsin; i++) {
            // Get the block number
            int blocknum = dirs[i];
            // Get the position in bytes and index to block
            unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
            struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
            if (inode == dir->inode && strcmp(dir->name, ".") == 0){
                do {
                
                // printf("%u\n", dir_inode);
                char *name = dir->name; 
                // Get the length of the current block and type
                int cur_len = dir->rec_len;
                // if we found the file in path
                if (dir->file_type == EXT2_FT_REG_FILE || dir->file_type == EXT2_FT_SYMLINK || dir->file_type == EXT2_FT_DIR){
                    if (strcmp(dir->name, ".") != 0 && strcmp(dir->name, "..") != 0){
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

struct ext2_dir_entry_2 *cd(unsigned int inode, int dirsin, int dirs[128]){
    for (int i = 0; i < dirsin; i++) {
            // Get the block number
            int blocknum = dirs[i];
            // Get the position in bytes and index to block
            unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
            struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
            if (inode == dir->inode && strcmp(dir->name, ".") == 0){
                return dir;
        }
    }
    return NULL;
}


