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

int main(int argc, char *argv[]) {
    char *err_message = "USAGE: ./ext2_ls disk_name path_to_disk [-a]\n";
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "%s", err_message);
        exit(1);
    }
    // name of disk
    char *name = argv[1];
    // path of disk
    char *path = argv[2];
    // the disk
    unsigned char *disk = saveImage(name);

    // check if absolute path & exists, if not return err

    // Index to the group descriptor, cast to the required struct
    struct   ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    // Get the attributes needed
    unsigned int block_bitmap = bgd->bg_block_bitmap;
    unsigned int inode_bitmap = bgd->bg_inode_bitmap;
    unsigned int inode_table = bgd->bg_inode_table;
    unsigned short free_blocks_count = bgd->bg_free_blocks_count;
    unsigned short free_inodes_count = bgd->bg_free_inodes_count;
    unsigned short used_dirs_count = bgd->bg_used_dirs_count;

    // Get inode bitmap
    char *bmi = (char *) (disk + (bgd->bg_inode_bitmap * EXT2_BLOCK_SIZE));
    // get inode
    struct ext2_inode* in = (struct ext2_inode*) (disk + inode_table * EXT2_BLOCK_SIZE);
    // get the attributes needed
    unsigned short mode = in->i_mode; // file mode
	unsigned short uid = in->i_uid; // Low 16 bits of Owner Uid
	unsigned int size = in->i_size; // Size in bytes 
    unsigned int block = in->i_block; //Pointers to blocks

    // if given path is file, print just the file   

    // else if given path is a directory, print all contents

    return 0;
}
