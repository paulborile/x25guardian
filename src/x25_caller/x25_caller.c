/*
 * $Id: x25_caller.c,v 1.1.1.1 1998/11/18 15:03:31 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_caller
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_caller.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:31  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.5  1995/09/29  17:21:58  px25
 * Restyling.
 *
 * Revision 1.4  1995/09/26  15:51:34  giorgio
 * Added the possibility to have no secondary_nua to call in routing.table (signed with a"-")
 *
 * Revision 1.3  1995/09/18  13:50:26  px25
 * Better display of diagnostics.
 *
 * Revision 1.2  1995/07/18  13:16:27  px25
 * Facility struict is copied correctly now.
 * No more ifdef TEST to fake source address
 *
 * Revision 1.1  1995/07/14  09:11:59  px25
 * Better errlog.
 * Added Management of x25xcall return code
 *
 * Revision 1.0  1995/07/07  10:17:27  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_caller.c,v 1.1.1.1 1998/11/18 15:03:31 paul Exp $";

/*  System include files                        */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include "sys/types.h"

#include    <eicon/x25.h>
#include <eicon/neterr.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "gp.h"
#include        "x25_caller.h"

/*  Module include files                        */

#include "debug.h"
#include "errlog.h"

/*  Extern functions used                       */

extern char *getenv();
extern char **lin_toks();

/*  Extern data used                            */

extern int gp_errno;

/*  Local constants                             */

#define  OPTS       "d:s:"
#define  HOSTNAME_LEN   32

#ifdef   DEBUG
#define  X25_OUTCALL  "x25_outcall.d"
#else
#define  X25_OUTCALL  "x25_outcall"
#endif

#define DEFAULT_CALL_TIMEOUT        30
#define DEFAULT_CLR_TIMEOUT     5
#define CALL_TIMEOUT    "output_call_timeout"
#define  CLR_TIMEOUT        "hangup_timeout"

/*  Local functions used								*/

void    terminate();
void        set_signals();
void     get_command_line();
void     x25_caller_usage();
void        myexit();
void        done();

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static char pid_str[20];
static int debuglevel;
static char debug_file[BUFSIZ];
static struct   sigaction act;

char remote[X25_ADDRLEN+X25_ADDREXT+2];        /* Remote DTE address */
char remote2[X25_ADDRLEN+X25_ADDREXT+2];       /* Remote DTE address */
char local[X25_ADDRLEN+X25_ADDREXT+2];         /* Local DTE address */

struct  x25doneinfo x25doneinfo;
struct  x25data facility;               /* Call/Listen facility */
struct  x25data *udata_p;
int info;                                      /* Call/Listen info parameter */
int port;                                      /* Call/Listen port parameter */
int cid;                                       /* Connection identifier */
int lsn;

int sd;                /* socket descriptor on command line */
int RC = -1;                  /* Set if read X25_ASY_DATA_PACKET from asy */

char socketbuffer[BUFSIZ];
char pname[BUFSIZ];
char cmd[16];
char cmdport[16];
char cmdlsn[16];
char cmdlocal[16];
char cmdremote[16];
char cmddebug[16];
int primary_done = -1;
char localcopy[20];
char remotecopy[20];

char msg[ENETMSGLEN];                         /* x25 string. */

static char myhostname[HOSTNAME_LEN];
char default_path[200];
char def_gen_path[200];
char *genfile;
int call_timeout, clr_timeout;

