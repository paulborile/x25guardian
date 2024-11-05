#include <stdio.h>
#include	<errno.h>
#include	<fcntl.h>
#include <sys/types.h>
#include	<sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>

/*
 * s e r v e r . c	: Example server using tcp protocol
 *
 * P.S.Borile 	:	Wed Apr 18 15:25:11 PDT 1990
 *
 */

#define	SERVICE		"px25test"
#define	PROTOCOL	"tcp"
#define	BACKLOG		5		/* connections accepted		*/

struct	sockaddr_in	local_sin, remote_sin;
struct	servent		*se;
struct	protoent	*pe;

int	rc;
int	s, new[5] = { -1, -1, -1, -1, -1};
char	str[80];

#ifdef DEBUG
#   define print(a) printf a
#else
#   define print(a)
#endif


main(argc, argv)
int		argc;
char	**argv;
{
	char		c;
	char		buf[64*1024];
	int		i = 0;
	char		tty_input[BUFSIZ];
	int		rc;
	fd_set		readfds, writefds, exceptfds;
	int		fd;
	int	len		= sizeof(remote_sin);
	char		cmd[BUFSIZ];

	/*
 	 * Select server and protocol 
	 */

	if (( se = getservbyname(SERVICE, PROTOCOL)) == NULL ) {
		fprintf(stderr, "%s : unknown service %s for protocol %s\n",
			argv[0], SERVICE, PROTOCOL);
		exit();
	}

	local_sin.sin_family	=	AF_INET;
	local_sin.sin_port		=	se->s_port;
	local_sin.sin_addr.s_addr	=	htonl(INADDR_ANY);

	/*
 	 * Get protocol number
	 */

	if (( pe = getprotobyname(PROTOCOL)) == NULL ) {
		fprintf(stderr, "%s : unknown protocol %s\n",
			argv[0], PROTOCOL);
		exit();
	}

	/*
 	 * Create Socket with all information collected
	 */

	if (( s = socket(PF_INET, SOCK_STREAM, pe->p_proto)) < 0 ) {
		perror("socket");
		exit();
	}

	/*
 	 * Bind the socket to local sock struct with port inside
	 */

	if ( bind(s, &local_sin, sizeof(local_sin)) < 0 ) {
		perror("bind");
		exit();
	}

	/* 
	 * Tell the kernel you are listening for clients
	 */

	if ( listen(s, BACKLOG) < 0 ) {
		perror("listen");
		exit();
	}

	/*
	 * Accept connections on a new sock struct (remote_sin)
	 */

	if (( fd = open(argv[1], O_RDWR|O_NDELAY)) < 0 )
	{
		perror(argv[1]);
		exit();
	}

again:

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	FD_SET(fd, &readfds);
	FD_SET(fd, &exceptfds);

	FD_SET(s, &readfds);
	FD_SET(s, &exceptfds);

	for (i=0; i<5; i++)
	{
		if ( new[i] != -1 )
		{
			FD_SET(new[i], &readfds);
			FD_SET(new[i], &exceptfds);
		}
	}

	print(("readfds %X writefds %X exceptfds %X\n",
						readfds, writefds, exceptfds));

	rc = select(20, &readfds, &writefds, &exceptfds, (struct timeval *) NULL);

	print(("Rc = %d errno %d\n", rc, errno));

	if ( rc == -1 ) exit();

	print(("readfds %d writefds %d exceptfds %d\n",
						readfds, writefds, exceptfds));
	
	if (FD_ISSET(fd, &readfds))
	{
		/* something to read on serial port	*/

		print(("To read on serial port\n"));

		rc = read(fd, &c, 1);
		write(1, &c, 1);
		print(("read rc = %d\n", rc));
	}

	if	(FD_ISSET(fd, &exceptfds))
	{
		/* Exception on serial	*/

		print(("Exception on serial port\n"));
	}

	if (FD_ISSET(s, &readfds))
	{
		print(("Read on socket s going to accept connection\n"));

		for (i=0; i<5; i++) if (new[i] == -1) break;

		printf("Read on socket %d going to accept connection\n", i);
		if (( new[i] = accept(s, &remote_sin, &len)) < 0 ) {
			perror("accept");
			exit();
		}
	}


	for (i=0; i<5; i++)
	{
		if (new[i] != -1)
			if (FD_ISSET(new[i], &readfds))
			{
				/* receive stuff from connected socket	*/

				print(("Read from socket new\n"));

				rc = mos_recv(new[i], buf, 64*1024);

				putchar('A' + i);
				fflush(stdout);
				print(("mos_recv rc = %d\n", rc));

				if ( rc == 0 )
				{
					close(new[i]);
					new[i] = -1;
				}
			}

	}
	goto again;
}
