SRCS = simpleloop.c matmul.c blocked.c fourDimArr.c
PROGS = simpleloop matmul blocked fourDimArr

all : $(PROGS)

$(PROGS) : % : %.c
	gcc -Wall -g -o $@ $<


traces: $(PROGS)
	./runit simpleloop
	./runit matmul 100
	./runit blocked 100 25
	./runit fourDimArr

.PHONY: clean
clean : 
	rm -f simpleloop matmul blocked fourDimArr tr-*.ref *.marker *~