main(argc, argv)
int argc;
char *argv[];
{
    struct  PKT_HDR *p = (struct PKT_HDR *) (socketbuffer);
    struct  X25_INFO *i = (struct X25_INFO *)
                          (socketbuffer + sizeof(struct PKT_HDR));
    char *packet     = (socketbuffer + sizeof(struct PKT_HDR));

    int rc;

    get_command_line(argc, argv);

#ifdef DEBUG
    sprintf(pid_str, "%d", getpid());
    sprintf(debug_file, "/tmp/x25_caller%05d.debug", getpid());
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    /*
     * set signals
     */

    set_signals();

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

    /*
     * get timeout value
     */

    if (( genfile = getenv(PX25_GEN_TABLE)) == NULL )
    {
        strcpy(def_gen_path, default_path);
        strcat(def_gen_path, "gen.tab");
        errlog(INT_LOG, "%s: defaulting PX25_GEN_TABLE to %s\n",
               pname, def_gen_path);
        genfile = def_gen_path;
    }

    debug((3, "main() - gen.tab is %s\n", genfile));

    if ( (call_timeout = get_param( genfile, CALL_TIMEOUT )) < 0 )
    {
        errlog(INT_LOG, "%s : get_param() gp_errno %d\n", pname, gp_errno);
        errlog(INT_LOG, "%s : defaulting CALL_TIMEOUT to %d\n",
               pname, DEFAULT_CALL_TIMEOUT);
        call_timeout = DEFAULT_CALL_TIMEOUT;
    }

    if ( (clr_timeout = get_param( genfile, CLR_TIMEOUT )) < 0 )
    {
        errlog(INT_LOG, "%s : get_param() gp_errno %d\n", pname, gp_errno);
        errlog(INT_LOG, "%s : defaulting CLR_TIMEOUT to %d\n",
               pname, DEFAULT_CLR_TIMEOUT);
        clr_timeout = DEFAULT_CLR_TIMEOUT;
    }

    /* Initialize the X.25 library. */

    if (x25init(0) < 0)
    {
        x25errormsg(msg);
        errlog(X25_LOG, "%s : x25init error %d, %s\n",
               pname, x25error(), msg);
#ifdef   DEBUG
        enddebug();
#endif
        x25exit();
        exit(1);
    }

    x25version(msg);                             /* Get the toolkit version. */
    debug((1, "%s", msg));

    /* While    x25_read()	*/

    while (( rc = mos_recv(sd, socketbuffer, BUFSIZ )) > 0 )
    {
        switch ( p->pkt_code )
        {
        case    X25_CALL_REQ:

            debug((3, "mos_recv() received X25_CALL_REQ with rc = %d\n", rc));

            if ( x25_outcall(socketbuffer) )
            {
                /* getconn now for later execl */

                if (( x25getconn(cid, &port, &lsn)) < 0 )
                {
                    x25errormsg(msg);
                    errlog(X25_LOG, "%s - x25getconn error %d, %s\n",
                           pname, x25error(), msg);
                    debug((1, "main() - x25getconn error %d %s\n", x25error(), msg));

                    /* initialize some execlp parameters now so you can   */
                    /* use'em in call to my exit									*/

                    strcpy(localcopy, i->source_nua);

                    if ( primary_done == 1 )
                    {
                        strcpy(remotecopy, i->NUA_X25.primary_nua);
                    }
                    else
                    {
                        if ( primary_done == 0 )
                        {
                            strcpy(remotecopy, i->NUA_X25.secondary_nua);
                        }
                    }

                    /* Send a REJECT packet */

                    p->pkt_code = X25_CALL_REJECT;

                    if (( RC = mos_send(sd, socketbuffer, sizeof(struct PKT_HDR))) < 0)
                    {
                        /* unable to write on socket, close and exit */

                        debug((1, "CALL REJECT - mos_send rc = %d errno %d\n",
                               RC, errno));
                        errlog(INT_LOG,
                               "%s : unable to mos_send CALL REJECT on socket,errno %d\n",
                               pname, errno);
                        myexit(1);
                    }
                    debug((1, "mos_send() - X25_CALL_REJECT with rc = %d\n", RC));
                    myexit(1);
                }
                debug((3, "x25getconn() cid = %d port = %d lsn %d\n",
                       cid, port, lsn));

                /*	Send an ACCEPT packet */

                p->pkt_code = X25_CALL_ACCEPT;

                if (( RC = mos_send(sd, socketbuffer, sizeof(struct PKT_HDR))) < 0)
                {
                    /* unable to write on socket, close and exit */

                    debug((1, "CALL_ACCEPT - mos_send() rc = %d errno %d\n",
                           RC, errno));

                    errlog(INT_LOG,
                           "%s : unable to mos_send CALL ACCEPT on socket, errno %d\n",
                           pname, errno);
                    myexit(1);
                }
                debug((1, "mos_send() - X25_CALL_ACCEPT with rc = %d\n", RC));

                /* exec new process to manage call  */

                sprintf(cmd, "-s%d", sd);
                sprintf(cmdport, "-p%d", port);
                sprintf(cmdlsn, "-l%d", lsn);
                sprintf(cmddebug, "-d%d", debuglevel);
                sprintf(cmdlocal, "-x%s", i->source_nua);
                sprintf(cmdremote, "-r%s",
                        ( primary_done == 1 )
                  ? i->NUA_X25.primary_nua : i->NUA_X25.secondary_nua);

                debug((3,
                       "executing %s socket %s port %s lsn %s loc %s rem %s debug%s\n",
                       X25_OUTCALL, cmd, cmdport, cmdlsn,
                       cmdlocal, cmdremote, cmddebug));

#ifdef   DEBUG
                execlp(X25_OUTCALL, X25_OUTCALL, cmd, cmdport, cmdlsn, cmdlocal, cmdremote, cmddebug, 0);
#else
                execlp(X25_OUTCALL, X25_OUTCALL, cmd, cmdport, cmdlsn, cmdlocal, cmdremote, 0);
#endif

                /* if reached means that execlp failed */

                debug((1, "main() execlp failed errno %d\n", errno));
                errlog(INT_LOG, "%s : Unable to execlp %s , errno %d\n",
                       pname, X25_OUTCALL, errno);
                myexit(1);
            }
            else
            {
                /* Cannot xcalling */
                /* Send a REJECT packet */

                p->pkt_code = X25_CALL_REJECT;

                if (( RC = mos_send(sd, socketbuffer, sizeof(struct PKT_HDR))) < 0)
                {
                    /* unable to write on socket, close and exit */

                    debug((1, " NO CALL REJECT - mos_send rc = %d errno %d\n",
                           RC, errno));
                    errlog(INT_LOG,
                           "%s : unable to mos_send CALL REJECT on socket,errno %d\n",
                           pname, errno);
                    myexit(1);
                }
                debug((1, "mos_send() - X25_CALL_REJECT with rc = %d\n", RC));
                close (sd);
                exit(1);
            }
            break;

        default:

            debug((1, "main() - No X25_CALL_REQ packet received\n"));
            errlog(INT_LOG, "%s : NO X25_CALL_REQ PKT RECEIVED\n", pname);
            myexit(1);
            break;
        }
    } /* mos_recv returned 0 = input socket closed */

    if ( rc == 0 )
    {
        debug((3, "mos_recv returned 0 = input socket closed\n"));

        /* close all connections	*/
        /* exit	*/
        close(sd);
        x25exit();
        exit(1);
    }
}


