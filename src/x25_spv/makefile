#
# Template makefile for PX25
# EXE = name of final executable
# SRC = list of source files
# LOCAL_CFLAGS = list of private Compiler flags
# LOCAL_LIBES = list of libraries to use ( in -lxxx format )
# DIP_LIBS = list of library pathnames used by the application
#

EXE = x25_spv
SRC = x25_spv.c x25_edit.c x25_show.c x25_status.c x25_tail.c x25_purge.c \
x25_board.c x25_enable.c x25_delete.c
LOCAL_CFLAGS = -g
LOCAL_LIBES = -llog -lnsl -lsmreq 
DIP_LIBS = ${PX25HOME}/lib/liblog.a

include	${PX25HOME}/doc/makefile.std
# DO NOT DELETE THIS LINE -- make depend depends on it.

x25_spv.o: /usr/include/stdio.h /usr/include/stdlib.h
x25_edit.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/unistd.h
x25_edit.o: /usr/include/sys/unistd.h /usr/px25/include/px25_globals.h
x25_edit.o: /usr/px25/include/group.h
x25_show.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/time.h
x25_show.o: /usr/px25/include/px25_globals.h
x25_status.o: /usr/include/stdio.h /usr/include/stdlib.h
x25_status.o: /usr/include/string.h /usr/include/rpc/rpc.h
x25_status.o: /usr/include/rpc/types.h /usr/include/sys/types.h
x25_status.o: /usr/include/sys/time.h /usr/include/sys/tiuser.h
x25_status.o: /usr/include/sys/fcntl.h /usr/include/netinet/in.h
x25_status.o: /usr/include/sys/stream.h /usr/include/sys/cred.h
x25_status.o: /usr/include/sys/vnode.h /usr/include/sys/mac.h
x25_status.o: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
x25_status.o: /usr/include/sys/byteorder.h /usr/include/sys/socket.h
x25_status.o: /usr/include/sys/t_kuser.h /usr/include/rpc/xdr.h
x25_status.o: /usr/include/rpc/auth.h /usr/include/rpc/clnt.h
x25_status.o: /usr/include/rpc/rpc_com.h /usr/include/rpc/rpc_msg.h
x25_status.o: /usr/include/rpc/auth_sys.h /usr/include/rpc/auth_des.h
x25_status.o: /usr/include/rpc/auth_esv.h /usr/include/rpc/token.h
x25_status.o: /usr/include/rpc/svc.h /usr/include/sys/select.h
x25_status.o: /usr/include/rpc/svc_auth.h /usr/px25/include/sm.h
x25_status.o: /usr/px25/include/px25_globals.h
x25_tail.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/time.h
x25_tail.o: /usr/px25/include/px25_globals.h
x25_purge.o: /usr/include/stdio.h /usr/include/stdlib.h
x25_purge.o: /usr/px25/include/px25_globals.h
x25_board.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
x25_board.o: /usr/px25/include/px25_globals.h
x25_enable.o: /usr/include/stdio.h /usr/include/stdlib.h
x25_enable.o: /usr/include/string.h /usr/include/rpc/rpc.h
x25_enable.o: /usr/include/rpc/types.h /usr/include/sys/types.h
x25_enable.o: /usr/include/sys/time.h /usr/include/sys/tiuser.h
x25_enable.o: /usr/include/sys/fcntl.h /usr/include/netinet/in.h
x25_enable.o: /usr/include/sys/stream.h /usr/include/sys/cred.h
x25_enable.o: /usr/include/sys/vnode.h /usr/include/sys/mac.h
x25_enable.o: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
x25_enable.o: /usr/include/sys/byteorder.h /usr/include/sys/socket.h
x25_enable.o: /usr/include/sys/t_kuser.h /usr/include/rpc/xdr.h
x25_enable.o: /usr/include/rpc/auth.h /usr/include/rpc/clnt.h
x25_enable.o: /usr/include/rpc/rpc_com.h /usr/include/rpc/rpc_msg.h
x25_enable.o: /usr/include/rpc/auth_sys.h /usr/include/rpc/auth_des.h
x25_enable.o: /usr/include/rpc/auth_esv.h /usr/include/rpc/token.h
x25_enable.o: /usr/include/rpc/svc.h /usr/include/sys/select.h
x25_enable.o: /usr/include/rpc/svc_auth.h /usr/px25/include/sm.h
x25_enable.o: /usr/px25/include/px25_globals.h
x25_delete.o: /usr/include/stdio.h /usr/include/stdlib.h
x25_delete.o: /usr/px25/include/px25_globals.h
# DEBUG Objects Dependencies

