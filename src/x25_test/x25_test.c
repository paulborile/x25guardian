/*
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_test
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_test.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:30  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/09/18  14:00:33  px25
 * Modified for new rt_find call.
 *
 * Revision 1.0  1995/07/07  10:22:26  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_test.c,v 1.1.1.1 1998/11/18 15:03:30 paul Exp $";

#define  MAX_STR 32              /* useful for not including smp.h */

/*  System include files                        */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>

#include    <x25.h>
#include    <neterr.h>

/*  Project include files                       */

#include    "px25_globals.h"
#include "sm.h"

/*  Module include files                        */

/*  Extern functions used                       */
extern struct NUA * rt_find();
extern struct BEST_ROUTE * sm_get_best_route();

/*  Local functions used                       */

void    get_command_line();
void    x25_listener_usage();
void  myexit();
void  terminate();

/*  Extern data used                            */

extern int sm_errno;
extern int rt_errno;

/*  Local constants                             */

#define OPTS    "p:"
#define  HOSTNAME_LEN   32
#define  PROTOCOL "tcp"

#ifdef   DEBUG
#define  X25_INCALL "x25_incall.d"
#else
#define  X25_INCALL  "x25_incall"
#endif

/*  Local types                                 */

/*  Local macros                                */

#define X25_ADLEN   (X25_ADDRLEN+X25_ADDREXT+2)

/*  Local data                                  */

struct  x25data facility;
struct  x25data user_data;

int rc;
int xrc;
char remote_addr[X25_ADLEN];
char local_addr[X25_ADLEN];
char *inbuf;
char *outbuf;
int cid;
int lsn;
int info = 0;

int port = -1;
struct   NUA *NUA_INFO;
char buf[BUFSIZ];
struct   BEST_ROUTE *BEST_ROUTE_INFO;

/* For call to unix services	*/

struct  BEST_ROUTE unix_ser;

int best_done = 0;
int xerr;

char pname[BUFSIZ];                     /* Program name */

static char pid_str[20];

int init_done = 0;

