/*
 * $Id: x25_shmgr.c,v 1.1.1.1 1998/11/18 15:03:36 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: Shell server
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: x25_shmgr.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:36  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/29  17:24:45  px25
 * Up to 3 Telnet connections
 *
 * Revision 1.1  1995/09/18  13:15:56  px25
 * Added Debug execution of shell_x28
 *
 * Revision 1.0  1995/07/14  09:18:26  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_shmgr.c,v 1.1.1.1 1998/11/18 15:03:36 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <limits.h>
#include        <sys/types.h>
#include        <errno.h>
#include        <signal.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <netdb.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "errlog.h"
#include        "debug.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

extern int sm_errno;

/*  Local constants                             */

#define OPTS        "d:"
#define SERVICE "px25shell"
#define PROTOCOL    "tcp"
#define BACKLOG 5
#define HOSTNAME_LEN    32
#define MAX_TELNET      3
#define  TTY_X28  "shell_x28"
#define  TTY_X28D  "shell_x28.d"

/*  Local functions used                       */

void    get_command_line();
void    x25sh_usage();
void    child_termination();
void    myexit();
void    terminate();

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static struct   sockaddr_in local_sin, remote_sin;
static struct   servent *se;
static struct   protoent *pe;
static int debuglevel;
static int s = -1;                      /* the socket we will listen on */
static int new = -1;                    /* the socket returned by accept	*/
static char myhostname[HOSTNAME_LEN];
char pname[BUFSIZ];                             /* Program name						*/


void    main(argc, argv)
int argc;
char **argv;
{
    int rc;
    pid_t pid;
    int len      = sizeof(remote_sin);
    char tty[PATH_MAX];
    char cmd[32];
    char ttycmd[PATH_MAX];
    char debugcmd[32];
    char *finger;
    int max_tty;

#ifdef  DEBUG
    char proc_pid[6];
    char debugfile[BUFSIZ];
#endif

    /*
     * Globalize name of the program
     */

    strcpy(pname, argv[0]);

    get_command_line(argc, argv);

#ifdef  DEBUG
    (void) sprintf(proc_pid, "%d", getpid());
    (void) sprintf(debugfile, "/tmp/x25_shmgr%05d.debug", getpid());
    (void) initdebug(debuglevel, debugfile, argv[0], proc_pid);
#endif

    /*
     * set signals
     */

    signal(SIGCHLD, child_termination);

    /*
     * get my hostname
     */

    if ( gethostname(myhostname, HOSTNAME_LEN) < 0 )
    {
        debug((1, "main() unable to gethostname() errno %d\n", errno));
        errlog(INT_LOG,
               "%s : unable to gethostname - errno %d\n", argv[0], errno);
        exit(FAILURE);
    }

    debug((1, "main() - my hostname %s\n", myhostname));

    rc = sm_create_route(myhostname,
                         TTY_NUA_STRING, SERVICE, 0, MAX_TELNET, TTY_NUA);

    if ( rc < 0 )
    {
        debug((1, "main() - sm_create_route() returned %d\n", rc));
        errlog(INT_LOG,
               "%s : unable to sm_create_route, sm_errno = %d\n", argv[0], sm_errno);
    }

    debug((1, "sm_create_route rc = %d\n", rc));

    /*
     * now initialize the network part
     */

    if (( se = getservbyname(SERVICE, PROTOCOL)) == (struct servent *) NULL )
    {
        debug((1, "main() - unknown service %s for protocol %s\n",
               argv[0], SERVICE, PROTOCOL));
        errlog(INT_LOG,
               "%s : Unknown service %s for protocol %s /etc/services\n",
               argv[0], SERVICE, PROTOCOL);
        myexit(FAILURE);
    }

    /*
     * Get protocol number
     */

    if (( pe = getprotobyname(PROTOCOL)) == (struct protoent *) NULL )
    {
        debug((1, "main() - unknown protocol %s\n", argv[0], PROTOCOL));
        errlog(INT_LOG,
               "%s : Unknown protocol %s /etc/protocols\n", argv[0], PROTOCOL);
        myexit(FAILURE);
    }

    /*
     * Create Socket INET type STREAM for tcp protocol
     */

    if (( s = socket(AF_INET, SOCK_STREAM, pe->p_proto)) < 0 )
    {
        debug((1, "main() - Unable to create socket errno = %d\n", errno));
        errlog(INT_LOG,
               "%s : Unable to create socket, errno %d\n", argv[0], errno);
        myexit(FAILURE);
    }

    /*
     * fill up local socket internet with port we are going to listen on
     */

    local_sin.sin_family            = AF_INET;
    local_sin.sin_port          = se->s_port;
    local_sin.sin_addr.s_addr   = htonl(INADDR_ANY);

    /*
     * Bind the socket to local sock struct with port inside
     */

    if ( bind(s, &local_sin, sizeof(struct sockaddr_in)) < 0 )
    {
        debug((1, "main() - Unable to bind socket errno = %d\n", errno));
        errlog(INT_LOG,
               "%s : Unable to bind socket, errno %d\n", argv[0], errno);
        myexit(FAILURE);
    }

    /*
     * Tell the kernel you are listening for clients
     */

    if ( listen(s, BACKLOG) < 0 )
    {
        debug((1, "main() - listen on socket failed, errno = %d\n", errno));
        errlog(INT_LOG,
               "%s : Listen on socket failed, errno %d\n", argv[0], errno);
        myexit(FAILURE);
    }

    signal(SIGTERM, terminate);
    signal(SIGINT, terminate);
    signal(SIGQUIT, terminate);

    /*
     * accept connections ; maybe daemonize first
     */

again:

    if (( new = accept(s, &remote_sin, &len)) < 0 )
    {
        if ( errno == EINTR)
        {
            goto again;
        }
        debug((1, "main() - error in accept errno = %d\n", errno));
        errlog(INT_LOG,
               "%s : Accept on socket failed, errno %d\n", argv[0], errno);
        goto again;
    }

    pid = fork();

    switch ( pid )
    {
    case  -1:

        debug((1, "main() - Unable to fork! errno = %d\n", errno));
        errlog(INT_LOG, "%s : Fork Failed, errno %d\n", argv[0], errno);
        close(s);
        close(new);
        myexit(FAILURE);

        break;

    case  0:      /* child */

        /* exec  new process to manage telnet connection */

        /* close listening socket	*/
        close(s);

        sprintf(cmd, "-s%d", new);

#ifdef  DEBUG
        sprintf(debugcmd, "-d%d", debuglevel);
#endif

        debug((1, "main() - going to execute %s %s %s\n",
               TTY_X28, cmd, debugcmd ));

        /* use execvp instead   */

#ifdef  DEBUG
        execlp(TTY_X28D, TTY_X28D, cmd, debugcmd, 0);
#else
        execlp(TTY_X28, TTY_X28, cmd, 0);
#endif

        /* if reached means that execlp failed */

        debug((1, "main() CHILD: execlp failed\n"));
        errlog(INT_LOG, "%s : Unable to execlp %s %s %s, errno %d\n", argv[0],
               TTY_X28, cmd, ttycmd, errno);

        close(new);
        sleep(3);
        exit(FAILURE);

        break;

    default:
        /*
         * The father
         * close new file descriptor
         */

        close(new);
        goto  again;

        break;
    }
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
        case 'd':
            debuglevel  =   atoi(optarg);
            break;

        default:
            errlog(INT_LOG, "%s : invalid flag %c.\n", argv[0], c);
            x25sh_usage(argc, argv);
            exit(FAILURE);
        }
    }

