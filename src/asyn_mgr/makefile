#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = asyn_mgr
SRC = asyn_main.c asyn_ttab.c
LOCAL_CFLAGS = -g
LOCAL_LIBES = -ldebug -lsmreq -llog -lmos -lsocket -lnsl
DIP_LIBS = ${PX25HOME}/lib/libdebug.a ${PX25HOME}/lib/libsmreq.a \
${PX25HOME}/lib/liblog.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

asyn_main.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/limits.h
asyn_main.o: /usr/include/sys/types.h /usr/include/errno.h
asyn_main.o: /usr/include/sys/errno.h /usr/include/signal.h
asyn_main.o: /usr/include/sys/signal.h /usr/include/sys/socket.h
asyn_main.o: /usr/include/netinet/in.h /usr/include/sys/stream.h
asyn_main.o: /usr/include/sys/cred.h /usr/include/sys/vnode.h
asyn_main.o: /usr/include/sys/time.h /usr/include/sys/mac.h
asyn_main.o: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
asyn_main.o: /usr/include/sys/byteorder.h /usr/include/netdb.h
asyn_main.o: /usr/px25/include/px25_globals.h /usr/px25/include/errlog.h
asyn_main.o: /usr/px25/include/debug.h /usr/px25/include/group.h
asyn_ttab.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
asyn_ttab.o: /usr/include/errno.h /usr/include/sys/errno.h
asyn_ttab.o: /usr/include/sys/types.h /usr/px25/include/px25_globals.h
asyn_ttab.o: /usr/px25/include/debug.h /usr/px25/include/errlog.h
# DEBUG Objects Dependencies

asyn_main.do: /usr/include/stdio.h /usr/include/stdlib.h
asyn_main.do: /usr/include/limits.h /usr/include/sys/types.h
asyn_main.do: /usr/include/errno.h /usr/include/sys/errno.h
asyn_main.do: /usr/include/signal.h /usr/include/sys/signal.h
asyn_main.do: /usr/include/sys/socket.h /usr/include/netinet/in.h
asyn_main.do: /usr/include/sys/stream.h /usr/include/sys/cred.h
asyn_main.do: /usr/include/sys/vnode.h /usr/include/sys/time.h
asyn_main.do: /usr/include/sys/mac.h /usr/include/sys/poll.h
asyn_main.do: /usr/include/sys/strmdep.h /usr/include/sys/byteorder.h
asyn_main.do: /usr/include/netdb.h /usr/px25/include/px25_globals.h
asyn_main.do: /usr/px25/include/errlog.h /usr/px25/include/debug.h
asyn_main.do: /usr/px25/include/group.h
asyn_ttab.do: /usr/include/stdio.h /usr/include/stdlib.h
asyn_ttab.do: /usr/include/string.h /usr/include/errno.h
asyn_ttab.do: /usr/include/sys/errno.h /usr/include/sys/types.h
asyn_ttab.do: /usr/px25/include/px25_globals.h /usr/px25/include/debug.h
asyn_ttab.do: /usr/px25/include/errlog.h
