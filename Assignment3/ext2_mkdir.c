/* from handout:
ext2_mkdir:
This program takes two command line arguments.
The first is the name of an ext2 formatted virtual disk.
The second is an absolute path on your ext2 formatted disk.

The program should work like mkdir,
creating the final directory on the specified path on the disk.

If any component on the path to the location
where the final directory is to be created does not exist
or if the specified directory already exists,
then your program should return the appropriate error (ENOENT or EEXIST).

Again, please read the specifications to make sure
you're implementing everything correctly
(e.g., directory entries should be aligned to 4B,
entry names are not null-terminated, etc.).
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include "ext2.h"
#include "helper.c"

int main(int argc, char *argv[]) {
    char *err_message = "USAGE: ./ext2_mkdir disk_name path_to_disk\n";
    if (argc != 3) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }
    // name of disk
    char *name = argv[1];
    // path of disk
    char *path = argv[2];
    // the disk
    init(name);
    if (strcmp(path, "/") == 0){
        fprintf(stderr, "%s", exist_err);
        return EEXIST;
    }
    char *path_p = dirname(path);
    printf("%s", path_p);
    char *dir_name = basename(path);
    char *parent = basename(path_p);
    char *cur = strtok(path, "/");
    printf("%s", cur);
    // for every item in the path
    if(cur){
        unsigned int file_inode = traverse(2, cur, dir_name);
        // if last item exists in path
        if(file_inode){
            fprintf(stderr, "%s", exist_err);
            return EEXIST;
        }

        // get the parent inode
    }
    

    // check if dir exists

    // if not, mkdir


    return 0;
}
