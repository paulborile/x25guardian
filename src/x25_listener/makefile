#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = x25_listener
SRC = x25_listener.c tablepid.c signals.c easyway.c argotel.c \
		security.c subnua.c

LOCAL_CFLAGS = -I/usr/include/eicon -g
LOCAL_LIBES = -ldebug -lmos -lrouter -llog -lsmreq -lsocket -lnsl -lx25 -lEec
DIP_LIBS = ${PX25HOME}/lib/libdebug.a ${PX25HOME}/lib/libmos.a \
				${PX25HOME}/lib/librouter.a ${PX25HOME}/lib/liblog.a \
				${PX25HOME}/lib/libsmreq.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

x25_listener.o: /usr/include/stdio.h /usr/include/sys/types.h
x25_listener.o: /usr/include/stdlib.h /usr/include/sys/socket.h
x25_listener.o: /usr/include/string.h /usr/include/errno.h
x25_listener.o: /usr/include/sys/errno.h /usr/include/signal.h
x25_listener.o: /usr/include/sys/signal.h /usr/include/eicon/x25.h
x25_listener.o: /usr/include/eicon/x25err.h /usr/include/eicon/x25proto.h
x25_listener.o: /usr/include/eicon/neterr.h /usr/px25/include/px25_globals.h
x25_listener.o: /usr/px25/include/sm.h /usr/px25/include/rt.h
x25_listener.o: /usr/px25/include/debug.h /usr/px25/include/errlog.h
tablepid.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
tablepid.o: /usr/include/errno.h /usr/include/sys/errno.h
tablepid.o: /usr/include/sys/types.h /usr/px25/include/sm.h
tablepid.o: /usr/px25/include/debug.h
signals.o: /usr/include/stdio.h /usr/include/signal.h
signals.o: /usr/include/sys/signal.h /usr/include/sys/types.h
signals.o: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
signals.o: /usr/include/eicon/x25proto.h /usr/px25/include/px25_globals.h
signals.o: /usr/px25/include/sm.h /usr/px25/include/errlog.h
signals.o: /usr/px25/include/debug.h
easyway.o: /usr/include/stdio.h /usr/include/sys/types.h
easyway.o: /usr/include/signal.h /usr/include/sys/signal.h
easyway.o: /usr/include/string.h /usr/include/sys/socket.h
easyway.o: /usr/include/netinet/in.h /usr/include/sys/stream.h
easyway.o: /usr/include/sys/cred.h /usr/include/sys/vnode.h
easyway.o: /usr/include/sys/time.h /usr/include/sys/mac.h
easyway.o: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
easyway.o: /usr/include/sys/byteorder.h /usr/include/netdb.h
easyway.o: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
easyway.o: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
easyway.o: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
easyway.o: /usr/px25/include/x25_caller.h /usr/px25/include/rt.h
easyway.o: /usr/px25/include/sm.h /usr/px25/include/errlog.h
easyway.o: /usr/px25/include/group.h
argotel.o: /usr/include/stdio.h /usr/include/sys/types.h
argotel.o: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
argotel.o: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
argotel.o: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
argotel.o: /usr/px25/include/errlog.h /usr/px25/include/sm.h
security.o: /usr/include/stdio.h /usr/include/eicon/x25.h
security.o: /usr/include/eicon/x25err.h /usr/include/eicon/x25proto.h
security.o: /usr/px25/include/px25_globals.h /usr/px25/include/errlog.h
security.o: /usr/px25/include/debug.h
subnua.o: /usr/include/stdio.h /usr/include/sys/types.h /usr/include/signal.h
subnua.o: /usr/include/sys/signal.h /usr/include/string.h
subnua.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
subnua.o: /usr/include/sys/stream.h /usr/include/sys/cred.h
subnua.o: /usr/include/sys/vnode.h /usr/include/sys/time.h
subnua.o: /usr/include/sys/mac.h /usr/include/sys/poll.h
subnua.o: /usr/include/sys/strmdep.h /usr/include/sys/byteorder.h
subnua.o: /usr/include/netdb.h /usr/include/eicon/x25.h
subnua.o: /usr/include/eicon/x25err.h /usr/include/eicon/x25proto.h
subnua.o: /usr/include/eicon/neterr.h /usr/px25/include/px25_globals.h
subnua.o: /usr/px25/include/debug.h /usr/px25/include/rt.h
subnua.o: /usr/px25/include/group.h /usr/px25/include/sm.h
subnua.o: /usr/px25/include/errlog.h
# DEBUG Objects Dependencies

