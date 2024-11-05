#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include    <signal.h>

#include <netinet/in.h>
#include <netdb.h>

#include    "px25_globals.h"

/*
 * c l i e n t . c  :  Example of net client using tcp protocoll
 *
 *
 * P.S.Borile	:	Wed Apr 18 15:25:50 PDT 1990
 *
 */
#define SERVICE     "telnet"
#define PROTOCOL    "tcp"

struct  sockaddr_in dest_sin;
struct  hostent *he;
struct  servent *se;
struct  protoent *pe;

int s;
#define MAXMSG  (128)

char buffer[BUFSIZ];
int del();

main(argc, argv)
int argc;
char **argv;
{
    struct  PKT_HDR *ph = (struct PKT_HDR *) buffer;
    char *service = NULL;
    int sz = 0;
    int rc = 0;
    int snd = 0;
    int i;
    char c;

    if (argc != 3 )
    {
        fprintf(stderr, "Usage : %s <hostname> <service>\n", argv[0]);
        exit();
    }

    service = argv[2];

    /*
     * Select service and protocol
     */

    if (( se = getservbyname(service, PROTOCOL)) == NULL )
    {
        fprintf(stderr, "%s : unknown service %s for protocol %s\n",
                argv[0], service, PROTOCOL);
        exit();
    }

    dest_sin.sin_port   =   se->s_port;

    /*
     * Get Host information
     */

    if (( he = gethostbyname(argv[1])) == NULL )
    {
        fprintf(stderr, "%s : host %s unknown\n", argv[0], argv[1]);
        exit();
    }

    dest_sin.sin_family = he->h_addrtype;
    memcpy((char *)&dest_sin.sin_addr, he->h_addr, he->h_length);

    /*
     * Get protocol number
     */

    if (( pe = getprotobyname(PROTOCOL)) == NULL )
    {
        fprintf(stderr, "%s : unknown protocol %s\n",
                argv[0], PROTOCOL);
        exit();
    }

    /*
     * Create Socket with all information collected
     */

    if (( s = socket(he->h_addrtype, SOCK_STREAM, pe->p_proto)) < 0 )
    {
        perror("socket");
        exit();
    }

    /*
     * connect to remote
     */

    if ( connect(s, (char *)&dest_sin, sizeof(dest_sin)) < 0)
    {
        perror("connect");
        exit();
    }

    signal(SIGINT, del);

    printf("Connect OK, press enter to continue\n");
    getchar();

    switch (fork())
    {
    case    -1:

        close(s);
        exit();

    case    0:


        while (( rc = read(0, &c, 1)) == 1 )
            write(s, &c, 1);

        close(s);
        exit();

    default:

        while (( rc = read(s, &c, 1)) == 1 )
            write(1, &c, 1);

        close(s);
        exit();
    }

    printf("End of while %d\n", rc);
    close(s);
}

del(sig)
{
    printf("Closing ( received SIGINT ) \n");
    shutdown(s, 2);
    close(s);
    exit();
}
