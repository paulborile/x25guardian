#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = x25_incall
SRC = x25_incall.c x25_incall_rec.c
LOCAL_CFLAGS =  -I/usr/include/eicon -g -DCHECKS
LOCAL_LIBES = -ldebug -lmos -llog -lsocket -lnsl -lx25 -lEec
DIP_LIBS = ${PX25HOME}/lib/libdebug.a ${PX25HOME}/lib/libmos.a \
				${PX25HOME}/lib/liblog.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

x25_incall.o: /usr/include/stdio.h /usr/include/stdlib.h
x25_incall.o: /usr/include/sys/types.h /usr/include/string.h
x25_incall.o: /usr/include/errno.h /usr/include/sys/errno.h
x25_incall.o: /usr/include/signal.h /usr/include/sys/signal.h
x25_incall.o: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
x25_incall.o: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
x25_incall.o: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
x25_incall.o: /usr/px25/include/errlog.h
x25_incall_rec.o: /usr/include/stdio.h /usr/include/signal.h
x25_incall_rec.o: /usr/include/sys/signal.h /usr/include/sys/types.h
x25_incall_rec.o: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
x25_incall_rec.o: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
x25_incall_rec.o: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
x25_incall_rec.o: /usr/px25/include/errlog.h
# DEBUG Objects Dependencies

x25_incall.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_incall.do: /usr/include/sys/types.h /usr/include/string.h
x25_incall.do: /usr/include/errno.h /usr/include/sys/errno.h
x25_incall.do: /usr/include/signal.h /usr/include/sys/signal.h
x25_incall.do: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
x25_incall.do: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
x25_incall.do: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
x25_incall.do: /usr/px25/include/errlog.h
x25_incall_rec.do: /usr/include/stdio.h /usr/include/signal.h
x25_incall_rec.do: /usr/include/sys/signal.h /usr/include/sys/types.h
x25_incall_rec.do: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
x25_incall_rec.do: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
x25_incall_rec.do: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
x25_incall_rec.do: /usr/px25/include/errlog.h