main(argc, argv)
int argc;
char **argv;
{
    char msg[ENETMSGLEN];
    char message[ENETMSGLEN];
    char *finger;
    char *p;

    /*
     * Globalize name of the program
     */

    strcpy(pname, argv[0]);

    get_command_line(argc, argv);

    /*
     * set signals
     */

    signal(SIGTERM, terminate);
    signal(SIGINT, terminate);
    signal(SIGQUIT, terminate);

    /*
     * Initialize x25 toolkit
     */

    if ( x25init(0) < 0 )
    {
        x25errormsg(msg);
        printf("%s : x25init error %d, %s\n", pname, x25error(), msg);
        exit(1);
    }

    init_done = 1;

    x25version(msg);
    printf("%s : %s\n", pname, msg);

    /*
     * now LISTEN on the specified port
     */

    printf("%s : Port %d, Going to listen ....\n", pname, port);

again1:
    strcpy(local_addr, "");
    strcpy(remote_addr, "");
    info = XI_QBIT;

    memset(&user_data, '\0', sizeof(struct x25data));
    memset(&facility, '\0', sizeof(struct x25data));

    if ( x25xlisten(&cid, X25WAIT, port, info, &facility, &user_data,
                    remote_addr, local_addr, X25NULLFN) < 0)
    {
        xerr = x25error();
        if ( xerr == EX25INTR)
        {
            goto again1;
        }

        x25errormsg(msg);
        printf("x25error %d %s\n", xerr, msg);
        myexit(FAILURE);
    }

    printf("CALL RECEIVED FROM %s TO %s cid %d\n", remote_addr, local_addr, cid);
    printf("USERDATA len = %d\n", user_data.xd_len);
    printf("FACILITY len = %d\n", facility.xd_len);
    printf("FACILITY %x %x %x %x\n", facility.xd_data[0], facility.xd_data[1],
           facility.xd_data[2], facility.xd_data[3]);


    if (( inbuf = x25alloc(BUFSIZ)) == NULL)
    {
        x25errormsg(msg);
        printf("x25alloc(inbuf) error %d %s\n", x25error(), msg);
        myexit(FAILURE);
    }

    if ((outbuf=x25alloc(BUFSIZ)) == NULL)
    {
        xerr = x25error();
        x25errormsg(msg);
        printf("x25alloc(outbuf) error %d, %s\n", xerr, msg);
        myexit(FAILURE);
    }

    if ( user_data.xd_len > 0 )
    {
        p = user_data.xd_data;

        if ((p[0] == 1) && (p[1] == 0) && (p[2]  == 0) && (p[3] == 0))
        {
            /* EASYWAY */

            printf("main() - EASYWAY Call User Data %s\n", &p[4]);

            if ((NUA_INFO = rt_find(user_data.xd_data, user_data.xd_len))
                == (struct NUA *)NULL)
            {
                printf("main() - rt_find() error, rt_errno %d\n", rt_errno);

                if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
                {
                    x25errormsg(msg);
                    printf("main() - x25hangup() failed error %d %s\n",
                           x25error(), msg);
                }
                else
                {
                    printf("main() - x25hangup sent\n");
                }
                goto again1;
            }
        }
        else
        {
            /*	ARGOTEL	*/

            printf("main() - ARGOTEL call\n");

            if (( rc =
                      x25recv(cid, inbuf, X25_DATA_PACKET_SIZE, NULL, X25NULLFN)) < 0)
            {
                x25errormsg(msg);
                printf("main() - x25recv error %d %s\n", x25error(), msg);
                myexit(FAILURE);
            }

            printf("main() - ARGOTEL recv %d bytes User Data  %s\n",
                   rc, inbuf);

            /*
             * Take out newline from user data string
             */

            if (( finger = strchr(inbuf, 13)) != NULL)
            {
                *finger = '\0';
            }

            if ((NUA_INFO = rt_find(inbuf, strlen(inbuf))) == (struct NUA *)NULL)
            {
                printf("main() - rt_find() rt_errno %d\n", rt_errno);

                if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
                {
                    x25errormsg(msg);
                    printf("main() - x25hangup error %d %s\n", x25error(), msg);

                }
                else
                {
                    printf("main() - x25hangup sent\n");
                }
                goto again1;
            }
        }
    }
    else
    {
        /* empty user data : sending a CLEAR */

        printf("main() - empty user data: sending a clear..\n");

        if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
        {
            xerr = x25error();
            x25errormsg(msg);
            printf("main() - x25hangup error %d %s\n", xerr, msg);

        }
        else
        {
            printf("main() - x25hangup sent\n");
        }
        goto again1;

    }

    /* ACCEPT side */

    if ( x25accept(&cid, info, NULL, NULL,
                   remote_addr, local_addr, X25NULLFN) < 0)
    {
        x25errormsg(msg);
        printf("main() - x25accept() error %d %s\n", x25error(), msg);
        myexit(FAILURE);
    }

    printf("main() - x25accept() OK\n");
    printf("Routing : pri_nua %s sec_nua %s nua_type %d\n",

           NUA_INFO->primary_nua, NUA_INFO->secondary_nua, NUA_INFO->nua_type);


    while (( rc =
                 x25recv(cid, inbuf, X25_DATA_PACKET_SIZE, &info, X25NULLFN)) > 0 )
    {
        printf("info %02X\n", info);
        if ( strncmp(inbuf, "quit", 4) == 0 )
        {
            break;
        }

        memcpy(outbuf, inbuf, rc);

        if (( xrc = x25send(cid, outbuf, rc, info, X25NULLFN)) < 0 )
        {
            xerr = x25error();
            x25errormsg(msg);
            printf("x25send() error %d, %s\n", xerr, msg);
            myexit(FAILURE);
        }

        printf("Hit enter to receive another packet\n");
    }

    printf("x25recv returned %d error %d info %x\n", rc, x25error(), info );

    switch (x25error())
    {
    case ENETSRESET:

        printf("ENETSRESET: \n");
        if (info == 0 || info == XI_MBIT)
        {
            /* Call x25recv() to get the cause and diagnostic. */
            if (x25recv(cid, inbuf, X25_DATA_PACKET_SIZE, &info,
                        X25NULLFN) > 0)
            {
                x25pcause(msg);
                x25pdiag(message);
                printf("%s - x25recv %s, %s\n", pname, msg, message);
            }
            else{
                x25errormsg(msg);
                printf("%s - x25recv error %d, %s\n",
                       pname, x25error(), msg);
            }
        }
        else if (info == XC_RRESETIND)
        {
            /* Get the cause and diagnostic */
            x25pcause(msg);
            x25pdiag(message);
            printf("%s - x25recv %s, %s\n", pname, msg, message);
        }
        break;

    case ENETCALLCLR:

        printf("ENETCALLCLR: info %x\n", info);

        x25pcause("cause : ");
        x25pdiag("diag : ");

        if (( xrc = x25hangup(cid, NULL, XH_IMM, X25NULLFN)) < 0 )
        {
            x25errormsg(msg);
            printf("x25hangup() xrc %d, %x, %s\n",
                   xrc, x25error(), msg);
        }
        break;

    case ENETIMSG:         /* incomplete message */
        printf("ENETIMSG: \n");
        /* need to incrementing size of inbuf */
        if (x25free(inbuf) < 0)
        {
            x25errormsg(msg);
            printf("%s - x25free error %d, %s\n",
                   pname, x25error(), msg);
        }
        if (( inbuf = x25alloc(2 * X25_DATA_PACKET_SIZE)) == NULL)
        {
            x25errormsg(msg);
            printf("%s - x25alloc() error %d, %s\n",
                   pname, x25error(), msg);
            myexit(1);
        }
        goto again1;
    default:
        printf("DEFAULT: \n");
        printf("%s - x25recv() error %d, %s\n",
               pname, x25error(), msg);
        break;
    }

    printf("Hit enter to hangup call and restart over\n");

    if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
    {
        x25errormsg(msg);
        printf("main() - x25hangup() failed error %d %s\n",
               x25error(), msg);
    }
    else
    {
        printf("main() - x25hangup sent\n");
    }
    goto again1;
}

