/*
 * $Id: x25_easyway.c,v 1.1.1.1 1998/11/18 15:03:32 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_easyway
 *
 * Contents: manage easyway calls to x25 network
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_easyway.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:32  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/07/14  09:13:30  px25
 * Better errlog.
 * Added parameters to execlp to incall (remote, local nua)
 *
 * Revision 1.0  1995/07/07  10:18:04  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_easyway.c,v 1.1.1.1 1998/11/18 15:03:32 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <sys/types.h>
#include        <sys/signal.h>
#include        <x25.h>
#include        <neterr.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "sm.h"
#include        "debug.h"
#include        "errlog.h"

/*  Module include files                        */

/*  Extern functions used                       */

void    get_command_line();
void    x25_easyway_usage();

/*  Extern data used                            */

extern int errno;

/*  Local constants                             */

#define OPTS                "d:s:p:l:x:r:"

#ifdef  DEBUG
#define X25_INCALL          "x25_incall.d"
#else
#define X25_INCALL          "x25_incall"
#endif

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

char buf[BUFSIZ];
struct  PKT_HDR *ph = (struct PKT_HDR *) (buf);
char pname[32];
char pid_str[32];
char debug_file[BUFSIZ];
int debuglevel = -1;

int cid;
int sock = -1;
int lsn = -1;
int port = -1;
char local[X25_ADDRLEN+X25_ADDREXT+2];
char remote[X25_ADDRLEN+X25_ADDREXT+2];

/*
 *
 * Procedure: main
 *
 * Parameters: argc argv
 *
 * Description: receive call accept or reject and then x25accept call or not
 *
 * Return: none
 *
 */


main(argc, argv)
int argc;
char **argv;
{
    int info = 0;
    int rc;
    int xerr;
    pid_t pid;
    char msg[ENETMSGLEN];
    char sockstr[MAX_STR], portstr[MAX_STR], lsnstr[MAX_STR],
         localstr[MAX_STR], remotestr[MAX_STR], debugstr[MAX_STR];


    signal(SIGPIPE, SIG_IGN);

    /*
     * Globalize name of the program
     */

    strcpy(pname, argv[0]);

    get_command_line(argc, argv);

#ifdef DEBUG
    sprintf(pid_str, "%d", getpid());
    sprintf(debug_file, "/tmp/x25_easyway%05d.debug", getpid());
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    /*
     * First initialize x25 toolkit and the mkconn
     */

    if (x25init(0) < 0)
    {
        x25errormsg(msg);
        errlog(X25_LOG, "%s : INITIALIZATION ERROR %s\n", remote, msg);
#ifdef  DEBUG
        enddebug();
#endif
        exit(1);
    }

    x25version(msg);                             /* Get the toolkit version. */
    debug((1, "main() - %s", msg));

    debug((3, "main() - mkconn port %d lsn %d\n", port, lsn));

    if ( x25mkconn(&cid, port, lsn, X25WAIT) < 0 )
    {
        xerr = x25error();
        x25errormsg(msg);

        errlog(X25_LOG, "%s : INITIALIZATION ERROR %s\n", remote, msg);
        debug((1, "main() - x25mkconn error %d %s\n", xerr, msg));

#ifdef  DEBUG
        enddebug();
#endif
        close(sock);
        exit(1);
    }

    debug((3, "main() - mkconn ok cid %d\n", cid));

    /*
     * now receive from the socket
     * and wait for a X25_CALL_ACCEPT or REJECT
     */

    if (( rc = mos_recv(sock, buf, BUFSIZ)) <= 0 )
    {
        /*
         * socket was closed already so hangup the connection
         */

        debug((1, "main() - mos_recv ret %d, errno %d\n", rc, errno));
        x25hangup(cid, NULL, XH_IMM, X25NULLFN);
        errlog(X25_LOG,
               "%s : ERROR WAITING FOR X25 OUTGOING CALL\n", remote, msg);
        x25exit();
        close(sock);
        exit(FAILURE);
    }

    /* Receive ok	*/

    debug((3, "main() - mos_received %d bytes\n", rc));

    switch (ph->pkt_code)
    {
    case    X25_CALL_REJECT:

        /* call not possible on x25_caller side	*/

        debug((3, "main() - X25_CALL_REJECT received\n"));

        x25hangup(cid, NULL, XH_IMM, X25NULLFN);
        errlog(X25_LOG, "%s : X25 OUTGOING CALL FAILED. HANGUP CONNECTION\n",
               remote);
        x25exit();
        close(sock);
        exit(FAILURE);
        break;

    case    X25_CALL_ACCEPT:

        debug((3, "main() - X25_CALL_ACCEPT received\n"));

        /*
         * execute accept back to incoming call
         */

        debug((3, "main() - before accept remote %s, local %s\n",
               remote, local));

        if ( x25accept(&cid, info, NULL, NULL, remote, local, X25NULLFN) < 0)
        {
            xerr = x25error(); x25errormsg(msg);

            errlog(X25_LOG, "%s - x25accept() error %d, %s\n", pname, xerr, msg);
            debug((1, "main() - x25accept() error %d %s\n", xerr, msg));
            x25hangup(cid, NULL, XH_IMM, X25NULLFN);
            x25exit();
            close(sock);
            exit(FAILURE);
        }

        debug((3, "main() - after accept remote %s, local %s\n",
               remote, local));

        break;

    default:

        debug((1, "main() - packet_code %x not foreseen\n", ph->pkt_code));
        errlog(X25_LOG, "%s : X25 CONFIRM %d NOT FORESEEN.HANGUP CONNECTION\n",
               remote, ph->pkt_code);
        x25hangup(cid, NULL, XH_IMM, X25NULLFN);
        x25exit();
        close(sock);
        exit(FAILURE);
        break;
    }

    /*
     * call was accepted so now getconn and fork real incall
     */

    if ( x25getconn(cid, &port, &lsn) < 0 )
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s - x25getconn error %d, %s\n", pname, xerr, msg);
        debug((1, "main() - x25getconn error %d %s\n", xerr, msg));
        x25hangup(cid, NULL, XH_IMM, X25NULLFN);
        x25exit();
        close(sock);
        exit(FAILURE);
    }

    /* exec new process to manage call  */

    sprintf(sockstr, "-s%d", sock);
    sprintf(portstr, "-p%d", port);
    sprintf(lsnstr, "-l%d", lsn);
    sprintf(debugstr, "-d%d", debuglevel);
    sprintf(localstr, "-x%s", local);
    sprintf(remotestr, "-r%s", remote);

    debug((3, "main() - execute %s socket %s port %s lsn %s loca %s rem %s debug %s\n",
           X25_INCALL, sockstr, portstr, lsnstr, localstr, remotestr, debugstr));

