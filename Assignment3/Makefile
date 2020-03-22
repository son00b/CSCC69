all: ext2_ls ext2_cp ext2_mkdir ext2_ln ext2_rm ext2_rm_bonus

%.o : %.c ext2.h
	gcc -Wall -o -c $<

ext2_ls : ext2_ls.o
	gcc -Wall -o $@ $^

ext2_cp : ext2_cp.o
	gcc -Wall -o $@ $^

ext2_mkdir : ext2_mkdir.o
	gcc -Wall -o $@ $^

ext2_ln : ext2_ln.o
	gcc -Wall -o $@ $^

ext2_rm : ext2_rm.o
	gcc -Wall -o $@ $^

ext2_rm_bonus : ext2_rm_bonus.o
	gcc -Wall -o $@ $^

clean :
	rm -f *.o ext2_ls ext2_cp ext2_mkdir ext2_ln ext2_rm ext2_rm_bonus
