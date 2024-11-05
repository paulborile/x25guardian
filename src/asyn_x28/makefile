#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = asy_x28
SRC = asy_x28.c x28_read.c
LOCAL_CFLAGS = -O
LOCAL_LIBES = -ldebug -lmos -llog -lsocket -lnsl
DIP_LIBS = ${PX25HOME}/lib/libdebug.a ${PX25HOME}/lib/libmos.a \
			  ${PX25HOME}/lib/liblog.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

asy_x28.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
asy_x28.o: /usr/include/unistd.h /usr/include/sys/unistd.h
asy_x28.o: /usr/include/fcntl.h /usr/include/sys/types.h
asy_x28.o: /usr/include/sys/fcntl.h /usr/include/errno.h
asy_x28.o: /usr/include/sys/errno.h /usr/include/ctype.h
asy_x28.o: /usr/include/signal.h /usr/include/sys/signal.h
asy_x28.o: /usr/include/sys/time.h /usr/include/sys/termio.h
asy_x28.o: /usr/include/sys/termios.h /usr/px25/include/px25_globals.h
asy_x28.o: /usr/px25/include/gp.h /usr/px25/include/debug.h
asy_x28.o: /usr/px25/include/errlog.h
x28_read.o: /usr/include/stdio.h /usr/include/sys/types.h
x28_read.o: /usr/include/sys/termio.h /usr/include/sys/termios.h
x28_read.o: /usr/px25/include/px25_globals.h
# DEBUG Objects Dependencies

asy_x28.do: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
asy_x28.do: /usr/include/unistd.h /usr/include/sys/unistd.h
asy_x28.do: /usr/include/fcntl.h /usr/include/sys/types.h
asy_x28.do: /usr/include/sys/fcntl.h /usr/include/errno.h
asy_x28.do: /usr/include/sys/errno.h /usr/include/ctype.h
asy_x28.do: /usr/include/signal.h /usr/include/sys/signal.h
asy_x28.do: /usr/include/sys/time.h /usr/include/sys/termio.h
asy_x28.do: /usr/include/sys/termios.h /usr/px25/include/px25_globals.h
asy_x28.do: /usr/px25/include/gp.h /usr/px25/include/debug.h
asy_x28.do: /usr/px25/include/errlog.h
x28_read.do: /usr/include/stdio.h /usr/include/sys/types.h
x28_read.do: /usr/include/sys/termio.h /usr/include/sys/termios.h
x28_read.do: /usr/px25/include/px25_globals.h
