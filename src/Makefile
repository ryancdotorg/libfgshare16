SHELL=/bin/bash

CC = gcc
CFLAGS = -Wall -std=c99 -O2 -g
LDFLAGS = -L.

PRGMBINS = gftest mktab
TESTBINS = test_gf65536
BINARIES = $(PRGMBINS) $(TESTBINS)

all: progs

progs: $(PRGMBINS)

tests: $(TESTBINS)

gftest: gftest.o gf65536.o
	$(CC) $(CFLAGS) gftest.o gf65536.o $(LDFLAGS) -o gftest

mktab: mktab.o gf65536.o
	$(CC) $(CFLAGS) mktab.o gf65536.o $(LDFLAGS) -o mktab

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

test_%: %.o test_%.o speed.o
	$(CC) $(CFLAGS) test_$*.o $*.o speed.o $(LDFLAGS) -lcheck -o test_$*

ctags:
	ctags -R --c-kinds=+p --fields=+S . 

clean: 
	rm -f *.o *.a $(BINARIES) $(TESTBINS) 
