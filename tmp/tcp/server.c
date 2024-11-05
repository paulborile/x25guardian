#include <stdio.h>
#include <sys/types.h>
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
int	s, new;
char	str[80];


main(argc, argv)
int		argc;
char	**argv;
{
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

again:
	if (( new = accept(s, &remote_sin, &len)) < 0 ) {
		perror("accept");
		exit();
	}

	printf("Accept ok\n", );

	switch ( fork() )
	{
		case	-1	:

			perror("fork");
			break;

		case	0	:	/* child	*/

			/* exec new process to manage call	*/

			sprintf(cmd, "%d", new);
			execl("./asy", "asy", cmd, 0);

			break;

		default		:

			/* close new file descriptor	*/

			close(new);
			goto	again;
			break;
	}
}
