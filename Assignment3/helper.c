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


int count_item_in_path(char* path) {
    char *copy = malloc((strlen(path) + 1) * sizeof(char));
    if (copy == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(copy, path);

    int count = 0;
    char *cur = strtok(copy, "/");
    while (cur != NULL)
    {
        count++;
        cur = strtok(NULL, "/");
    }
    free(copy);
    return count;
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

char** arr_names(int count, char* path) {
    char *copy = malloc((strlen(path) + 1) * sizeof(char));
    if (copy == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(copy, path);

    char **names = malloc(count *  sizeof(char*));
    int j = 0;
    char *token = strtok(copy, "/");
    while (token != NULL) {
        names[j] = malloc(EXT2_NAME_LEN * sizeof(char));
        strcpy(names[j], token);
        j++;
        token = strtok(NULL, "/");
    }
    free(copy);
    return names;
}

unsigned int find_free_block(){
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    char *bm = (char *) (disk + (bgd->bg_block_bitmap * EXT2_BLOCK_SIZE));
    // counter for shift
    int index = 0;
    for (int i = 0; i < sb->s_blocks_count; i++) {
        unsigned c = bm[i / 8];                     // get the corresponding byte
        if ((c & (1 << index)) == 0) { 
            bm[i/8] = bm[i/8] | (1 << index);
            printf("%d %d", (c & (1 << index)) > 0, i+1);
            return i + 1;
        }
        if (++index == 8) (index = 0); // increment shift index, if > 8 reset.
    }
    return 0;
}

unsigned int find_free_inode(){
    // Index to the group descriptor, cast to the required struct
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    char *bmi = (char *) (disk + (bgd->bg_inode_bitmap * EXT2_BLOCK_SIZE));
    unsigned int inode = 12;

    int index2 = 0;
    for (int i = 0; i < sb->s_inodes_count; i++) {
        unsigned c = bmi[i / 8];                     // get the corresponding byte
               // Print the correcponding bit
        // If that bit was a 1, inode is used, store it into the array.
        // Note, this is the index number, NOT the inode number
        // inode number = index number + 1
        if ((c & (1 << index2)) == 0 && i > 10) {    // > 10 because first 11 not used
            bmi[i/8] = bmi[i/8] | (1 << index2);
            printf("%d %d", (c & (1 << index2)) > 0, i+1);
            return i + 1;
        }
        if (++index2 == 8) (index2 = 0); // increment shift index, if > 8 reset.
    }
    return 0;
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

int create_link(unsigned int parent_inode, unsigned int inode, char *name, unsigned char mode){
    int total_size = round_up(sizeof(unsigned int) + sizeof(unsigned short) + sizeof(name) + strlen(name) + sizeof(EXT2_FT_REG_FILE));
    unsigned long pos = find_dir_block_pos(parent_inode);
    struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
    do {
        // Get the length of the current block and type
        int cur_len = dir->rec_len;
        int dir_size = round_up(sizeof(dir->inode) + sizeof(dir->rec_len) + sizeof(dir->name_len) + dir->name_len + sizeof(dir->file_type));
        if (dir_size + total_size <= cur_len){
            dir->rec_len = dir_size;
            // set up new directory
            dir = (void*) dir + dir->rec_len;
            dir->file_type = mode;
            strcpy(dir->name, name);
            dir->name_len = strlen(name);
            dir->rec_len = cur_len - dir_size;
            dir->inode = inode;
            // set link count;
            
            return 1;
        }
        pos = pos + cur_len;
        dir = (struct ext2_dir_entry_2 *) pos;
    }while (pos % EXT2_BLOCK_SIZE != 0);
    return 0;
}

void allocate (unsigned int inode, unsigned int parent_inode, unsigned short mode) {
    struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    struct ext2_inode* in = (struct ext2_inode*) (disk + bgd->bg_inode_table * EXT2_BLOCK_SIZE);
    struct ext2_inode *new = in + (inode - 1);
    new->i_mode = mode;
    new->i_links_count++;
    if (S_ISDIR(mode)){
        new->i_links_count++;
        new->i_blocks = 2;
        new->i_size = EXT2_BLOCK_SIZE;
        unsigned int block = find_free_block();
        new->i_block[0] = block;
        // set for .
        unsigned long pos = (unsigned long) disk + block * EXT2_BLOCK_SIZE;
        struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
        dir->rec_len = 12;
        strcpy(dir->name, ".");
        dir->name_len = 1;
        dir->file_type = EXT2_FT_DIR;
        dir->inode = inode;
        // set for ..
        pos = pos + dir->rec_len;
        dir = (struct ext2_dir_entry_2 *) pos;
        dir->rec_len = 1012;
        strcpy(dir->name, "..");
        dir->name_len = 2;
        dir->file_type = EXT2_FT_DIR;
        dir->inode = parent_inode;
        // update parent link count
        struct ext2_inode *parent = in + (parent_inode - 1);
        parent->i_links_count++;
    }
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
    if(strcmp(filename, "/") == 0){
        return EXT2_ROOT_INO;
    }
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


