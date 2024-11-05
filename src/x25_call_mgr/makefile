#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = x25_call_mgr
SRC = x25_call_mgr.c syn_tpid.c
LOCAL_CFLAGS = -g 
LOCAL_LIBES = -ldebug -lsmreq -llog -lsocket -lnsl
DIP_LIBS = ${PX25HOME}/lib/libdebug.a ${PX25HOME}/lib/libsmreq.a \
${PX25HOME}/lib/liblog.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

x25_call_mgr.o: /usr/include/stdio.h /usr/include/stdlib.h
x25_call_mgr.o: /usr/include/sys/types.h /usr/include/string.h
x25_call_mgr.o: /usr/include/errno.h /usr/include/sys/errno.h
x25_call_mgr.o: /usr/include/signal.h /usr/include/sys/signal.h
x25_call_mgr.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
x25_call_mgr.o: /usr/include/sys/stream.h /usr/include/sys/cred.h
x25_call_mgr.o: /usr/include/sys/vnode.h /usr/include/sys/time.h
x25_call_mgr.o: /usr/include/sys/mac.h /usr/include/sys/poll.h
x25_call_mgr.o: /usr/include/sys/strmdep.h /usr/include/sys/byteorder.h
x25_call_mgr.o: /usr/include/netdb.h /usr/px25/include/px25_globals.h
x25_call_mgr.o: /usr/px25/include/errlog.h /usr/px25/include/debug.h
syn_tpid.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
syn_tpid.o: /usr/include/errno.h /usr/include/sys/errno.h
syn_tpid.o: /usr/include/sys/types.h /usr/px25/include/debug.h
# DEBUG Objects Dependencies

x25_call_mgr.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_call_mgr.do: /usr/include/sys/types.h /usr/include/string.h
x25_call_mgr.do: /usr/include/errno.h /usr/include/sys/errno.h
x25_call_mgr.do: /usr/include/signal.h /usr/include/sys/signal.h
x25_call_mgr.do: /usr/include/sys/socket.h /usr/include/netinet/in.h
x25_call_mgr.do: /usr/include/sys/stream.h /usr/include/sys/cred.h
x25_call_mgr.do: /usr/include/sys/vnode.h /usr/include/sys/time.h
x25_call_mgr.do: /usr/include/sys/mac.h /usr/include/sys/poll.h
x25_call_mgr.do: /usr/include/sys/strmdep.h /usr/include/sys/byteorder.h
x25_call_mgr.do: /usr/include/netdb.h /usr/px25/include/px25_globals.h
x25_call_mgr.do: /usr/px25/include/errlog.h /usr/px25/include/debug.h
syn_tpid.do: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
syn_tpid.do: /usr/include/errno.h /usr/include/sys/errno.h
syn_tpid.do: /usr/include/sys/types.h /usr/px25/include/debug.h