/*
 *
 * Procedure: x25_outcall()
 *
 * Parameters: socket buffer
 *
 * Description: generate outgoing call with info coming from
 *					x25_listener
 *
 * Return: TRUE|FALSE
 *
 */

int x25_outcall(buf)
char *buf;
{
    int xerr;

    struct  X25_INFO *i = (struct X25_INFO *) (buf + sizeof(struct PKT_HDR));
    char ea_to_route[3*MAX_USER_DATA_LEN];


    char causemsg[ENETMSGLEN];
    char msg[ENETMSGLEN];
    char message[ENETMSGLEN];

    debug((3, "x25_outcall() - mos_receiving X25_INFO structure ....\n"));

    /* Initialize the remote and local addresses */

    debug((3, "remote %s\n", i->NUA_X25.primary_nua));

    /*******************
       errlog(X25_LOG, "%s : CALLING PRIMARY %s, SECONDARY %s\n", i->source_nua,
                    i->NUA_X25.primary_nua, i->NUA_X25.secondary_nua);
     ********************/

    /* value for xcall() */

    strcpy(remote, i->NUA_X25.primary_nua);
    strcpy(remote2, i->NUA_X25.secondary_nua);

#ifdef  USE_LOCAL_NUA
    if ( get_local_nua(i->port, local) < 0 )
    {
        errlog(X25_LOG, "%s : LOCAL NUA FOR PORT %d NOT FOUND\n",
               i->source_nua, i->port);
        errlog(X25_LOG, "%s : USING INCOMING CALL REMOTE NUA\n", i->source_nua);
        strcpy(local, i->source_nua);
    }
#else
    strcpy(local, i->source_nua);
#endif

    port = i->port;

    debug((3, "port %d\n", i->port));

    /* Choose user data */

    if ( i->userdata.xd_len > 0 )
    {
        debug((3, "user data len = %d\n", i->userdata.xd_len));
        udata_p = &(i->userdata);
        bin_to_ea(i->userdata.xd_data, ea_to_route, i->userdata.xd_len);
    }
    else
    {
        debug((3, "user data len = 0\n"));
        udata_p = NULL;
    }

    debug((3, "userdata.xd_data = %x %x %x %x\n",
           i->userdata.xd_data[0], i->userdata.xd_data[1],
           i->userdata.xd_data[2], i->userdata.xd_data[3]));

    /* facilities  */

    facility = i->facility;

    debug((3, "facility.xd_data = %x %x, xd_len = %d\n",
           facility.xd_data[0], facility.xd_data[1],
           facility.xd_len));

    /* info  */
    info = 0;

    /* Establish the X.25 connection in NOWAIT mode. */

    debug((3, "doing x25xcall() to remote %s from %s port %d\n",
           remote, local, port));

    if (x25xcall(&cid, X25NOWAIT, port, info, &facility, udata_p,
                 remote, local, X25NULLFN) < 0)
    {
        x25causemsg(causemsg);
        x25diagmsg(message);
        x25errormsg(msg);

        debug((1,
               "x25xcall() to remote %s from local %s failed with error %d, %s\ncause: %s, diag: %s\n",
               i->NUA_X25.primary_nua, local, x25error(), msg, causemsg, message));
        errlog(X25_LOG, "%s : CALL TO NUA %s FAILED: %s\n",
               i->source_nua, i->NUA_X25.primary_nua, message);

        /* try on secondary nua */
again1:

        if (strcmp(remote2, "-") != 0)
        {
            debug((3, "trying on secondary nua\n"));

            strcpy(remote, remote2);

            if (x25xcall(&cid, X25NOWAIT, port, info, &facility, udata_p,
                         remote, local, X25NULLFN) < 0)
            {
                x25causemsg(causemsg);
                x25diagmsg(message);
                x25errormsg(msg);
                debug((1,
                       "x25xcall() secondary nua failed error %d %s, cause: %s, diag: %s\n",
                       x25error(), msg, causemsg, message));

                if ( udata_p == NULL )
                {
                    errlog(X25_LOG, "%s : CALL TO NUA %s NO UD, FAILED: %s\n",
                           i->source_nua, i->NUA_X25.secondary_nua, message);
                }
                else
                {
                    errlog(X25_LOG, "%s : CALL TO NUA %s UD %s, FAILED: %s\n",
                           i->source_nua, i->NUA_X25.secondary_nua, ea_to_route, message);
                }
                return(0);
            }
            else
            {
                if ( x25done(cid, call_timeout, &x25doneinfo) < 0 )
                {
                    xerr = x25error();
                    x25errormsg(msg);
                    x25causemsg(causemsg);
                    x25diagmsg(message);
                    debug((1, "main() - x25done error %d %s, cause: %s, diag: %s\n",
                           xerr, msg, causemsg, message));
                    if ( xerr == EX25DONETO )
                    {
                        /* receive timed out */

                        if ( udata_p == NULL )
                        {
                            errlog(X25_LOG,
                                   "%s : CALL TO NUA %s NO UD, FAILED : TIMED OUT\n",
                                   i->source_nua, i->NUA_X25.secondary_nua);
                        }
                        else
                        {
                            errlog(X25_LOG,
                                   "%s : CALL TO NUA %s UD %s, FAILED : TIMED OUT\n",
                                   i->source_nua, i->NUA_X25.secondary_nua, ea_to_route);
                        }
                        return (0);
                    }
                }

                if ( x25doneinfo.xi_retcode != 0 )
                {
                    x25diagmsg(message);
                    switch (x25doneinfo.xi_retcode)
                    {
                    case  ENETCALLCLR:
                        errlog(X25_LOG, "%s: CLEAR RECEIVED:%s\n",
                               i->source_nua, message);
                        break;
                    default:

                        if ( udata_p == NULL )
                        {
                            errlog(X25_LOG,
                                   "%s : CALL TO NUA %s NO UD, FAILED: %s\n",
                                   i->source_nua, i->NUA_X25.secondary_nua, message);
                        }
                        else
                        {
                            errlog(X25_LOG,
                                   "%s : CALL TO NUA %s UD %s, FAILED: %s\n",
                                   i->source_nua, i->NUA_X25.secondary_nua,
                                   ea_to_route, message);
                        }
                    }
                    return (0);
                }

                primary_done = 0;
                debug((3, "x25xcall() on secondary nua OK cid = %d\n", cid));
                if ( udata_p == NULL )
                {
                    errlog(X25_LOG, "%s : CALL TO NUA %s SUCCEDED, NO UD.\n",
                           i->source_nua, i->NUA_X25.secondary_nua);
                }
                else
                {
                    errlog(X25_LOG, "%s : CALL TO NUA %s OK, UD %s\n",
                           i->source_nua, i->NUA_X25.secondary_nua, ea_to_route);
                }
            }
        }
        else     /* no secondary nua to call */
        {
            primary_done == -1;
            debug((3, "NO secondary nua to call\n"));
            /* errlog(X25_LOG,"%s : NO secondary nua to call\n", i->source_nua); */
            return (0);
        }
    }
    else
    {
        /*  call on primary nua done with NO error */

        if ( x25done(cid, call_timeout, &x25doneinfo) < 0 )
        {
            xerr = x25error();
            x25errormsg(msg);
            debug((1, "main() - x25done error %d %s\n", xerr, msg));
            if ( xerr == EX25DONETO )
            {
                /* receive timed out */

                if ( udata_p == NULL )
                {
                    errlog(X25_LOG,
                           "%s : CALL TO NUA %s NO UD, FAILED : TIMED OUT\n",
                           i->source_nua, i->NUA_X25.primary_nua);
                }
                else
                {
                    errlog(X25_LOG,
                           "%s : CALL TO NUA %s UD %s, FAILED : TIMED OUT\n",
                           i->source_nua, i->NUA_X25.primary_nua, ea_to_route);
                }
            }
            goto again1;     /*** goto xcall on secondary nua ********/
        }

        if ( x25doneinfo.xi_retcode != 0 )
        {
            x25diagmsg(message);
            switch (x25doneinfo.xi_retcode)
            {
            case  ENETCALLCLR:
                errlog(X25_LOG, "%s: CLEAR RECEIVED: %s \n",
                       i->source_nua, message);
                break;
            default:

                if ( udata_p == NULL )
                {
                    errlog(X25_LOG, "%s : CALL TO NUA %s NO UD, FAILED: %s\n",
                           i->source_nua, i->NUA_X25.primary_nua, message);
                }
                else
                {
                    errlog(X25_LOG, "%s : CALL TO NUA %s UD %s, FAILED: %s\n",
                           i->source_nua, i->NUA_X25.primary_nua,
                           ea_to_route, message);
                }
                break;
            }
            goto again1;
        }
        primary_done = 1;
        debug((3, "x25xcall() on primary nua OK cid = %d\n", cid));

        if ( udata_p == NULL )
        {
            errlog(X25_LOG, "%s : CALL TO NUA %s OK, NO UD\n",
                   i->source_nua, i->NUA_X25.primary_nua);
        }
        else
        {
            errlog(X25_LOG, "%s : CALL TO NUA %s OK, UD %s\n",
                   i->source_nua, i->NUA_X25.primary_nua, ea_to_route);
        }
    }

    return(1);
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

        default:
            printf("%s : invalid flag.\n", argv[0]);
            x25_caller_usage(argc, argv);
            exit(FAILURE);
        }
    }

