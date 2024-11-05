CSRC =  client.c

COBJ =  client.o x28_read.o

CFLAGS = -I${PX25HOME}/include -g
LIBES = -L${PX25HOME}/lib -lmos -linet

client	:	$(COBJ)
		$(CC) -o $@ $(COBJ) $(LIBES)

