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

        char **names1 = arr_names(count2, path1);
        char **names2 = arr_names(count2, path2);
        char *filename = names1[count1-1];

        unsigned int parent_inode;
        char *parent_path = get_parent_path(count2, path2);

        char* parent_name;
        unsigned int inode2;
        if (count2 == 0){
            parent_inode = 2;
            inode2 = traverse(2, "/", filename);
        }
        else if(count2 == 1){
            parent_name = "/";
            char *cur2 = strtok(path2, "/");
            inode2 = traverse(2, cur2, filename);
            char *cur_p = strtok(parent_path, "/");
            parent_inode = traverse(2, cur_p, parent_name);
        } else{
            parent_name = names2[count2-2];
            char *cur2 = strtok(path2, "/");
            inode2 = traverse(2, cur2, filename);
            char *cur_p = strtok(parent_path, "/");
            parent_inode = traverse(2, cur_p, parent_name);
        }

        if(inode2){
            printf("%s", "asd");
            fprintf(stderr, "%s", exist_err);
            return EEXIST;
        }

        if (!parent_inode){
            fprintf(stderr, "%s", dne_err);
            return ENOENT;
        }

        // print the name if it's file or link 
        
        int inode = find_free_inode();
        int succ = create_link(2, inode, filename, s.st_mode);
        if (succ){
            // create new block
            allocate(inode, 2, s.st_mode, NULL);
        }
        // create_link(parent_inode, inode1, filename2, EXT2_FT_REG_FILE);
        // allocate(inode1, parent_inode, EXT2_S_IFREG);


        // freeing
        // for (int k = 0; k < count2; k++) {
        //         free(names2[k]);
        // }
        // free(names2);
        // free(parent_path);

    } 

    return 0;
}
