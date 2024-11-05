/*
 * $Id: x25_incall.c,v 1.1.1.1 1998/11/18 15:03:29 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_incall.c
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_incall.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:29  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.3  1995/09/29  17:10:56  px25
 * Moved x25hangup call before cause and diag (EICON Bug Fix)
 *
 * Revision 1.2  1995/09/18  10:49:33  giorgio
 * Management of QBIT to send to X25 lines.
 * Different default path for configuration files.
 *
 * Revision 1.1  1995/07/14  09:14:25  px25
 * New get_param function used to retrieve general parameters
 * Datascope functions added.
 * Get_command_line() modified to accept local and remote nua.
 *
 * Revision 1.0  1995/07/07  10:19:15  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_incall.c,v 1.1.1.1 1998/11/18 15:03:29 paul Exp $";

/*  System include files                        */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include    <string.h>
#include <errno.h>
#include <signal.h>
#include <x25.h>
#include <neterr.h>

/*  Project include files                       */

#include    "px25_globals.h"

/*  Module include files                        */

#include "debug.h"
#include "errlog.h"

/*  Extern functions used                       */

extern char *getenv();

/*  Extern data used                            */

extern int gp_errno;
extern int cid1;        /* the child cid	*/

/*  Local constants                             */

#define     OPTS     "d:s:p:l:x:r:"
#define     HOSTNAME_LEN   32

/*  Local functions used								*/

int     sigterm();
void    sigterm1();
void     get_command_line();
void        x25_incall_usage();
void        child_termination();
void        child_termination1();
void        myexit();

/*  Local types                                 */

/*  Local macros                                */

#define  X25_ADLEN   (X25_ADDRLEN+X25_ADDREXT+2)

/*  Local data                                  */
static char pid_str[20];
static int debuglevel;
static char debug_file[BUFSIZ];
char arg[20];
char *tok;

static char myhostname[HOSTNAME_LEN];
int PID = -1;
char pname[BUFSIZ];
int xerr;
int rc;
int RC;
int cid;
int info = 0;
int sd = -1;                /* socket descriptor on command line */
int port;              /* port descriptor on command line */
int lsn;               /* lsn descriptor on command line */
char *inbuf;
char *outbuf;
char x25name[20];
char buf[BUFSIZ];
char message[ENETMSGLEN];
char msg[ENETMSGLEN];
int mkconn_done = 0;
int in_alloc_done = 0, out_alloc_done = 0;
int child_dead = 0, sigterm_received = 0;
struct  sigaction act;
char local[X25_ADDRLEN+X25_ADDREXT+2];
char remote[X25_ADDRLEN+X25_ADDREXT+2];
char *genfile;
char def_gen_path[200];
char default_path[200];

/* contiene il flag per attivare o meno la funzionalita' di datascope */
int datascope = 0;

