#include <stdio.h>
#include	<errno.h>
#include	<fcntl.h>
#include <sys/types.h>
#include	<sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>

/*
 * P.S.Borile 	:	Wed Apr 18 15:25:11 PDT 1990
 */

#define	SERVICE		"ttyserver"
#define	PROTOCOL	"tcp"
#define	BACKLOG		5		/* connections accepted		*/

struct	sockaddr_in	local_sin, remote_sin;
struct	servent		*se;
struct	protoent	*pe;

int	rc;
int	s, new;
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
	int		c,b;
	int		pipein[2];
	int		pipeout[2];
	int		pid, cpid;
	char		buffer[64*1024];
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

	if (( se = getservbyname(SERVICE, PROTOCOL)) == NULL )
	{
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

	if (( s = socket(PF_INET, SOCK_STREAM, pe->p_proto)) < 0 )
	{
		perror("socket");
		exit();
	}

	/*
 	 * Bind the socket to local sock struct with port inside
	 */

	if ( bind(s, &local_sin, sizeof(local_sin)) < 0 )
	{
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

	if (( new = accept(s, &remote_sin, &len)) < 0 )
	{
		perror("accept");
		exit();
	}

	printf("Call received\n");

	pid = fork();

	if ( pipe(pipein) < 0 )
	{
		perror("pipe : ");
		exit();
	}

	if ( pipe(pipeout) < 0 )
	{
		perror("pipe : ");
		exit();
	}

	switch (fork())
	{
		case	-1	:

			printf("Fork error\n");
			exit();
			break;

		case	0	:

			close(0);
			dup(pipein[0]);
			close(1);
			dup(pipeout[0]);
			execl("/usr/bin/sh", "sh", "-i", 0);
			exit();

		default	:

			switch (fork())
			{
				case	0	:

					while (( c = read(s, buffer, BUFSIZ)) > 0 )
						write(pipein[1], buffer, c);

					exit();
					break;

				default	:

					while (( b = read(pipeout[1], buffer, BUFSIZ)) > 0 )
						write(s, buffer, BUFSIZ);

					exit();
			}
	}
	exit();
}
