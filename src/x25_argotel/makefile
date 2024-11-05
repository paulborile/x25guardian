#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = x25_argotel
SRC = x25_argotel.c
LOCAL_CFLAGS = -I/usr/include/eicon -g
LOCAL_LIBES = -ldebug -lmos -llog -lrouter -lsmreq -lsocket -lnsl -lx25 -lEec
DIP_LIBS = ${PX25HOME}/lib/libdebug.a ${PX25HOME}/lib/libmos.a \
				${PX25HOME}/lib/liblog.a ${PX25HOME}/lib/librouter.a \
				${PX25HOME}/lib/libsmreq.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

x25_argotel.o: /usr/include/stdio.h /usr/include/stdlib.h
x25_argotel.o: /usr/include/sys/types.h /usr/include/sys/socket.h
x25_argotel.o: /usr/include/netinet/in.h /usr/include/sys/stream.h
x25_argotel.o: /usr/include/sys/cred.h /usr/include/sys/vnode.h
x25_argotel.o: /usr/include/sys/time.h /usr/include/sys/mac.h
x25_argotel.o: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
x25_argotel.o: /usr/include/sys/byteorder.h /usr/include/netdb.h
x25_argotel.o: /usr/include/string.h /usr/include/eicon/x25.h
x25_argotel.o: /usr/include/eicon/x25err.h /usr/include/eicon/x25proto.h
x25_argotel.o: /usr/include/eicon/neterr.h /usr/px25/include/px25_globals.h
x25_argotel.o: /usr/px25/include/x25_caller.h /usr/px25/include/rt.h
x25_argotel.o: /usr/px25/include/group.h /usr/px25/include/sm.h
x25_argotel.o: /usr/px25/include/debug.h /usr/px25/include/errlog.h
# DEBUG Objects Dependencies

x25_argotel.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_argotel.do: /usr/include/sys/types.h /usr/include/sys/socket.h
x25_argotel.do: /usr/include/netinet/in.h /usr/include/sys/stream.h
x25_argotel.do: /usr/include/sys/cred.h /usr/include/sys/vnode.h
x25_argotel.do: /usr/include/sys/time.h /usr/include/sys/mac.h
x25_argotel.do: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
x25_argotel.do: /usr/include/sys/byteorder.h /usr/include/netdb.h
x25_argotel.do: /usr/include/string.h /usr/include/eicon/x25.h
x25_argotel.do: /usr/include/eicon/x25err.h /usr/include/eicon/x25proto.h
x25_argotel.do: /usr/include/eicon/neterr.h /usr/px25/include/px25_globals.h
x25_argotel.do: /usr/px25/include/x25_caller.h /usr/px25/include/rt.h
x25_argotel.do: /usr/px25/include/group.h /usr/px25/include/sm.h
x25_argotel.do: /usr/px25/include/debug.h /usr/px25/include/errlog.h