main(argc, argv)
int argc;
char *argv[];
{
    struct   PKT_HDR *p = (struct PKT_HDR *) (buf);
    pid_t child;

    get_command_line(argc, argv);

#ifdef DEBUG
    sprintf(pid_str, "%d", getpid());
    strcpy(arg, argv[0]);
    tok = strtok(arg, ".");
    sprintf(debug_file, "/tmp/%s%05d.debug", tok, getpid());
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    /*
     * Globalize program name
     */

    strcpy(pname, argv[0]);

    /*
     * Setting default paths for configuration files
     */

    strcpy(default_path, "/usr3/px25/conf/");

    if ( gethostname(myhostname, HOSTNAME_LEN) < 0 )
    {
        debug((1, "main() - unable to gethostname() errno %d\n", errno));
        errlog(INT_LOG,
               "%s : unable to gethostname - errno %d\n", pname, errno);
    }
    else
    {
        strcat(default_path, myhostname);
        strcat(default_path, "/");
    }

    /* get the general configuration file */

    if (( genfile = getenv(PX25_GEN_TABLE)) == NULL )
    {
        strcpy(def_gen_path, default_path);
        strcat(def_gen_path, "gen.tab");
        errlog(INT_LOG, "%s: defaulting PX25_GEN_TABLE to %s\n",
               pname, def_gen_path);
        genfile = def_gen_path;
    }
    debug((3, "main() - gen.tab is %s\n", genfile));

    /* datascope function will be active ? */

    if ( (datascope = get_param( genfile, "x25_datascope" )) < 0 )
    {
        errlog(INT_LOG, "%s : get_param() gp_errno %d\n", pname, gp_errno);
        errlog(INT_LOG, "%s : defaulting DATASCOPE FUNCTION TO OFF to %d\n",
               pname);
        datascope = 0;
    }

    if ( (datascope != 1) && (datascope !=0) )
    {
        errlog(INT_LOG, "%s : datascope value not valid in file %s: forced to 0\n",
               pname, genfile);
        datascope = 0;
    }

    debug((3, "datascope value = %d\n", datascope));

    if (datascope == 1)
    {
        sprintf(x25name, "X25.%s", remote);
        debug((3, "Doing datascope ...\n"));
        dscope_mark(x25name, "begin read", 'i');
        dscope_mark(x25name, "begin write", 'o');
    }

    act.sa_handler      = child_termination1;
    act.sa_flags        = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGCHLD, &act, NULL);

    act.sa_handler      = sigterm1;

    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);

    act.sa_handler      = SIG_IGN;
    act.sa_flags        = 0;

    sigaction(SIGPIPE, &act, NULL);

    /* Initialize the X.25 library. */

    if (x25init(0) < 0)
    {
        x25errormsg(msg);
        errlog(INT_LOG, "%s : x25init error %d, %s\n",
               pname, x25error(), msg);
#ifdef  DEBUG
        enddebug();
#endif
        if (datascope == 1)
        {
            dscope_mark(x25name, "end read", 'i');
            dscope_mark(x25name, "end write", 'o');
        }
        exit(1);
    }

    x25version(msg);                             /* Get the toolkit version. */
    debug((1, "%s", msg));


    if ( x25mkconn(&cid, port, lsn, X25WAIT) < 0 )
    {
        xerr = x25error();
        x25errormsg(msg);
        errlog(INT_LOG, "x25incall - x25mkconn error %d, %s\n",
               xerr, msg);
        debug((1, "main() - x25mkconn error %d %s\n", xerr, msg));
#ifdef  DEBUG
        enddebug();
#endif
        if (datascope == 1)
        {
            dscope_mark(x25name, "end read", 'i');
            dscope_mark(x25name, "end write", 'o');
        }
        exit(1);
    }

    mkconn_done = 1;

    debug((1, "main() - x25mkconn() OK with cid %d, port %d, lsn %d\n",
           cid, port, lsn));

    if (( inbuf = x25alloc(BUFSIZ)) == NULL)
    {
        xerr = x25error();
        x25errormsg(msg);
        errlog(INT_LOG, "%s - x25alloc() inbuf error %d, %s\n", pname, xerr, msg);
        debug((1, "main() - x25alloc inbuf error %d %s\n", xerr, msg));
        x25delconn(cid);
#ifdef  DEBUG
        enddebug();
#endif
        if (datascope == 1)
        {
            dscope_mark(x25name, "end read", 'i');
            dscope_mark(x25name, "end write", 'o');
        }
        exit(1);
    }
    in_alloc_done = 1;

    debug((3, "main() - x25alloc OK inbuf %d\n", inbuf));

    if (( outbuf = x25alloc(BUFSIZ)) == NULL)
    {
        xerr = x25error();
        x25errormsg(msg);
        errlog(INT_LOG, "%s - x25alloc() outbuf error %d, %s\n", pname, xerr, msg);
        debug((1, "main() - x25alloc outbuf error %d %s\n", xerr, msg));
        x25delconn(cid);
        x25free(inbuf);
#ifdef  DEBUG
        enddebug();
#endif
        if (datascope == 1)
        {
            dscope_mark(x25name, "end read", 'i');
            dscope_mark(x25name, "end write", 'o');
        }
        exit(1);
    }

    debug((3, "main() - x25alloc OK outbuf %d\n", outbuf));

    out_alloc_done = 1;

    if ((child = spawn_receiver()) < 0)
    {
        myexit(1);
    }

    debug((3, "main() - spawned %s_rec pid %d\n", pname, child));

