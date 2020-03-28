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
#include <errno.h>

#include "ext2.h"
#include "helper.c"

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

int main(int argc, char *argv[]) {
    char *err_message = "USAGE: ./ext2_ls disk_name path_to_disk [-a]\n";
    
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }

    int aflag = 0; // 1 when "-a" is provided; 0 when "-a" not provided.
    int diskindex = 1;
    int pathindex = 2;
    if (argc == 4) {
        int i;
        for (i = 1; i < argc; i++) {
            if (strncmp(argv[i], "-a", strlen("-a")) == 0) {
                if (i == 1) {
                    fprintf(stderr, "%s", "-a should be after the disk image argument \n");
                    exit(1);
                } else if (i == 2) {
                    pathindex = 3;
                }
                aflag = 1;
            }
        }
        // if there are 3 command line argument, "-a" should be provided.
        if (aflag == 0) {
            fprintf(stderr, "-a should be provided.");
            exit(1);
        }
    }

    // name of disk
    char *disk_name = argv[diskindex];
    // path of file in disk
    char path[strlen(argv[pathindex])];
    strcpy(path, argv[pathindex]);
    // the disk
    init(disk_name);
    // if given path is root directory
    if (strcmp(path, "/") == 0){
        ls_block(2, aflag);
    } 
    // if the given path isn't root
    else{
        // get the file name
        char *filename = basename(path);
        // skip root directory
        char *cur = strtok(path, "/");
        
        // for every item in the path
        if(cur){
            unsigned int file_inode = traverse(2, cur, filename);
            // if last item exists in path
            if(file_inode){
                // get the item as directory entry
                struct ext2_dir_entry_2 *dir_file = find_dir_entry(file_inode, filename);
                // print the name if it's file or link 
                if (dir_file->file_type == EXT2_FT_SYMLINK || dir_file->file_type == EXT2_FT_REG_FILE){
                    printf("%.*s\n", dir_file->name_len, dir_file->name);
                }
                // print everything in directory if it's a directory, also print hidden files if -a specified
                else{
                    ls_block(file_inode, aflag);
                }
                return 0;
            }
        }
        fprintf(stderr, "%s", dne_err);
        return ENOENT;
    }

    
}
