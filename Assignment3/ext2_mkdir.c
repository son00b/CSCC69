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
// #include <libgen.h>
#include <string.h>
#include <errno.h>

#include "ext2.h"
#include "helper.c"

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

void print_names(int count, char* path, char** names) {
    printf("There are %d names in path %s\n", count, path);
    for (int y = 0; y < count; y++) {
        printf("    index %d: %s\n", y, names[y]);
    }
}

char *get_parent_path(char* path) {
    char *copy = malloc((strlen(path) + 1) * sizeof(char));
    if (copy == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(copy, path);
    if (strlen(copy) >= 1) {
        if (copy[strlen(copy) - 1] == '/') {
            copy[strlen(copy) - 1] = 0;
        }
    }
    char *final_slash = strrchr(copy, '/');
    if (final_slash) {
      *(final_slash) = 0;
    }
    return copy;
}

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

// char *dir_name = basename(path); //segfault
// char *path_p = dirname(path); //segfault
// char *parent = basename(path_p); //segfault

    int count = count_item_in_path(path);
    char **names = arr_names(count, path);
print_names(count, path, names); // debug
    char* dir_name;
    char* parent_name;
    char* parent_path;
    if (count == 0) { // path: /
        // error?
    } else if (count == 1) { // path: /level1

    } else {
        dir_name = names[count - 1];
        parent_name = names[count - 2];
        parent_path = get_parent_path(path);
printf("%s\n", dir_name);
printf("%s\n", parent_name);
printf("%s\n", parent_path);
printf("%s\n", path);
    }

    // for every item in the path
    // if(cur){
    //     unsigned int file_inode = traverse(2, cur, dir_name);
    //     // if last item exists in path
    //     if(file_inode){
    //         fprintf(stderr, "%s", exist_err);
    //         return EEXIST;
    //     }

    //     // get the parent inode
    // }

    // check if dir exists

    // if not, mkdir

    for (int k = 0; k < count; k++) {
        free(names[k]);
    }
    free(names);
    free(parent_path);
    return 0;
}
