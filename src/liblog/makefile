#
# LIB = library name
# SRC = list of source files the library is made of.
# LOCAL_CFLAGS = list of private Compiler flags
# WARNING : if library is a local one, comment the line after install:
# ( Do not install in project lib dir )
#

LIB = liblog.a
SRC = errlog.c dscope.c binhex.c getparam.c lin_toks.c
LOCAL_CFLAGS = -O

.SUFFIXES: .uil .uid .do

include	${PX25HOME}/doc/makefile.libstd
# DO NOT DELETE THIS LINE -- make depend depends on it.

errlog.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/sys/types.h
errlog.o: /usr/include/time.h /usr/include/sys/stat.h /usr/include/sys/time.h
errlog.o: /usr/include/varargs.h /usr/px25/include/errlog.h
dscope.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/sys/types.h
dscope.o: /usr/include/sys/times.h /usr/include/sys/stat.h
dscope.o: /usr/include/sys/time.h /usr/include/string.h
dscope.o: /usr/px25/include/px25_globals.h /usr/px25/include/errlog.h
binhex.o: /usr/include/stdio.h
getparam.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
getparam.o: /usr/include/errno.h /usr/include/sys/errno.h
getparam.o: /usr/include/ctype.h /usr/include/sys/types.h
getparam.o: /usr/include/sys/time.h /usr/px25/include/gp.h
lin_toks.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