#ifdef  DEBUG
    execlp(X25_INCALL, X25_INCALL, sockstr, portstr, lsnstr, localstr, remotestr, debugstr, 0);
#else
    execlp(X25_INCALL, X25_INCALL, sockstr, portstr, lsnstr, localstr, remotestr, 0);
#endif

    /* if reached means that execlp failed */

    debug((1, "main() - execlp failed errno %d\n", errno));

    errlog(INT_LOG,
           "%s : UNABLE TO EXECLP %s , ERRNO %d\n", pname, X25_INCALL, errno);

    errlog(X25_LOG, "%s : INTERNAL ERROR, CLEARING CALL\n", remote);

    x25hangup(cid, NULL, XH_IMM, X25NULLFN);
    x25exit();
    close(sock);
    exit(FAILURE);
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

    local[0] = remote[0] = '\0';

    while ((c = getopt(argc, argv, OPTS)) !=EOF)
    {
        switch (c)
        {
        case    'd':

            debuglevel  =   atoi(optarg);
            break;

        case    's':

            sock            =   atoi(optarg);
            break;

        case    'p':

            port = atoi(optarg);
            break;

        case    'l':

            lsn =   atoi(optarg);
            break;

        case    'x':

            strcpy(local, optarg);
            break;

        case    'r':

            strcpy(remote, optarg);
            break;

        default:
            printf("%s : invalid flag.\n", argv[0]);
            x25_easyway_usage(argc, argv);
            exit(FAILURE);
        }
    }

#ifdef DEBUG
    if ( debuglevel == -1 )
    {
        fatal++;
    }
#endif

    if ( port == -1 )
    {
        fatal++;
    }
    if ( sock == -1 )
    {
        fatal++;
    }
    if ( lsn == -1 )
    {
        fatal++;
    }
    if ( local[0] == '\0' )
    {
        fatal++;
    }
    if ( remote[0] == '\0' )
    {
        fatal++;
    }

    if ( fatal )
    {
        x25_easyway_usage(argc, argv);
        exit(FAILURE);
    }
}

/*
 *
 *  Procedure: x25_easyway_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void    x25_easyway_usage(argc, argv)
int argc;
char **argv;
{
#ifdef  DEBUG
    printf("Usage: %s -d<debuglevel> -s<socket> -p<port> -l<lsn> -x<local_x25_addr> -r<remote_x25_addr>\n", argv[0]);
#else
    printf("Usage: %s -s<socket> -p<port> -l<lsn> -x<local_x25_addr> -r<remote_x25_addr>\n", argv[0]);
#endif
}
