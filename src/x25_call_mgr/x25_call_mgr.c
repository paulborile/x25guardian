/*
 * $Id: x25_call_mgr.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: X25_call_mgr.c
 *
 * Contents: receiving call to x25 (INDEPENDENT from x25 library)
 *
 * Author(s): C.U. S.r.l - P.Borile, G.Priveato
 *
 * $Log: x25_call_mgr.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:25  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/29  17:19:54  px25
 * Active field in sync tab not to be used in sm_create_route.
 *
 * Revision 1.1  1995/09/18  13:47:21  px25
 * errlog on taermination taken out
 *
 * Revision 1.0  1995/07/07  10:16:07  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_call_mgr.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include    <string.h>
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

#define OPTS        "d:s:"
#define SERVICE "px25callmgr"
#define PROTOCOL    "tcp"
#define BACKLOG 5
#define HOSTNAME_LEN    32
#define LEN 20
#define  MAXSTRING 128

#ifdef  DEBUG
#define X25_CALLER  "x25_caller.d"
#else
#define X25_CALLER  "x25_caller"
#endif

/*  Local functions used                       */

void    get_command_line();
void    x25_call_mgr_usage();
void    child_termination();
void    myexit();
void    terminate();

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static struct   sockaddr_in local_sin, remote_sin;
static struct   servent *se;
static struct   protoent *pe;
static int debuglevel = -1;
static int s = -1;                      /* the socket we will listen on */
static int new = -1;                    /* the socket returned by accept	*/
static char myhostname[HOSTNAME_LEN];
char buf[BUFSIZ];
FILE *fp;
int active, max;
char portlist[LEN][MAXSTRING];          /* physic lines readed by sync.table */
char nua[MAXSTRING];                    /* nua address of physic line */
static int i = 0;                  /* index of portlist */
char pname[BUFSIZ];                 /* Program name          */

