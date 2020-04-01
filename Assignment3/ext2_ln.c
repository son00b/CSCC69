/* from handout:
This program takes three command line arguments.
The first is the name of an ext2 formatted virtual disk.
The other two are absolute paths on your ext2 formatted disk.

The program should work like ln,
creating a link from the first specified file to the second specified path.

If the source file does not exist (ENOENT),
if the link name already exists (EEXIST),
or if either location refers to a directory (EISDIR),
then your program should return the appropriate error.

Note that this version of ln only works with files.

Additionally, this command may take a "-s" flag,
after the disk image argument.
When this flag is used,
your program must create a symlink instead (other arguments remain the same).

If in doubt about correct operation of links,
use the ext2 specs and ask on the discussion board.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

#include "ext2.h"
#include "helper.c"

int main(int argc, char *argv[]) {
    printf("%s", "asd");
    char *err_message = "USAGE: ./ext2_ln disk_name path_to_disk1 path_to_disk2 [-s]\n";
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }

    int sflag = 0; // 1 when "-s" is provided; 0 when "-s" not provided.
    int diskindex = 1, pathindexA = 2, pathindexB = 3;
    if (argc == 5) {
        int i;
        for (i = 1; i < argc; i++) {
            if (strncmp(argv[i], "-s", strlen("-s")) == 0) {
                if (i == 1) {
                    fprintf(stderr, "-s should be after the disk image argument \n");
                    exit(1);
                } else if (i == 2) {
                    pathindexA = 3;
                    pathindexB = 4;
                } else if (i == 3) {
                    pathindexB = 4;
                }
                sflag = 1;
            }
        }
        // if there are 4 command line argument, "-s" should be provided.
        if (sflag == 0) {
            fprintf(stderr, "-s should be provided.");
            exit(1);
        }
    }
    
    char *disk_name = argv[diskindex];
    char *path1 = argv[pathindexA];
    char *path2 = argv[pathindexB];

    int count1 = count_item_in_path(path1);
    int count2 = count_item_in_path(path2);
    

    init(disk_name);

    if(strcmp(path1, "/") == 0 || strcmp(path2, "/") == 0){
        fprintf(stderr, "%s", dir_err);
        return EISDIR;
    }
    
    if (count1 == 0 || count2 == 0){
        fprintf(stderr, "%s", dne_err);
        return ENOENT;
    }
    else {
        // check if path1 exists
        char **names1 = arr_names(count1, path1);
        char *filename1 = names1[count1-1];
        char *cur1 = strtok(path1, "/");
        unsigned int inode1 = traverse(2, cur1, filename1);
        // if not, return error
        if (!inode1){
            fprintf(stderr, "%s", dne_err);
            return ENOENT;
        }

        char **names2 = arr_names(count2, path2);
        char *filename2 = names2[count2-1];

        char* parent_name = names2[count2-2];
        char* parent_path;

        unsigned int parent_inode;
        parent_path = get_parent_path(count2, path2);
        char *cur2 = strtok(path2, "/");
        unsigned int inode2 = traverse(2, cur2, filename2);

        if(inode2){
            fprintf(stderr, "%s", exist_err);
            return EEXIST;
        }
        
        // get the item as directory entry
        struct ext2_dir_entry_2 *dir_file = find_dir_entry(inode1, filename1);
        // print the name if it's file or link 
        if (dir_file->file_type == EXT2_FT_SYMLINK || dir_file->file_type == EXT2_FT_REG_FILE){
            char *cur_p = strtok(parent_path, "/");
            parent_name = basename(parent_path);
            parent_inode = traverse(2, cur_p, parent_name);
            create_link(parent_inode, inode1, filename2, EXT2_FT_REG_FILE);
            allocate(inode1, parent_inode, EXT2_S_IFREG);
        }
        else{
            fprintf(stderr, "%s", dir_err);
            return EISDIR;
        }

        // freeing
        for (int k = 0; k < count1; k++) {
                free(names1[k]);
        }
        for (int k = 0; k < count2; k++) {
                free(names2[k]);
        }
        free(names2);
        free(parent_path);

    } 

    
    
    return ENOENT;
}
