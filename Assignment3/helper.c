#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

// error messages
char *dir_err = "given path cannot be directory\n";
char *dne_err = "given path does not exist\n";
char *exist_err = "given name already exists\n";

unsigned char *disk;
/**** The following array is used to keep track of directories ****/
int dirs[128];
int dirsin = 0;
int inumc = 1;  // because we stored the first one
// Want to keep track of the used inodes in this array
int inum[32];

void init(char *name) {
    
    // copied from readimage.c
    // except this time return the disk
    int fd = open(name, O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    } else{
        struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
        // Index to the group descriptor, cast to the required struct
        struct   ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
        // Get inode bitmap
        char *bmi = (char *) (disk + (bgd->bg_inode_bitmap * EXT2_BLOCK_SIZE));
        
        inum[0] = 1;    // Root inode is Inode number 2, index 1
        // current size of the array
        
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
}


char *get_parent_path(int count, char* path) {
    char *copy = malloc((strlen(path) + 1) * sizeof(char));
    if (copy == NULL) {
        perror("malloc");
        exit(1);
    }
    if (count == 1) {
        strcpy(copy, "/");
    } else {
        strcpy(copy, path);
        // trim / at the end of string if it exists
        if (strlen(copy) >= 1) {
            if (copy[strlen(copy) - 1] == '/') {
                copy[strlen(copy) - 1] = 0;
            }
        }
        // remove anything after the / before the base filename
        char *final_slash = strrchr(copy, '/');
        if (final_slash) {
            *(final_slash) = 0;
        }
    }
    return copy;
}

int round_up(int n){
    int remainder = n % 4;
    return n + (4 - remainder);
}


int allocate(unsigned int inode, int size){
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
                int dir_size = round_up(sizeof(dir->inode) + sizeof(dir->rec_len) + sizeof(dir->name_len) + dir->name_len + sizeof(dir->file_type));

                if (size + dir_size <= cur_len){
                    dir->rec_len = dir_size;
                    return cur_len - dir_size;
                }
                pos = pos + cur_len;
                dir = (struct ext2_dir_entry_2 *) pos;
            }while (pos % EXT2_BLOCK_SIZE != 0);
        }
    }
    return 0;
}

// return 1 if it's free and 0 if not
int is_free_inode(unsigned int inode){
    for (int i = 0; i < dirsin; i++) {
        // Get the block number
        int blocknum = dirs[i];
        // Get the position in bytes and index to block
        unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
        struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
        if (inode == dir->inode && strcmp(dir->name, ".") == 0){
            return 0;
        }
    }
    return 1;
}

unsigned int find_free_inode(){
    unsigned int inode = 11;
    int find = 0;
    while(!find){
        if (is_free_inode(inode)){
            find = 1;
        } else{
            inode++;
        }
    }
    return inode;
}

void allocate_inode(unsigned int inode){
    // get inode bitmap
    struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    bgd->bg_free_inodes_count--;
    bgd->bg_used_dirs_count++;
    char *bmi = (char *) (disk + (bgd->bg_inode_bitmap * EXT2_BLOCK_SIZE));
    inumc++;
    inum[inumc] = inode;
    // Remember array stores the index
    struct ext2_inode* in = (struct ext2_inode*) (disk + bgd->bg_inode_table * EXT2_BLOCK_SIZE);
    struct ext2_inode* new = in + inum[inumc];
    new->i_mode = EXT2_FT_DIR;
    new->i_links_count = 1;
    unsigned int *arr = new->i_block;
    // Loop through and print all value till a 0 is seen in the array
    while(1) {
        if (*arr == 0) {
            break;
        }
        // If it's a directory, add to the array.
        dirs[dirsin++] = *arr;
        arr++;
    }
}

unsigned long find_dir_block_pos(unsigned int inode){
    for (int i = 0; i < dirsin; i++) {
        // Get the block number
        int blocknum = dirs[i];
        // Get the position in bytes and index to block
        unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
        struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
        if (inode == dir->inode && strcmp(dir->name, ".") == 0){
            return pos;
        }
    }
    return 0;
}

int create_dir(unsigned int parent_inode, char *dir_name){
    
    int total_size = round_up(sizeof(unsigned int) + sizeof(unsigned short) + sizeof(strlen(dir_name)) + strlen(dir_name) + sizeof(EXT2_FT_DIR));
    
    // get parent dir
    unsigned long pos = find_dir_block_pos(parent_inode);
    struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
    do {
        // Get the length of the current block and type
        int cur_len = dir->rec_len;
        int dir_size = round_up(sizeof(dir->inode) + sizeof(dir->rec_len) + sizeof(dir->name_len) + dir->name_len + sizeof(dir->file_type));
        if (dir_size + total_size <= cur_len){
            dir->rec_len = dir_size;
            
            cur_len = cur_len - dir_size;;
            // set up new directory
            struct ext2_dir_entry_2 *new = (struct ext2_dir_entry_2 *) (pos + cur_len);
            new->file_type = EXT2_FT_DIR;
            strcpy(new->name, dir_name);
            new->name_len = strlen(dir_name);
            new->rec_len = cur_len - dir_size;
            unsigned int inode = find_free_inode();
            new->inode = inode;
            printf("%u", new->inode);
            return 1;
        }
        pos = pos + cur_len;
        dir = (struct ext2_dir_entry_2 *) pos;
    }while (pos % EXT2_BLOCK_SIZE != 0);
    return 0;
}

// finds previous position before given an inode (and filename)
unsigned long find_pre_pos(unsigned int inode, char* filename){
    for (int i = 0; i < dirsin; i++) {
        // Get the block number
        int blocknum = dirs[i];
        // Get the position in bytes and index to block
        unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
        unsigned long pre_pos = pos;
        struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
        do {
            // Get the length of the current block and type
            int cur_len = dir->rec_len;
            // if we found the next dir
            if (strncmp (filename, dir->name, dir->name_len) == 0 && strlen(filename) == dir->name_len){
                return pre_pos;
            }
            // Update position and index into it
            pre_pos = pos;
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
            do {
                // Get the length of the current block and type
                int cur_len = dir->rec_len;
                // if we found the next dir
                if (strncmp (cur, dir->name, dir->name_len) == 0 && strlen(cur) == dir->name_len){
                    if (strncmp(dir->name, filename, dir->name_len) == 0 && strlen(filename) == dir->name_len){
                        return dir->inode;
                    }else{
                        cur = strtok(NULL, "/");
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