void    main(argc, argv)
int argc;
char **argv;
{
    int rc;
    pid_t pid;
    int len      = sizeof(remote_sin);
    char cmd[16];
    char cmddebug[16];
    char *finger;
    char *tok;
    char *newtok;

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
    (void) sprintf(debugfile, "/tmp/x25_call_mgr%05d.debug", getpid());
    (void) initdebug(debuglevel, debugfile, argv[0], proc_pid);
#endif

    /*
     * set signals
     */

    signal(SIGCHLD, child_termination);
    signal(SIGTERM, terminate);
    signal(SIGINT, terminate);
    signal(SIGQUIT, terminate);

    /*
     * set pid table
     */

    set_pid();

    /*
     * get my hostname
     */

    if ( gethostname(myhostname, HOSTNAME_LEN) < 0 )
    {
        debug((1, "main() - unable to gethostname() errno %d\n", errno));
        errlog(INT_LOG,
               "%s : unable to gethostname - errno %d\n", pname, errno);
#ifdef   DEBUG
        enddebug();
#endif
        exit(1);
    }

    debug((3, "main() - my hostname %s\n", myhostname));

    /*
     * Load the sync table
     */

    if (( finger = getenv(PX25_SYN_TABLE_PATH)) == NULL )
    {
        debug((1, "main() - %s env var not set\n", PX25_SYN_TABLE_PATH));
        errlog(INT_LOG,
               "%s : %s env var not set\n", pname, PX25_SYN_TABLE_PATH);

#ifdef   DEBUG
        enddebug();
#endif
        exit(1);
    }

    debug((3, "main() - %s = %s\n", PX25_SYN_TABLE_PATH, finger));

    debug((3, "main() - starting to load %s\n", finger));

    if (( fp = fopen(finger, "r")) == NULL )
    {
        debug((1, "main() - fopen error, errno = %d\n", errno));
        errlog(INT_LOG,
               "%s : unable to load %s\n", pname, finger);

#ifdef   DEBUG
        enddebug();
#endif
        exit(1);
    }

    while ( fgets(buf, BUFSIZ, fp) != NULL )
    {
        debug((5, "main() - read %s", buf));

        if (( buf[0] == '#' ) || ( buf[0] == '\n' ))
        {
            continue;
        }
        else
        {
            debug((3, "main() - found line!\n"));
            if (isdigit(buf[0]) != 0)
            {
                debug((3, "main() - buf[0] is digit\n"));
                if ((tok = strtok(buf, ":")) != NULL )
                {
                    strcpy(portlist[i], tok);
                    if ((newtok = strtok(NULL, ":")) != NULL)
                    {
                        active = atoi(newtok);
                    }
                    else {
                        errlog(INT_LOG, "%s : incorrect sync table\n");
#ifdef   DEBUG
                        enddebug();
#endif
                        exit(1);
                    }
                    if ((newtok = strtok(NULL, ":")) != NULL)
                    {
                        max = atoi(newtok);
                    }
                    else {
                        errlog(INT_LOG, "%s : incorrect sync table\n");
#ifdef   DEBUG
                        enddebug();
#endif
                        exit(1);
                    }
                    if ((newtok = strtok(NULL, "\n")) != NULL)
                    {
                        strcpy(nua, newtok);
                    }
                    else {
                        errlog(INT_LOG,
                               "%s : incorrect sync table\n");
#ifdef   DEBUG
                        enddebug();
#endif
                        exit(1);
                    }
                }
                rc = sm_create_route(myhostname,
                                     portlist[i], SERVICE, 0, max, X25_NUA);

                if ( rc < 0 )
                {
                    debug((1, "main() - unable to sm_create_route(), rc =  %d\n",
                           rc));
                    errlog(INT_LOG,
                           "%s : unable to sm_create_route, rc = %d\n", pname, errno);
#ifdef   DEBUG
                    enddebug();
#endif
                    exit(1);
                }

                debug((3, "sm_create_route rc = %d\n", rc));

                debug((3, "inserted entry %s %d %d with nua %s\n",
                       portlist[i], 0, max, nua));
                ++i;
            }
        }
    }

    debug((3, "main() - Finished processing of %s\n", finger));
    fclose(fp);

    /*
     * now initialize the network part
     */

    if (( se = getservbyname(SERVICE, PROTOCOL)) == (struct servent *) NULL )
    {
        debug((1, "main() - unknown service %s for protocol %s\n",
               SERVICE, PROTOCOL));
        errlog(INT_LOG,
               "%s : Unknown service %s for protocol %s /etc/services\n",
               pname, SERVICE, PROTOCOL);
        myexit(FAILURE);
    }

    /*
     * Get protocol number
     */

    if (( pe = getprotobyname(PROTOCOL)) == (struct protoent *) NULL )
    {
        debug((1, "main() - unknown protocol %s\n", PROTOCOL));
        errlog(INT_LOG,
               "%s : Unknown protocol %s in /etc/protocols\n", pname, PROTOCOL);
        myexit(FAILURE);
    }

    /*
     * Create Socket INET type STREAM for tcp protocol
     */

    if (( s = socket(AF_INET, SOCK_STREAM, pe->p_proto)) < 0 )
    {
        debug((1, "main() - Unable to create socket errno = %d\n", errno));
        errlog(INT_LOG,
               "%s : Unable to create socket, errno %d\n", pname, errno);
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
               "%s : Unable to bind socket, errno %d\n", pname, errno);
        myexit(FAILURE);
    }

    /*
     * Tell the kernel you are listening for clients
     */

    if ( listen(s, BACKLOG) < 0 )
    {
        debug((1, "main() - Unable to bind socket errno = %d\n", errno));
        errlog(INT_LOG,
               "%s : Listen on socket failed, errno %d\n", pname, errno);
        myexit(FAILURE);
    }

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
        debug((1, "main() - error in accept()  errno = %d\n", errno));
        errlog(INT_LOG,
               "%s : Accept on socket failed, errno %d\n", pname, errno);
        goto again;
    }

    debug((3, "main() - going to fork\n"));

    pid = fork();

    switch ( pid )
    {
    case  -1:

        debug((1, "main() - Unable to fork! errno = %d\n", errno));
        errlog(INT_LOG, "%s : Fork Failed, errno %d\n", pname, errno);
        myexit(FAILURE);
        break;

    case  0:      /* child */

        /* exec new process to manage call  */

        /* close listening socket	*/
        close(s);

        sprintf(cmddebug, "-d%d", debuglevel);
        sprintf(cmd, "-s%d", new);
        debug((3, "main() - going to execute %s with socket %s debug %s\n",
               X25_CALLER, cmd, cmddebug));

        /* use execlp instead	*/

#ifdef  DEBUG
        execlp(X25_CALLER, X25_CALLER, cmd, cmddebug, 0);
#else
        execlp(X25_CALLER, X25_CALLER, cmd, 0);
#endif

        /* if reached means that execlp failed	*/

        debug((1, "main() CHILD: execlp failed errno %d\n", errno));
        errlog(INT_LOG,
               "%s : Unable to execlp %s , errno %d\n", pname, cmd, errno);
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

        if (pid_push(pid) == -1)            /* pid table is full */
        {
            errlog(INT_LOG,
                   "%s : Pid table is full - going to SIGTERM process %d",
                   pname, pid);

            kill(SIGTERM, pid);
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
            printf("%s : invalid flag.\n", pname);
            x25_call_mgr_usage(argc, argv);
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
        x25_call_mgr_usage(argc, argv);
        exit(FAILURE);
    }
}


/*
 *
 *  Procedure: x25_call_mgr_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return:
 *
 */

void    x25_call_mgr_usage(argc, argv)
int argc;
char **argv;
{
#ifdef  DEBUG
    printf("Usage: %s -d<debuglevel>\n", pname);
#else
    printf("Usage: %s\n", pname);
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

    debug((3, "child_termination() - Starting\n"));

    signal(SIGCHLD, SIG_DFL);

    pid = wait(&status);

    if (pid_pop(pid) == -1)
    {
        errlog(INT_LOG, "%s : pid_pop() - pid not find\n", pname);
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
 *  Description: close file descriptors, delete routes and exit
 *
 *  Return:
 *
 */

void    myexit(code)
int code;
{
    int ret, i;

    debug((3, "myexit() - terminating code %d\n", code));

    for (i=0; i<=LEN; i++)
    {
        if (( ret = sm_delete_route(myhostname, portlist[i])) < 0)
        {
            debug((3, "myexit() - sm_delete_route() ret = %d sm_errno %d\n",
                   ret, sm_errno));
        }
        else
        {
            debug((3, "myexit() - sm_delete_route() returned %d\n", ret));
        }
    }

    if ( s != -1 )
    {
        close(s);
    }
    if ( new != -1 )
    {
        close(new);
    }

    debug((3, "myexit() - Terminating\n"));

    pid_kill(SIGTERM);

#ifdef DEBUG
    enddebug();
#endif

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
    debug((3, "terminate() - terminating on sig  %d\n", sig));
    myexit(SUCCESS);
}