#ifdef DEBUG
    if ( debuglevel == -1 )
    {
        fatal++;
    }
#endif

    if ( fatal )
    {
        x25sh_usage(argc, argv);
        exit(FAILURE);
    }
}


/*
 *
 *  Procedure: x25sh_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void    x25sh_usage(argc, argv)
int argc;
char **argv;
{
#ifdef  DEBUG
    errlog(INT_LOG, "Usage: %s -d<debuglevel>\n", argv[0]);
#else
    errlog(INT_LOG, "Usage: %s\n", argv[0]);
#endif
}


/*
 *
 *  Procedure: child_termination
 *
 *  Parameters: standard signal
 *
 *  Description: checkout ttab for pid terminated
 *
 *  Return:  none
 *
 */

void    child_termination()
{
    int rc = 0;
    long status = 0;
    pid_t pid;

    debug((3, "child_termination() - Starting.\n"));

    signal(SIGCHLD, SIG_DFL);

    pid = wait(&status);

    signal(SIGCHLD, (void (*)()) child_termination);
    debug((3, "child_termination() - terminated %d with %x\n", pid, status));
}

/*
 *
 *  Procedure: myexit
 *
 *  Parameters: none
 *
 *  Description: close file descriptors and exit
 *
 *  Return:
 *
 */

void    myexit(code)
int code;
{
    int ret;

    debug((3, "myexit() - code %d\n", code));


    if (( ret = sm_delete_route(myhostname, TTY_NUA_STRING)) < 0)
    {
        errlog(INT_LOG, "%s : sm_delete_route failed, sm_errno %d\n",
               pname, sm_errno);
        debug((1,
               "myexit() - sm_delete_route() failed with sm_errno %d\n", sm_errno));
    }
    else{
        debug((3, "myexit() - sm_delete_route() returned %d\n", ret));
    }

    if ( s != -1 )
    {
        close(s);
    }
    if ( new != -1 )
    {
        close(new);
    }

    exit(code);
}

/*
 *
 *  Procedure: terminate
 *
 *  Parameters: signal received
 *
 *  Description:
 *
 *  Return:
 *
 */

void    terminate(signal)
int signal;
{
    debug((3, "terminate() - sisgnal %d\n", signal));
    myexit(SUCCESS);
}
