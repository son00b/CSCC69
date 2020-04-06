/* from handout:
This program takes two command line arguments.
The first is the name of an ext2 formatted virtual disk,
and the second is an absolute path to a file or link (not a directory) on that disk.

The program should work like rm,
removing the specified file from the disk.

If the file does not exist or if it is a directory,
then your program should return the appropriate error.

Once again, please read the specifications of ext2 carefully,
to figure out what needs to actually happen when a file or link is removed
(e.g., no need to zero out data blocks,
must set i_dtime in the inode,
removing a directory entry need not shift the directory entries after the one being deleted, etc.).

Bonus(5% extra):
Implement an additional "-r" flag
(after the disk image argument),
which allows removing directories as well.
In this case, you will have to recursively remove
all the contents of the directory specified in the last argument.

If "-r" is used with a regular file or link,
then it should be ignored
(the ext2_rm operation should be carried out as if the flag had not been entered).

If you decide to do the bonus, make sure first that your ext2_rm works,
then create a new copy of it and rename it to ext2_rm_bonus.c,
and implement the additional functionality in this separate source file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "ext2.h"
#include "helper.c"
#include "str_helper.c"

int rflag = 0; // 1 when "-r" is provided; 0 when "-r" not provided.

int remove_r(unsigned int inode, char *filename){
    int result = 0;
    struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    struct ext2_inode* in = (struct ext2_inode*) (disk + bgd->bg_inode_table * EXT2_BLOCK_SIZE);
    struct ext2_inode *new = in + (inode - 1);
    struct ext2_dir_entry_2 *dir_file = find_dir_entry(inode, filename);
    printf("%lu", inode);
    // remove if it's file or link 
    // printf("%d", dir_file->file_type == EXT2_FT_REG_FILE);
    // char type = (S_ISDIR(new->i_mode)) ? 'd' : ((S_ISREG(new->i_mode)) ? 'f' : 's');
    //     printf("[%d] type: %c size: %d links: %d blocks: %d\n", inode - 1, type, \
    //         new->i_size, new->i_links_count, new->i_blocks);     // Print Inode info
        
    if (dir_file->file_type == EXT2_FT_SYMLINK || dir_file->file_type == EXT2_FT_REG_FILE){
        unsigned long pre_pos = find_pre_pos(inode, filename);
        if (pre_pos){
            struct ext2_dir_entry_2 *pre_dir = (struct ext2_dir_entry_2 *) pre_pos;
            pre_dir->rec_len = pre_dir->rec_len + dir_file->rec_len;
        }
        dir_file->inode = 0;
        remove_link(inode);
        return 1;
    } else if (dir_file->file_type == EXT2_FT_DIR && rflag == 1){
        
        int index = 0;
        while (index < new->i_blocks){
            int blocknum = new->i_block[index];
            unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
            struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
            do {
                // Get the length of the current block and type
                int cur_len = dir->rec_len;
                // if file is regula file, link, or directory then print
                if (dir->file_type == EXT2_FT_REG_FILE || dir->file_type == EXT2_FT_SYMLINK || dir->file_type == EXT2_FT_DIR){
                    if (strncmp(dir->name, ".", dir->name_len) != 0 && strncmp(dir->name, "..", dir->name_len) != 0){
                        if(index == 0 || result == 1){
                            // printf("\n inode  %u %s \n", dir->inode, dir->name);
                            char *name = (char*) malloc((dir->name_len + 1)*sizeof(char));
                            strncpy(name, dir->name, dir->name_len);
                            result = remove_r(dir->inode, name);
                            free(name);
                        }
                    }
                }
                printf("%d ", pos);
                // Update position and index into it
                pos = pos + cur_len;
                dir = (struct ext2_dir_entry_2 *) pos;
                // Last directory entry leads to the end of block. Check if 
                // Position is multiple of block size, means we have reached the end
            } while (pos % EXT2_BLOCK_SIZE != 0);
            index++;
        }
        unsigned long pre_pos = find_pre_pos(inode, filename);
        if (pre_pos){
            struct ext2_dir_entry_2 *pre_dir = (struct ext2_dir_entry_2 *) pre_pos;
            pre_dir->rec_len = pre_dir->rec_len + dir_file->rec_len;
        }
        dir_file->inode = 0;
        remove_link(inode);
    } else if (dir_file->file_type == EXT2_FT_DIR && rflag == 0){
        return EISDIR;
    }
    printf("%d\n", result);
    return result;
}

int main(int argc, char *argv[]) {
    char *err_message = "USAGE: ./ext2_rm_bonus disk_name path_to_file [-r]\n";
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }

    
    int diskindex = 1;
    int pathindex = 2;
    if (argc == 4) {
        int i;
        for (i = 1; i < argc; i++) {
            if (strncmp(argv[i], "-r", strlen("-a")) == 0) {
                if (i == 1) {
                    fprintf(stderr, "%s", "-r should be after the disk image argument \n");
                    exit(1);
                } else if (i == 2) {
                    pathindex = 3;
                }
                rflag = 1;
            }
        }
        // if there are 3 command line argument, "-r" should be provided.
        if (rflag == 0) {
            fprintf(stderr, "-r should be provided.");
            exit(1);
        }
    }

    
    // name of disk
    char *disk_name = argv[diskindex];
    // path of file in disk
    char *path = argv[pathindex];
    // the disk
    init(disk_name);

    // if given path is root directory
    if (strcmp(path, "/") == 0){
        fprintf(stderr, "%s", dir_err);
        exit(1);
    } else{
        // get the file name
        char *filename = basename(path);
        if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0){
            fprintf(stderr, "%s", dir_err);
            exit(1);
        }
        char *cur = strtok(path, "/");
        // for every item in the path
        if(cur){
            unsigned int file_inode = traverse(2, cur, filename);
            // if last item exists, return 0 otherwise return enoent
            if(file_inode){
                int result = remove_r(file_inode, filename);
                if (result == EISDIR) {
                    // path cannot be directory
                    fprintf(stderr, "%s", dir_err);
                    return EISDIR;
                } else if (result == 1) {
                    return 0;
                }
            }
        }
        fprintf(stderr, "%s", dne_err);
        return ENOENT;
    }
}
