#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = x25_start
SRC = x25_start.c x25_spawn_proc.c pid.c
LOCAL_CFLAGS = -O
LOCAL_LIBES = -llog -lnsl -ldebug
DIP_LIBS = ${PX25HOME}/lib/liblog.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

x25_start.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/unistd.h
x25_start.o: /usr/include/sys/unistd.h /usr/include/signal.h
x25_start.o: /usr/include/sys/signal.h /usr/include/sys/types.h
x25_start.o: /usr/px25/include/px25_globals.h /usr/px25/include/errlog.h
x25_start.o: /usr/px25/include/debug.h
x25_spawn_proc.o: /usr/include/stdio.h /usr/px25/include/debug.h
x25_spawn_proc.o: /usr/px25/include/errlog.h
pid.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
pid.o: /usr/include/errno.h /usr/include/sys/errno.h /usr/include/sys/types.h
pid.o: /usr/include/sys/signal.h /usr/include/sys/procset.h
pid.o: /usr/px25/include/debug.h
# DEBUG Objects Dependencies

x25_start.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_start.do: /usr/include/unistd.h /usr/include/sys/unistd.h
x25_start.do: /usr/include/signal.h /usr/include/sys/signal.h
x25_start.do: /usr/include/sys/types.h /usr/px25/include/px25_globals.h
x25_start.do: /usr/px25/include/errlog.h /usr/px25/include/debug.h
x25_spawn_proc.do: /usr/include/stdio.h /usr/px25/include/debug.h
x25_spawn_proc.do: /usr/px25/include/errlog.h
pid.do: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
pid.do: /usr/include/errno.h /usr/include/sys/errno.h
pid.do: /usr/include/sys/types.h /usr/include/sys/signal.h
pid.do: /usr/include/sys/procset.h /usr/px25/include/debug.h
