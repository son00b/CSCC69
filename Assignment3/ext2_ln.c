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

int main(int argc, char *argv[]) {
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
    return 0;
}
