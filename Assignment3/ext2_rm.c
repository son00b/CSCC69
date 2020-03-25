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

#include "ext2.h"
#include "helper.c"

int main(int argc, char *argv[]) {
    char *err_message = "USAGE: ./ext2_rm disk_name path_to_file/link\n";
    if (argc != 3) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }
    char *type_err = "given path has to be regular file or link\n";
    char *dne_err = "given path does not exist\n";
    // name of disk
    char *disk_name = argv[1];
    // path of file in disk
    char *path = argv[2];
    // the disk
    unsigned char *disk = saveImage(disk_name);
    init();

    // if given path is root directory
    if (strcmp(path, "/") == 0){
        fprintf(stderr, "%s", type_err);
        exit(1);
    } else{
        // get the file name
        char *filename = basename(path);
        char *cur = strtok(path, "/");
        // for every item in the path
        while(cur){
            // For all the directory blocks
            for (int i = 0; i < dirsin; i++) {
                // Get the block number
                int blocknum = dirs[i];
                // Get the position in bytes and index to block
                unsigned long pos = (unsigned long) disk + blocknum * EXT2_BLOCK_SIZE;
                struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
                unsigned long pre_pos = pos;
                do {
                    
                    char *name = dir->name; 
                    // Get the length of the current block and type
                    int cur_len = dir->rec_len;
                    // if we found the file in path
                    if (strncmp(name, cur, dir->name_len) == 0 && strlen(cur) == dir->name_len){
                        // if this file is the last item in path
                        if (strncmp(name, filename, dir->name_len) == 0 && strlen(filename) == dir->name_len){
                            // if the last item is file or link, remove
                            if (dir->file_type == EXT2_FT_REG_FILE || dir->file_type == EXT2_FT_SYMLINK){
                                // Update position and index into it
                                struct ext2_dir_entry_2 *pre_dir = (struct ext2_dir_entry_2 *) pre_pos;
                                pre_dir->rec_len = pre_dir->rec_len + dir->rec_len;
                                dir->inode = 0;
                                return 0;
                            }
                            else if (dir->file_type == EXT2_FT_DIR){
                                fprintf(stderr, "%s", type_err);
                                exit(1);
                            }
                        } 
                        // if not, get the last item in path, and remove if exists
                        else {
                            unsigned int file_inode = traverse(dir->inode, cur, filename);
                            // if last item exists, return 0 otherwise return enoent
                            if(file_inode){
                                // get the item as directory entry
                                struct ext2_dir_entry_2 *dir_file = find_dir_entry(file_inode, filename);
                                // remove if it's file or link 
                                if (dir_file->file_type == EXT2_FT_SYMLINK || dir_file->file_type == EXT2_FT_REG_FILE){
                                    unsigned long pre_pos = find_pre_pos(file_inode, filename);
                                    struct ext2_dir_entry_2 *pre_dir = (struct ext2_dir_entry_2 *) pre_pos;
                                    pre_dir->rec_len = pre_dir->rec_len + dir_file->rec_len;
                                    dir_file->inode = 0;
                                    return 0;
                                }
                                // print everything in directory if it's a directory, also print hidden files if -a specified
                                else{
                                    printf("%s", type_err);
                                    exit(1);
                                }
                            } 
                            else{
                                printf("%s", dne_err);
                                exit(1);
                            }
                        }
                    }
                    pre_pos = pos;
                    // Update position and index into it
                    pos = pos + cur_len;
                    dir = (struct ext2_dir_entry_2 *) pos;

                    // Last directory entry leads to the end of block. Check if 
                    // Position is multiple of block size, means we have reached the end
                } while (pos % EXT2_BLOCK_SIZE != 0);
            }
            cur = strtok(NULL, "/");
        }
        fprintf(stderr, "%s", dne_err);
        exit(1);
    }
}
