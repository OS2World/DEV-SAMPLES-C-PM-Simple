# nmake makefile
# 
# Tools used:
#  Compile::Resource Compiler
#  Compile::GNU C
#  Make: nmake

CFLAGS=-Wall -Zomf -c -O2
DEBUGFLAGS=-g -Og
INC=-I/@unixroot/usr/include -I/@unixroot/usr/include/os2tk45

all : simple.exe

debug : CFLAGS += $(DEBUGFLAGS)

debug : simple.obj simple.exe


simple.exe: simple.obj simple.def 
	gcc -Zomf simple.obj simple.def -o simple.exe

simple.obj: simple.c simple.h
	gcc $(CFLAGS) simple.c -o simple.obj

clean :
	rm -rf *exe *res *obj *map
