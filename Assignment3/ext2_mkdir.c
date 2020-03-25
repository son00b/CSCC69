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
    unsigned char *disk = saveImage(name);
    init();

    // get the parent inode

    // check if dir exists

    // if not, mkdir


    return 0;
}
