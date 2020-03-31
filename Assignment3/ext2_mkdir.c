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

    int count = count_item_in_path(path);
    char **names = arr_names(count, path);
    char* dir_name;
    char* parent_name;
    char* parent_path;
    // for every item in the path
    if (count == 0){
        fprintf(stderr, "%s", dne_err);
        return ENOENT;
    }
    else if (count == 1){
        dir_name = names[count - 1];
        char *cur = strtok(path, "/");
        unsigned int dir_inode = traverse(2, cur, dir_name);
            if (dir_inode){
                fprintf(stderr, "%s", exist_err);
                return EEXIST;
            }
        int inode = find_free_inode();
        int succ = create_link(2, inode, dir_name);
        if (succ){
            printf("%s", "yolo1");
        }
    } 
    else {
        dir_name = names[count - 1];
        parent_name = names[count - 2];
        parent_path = get_parent_path(count, path);
        printf("%s", "yolo");
        char *cur = strtok(path, "/");
        char *cur_p = strtok(parent_path, "/");
        unsigned int parent_inode = traverse(2, cur_p, parent_name);
        // if last item exists in path
        if(parent_inode){
            unsigned int dir_inode = traverse(2, cur, dir_name);
            if (dir_inode){
                fprintf(stderr, "%s", exist_err);
                return EEXIST;
            }
            // create a new directory entry for the new dir
            struct ext2_dir_entry_2 *new = malloc(sizeof(struct ext2_dir_entry_2));
            printf("%s", "yolo");
        }
        fprintf(stderr, "%s", dne_err);
        for (int k = 0; k < count; k++) {
            free(names[k]);
        }
        free(names);
        free(parent_path);
        return ENOENT;
    }

    return 0;
}
