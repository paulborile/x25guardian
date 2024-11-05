#
# LIB = library name
# SRC = list of source files the library is made of.
# LOCAL_CFLAGS = list of private Compiler flags
# WARNING : if library is a local one, comment the line after install:
# ( Do not install in project lib dir )
# use -DDINAMIC_RELOAD to enable automatic reload of tables when are modified
#

LIB = librouter.a
SRC = router.c ea_to_bin.c bin_to_ea.c sub_router.c
LOCAL_CFLAGS = -O -DDYNAMIC_RELOAD

.SUFFIXES: .uil .uid .do

include	${PX25HOME}/doc/makefile.libstd
# DO NOT DELETE THIS LINE -- make depend depends on it.

router.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/fcntl.h
router.o: /usr/include/sys/types.h /usr/include/sys/fcntl.h
router.o: /usr/include/sys/param.h /usr/include/sys/fs/s5param.h
router.o: /usr/include/sys/stat.h /usr/include/sys/time.h
router.o: /usr/px25/include/px25_globals.h /usr/px25/include/rt.h
ea_to_bin.o: /usr/include/ctype.h /usr/px25/include/px25_globals.h
ea_to_bin.o: /usr/px25/include/rt.h
bin_to_ea.o: /usr/include/ctype.h /usr/px25/include/px25_globals.h
bin_to_ea.o: /usr/px25/include/rt.h
sub_router.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/fcntl.h
sub_router.o: /usr/include/sys/types.h /usr/include/sys/fcntl.h
sub_router.o: /usr/include/string.h /usr/include/sys/param.h
sub_router.o: /usr/include/sys/fs/s5param.h /usr/include/sys/stat.h
sub_router.o: /usr/include/sys/time.h /usr/px25/include/px25_globals.h
sub_router.o: /usr/px25/include/group.h