#ifdef DEBUG
    if ( debuglevel == -1 )
    {
        fatal++;
    }
#endif

    if ( sd == -1 )
    {
        fatal++;
    }

    if ( fatal )
    {
        x25_caller_usage(argc, argv);
        exit(FAILURE);
    }
}

/*
 *
 *  Procedure: x25_caller_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void  x25_caller_usage(argc, argv)
int argc;
char **argv;
{
#ifdef   DEBUG
    printf("Usage: %s -d<debuglevel> -s<socket>\n", argv[0]);
#else
    printf("Usage: %s -s<socket>\n", argv[0]);
#endif
}

/*
 *
 *  Procedure: myexit
 *
 *  Parameters: exit code
 *
 *  Description: close sd , hangup and exit
 *
 *  Return:
 *
 */

void  myexit(code)
int code;
{
    int RC, rc, xerr;
    char msg[ENETMSGLEN];
    struct   x25doneinfo xdi;

    debug((3, "myexit() - starting code = %d\n", code));

    if ( sd != -1 )
    {
        close(sd);
    }

    /* No wait mode hangup needs x25done   */

    if ((RC = x25hangup(cid, NULL, XH_IMM, X25NULLFN)) < 0)
    {
        x25errormsg(msg);
        errlog(X25_LOG, "%s - x25hangup error %d, %s\n",
               pname, x25error(), msg);

        debug((1, "myexit() - x25hangup failed %d, %s\n",
               x25error(), msg));
    }
    else
    {
        if (( rc = x25done(cid, clr_timeout, &xdi)) < 0 )
        {
            xerr = x25error();
            x25errormsg(msg);
            debug((1, "main() - x25done error %d %s\n", xerr, msg));
            if ( xerr == EX25DONETO )
            {
                errlog(X25_LOG, "%s : TIME OUT ON SENDING A CLEAR \n",
                       localcopy, remotecopy);
            }
        }
        else
        {
            debug((3, "hangup() - x25done rc %d\n", rc));
            debug((3, "myexit() - x25hangup(%d) rc %d\n", cid, RC));
            errlog(X25_LOG, "%s : CLR SENT TO %s\n", localcopy, remotecopy );
        }
    }

#ifdef   DEBUG
    enddebug();
#endif

    x25exit();
    exit(code);
}

