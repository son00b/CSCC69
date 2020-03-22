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
#include "ext2.h"
#include "helper.c"

int main(int argc, char *argv[]) {
    char *err_message = "USAGE: ./ext2_ls disk_name path_to_disk [-a]\n";
    
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }
    // name of disk
    char *disk_name = argv[1];
    // path of disk
    char *disk_path = argv[2];
    // the disk
    unsigned char *disk = saveImage(disk_name);
    fprintf(stderr, "%s", disk);

    // Index to the group descriptor, cast to the required struct
    struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);

    fprintf(stderr, "%s", "disk");
    // Get the attributes needed
    unsigned int inode_table = bgd->bg_inode_table;
    // get inode
    struct ext2_inode* in = (struct ext2_inode*) (disk + inode_table * EXT2_BLOCK_SIZE);
    // get the attributes needed
    unsigned short mode = in->i_mode; // file mode
    fprintf(stderr, "%d", mode);
    unsigned int *block = in->i_block; //Pointers to blocks

    // if given path is file or link, print just the file   
    if (mode & EXT2_S_IFREG || mode & EXT2_S_IFLNK ) {
        // check if file/link exists, if so print
        printf("%s", basename(disk_path));
    } 
    // if given path is a directory, print everything in the directory
    else if (mode & EXT2_S_IFDIR) {
        fprintf(stderr, "%s", "asd");
        unsigned long pos = (unsigned long) disk + (int) *block * EXT2_BLOCK_SIZE;
        struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) pos;
        do {
            // Get the length of the current block and type
            int cur_len = dir->rec_len;
            // Print the current directory entry
            printf("name: %s\n", dir->name);
            // Update position and index into it
            pos = pos + cur_len;
            dir = (struct ext2_dir_entry_2 *) pos;

            // Last directory entry leads to the end of block. Check if 
            // Position is multiple of block size, means we have reached the end
        } while (pos % EXT2_BLOCK_SIZE != 0);
    }
    
    return 0;
}
