#
# LIB = library name
# SRC = list of source files the library is made of.
# LOCAL_CFLAGS = list of private Compiler flags
# WARNING : if library is a local one, comment the line after install:
# ( Do not install in project lib dir )
#

LIB = libmos.a
SRC = mos_send.c mos_receive.c
LOCAL_CFLAGS = -g

.SUFFIXES: .uil .uid .do

include	${PX25HOME}/doc/makefile.libstd
# DO NOT DELETE THIS LINE -- make depend depends on it.

mos_send.o: /usr/include/stdio.h /usr/include/errno.h
mos_send.o: /usr/include/sys/errno.h
mos_receive.o: /usr/include/stdio.h /usr/include/errno.h
mos_receive.o: /usr/include/sys/errno.h
