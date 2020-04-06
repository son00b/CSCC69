/* from handout:
ext2_cp:

This program takes three command line arguments.
The first is the name of an ext2 formatted virtual disk.
The second is the path to a file on your native operating system,
and the third is an absolute path on your ext2 formatted disk.

The program should work like cp,
copying the file on your native file system onto the specified location on the disk.

If the specified file or target location does not exist,
then your program should return the appropriate error (ENOENT).

Please read the specifications of ext2 carefully,
some things you will not need to worry about (like permissions, gid, uid, etc.),
while setting other information in the inodes may be important (e.g., i_dtime).
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ext2.h"
#include "helper.c"
#include "str_helper.c"

void copy(unsigned int inode, unsigned int parent_inode, char* filepath){
    struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    struct ext2_inode* in = (struct ext2_inode*) (disk + bgd->bg_inode_table * EXT2_BLOCK_SIZE);
    struct ext2_inode *new = in + (inode - 1);
    int index = 0;
    FILE *file  = fopen(filepath, "r");
    char buffer [EXT2_BLOCK_SIZE];
    unsigned int blocknum;
    int total_size = 0;
    int size = fread(buffer,1,EXT2_BLOCK_SIZE, file);
    while (size > 0 && index <= 11) {
        total_size += size;
        blocknum = find_free_block();
        char *block = (char*) disk + (EXT2_BLOCK_SIZE * blocknum);
        new->i_block[index] = blocknum;
        new->i_blocks++;
        index++;
        strcpy(block, buffer);
        size = fread(buffer,1,EXT2_BLOCK_SIZE, file);
    }
    if (index <= 11){
        total_size += size;
        blocknum = find_free_block();
        char *block = (char*) disk + (EXT2_BLOCK_SIZE * blocknum);
        new->i_block[index] = blocknum;
        new->i_blocks++;
        index++;
        strncpy(block, buffer, size);
    }
    new->i_size = total_size;
}

int main(int argc, char *argv[]) {
    char *err_message = "USAGE: ./ext2_cp disk_name path_to_file path_to_disk\n";
    if (argc != 4) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }
    
    char *disk_name = argv[1];
    char *path1 = argv[2];
    char *path2 = argv[3];

    int count1 = count_item_in_path(path1);
    int count2 = count_item_in_path(path2);
    
    init(disk_name);

    if(strcmp(path1, "/") == 0){
        fprintf(stderr, "%s", dir_err);
        return EISDIR;
    }
    
    if (count1 == 0){
        fprintf(stderr, "%s", dne_err);
        return ENOENT;
    }
    else {
        // check if path1 exists
        struct stat s;
        int exist = stat(path1, &s);
        if(exist){
            fprintf(stderr, "%s", dne_err);
            return ENOENT;
        }

        char **names1 = arr_names(count1, path1);
        char **names2 = arr_names(count2, path2);
        char *filename = names1[count1-1];
        char *path = (char*) malloc((strlen(path2) + strlen(filename) + 2)*sizeof(char));
        strcpy(path, path2);
        strcat(path, "/");
        strcat(path, filename);
        unsigned int parent_inode;
        char* parent_name;
        unsigned int inode2;
        char *cur = strtok(path, "/");
        inode2 = traverse(2, cur, filename);
        if (count2 == 0){
            parent_inode = 2;
            parent_name = "/";
        }
        else if(count2 == 1){
            parent_name = names2[count2-1];
            char *cur_p = strtok(path2, "/");
            parent_inode = traverse(2, cur_p, parent_name);
        } else{
            parent_name = names2[count2-1];
            char *cur_p = strtok(path2, "/");
            parent_inode = traverse(2, cur_p, parent_name);
        }

        if(inode2){
            fprintf(stderr, "%s", exist_err);
            return EEXIST;
        }

        if (!parent_inode){
            fprintf(stderr, "%s", dne_err);
            return ENOENT;
        }

        // // print the name if it's file or link 
        
        int inode = find_free_inode();
        int succ = create_link(parent_inode, inode, filename, EXT2_FT_REG_FILE);
        if (succ){
            // create new block
            allocate(inode, 2, EXT2_S_IFREG, NULL);
            copy(inode, parent_inode, path1);
        }
        // freeing
        for (int k = 0; k < count1; k++) {
                free(names1[k]);
        }
        for (int k = 0; k < count2; k++) {
                free(names2[k]);
        }
        free(names1);
        free(names2);
        free(path);

    } 

    return 0;
}
