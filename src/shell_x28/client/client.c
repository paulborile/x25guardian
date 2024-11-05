#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include	<sys/ioctl.h>
#include	<sys/termio.h>
#include	<signal.h>
#include <netinet/in.h>
#include <netdb.h>

#include	"px25_globals.h"

/*
 * c l i e n t . c 	:  Example of net client using tcp protocoll
 *
 *
 * P.S.Borile	:	Wed Apr 18 15:25:50 PDT 1990
 *
 */

#define	SERVICE		"px25shell"
#define	PROTOCOL	"tcp"

struct	sockaddr_in	dest_sin;
struct	hostent		*he;
struct	servent		*se;
struct	protoent	*pe;

int	PID=-1;
int	count=0;
char	c;
int	s;
#define	MAXMSG	(128)

char 	buffer[BUFSIZ];
char	buftemp[BUFSIZ];
int	del();
void  child_termination();
int	blocksize = 128;

main(argc, argv)
int	argc;
char	**argv;
{
	struct	PKT_HDR	*ph = (struct PKT_HDR *) buffer;
	char		*service = NULL;
	int		sz = 0;
	int		rc = 0;
	int		snd = 0;
	int		i;
	struct	termio	t, save;

	if (argc != 3 ) {
		fprintf(stderr, "Usage : %s <hostname> <service>\n", argv[0]);
		exit();
	}

	service = argv[2];

	/*
 	 * Select service and protocol 
	 */

	if (( se = getservbyname(service, PROTOCOL)) == NULL ) {
		fprintf(stderr,"%s : unknown service %s for protocol %s\n",
			argv[0], SERVICE, PROTOCOL);
		exit();
	}

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

	signal(SIGINT, del);
	signal(SIGCHLD, child_termination);

	PID = fork();

	switch (PID)
   {
      case  -1 :

         printf("%s : cannot fork\n", argv[0]);
         close(s);
			exit(0);
			break;

      case  0  :

         /* This is child process: mos_receiver */
			
			while (( rc = mos_recv(s, buffer, BUFSIZ)) > 0 )
			{
				switch (ph->pkt_code)
				{
					case	ASY_DATA_PACKET	:

						write(1, buffer + sizeof(struct PKT_HDR),
												rc - sizeof(struct PKT_HDR));
/*						printf("%s", buffer + sizeof(struct PKT_HDR)); */

						break;

					case	ASY_CONNECTION_END	:

						printf("ASY_CONNECTION_END received\n");
						close(s);
						exit();
						break;
						
					default	:

						printf("Unknown packet header %d\n", ph->pkt_code);
						break;
				}
			}

			del(1);
			break;

		default:

			if ( ioctl(0, TCGETA, &t) < 0 )
			{
				perror("ioctl tcgeta");
			}

			save = t;

			t.c_cc[VTIME] = 1;
			t.c_lflag &= ~ICANON;
			t.c_lflag &= ~ECHO;

			if ( ioctl(0, TCSETA, &t) < 0 )
			{
				perror("ioctl tcseta");
			}

			while (( rc = x28_read(0, buffer)) > 0 )
			{
				ph->pkt_code = X25_DATA_PACKET;
				mos_send(s, buffer, sizeof(struct PKT_HDR)+rc);
			}

			if ( ioctl(0, TCSETA, &save) < 0 )
			{
				perror("ioctl tcseta save");
			}

			del(1);
			break;
	}
}

del(sig)
{
	shutdown(s, 2);
	close(s);
	exit();
}

void  child_termination()
{
   long     status = 0;
	short	pid;

   signal(SIGCHLD, SIG_DFL);

   pid = wait(&status);

   signal(SIGCHLD, (void (*)())child_termination);
}
