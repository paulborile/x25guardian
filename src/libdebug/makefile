#
# LIB = library name
# SRC = list of source files the library is made of.
# LOCAL_CFLAGS = list of private Compiler flags
# WARNING : if library is a local one, comment the line after install:
# ( Do not install in project lib dir )
#

LIB = libdebug.a
SRC = debug.c
LOCAL_CFLAGS = -g

.SUFFIXES: .uil .uid .do

include	${PX25HOME}/doc/makefile.libstd
# DO NOT DELETE THIS LINE -- make depend depends on it.

debug.o: /usr/include/sys/types.h /usr/include/sys/times.h
debug.o: /usr/include/fcntl.h /usr/include/sys/fcntl.h /usr/include/stdio.h
debug.o: /usr/include/varargs.h /usr/px25/include/debug.h
