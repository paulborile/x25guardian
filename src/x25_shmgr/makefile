#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = x25_shmgr
SRC = x25_shmgr.c
LOCAL_CFLAGS = -g
LOCAL_LIBES = -lnsl -lsocket -ldebug -llog -lmos -lsmreq
DIP_LIBS = ${PX25HOME}/lib/liblog.a ${PX25HOME}/lib/libmos.a \
${PX25HOME}/lib/libsmreq.a ${PX25HOME}/lib/libdebug.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

x25_shmgr.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/limits.h
x25_shmgr.o: /usr/include/sys/types.h /usr/include/errno.h
x25_shmgr.o: /usr/include/sys/errno.h /usr/include/signal.h
x25_shmgr.o: /usr/include/sys/signal.h /usr/include/sys/socket.h
x25_shmgr.o: /usr/include/netinet/in.h /usr/include/sys/stream.h
x25_shmgr.o: /usr/include/sys/cred.h /usr/include/sys/vnode.h
x25_shmgr.o: /usr/include/sys/time.h /usr/include/sys/mac.h
x25_shmgr.o: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
x25_shmgr.o: /usr/include/sys/byteorder.h /usr/include/netdb.h
x25_shmgr.o: /usr/px25/include/px25_globals.h /usr/px25/include/errlog.h
x25_shmgr.o: /usr/px25/include/debug.h
# DEBUG Objects Dependencies

x25_shmgr.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_shmgr.do: /usr/include/limits.h /usr/include/sys/types.h
x25_shmgr.do: /usr/include/errno.h /usr/include/sys/errno.h
x25_shmgr.do: /usr/include/signal.h /usr/include/sys/signal.h
x25_shmgr.do: /usr/include/sys/socket.h /usr/include/netinet/in.h
x25_shmgr.do: /usr/include/sys/stream.h /usr/include/sys/cred.h
x25_shmgr.do: /usr/include/sys/vnode.h /usr/include/sys/time.h
x25_shmgr.do: /usr/include/sys/mac.h /usr/include/sys/poll.h
x25_shmgr.do: /usr/include/sys/strmdep.h /usr/include/sys/byteorder.h
x25_shmgr.do: /usr/include/netdb.h /usr/px25/include/px25_globals.h
x25_shmgr.do: /usr/px25/include/errlog.h /usr/px25/include/debug.h