/*
 *
 *  Procedure: get_command_line
 *
 *  Parameters: argc, argv
 *
 *  Description: parse command line
 *
 *  Return: none
 *
 */

void    get_command_line(argc, argv)
int argc;
char **argv;
{
    int fatal = 0;
    int c;

    extern char *optarg;

    while ((c = getopt(argc, argv, OPTS)) !=EOF)
    {
        switch (c)
        {
        case    'p':
            port = atoi(optarg);
            break;

        default:
            printf("%s : invalid flag.\n", argv[0]);
            x25_listener_usage(argc, argv);
            exit(FAILURE);
        }
    }

    if ( port == -1 )
    {
        fatal++;
    }

    if ( fatal )
    {
        x25_listener_usage(argc, argv);
        exit(FAILURE);
    }
}


/*
 *
 *  Procedure: x25_listener_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void    x25_listener_usage(argc, argv)
int argc;
char **argv;
{
    printf("Usage: %s -p<port>\n", argv[0]);
}


/*
 *
 *  Procedure: myexit
 *
 *  Parameters: none
 *
 *  Description: close file descriptors,
 *						delete route associated with X25_listener,
 *						send SIGTERM to childs and exit
 *
 *  Return:
 *
 */

void  myexit(code)
int code;
{
    int ret;

    printf("myexit() - starting\n");
    x25exit();
    exit(code);
}


/*
 *
 *  Procedure: terminate
 *
 *  Parameters: none
 *
 *  Description: call myexit()
 *
 *  Return:
 *
 */

void  terminate(sig)
int sig;
{
    printf("terminate() - terminating on sig  %d\n", sig);
    myexit(SUCCESS);
}
