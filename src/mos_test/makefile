CSRC =  client.c
SSRC =  server.c

COBJ =  client.o
SOBJ =  server.o

CFLAGS = -I${PX25HOME}/include -g
LIBES = -L${PX25HOME}/lib -lmos -lsocket -lnsl

all		:
			make client
			make server
			make asy

client	:	$(COBJ)
		$(CC) -o $@ $(COBJ) $(LIBES)

tclient	:	tclient.o
		$(CC) -o $@ tclient.o $(LIBES)

server	:	$(SOBJ)
		$(CC) -o $@ $(SOBJ) $(LIBES)

asy	:	asy.o
		$(CC) -o $@ asy.o $(LIBES)