again1:

    while (( rc =
                 x25recv(cid, inbuf, X25_DATA_PACKET_SIZE, &info, X25NULLFN)) > 0 )
    {
        debug((3, "main(father) - x25recv %d bytes info %x\n", rc, info));

        p->pkt_code = X25_DATA_PACKET;
        p->pkt_len = rc;
        p->pkt_error = 0;

        p->flags = 0;

        if (info == XI_MBIT)
        {
            debug((3, "main(father) - x25recv() setting more bit\n"));
            p->flags = p->flags | MORE_BIT;
        }
        if (info == XI_QBIT)
        {
            debug((3, "main(father) -  x25recv() setting qbit\n"));
            p->flags = p->flags | QBIT;
        }

        if (datascope == 1)
        {
            if (dscope_log(x25name, inbuf, rc, 'i') == -1)
            {
                errlog(INT_LOG, "%s : unable to open dscope file\n",
                       pname);
                debug((1, "Unable to open dscope file\n"));
            }
        }

        memcpy(buf + sizeof(struct PKT_HDR), inbuf, rc);

        debug((3, "main(father) - pckt code %x err %x, flags %x, len %x\n",
               p->pkt_code, p->pkt_error, p->flags, p->pkt_len ));

        if (( RC = mos_send(sd, buf, rc + sizeof(struct PKT_HDR))) < 0)
        {
            /* unable to write on socket, close and exit */

            debug((1, "main(father) - mos_send rc = %d errno %d\n", RC, errno));

            errlog(INT_LOG, "%s : unable to mos_send on socket, errno %d\n",
                   pname, errno);
            close(sd);
            myexit(1);
        }
        debug((3, "main() -  mos_sent with rc = %d\n", RC));
    }

    debug((3, "main(father) - x25recv rc = %d \n", rc));

    switch (x25error())
    {
    case    EX25INTR:

        debug((3, "main(father) - EX25INTR , child_dead %d, sigterm %d\n",
               child_dead, sigterm_received));

        if (( rc = x25cancel(cid, inbuf)) == -1 )
        {
            xerr = x25error(); x25errormsg(msg);
            debug((3, "main(father) - x25cancel error %d %s\n", xerr, msg));
        }

        debug((3, "main(father) - x25cancel ret %d\n", rc));
        if ( child_dead )
        {
            child_termination();
        }
        if ( sigterm_received )
        {
            sigterm(SIGTERM);
        }
        goto again1;

        break;

    case ENETSRESET:

        debug((3, "ENETSRESET:  info %02x\n", info));

        if ((info & 3) == XC_DATA )
        {
            /* Call x25recv() to get the cause and diagnostic. */

            if (x25recv(cid, inbuf, X25_DATA_PACKET_SIZE, &info, X25NULLFN) > 0)
            {
                x25causemsg(msg);
                x25diagmsg(message);
                debug((3, "ENETRESET: x25recv - cause: %s, diag: %s\n",
                       msg, message));
                errlog(X25_LOG, "%s : SRESET CAUSE %s DIAG %s\n",
                       remote, msg, message);
            }
            else
            {
                x25errormsg(msg);
                errlog(X25_LOG, "%s : x25recv error %d, %s\n",
                       remote, x25error(), msg);
            }
        }
        else
        {
            switch (info)
            {
            case    XC_RRESETIND:
                errlog(X25_LOG, "%s : REMOTE RESET IND\n", remote);
                break;
            case    XC_LRESETIND:
                errlog(X25_LOG, "%s : LOCAL RESET IND\n", remote);
                break;
            case    XC_ACKRRESETIND:
                errlog(X25_LOG, "%s : REMOTE RESET IND - CONF EXP\n", remote);
                break;
            default:
                errlog(X25_LOG, "%s : INFO %02x RESET IND\n", remote, info);
                break;
            }

            /* Get the cause and diagnostic */

            x25causemsg(msg);
            x25diagmsg(message);
            errlog(X25_LOG, "%s : CAUSE %s, DIAG %s\n", remote, msg, message);
            break;
        }
        break;

    case ENETCALLCLR:

        debug((3, "main() - ENETCALLCLR: info %x\n", info));

        x25hangup(cid, NULL, XH_IMM, X25NULLFN);
        x25causemsg(msg);
        x25diagmsg(message);

        debug((1, "main() - cause: %s, diag: %s\n", msg, message));

        switch (info)
        {
        case    XC_RCLRIND:

            errlog(X25_LOG,
                   "%s : CLEAR INDICATION RECEIVED, CAUSE : %s\n", remote, msg);
            break;

        case    XC_LCLRIND:

            errlog(X25_LOG,
                   "%s : CLEAR INDICATION GENERATED, CAUSE : %s\n", remote, msg);
            break;

        case    XC_RCLRCONF:

            errlog(X25_LOG,
                   "%s : CLEAR CONFIRM RECEIVED, CAUSE : %s\n", remote, msg);
            break;


        case            0:

            errlog(X25_LOG, "%s : CLEAR REQ GENERATED.\n", remote);
            break;

        default:

            errlog(X25_LOG, "%s : CLEAR RECEIVED, INFO %02x.\n",
                   remote, info);
            break;
        }


        break;

    case ENETIMSG:         /* incomplete message */
        debug((1, "main(father) - x25recv error ENETIMSG: \n"));

        /* Still data to read	*/

        goto again1;

    default:
        debug((3, "DEFAULT: \n"));

        x25errormsg(msg);
        errlog(X25_LOG, "%s : x25recv() error %d, %s\n",
               remote, x25error(), msg);

        break;
    }

    debug((3, "main(father) - killing child %d \n", PID));

    if ( kill(PID, SIGTERM) != -1 )
    {
        debug((3, "main(father) - kill ok, pausing for SIGCHLD\n"));
        if ( child_dead )
        {
            child_termination();
        }
        else{
            pause();
        }
    }

    if ( child_dead )
    {
        child_termination();
    }
}

