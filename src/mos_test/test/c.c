#include <stdio.h>
#include <errno.h>
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
#define PROTOCOL    "tcp"

struct  sockaddr_in dest_sin;
struct  hostent *he;
struct  servent *se;
struct  protoent *pe;

int s;

#define BIG_BUFFER  (64*BUFSIZ)

char buffer[BIG_BUFFER];
void    del();
void    pipe();
void    dead();
static struct   sigaction act;
extern int errno;

main(argc, argv)
int argc;
char **argv;
{
    char *service = NULL;
    int *plen = (int *) buffer;
    int count = 0;
    int snd;

    srand(1000);

    if (argc != 4 )
    {
        fprintf(stderr, "Usage : %s <hostname> <service> <count>\n", argv[0]);
        exit();
    }

    service = argv[2];
    count = atoi(argv[3]);

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

    if ( connect(s, (struct sockaddr *) &dest_sin, sizeof(dest_sin)) < 0)
    {
        perror("connect");
        exit();
    }

#ifdef  SIGACTION
    act.sa_handler    = del;
    act.sa_flags      = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
#else
    signal(SIGINT, del);
#endif

#ifdef  SIGACTION
    act.sa_handler    = pipe;
    act.sa_flags      = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGPIPE, &act, NULL);
#else
    signal(SIGPIPE, pipe);
#endif

#ifdef  SIGACTION
    act.sa_handler    = dead;
    act.sa_flags      = SA_RESTART;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);
#else
    signal(SIGCHLD, dead);
#endif

    if ( fork() == 0 )
    {
        sleep(10);
        exit();
    }

    while ( count-- )
    {
        *plen = ((rand() % (BIG_BUFFER - sizeof(int) - 1)) + sizeof(int));
again:
        if (( snd = mos_send(s, buffer, *plen)) == -1 )
        {
            if ( errno == EINTR )
            {
                printf("mos_send ret -1 errno EINTR\n");
                goto again;
            }
            printf("mos_send ret -1 errno %d\n", errno);
            shutdown(s, 2);
            close(s);
            exit(0);
        }
    }
    printf("Finished ....\n");
    shutdown(s, 2);
    close(s);
    exit(0);
}

void    del(sig)
{
#ifndef SIGACTION
    signal(SIGINT, del);
#endif
    printf("received SIGINT\n");
}

void    pipe(sig)
{
    printf("Closing ( received SIGPIPE ) \n");
    shutdown(s, 2);
    close(s);
    exit();
}

void    dead(sig)
{
    long l;

    printf("Waiting ( received SIGCHLD ) \n");
    wait(&l);
}