/*
 *
 *  Procedure: set_signals
 *
 *  Parameters: none
 *
 *  Description: set initial signals using sigaction
 *
 *  Return:  none
 *
 */

void    set_signals()
{
    debug((3, "set_signals() - setting signals\n"));

    act.sa_flags        = SA_RESTART;
    sigemptyset(&act.sa_mask);

    act.sa_handler      = terminate;

    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);

    debug((3, "set_signals() - done\n"));
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

/*
 *
 *  Procedure: get_local_nua
 *
 *  Parameters: port, string to fill with local nua
 *
 *  Description: find out local nua for port
 *
 *  Return: 0 if ok -1 if local nua not found
 *
 */

int get_local_nua(port, lnua)
int port;
char *lnua;
{
    char **fields;
    char buffer[BUFSIZ];
    FILE *fp;
    char *finger;
    char portstr[16];

#define COMMENT '#'
#define SYNC_FILE_FIELDS        4

    sprintf(portstr, "%d", port);

    debug((3, "get_local_nua() - starting port = %d\n", port));

    /*
     * Get env var to point to sync file
     */

    if (( finger = getenv(PX25_SYN_TABLE_PATH)) == NULL )
    {
        errlog(INT_LOG, "%s : PX25_SYN_TABLE_PATH not set !\n", pname);
        return(-1);
    }

    /*
     * open it to find out my nua
     */

    if (( fp = fopen(finger, "r")) == NULL )
    {
        errlog(INT_LOG, "%s : CANNOT OPEN %s !\n", pname, finger);
        return(-1);
    }

    while ( fgets(buffer, BUFSIZ, fp) != NULL )
    {
        if (( buffer[0] == COMMENT ) || ( buffer[0] == '\n' ))
        {
            continue;
        }

        if (( fields = lin_toks(buffer, SYNC_FILE_FIELDS)) != NULL )
        {
            if ( strcmp(portstr, fields[0]) == 0 )
            {
                /* found	correct port	*/

                strcpy(lnua, fields[SYNC_FILE_FIELDS-1]);
                fclose(fp);
                return(0);
            }
        }
    }

    fclose(fp);

    /* if reached means that local nua not found	*/

    errlog(INT_LOG, "%s : LOCAL NUA FOR PORT %d NOT FOUND\n", pname, port);
    debug((3, "load_tables() - end of synctab, local nua not found\n"));
    return(-1);
}
