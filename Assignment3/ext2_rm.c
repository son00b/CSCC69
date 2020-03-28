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

// decrease link count by 1 given inode
void remove_link(unsigned int inode){
    struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    struct ext2_inode* in = (struct ext2_inode*) (disk + bgd->bg_inode_table * EXT2_BLOCK_SIZE);
    // Go through all the used inodes stored in the array above
    for (int i = 0; i < inumc; i++) {
        // Remember array stores the index
        struct ext2_inode* curr = in + inum[i];
        int inodenum = inum[i] + 1;
        if (inodenum == inode){
            curr->i_links_count = curr->i_links_count - 1;
        }
    }
}

int main(int argc, char *argv[]) {
    char *err_message = "USAGE: ./ext2_rm disk_name path_to_file/link\n";
    if (argc != 3) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }
    
    // name of disk
    char *disk_name = argv[1];
    // path of file in disk
    char *path = argv[2];
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
                // get the item as directory entry
                struct ext2_dir_entry_2 *dir_file = find_dir_entry(file_inode, filename);
                // remove if it's file or link 
                if (dir_file->file_type == EXT2_FT_SYMLINK || dir_file->file_type == EXT2_FT_REG_FILE){
                    unsigned long pre_pos = find_pre_pos(file_inode, filename);
                    if (pre_pos){
                        struct ext2_dir_entry_2 *pre_dir = (struct ext2_dir_entry_2 *) pre_pos;
                        pre_dir->rec_len = pre_dir->rec_len + dir_file->rec_len;
                    }
                    dir_file->inode = 0;
                    remove_link(file_inode);
                    return 0;
                }
                // path cannot be directory
                fprintf(stderr, "%s", dir_err);
                return EISDIR;
            }
        }
        fprintf(stderr, "%s", dne_err);
        return ENOENT;
    }
}