/*
 *
 * Procedure: spawn_receiver
 *
 * Parameters:
 *
 * Description: spawn x25_incall_rec() function as a new process
 *
 * Return: -1 if fork fails, child process id otherwise
 *
 */
int   spawn_receiver()
{

    PID = fork();

    switch (PID)
    {
    case  -1:

        errlog(INT_LOG, "%s : cannot fork, errno %d\n", pname, errno);
        return(-1);
        break;

    case  0:

        /* This is child process   */

        debug((1, "spawn_receiver() - Starting %s_rec\n", pname));

#ifdef UNIXWARE
        ecclose();
        ecopen(0);
#endif

        if (x25incall_rec() == -1)
        {
            if (datascope == 1)
            {
                dscope_mark(x25name, "end write", 'o');
            }
            exit(FAILURE);
        }
        if (datascope == 1)
        {
            dscope_mark(x25name, "end write", 'o');
        }
        exit(SUCCESS);
        break;

    default:

        return(PID);
        break;
    }
}

/*
 *
 *  Procedure: sigterm
 *
 *  Parameters: none
 *
 *  Description: close sd and exit
 *
 *  Return:
 *
 */

void    sigterm1(sig)
int sig;
{
    sigterm_received = 1;
}

sigterm(sig)
int sig;
{
    int rc;
    debug((1, "sigterm() - %s terminating\n", (PID == 0) ? "child" : "father"));

    sigterm_received = 0;

    if ( sd != -1 )
    {
        close(sd);
    }

    if ( PID == 0 )
    {
        debug((1, "sigterm() - child executing delconn\n"));
        rc = x25delconn(cid1);
        debug((1, "sigterm() - child delconn ret %d\n", rc));
    }

    errlog(X25_LOG, "%s : Terminated %s\n", pname,
           (PID == 0) ? "x25_receiver" : "x25_sender" );

    if ( datascope )
    {
        if (PID == 0)
        {
            /* child process  */
            dscope_mark(x25name, "end write", 'o');
        }
        else
        {
            dscope_mark(x25name, "end read", 'i');
        }
    }

    x25exit();
    exit(0);
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

void  get_command_line(argc, argv)
int argc;
char **argv;
{
    int fatal = 0;
    int c;
    extern char *optarg;

    local[0] = remote[0] = '\0';

    while ((c = getopt(argc, argv, OPTS)) !=EOF)
    {
        switch (c)
        {
        case  'd':
            debuglevel  =   atoi(optarg);
            break;

        case  's':
            sd   =  atoi(optarg);
            break;

        case  'p':
            port    = atoi(optarg);
            break;

        case  'l':
            lsn = atoi(optarg);
            break;

        case    'x':

            strcpy(local, optarg);
            break;

        case    'r':

            strcpy(remote, optarg);
            break;

        default:
            printf("%s : invalid flag.\n", argv[0]);
            x25_incall_usage(argc, argv);
            exit(FAILURE);
        }
    }

#ifdef DEBUG
    if ( debuglevel == -1 )
    {
        fatal++;
    }
#endif

    if ( local[0] == '\0' )
    {
        strcpy(local, "local_address");
    }
    if ( remote[0] == '\0' )
    {
        strcpy(remote, "remote_address");
    }

    if ( sd == -1 )
    {
        fatal++;
    }

    if ( port == -1 )
    {
        fatal++;
    }

    if ( lsn == -1 )
    {
        fatal++;
    }

    if ( fatal )
    {
        x25_incall_usage(argc, argv);
        exit(FAILURE);
    }
}

/*
 *
 *  Procedure: x25_incall_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void  x25_incall_usage(argc, argv)
int argc;
char **argv;
{
#ifdef   DEBUG
    printf("Usage: %s -d<debuglevel> -s<socket> -p<port> -l<lsn>\n", argv[0]);
#else
    printf("Usage: %s -s<socket> -p<port> -l<lsn>\n", argv[0]);
#endif
}


/*
 *
 *  Procedure: myexit
 *
 *  Parameters: exit code
 *
 *  Description: free mem if allocated, delconn if mkconn
 *
 *  Return:
 *
 */

void  myexit(code)
int code;
{
    int rc;

    debug((3, "myexit() - starting code = %d\n", code));

    if ( mkconn_done )
    {
        rc = x25delconn(cid);
        debug((3, "myexit() - x25delconn ret %d\n", rc));
    }

    if (in_alloc_done)
    {
        rc = x25free(inbuf);
        debug((3, "myexit() - x25free(inbuf)  ret %d\n", rc));
    }

    if (out_alloc_done)
    {
        rc = x25free(outbuf);
        debug((3, "myexit() - x25free(outbuf) ret %d\n", rc));
    }

    if ( sd != -1 )
    {
        close(sd);
    }

    x25exit();

#ifdef  DEBUG
    enddebug();
#endif

    if ( datascope )
    {
        if (PID == 0)
        {
            /* child process  */
            dscope_mark(x25name, "end write", 'o');
        }
        else
        {
            dscope_mark(x25name, "end read", 'i');
        }
    }

    exit(code);
}

/*
 *
 *  Procedure: child_termination
 *
 *  Parameters: standard signal
 *
 *  Description:
 *
 *  Return:  none
 *
 */

void  child_termination1()
{
    child_dead = 1;
}

void  child_termination()
{
    int ret;
    long status = 0;
    pid_t pid;

    child_dead = 0;

    debug((3, "child_termination() - Starting.\n"));

    signal(SIGCHLD, SIG_DFL);

    pid = wait(&status);

    debug((3, "child_termination() - terminated %d with status %x\n",
           pid, status));

    myexit(FAILURE);
}