x25_listener.do: /usr/include/stdio.h /usr/include/sys/types.h
x25_listener.do: /usr/include/stdlib.h /usr/include/sys/socket.h
x25_listener.do: /usr/include/string.h /usr/include/errno.h
x25_listener.do: /usr/include/sys/errno.h /usr/include/signal.h
x25_listener.do: /usr/include/sys/signal.h /usr/include/eicon/x25.h
x25_listener.do: /usr/include/eicon/x25err.h /usr/include/eicon/x25proto.h
x25_listener.do: /usr/include/eicon/neterr.h /usr/px25/include/px25_globals.h
x25_listener.do: /usr/px25/include/sm.h /usr/px25/include/rt.h
x25_listener.do: /usr/px25/include/debug.h /usr/px25/include/errlog.h
tablepid.do: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
tablepid.do: /usr/include/errno.h /usr/include/sys/errno.h
tablepid.do: /usr/include/sys/types.h /usr/px25/include/sm.h
tablepid.do: /usr/px25/include/debug.h
signals.do: /usr/include/stdio.h /usr/include/signal.h
signals.do: /usr/include/sys/signal.h /usr/include/sys/types.h
signals.do: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
signals.do: /usr/include/eicon/x25proto.h /usr/px25/include/px25_globals.h
signals.do: /usr/px25/include/sm.h /usr/px25/include/errlog.h
signals.do: /usr/px25/include/debug.h
easyway.do: /usr/include/stdio.h /usr/include/sys/types.h
easyway.do: /usr/include/signal.h /usr/include/sys/signal.h
easyway.do: /usr/include/string.h /usr/include/sys/socket.h
easyway.do: /usr/include/netinet/in.h /usr/include/sys/stream.h
easyway.do: /usr/include/sys/cred.h /usr/include/sys/vnode.h
easyway.do: /usr/include/sys/time.h /usr/include/sys/mac.h
easyway.do: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
easyway.do: /usr/include/sys/byteorder.h /usr/include/netdb.h
easyway.do: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
easyway.do: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
easyway.do: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
easyway.do: /usr/px25/include/x25_caller.h /usr/px25/include/rt.h
easyway.do: /usr/px25/include/sm.h /usr/px25/include/errlog.h
easyway.do: /usr/px25/include/group.h
argotel.do: /usr/include/stdio.h /usr/include/sys/types.h
argotel.do: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
argotel.do: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
argotel.do: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
argotel.do: /usr/px25/include/errlog.h /usr/px25/include/sm.h
security.do: /usr/include/stdio.h /usr/include/eicon/x25.h
security.do: /usr/include/eicon/x25err.h /usr/include/eicon/x25proto.h
security.do: /usr/px25/include/px25_globals.h /usr/px25/include/errlog.h
security.do: /usr/px25/include/debug.h
subnua.do: /usr/include/stdio.h /usr/include/sys/types.h
subnua.do: /usr/include/signal.h /usr/include/sys/signal.h
subnua.do: /usr/include/string.h /usr/include/sys/socket.h
subnua.do: /usr/include/netinet/in.h /usr/include/sys/stream.h
subnua.do: /usr/include/sys/cred.h /usr/include/sys/vnode.h
subnua.do: /usr/include/sys/time.h /usr/include/sys/mac.h
subnua.do: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
subnua.do: /usr/include/sys/byteorder.h /usr/include/netdb.h
subnua.do: /usr/include/eicon/x25.h /usr/include/eicon/x25err.h
subnua.do: /usr/include/eicon/x25proto.h /usr/include/eicon/neterr.h
subnua.do: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
subnua.do: /usr/px25/include/rt.h /usr/px25/include/group.h
subnua.do: /usr/px25/include/sm.h /usr/px25/include/errlog.h
