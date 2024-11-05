/*
 * $Id: asyn_main.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: asyn_main.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:14  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.3  1995/09/29  17:12:32  px25
 * Added check on termination of asyn_x28 for EXIT_DELETE_ROUTE
 * in order to sm_delete_route channel not working anymore
 *
 * Revision 1.2  1995/09/18  13:30:17  px25
 * Modified according to new adt pid-table
 *
 * Revision 1.1  1995/07/14  09:05:24  px25
 * Better diagnostic on failed sm_create_route
 *
 * Revision 1.0  1995/07/07  10:04:46  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: asyn_main.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $";

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
#include        "group.h"

/*  Module include files                        */

/*  Extern functions used                       */

extern int tt_init();
extern char *tt_get_device();
extern int tt_set_pid();
extern int tt_free();
extern void    tt_kill();
extern void    tt_create_route();
extern void    tt_delete_route();

/*  Extern data used                            */

/*  Local constants                             */

#define OPTS        "d:"
#define SERVICE "px25asynmgr"
#define PROTOCOL    "tcp"
#define BACKLOG 5
#define HOSTNAME_LEN    32

#ifdef  DEBUG
#define ASY_X28 "asy_x28.d"
#else
#define ASY_X28 "asy_x28"
#endif

/*  Local functions used                       */

void    get_command_line();
void    asy_usage();
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
    char buf[BUFSIZ];
    struct  PKT_HDR *ph = (struct PKT_HDR *) (buf);
    struct  GROUP *grp = (struct GROUP *) (buf + sizeof(struct PKT_HDR));

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
    (void) sprintf(debugfile, "/tmp/asymgr%05d.debug", getpid());
    (void) initdebug(debuglevel, debugfile, argv[0], proc_pid);
#endif

    /*
     * set signals
     */

    signal(SIGCHLD, child_termination);

    /*
     * Load the asyn tty table
     */

    if (( finger = getenv(PX25_ASY_TABLE_PATH)) == NULL )
    {
        errlog(INT_LOG,
               "%s : %s env var not set\n", argv[0], PX25_ASY_TABLE_PATH);
        exit(FAILURE);
    }

    debug((1, "main() - %s = %s\n", PX25_ASY_TABLE_PATH, finger));

    if (( max_tty = tt_init(finger)) == -1 )
    {
        errlog(INT_LOG,
               "%s : unable to load %s\n", argv[0], finger);
        exit(FAILURE);
    }

    debug((1, "main() - tt_init found %d ttys\n", max_tty));

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

    tt_create_route(myhostname, SERVICE, argv[0]);

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

    /* connection accepted: get a tty port	*/
    /* Receive ASY_CALL_REQ						*/

    if (( rc = mos_recv(new, buf, BUFSIZ )) <= 0 )
    {
        debug((1, "main() - mos_recv() ret %d\n", rc));
        errlog(INT_LOG, "%s : ERROR RECEIVING GROUP CALL\n", argv[0]);
        close(new);
        goto again;
    }

    /* packet is good	*/

    if ( ph->pkt_code != ASY_CALL_REQ )
    {
        debug((1, "main() - mos_recv() ret %d\n", rc));
        errlog(INT_LOG, "%s : PKT RECEIVED (%d) NOT ASY_CALL_REQ\n",
               argv[0], ph->pkt_code);
        close(new);
        goto again;
    }

    debug((3, "main() - received grp_name %s, grp_num %d\n",
           grp->grp_name, grp->grp_num));

    if (( finger = tt_get_device(grp->grp_name, grp->grp_num)) == NULL )
    {
        /* call cannot be accepted - no more free ttys	*/

        debug((1, "main() - tt_get_device() ret NULL, no desired dev\n"));
        errlog(INT_LOG, "%s : CANNOT ACCEPT CALL, %s,%d NOT FREE\n",
               argv[0], grp->grp_name, grp->grp_num);
        close(new);
        goto again;
    }

    strcpy(tty, finger);

    debug((1, "main() - going to fork and use port %s\n", tty));

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

        /* exec new process to manage call  */

        /* close listening socket	*/
        close(s);

        sprintf(cmd, "-s%d", new);
        sprintf(ttycmd, "-t%s", tty);
        sprintf(debugcmd, "-d%d", debuglevel);

        debug((1, "main() - going to execute %s %s %s %s\n",
               ASY_X28, cmd, ttycmd, debugcmd));

        /* use execvp instead	*/

#ifdef  DEBUG
        execlp(ASY_X28, ASY_X28, cmd, ttycmd, debugcmd, 0);
#else
        execlp(ASY_X28, ASY_X28, cmd, ttycmd, 0);
#endif


        /* if reached means that execlp failed	*/

        debug((1, "main() CHILD: execlp failed\n"));
        errlog(INT_LOG,
               "%s : Unable to execlp %s %s %s, errno %d\n", argv[0],
               ASY_X28, cmd, ttycmd, errno);
        close(new);
        sleep(3);
        exit(FAILURE);

        break;
    default:

        /*
         * The father
         * close new file descriptor
         * set ttab entry with new pid
         */

        if ((rc = tt_set_pid(tty, pid)) < 0 )
        {
            debug((1, "main() - ttset_pid() ret %d\n", rc));
            errlog(INT_LOG, "%s : Cannot ttset_pid(%s,%d) rc %d\n",
                   argv[0], tty, pid, errno);
        }
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
            printf("%s : invalid flag.\n", argv[0]);
            asy_usage(argc, argv);
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
        asy_usage(argc, argv);
        exit(FAILURE);
    }
}


/*
 *
 *  Procedure: asy_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void    asy_usage(argc, argv)
int argc;
char **argv;
{
#ifdef  DEBUG
    printf("Usage: %s -d<debuglevel>\n", argv[0]);
#else
    printf("Usage: %s\n", argv[0]);
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
    char link[BUFSIZ];
    pid_t pid;

#define     STATUSLOW   0x000000ff
#define     STATUSHIGH  0x0000ff00

    debug((3, "child_termination() - Starting.\n"));

    signal(SIGCHLD, SIG_DFL);

    pid = wait(&status);

    if (( rc = tt_free(pid, link)) < 0 )
    {
        /* pid not found in ttab	*/

        debug((1, "child_termination() - tt_free ret %d\n", rc));
        errlog(INT_LOG,
               "%s : tt_free did not find pid %d in table\n", pname, pid);
    }

    if (( status != 0 ) && ((status & STATUSLOW) == 0))
    {
        debug((1, "child_termination() - Process %d Exits with code %d\n",
               pid, (status >> 8) & 0x000000ff ));

        if (((status >> 8) & 0x000000ff) == EXIT_AND_DELETE_ROUTE )
        {
            errlog(INT_LOG, "%s : DELETING %s\n", pname, link);
            sm_delete_route(myhostname, link);
        }
    }

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

    tt_delete_route(myhostname, pname);

    if ( s != -1 )
    {
        close(s);
    }
    if ( new != -1 )
    {
        close(new);
    }

    tt_kill(SIGTERM);

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
