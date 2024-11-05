#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = x25_listener
SRC = x25_listener.c tablepid.c signals.c
LOCAL_CFLAGS = -I/usr/include/eicon -O
LOCAL_LIBES = -ldebug -lmos -lrouter -llog -lsmreq -lsocket -lnsl -lx25 -lEec
DIP_LIBS = ${PX25HOME}/lib/libdebug.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

x25_listener.o: /usr/include/stdio.h /usr/include/sys/types.h
x25_listener.o: /usr/include/sys/socket.h /usr/include/string.h
x25_listener.o: /usr/include/errno.h /usr/include/sys/errno.h
x25_listener.o: /usr/include/signal.h /usr/include/sys/signal.h
x25_listener.o: /usr/include/netinet/in.h /usr/include/sys/stream.h
x25_listener.o: /usr/include/sys/cred.h /usr/include/sys/vnode.h
x25_listener.o: /usr/include/sys/time.h /usr/include/sys/mac.h
x25_listener.o: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
x25_listener.o: /usr/include/sys/byteorder.h /usr/include/netdb.h
x25_listener.o: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
x25_listener.o: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
x25_listener.o: /usr3/px25/include/px25_globals.h
x25_listener.o: /usr3/px25/include/x25_caller.h /usr3/px25/include/sm.h
x25_listener.o: /usr3/px25/include/debug.h /usr3/px25/include/errlog.h
tablepid.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
tablepid.o: /usr/include/errno.h /usr/include/sys/errno.h
tablepid.o: /usr/include/sys/types.h /usr3/px25/include/sm.h
tablepid.o: /usr3/px25/include/debug.h
signals.o: /usr/include/stdio.h /usr/include/signal.h
signals.o: /usr/include/sys/signal.h /usr/include/sys/types.h
signals.o: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
signals.o: /usr/include/eicon/x25proto.h /usr3/px25/include/px25_globals.h
signals.o: /usr3/px25/include/sm.h /usr3/px25/include/errlog.h
signals.o: /usr3/px25/include/debug.h
# DEBUG Objects Dependencies

x25_listener.do: /usr/include/stdio.h /usr/include/sys/types.h
x25_listener.do: /usr/include/sys/socket.h /usr/include/string.h
x25_listener.do: /usr/include/errno.h /usr/include/sys/errno.h
x25_listener.do: /usr/include/signal.h /usr/include/sys/signal.h
x25_listener.do: /usr/include/netinet/in.h /usr/include/sys/stream.h
x25_listener.do: /usr/include/sys/cred.h /usr/include/sys/vnode.h
x25_listener.do: /usr/include/sys/time.h /usr/include/sys/mac.h
x25_listener.do: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
x25_listener.do: /usr/include/sys/byteorder.h /usr/include/netdb.h
x25_listener.do: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
x25_listener.do: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
x25_listener.do: /usr3/px25/include/px25_globals.h
x25_listener.do: /usr3/px25/include/x25_caller.h /usr3/px25/include/sm.h
x25_listener.do: /usr3/px25/include/debug.h /usr3/px25/include/errlog.h
tablepid.do: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
tablepid.do: /usr/include/errno.h /usr/include/sys/errno.h
tablepid.do: /usr/include/sys/types.h /usr3/px25/include/sm.h
tablepid.do: /usr3/px25/include/debug.h
signals.do: /usr/include/stdio.h /usr/include/signal.h
signals.do: /usr/include/sys/signal.h /usr/include/sys/types.h
signals.do: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
signals.do: /usr/include/eicon/x25proto.h /usr3/px25/include/px25_globals.h
signals.do: /usr3/px25/include/sm.h /usr3/px25/include/errlog.h
signals.do: /usr3/px25/include/debug.h
