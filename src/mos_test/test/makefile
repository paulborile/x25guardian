# OBJ =  s.o
OBJ =  s.o mos_send.o mos_recv.o

CFLAGS = -I${PX25HOME}/include -g -DDEBUG -DMYWRITEV
LIBES = -L${PX25HOME}/lib
# LIBES = -L${PX25HOME}/lib -lsocket -lnsl -lmos

all		:
			make s
			make c

client	:	$(COBJ)
		$(CC) -o $@ $(COBJ) $(LIBES)

s	:	$(OBJ)
		$(CC) -o $@ $(OBJ) $(LIBES)

# c	:  c.o mos_send.o mos_recv.o
#	$(CC) -o $@ c.o $(LIBES)

c	:  c.o mos_send.o mos_recv.o
	$(CC) -o $@ c.o mos_send.o mos_recv.o $(LIBES)