x25_spv.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_edit.do: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/unistd.h
x25_edit.do: /usr/include/sys/unistd.h /usr/px25/include/px25_globals.h
x25_edit.do: /usr/px25/include/group.h
x25_show.do: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/time.h
x25_show.do: /usr/px25/include/px25_globals.h
x25_status.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_status.do: /usr/include/string.h /usr/include/rpc/rpc.h
x25_status.do: /usr/include/rpc/types.h /usr/include/sys/types.h
x25_status.do: /usr/include/sys/time.h /usr/include/sys/tiuser.h
x25_status.do: /usr/include/sys/fcntl.h /usr/include/netinet/in.h
x25_status.do: /usr/include/sys/stream.h /usr/include/sys/cred.h
x25_status.do: /usr/include/sys/vnode.h /usr/include/sys/mac.h
x25_status.do: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
x25_status.do: /usr/include/sys/byteorder.h /usr/include/sys/socket.h
x25_status.do: /usr/include/sys/t_kuser.h /usr/include/rpc/xdr.h
x25_status.do: /usr/include/rpc/auth.h /usr/include/rpc/clnt.h
x25_status.do: /usr/include/rpc/rpc_com.h /usr/include/rpc/rpc_msg.h
x25_status.do: /usr/include/rpc/auth_sys.h /usr/include/rpc/auth_des.h
x25_status.do: /usr/include/rpc/auth_esv.h /usr/include/rpc/token.h
x25_status.do: /usr/include/rpc/svc.h /usr/include/sys/select.h
x25_status.do: /usr/include/rpc/svc_auth.h /usr/px25/include/sm.h
x25_status.do: /usr/px25/include/px25_globals.h
x25_tail.do: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/time.h
x25_tail.do: /usr/px25/include/px25_globals.h
x25_purge.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_purge.do: /usr/px25/include/px25_globals.h
x25_board.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_board.do: /usr/include/string.h /usr/px25/include/px25_globals.h
x25_enable.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_enable.do: /usr/include/string.h /usr/include/rpc/rpc.h
x25_enable.do: /usr/include/rpc/types.h /usr/include/sys/types.h
x25_enable.do: /usr/include/sys/time.h /usr/include/sys/tiuser.h
x25_enable.do: /usr/include/sys/fcntl.h /usr/include/netinet/in.h
x25_enable.do: /usr/include/sys/stream.h /usr/include/sys/cred.h
x25_enable.do: /usr/include/sys/vnode.h /usr/include/sys/mac.h
x25_enable.do: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
x25_enable.do: /usr/include/sys/byteorder.h /usr/include/sys/socket.h
x25_enable.do: /usr/include/sys/t_kuser.h /usr/include/rpc/xdr.h
x25_enable.do: /usr/include/rpc/auth.h /usr/include/rpc/clnt.h
x25_enable.do: /usr/include/rpc/rpc_com.h /usr/include/rpc/rpc_msg.h
x25_enable.do: /usr/include/rpc/auth_sys.h /usr/include/rpc/auth_des.h
x25_enable.do: /usr/include/rpc/auth_esv.h /usr/include/rpc/token.h
x25_enable.do: /usr/include/rpc/svc.h /usr/include/sys/select.h
x25_enable.do: /usr/include/rpc/svc_auth.h /usr/px25/include/sm.h
x25_enable.do: /usr/px25/include/px25_globals.h
x25_delete.do: /usr/include/stdio.h /usr/include/stdlib.h
x25_delete.do: /usr/px25/include/px25_globals.h
