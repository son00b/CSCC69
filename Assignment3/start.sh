# create a directory in /tmp and go there
mkdir -m 700 /tmp/liuheng7-csc369h
cd /tmp/liuheng7-csc369h

# to create your own disk image
dd if=/dev/zero of=DISKNAME.img bs=1024 count=128
/sbin/mke2fs -N 32 -F DISKNAME.img

# create a mount point and mount the image
# CWD is /tmp/<CDFID>-csc369h
mkdir mnt
fuseext2 -o rw+ DISKNAME.img mnt

# check to see if it is mounted
df -hl

# now you can use the mounted file system, for example
mkdir mnt/test

# unmount the image
fusermount -u mnt