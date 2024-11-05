#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#
# -DUSE_LOCAL_NUA to set source nua in output call to the one
# contained in sync.tab

EXE = x25_caller
SRC = x25_caller.c 
LOCAL_CFLAGS =  -I/usr/include/eicon -g -DUSE_LOCAL_NUA
LOCAL_LIBES = -ldebug -lmos -llog -lrouter -lsocket -lnsl -lx25 -lEec
DIP_LIBS = ${PX25HOME}/lib/libdebug.a ${PX25HOME}/lib/libmos.a \
				${PX25HOME}/lib/liblog.a ${PX25HOME}/lib/librouter.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

x25_caller.o: /usr/include/stdio.h /usr/include/stdlib.h
x25_caller.o: /usr/include/string.h /usr/include/fcntl.h
x25_caller.o: /usr/include/sys/types.h /usr/include/sys/fcntl.h
x25_caller.o: /usr/include/errno.h /usr/include/sys/errno.h
x25_caller.o: /usr/include/ctype.h /usr/include/signal.h
x25_caller.o: /usr/include/sys/signal.h /usr/include/eicon/x25.h
x25_caller.o: /usr/include/eicon/x25err.h /usr/include/eicon/x25proto.h
x25_caller.o: /usr/include/eicon/neterr.h /usr/px25/include/px25_globals.h
x25_caller.o: /usr/px25/include/gp.h /usr/px25/include/x25_caller.h
x25_caller.o: /usr/px25/include/debug.h /usr/px25/include/errlog.h
# DEBUG Objects Dependencies

x25_caller.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_caller.do: /usr/include/string.h /usr/include/fcntl.h
x25_caller.do: /usr/include/sys/types.h /usr/include/sys/fcntl.h
x25_caller.do: /usr/include/errno.h /usr/include/sys/errno.h
x25_caller.do: /usr/include/ctype.h /usr/include/signal.h
x25_caller.do: /usr/include/sys/signal.h /usr/include/eicon/x25.h
x25_caller.do: /usr/include/eicon/x25err.h /usr/include/eicon/x25proto.h
x25_caller.do: /usr/include/eicon/neterr.h /usr/px25/include/px25_globals.h
x25_caller.do: /usr/px25/include/gp.h /usr/px25/include/x25_caller.h
x25_caller.do: /usr/px25/include/debug.h /usr/px25/include/errlog.h
