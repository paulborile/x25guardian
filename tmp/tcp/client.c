#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>

/*
 * c l i e n t . c 	:  Example of net client using tcp protocoll
 *
 *
 * P.S.Borile	:	Wed Apr 18 15:25:50 PDT 1990
 *
 */
#define	SERVICE		"px25test"
#define	PROTOCOL	"tcp"

struct	sockaddr_in	dest_sin;
struct	hostent		*he;
struct	servent		*se;
struct	protoent	*pe;

int	s;
char	str[80];	/* character string used to send info	*/


main(argc, argv)
int	argc;
char	**argv;
{
	if (argc != 2 ) {
		fprintf(stderr, "Usage : %s <hostname>\n", argv[0]);
		exit();
	}

	/*
 	 * Select service and protocol 
	 */

	if (( se = getservbyname(SERVICE, PROTOCOL)) == NULL ) {
		fprintf(stderr,"%s : unknown service %s for protocol %s\n",
			argv[0], SERVICE, PROTOCOL);
		exit();
	}

	printf("se->s_port = %d ntohs(se->s_port) %d\n",
								se->s_port, ntohs(se->s_port));
	dest_sin.sin_port	=	se->s_port;

	/*
	 * Get Host information
	 */

	if (( he = gethostbyname(argv[1])) == NULL ) {
		fprintf(stderr, "%s : host %s unknown\n", argv[0], argv[1]);
		exit();
	}

	dest_sin.sin_family = he->h_addrtype;
	memcpy((char *)&dest_sin.sin_addr, he->h_addr, he->h_length);

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

	if (( s = socket(he->h_addrtype, SOCK_STREAM, pe->p_proto)) < 0 ) {
		perror("socket");
		exit();
	}

	/*
 	 * connect to remote 
	 */

	if ( connect(s, (char *)&dest_sin, sizeof(dest_sin)) < 0) {
		perror("connect");
		exit();
	}

	while (gets(str)) {
		write(s, str, sizeof(str));
	}
	shutdown(s, 2);
	close(s);
}
