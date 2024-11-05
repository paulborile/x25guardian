
#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = debugfun
SRC = debugfun.c
LOCAL_CFLAGS = -O -I${PX25HOME}/include
LOCAL_LIBES = -ldebug 
DIP_LIBS = ${PX25HOME}/src/libdebug/libdebug.a

include	${PX25HOME}/doc/makefile.std
