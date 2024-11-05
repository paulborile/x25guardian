/*
 * $Id: argotel.c,v 1.1.1.1 1998/11/18 15:03:26 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: argotel.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:26  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.3  1995/09/18  12:40:43  px25
 * Changed way to decide if udata and facilities are to be passed.
 * Now everything is controlled by routing table.
 *
 * Revision 1.2  1995/07/14  09:16:07  px25
 * Parameters transfer udata and facil.
 *
 * Revision 1.1  1995/07/12  13:49:54  px25
 * Added check on transfer_udata,facilities before passing -u -f flags
 * to x25_argotel process.
 *
 * Revision 1.0  1995/07/07  10:21:16  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: argotel.c,v 1.1.1.1 1998/11/18 15:03:26 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <sys/types.h>
#include        <x25.h>
#include        <neterr.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "debug.h"
#include        "errlog.h"
#include        "sm.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

extern char pname[];
extern int debuglevel;
extern int errno;

/*  Local constants                             */

#define X25_ADLEN   (X25_ADDRLEN+X25_ADDREXT+2)

#ifdef  DEBUG
#define X25_ARGOTEL_PROC    "x25_argotel.d"
#else
#define X25_ARGOTEL_PROC    "x25_argotel"
#endif

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static char msg[ENETMSGLEN];
int port;
int lsn;

char remote[X25_ADLEN];
char local[X25_ADLEN];

/*
 *
 * Procedure: argotel
 *
 * Parameters: cid, user data, facilities, remote and local x25 addrs, info
 *
 * Description: manage argotel incoming calls
 *
 * Return: -1 if errors found, 0 if OK
 *
 */

int     argotel(c, udata, facil, r, l, info)
int c;
struct  x25data *udata, *facil;
char *r, *l;
int info;
{
    char portstr[MAX_STR], lsnstr[MAX_STR], localstr[MAX_STR],
         remotestr[MAX_STR], debugstr[MAX_STR],
         udatastr[BUFSIZ], facilstr[BUFSIZ];
    char dummy[BUFSIZ];
    int pid;
    int xerr;

    /*
     * make local copies of r and l parameters cause x25_accept modifies
     * previous x25listen ones
     */

    strcpy(local, l);
    strcpy(remote, r);

    debug((3, "argotel() - remote %s local %s\n", remote, local));

    /*
     * Accept the call, execute getconn and then fork
     * x25_argotel -p<port> -l<lsn> -x<local_nua> -r<remote_nua>
     */

    if ( x25accept(&c, info, NULL, NULL, r, l, X25NULLFN) < 0)
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s : CALL ACCEPT ERROR : %s\n", remote, msg);
        debug((1, "argotel() - x25accept() error %d %s\n", xerr, msg));
        return(-1);
    }

    debug((3, "argotel() - after accept remote %s local %s\n", remote, local));

    /*
     * get connection info to fork x25_argotel
     */

    if ( x25getconn(c, &port, &lsn) < 0 )
    {
        xerr = x25error(); x25errormsg(msg);

        errlog(X25_LOG, "%s - x25getconn error %d, %s\n", pname, xerr, msg);
        debug((1, "argotel() - x25getconn error %d %s\n", xerr, msg));
        return(-1);
    }

    debug((3, "argotel() - before fork remote %s local %s\n", remote, local));

    pid = fork();
    switch (pid)
    {
    case    -1:

        /*
         * unable to fork more processes
         */

        debug((1, "argotel() - Unable to fork! errno = %d\n", errno));
        errlog(INT_LOG, "%s : Fork Failed, errno %d\n", pname, errno);
        return(-1);
        break;

    case    0:

        /* exec new process to manage call  */


        if ( udata->xd_len != 0 )
        {
            debug((3, "argotel(child) - udata->xd_len %d\n", udata->xd_len));
            bin2hex(udata->xd_data, udata->xd_len, dummy);
            sprintf(udatastr, "-u%s", dummy);
            debug((3, "argotel(child) - udata %s\n", udatastr));
        }

        if ( facil->xd_len != 0 )
        {
            debug((3, "argotel(child) - facil->xd_len %d\n", facil->xd_len));
            bin2hex(facil->xd_data, facil->xd_len, dummy);
            sprintf(facilstr, "-f%s", dummy);
            debug((3, "argotel(child) - facil %s\n", facilstr));
        }

        debug((3, "argotel() - before fork remote %s local %s\n", remote, local));

        sprintf(portstr, "-p%d", port);
        sprintf(lsnstr, "-l%d", lsn);
        sprintf(localstr, "-x%s", local);
        sprintf(remotestr, "-r%s", remote);
        sprintf(debugstr, "-d%d", debuglevel);

        debug((3,
               "argotel(child) - %s port %s lsn %s loc %s rem %s debug %s udata %s faci %s\n",
               X25_ARGOTEL_PROC, portstr, lsnstr, localstr,
               remotestr, debugstr, udatastr, facilstr));

        if ( udata->xd_len )
        {
            if ( facil->xd_len )
            {
                execlp(X25_ARGOTEL_PROC, X25_ARGOTEL_PROC,
                       portstr, lsnstr, localstr, remotestr, debugstr,
                       udatastr, facilstr, 0);
            }
            else
            {
                execlp(X25_ARGOTEL_PROC, X25_ARGOTEL_PROC,
                       portstr, lsnstr, localstr, remotestr, debugstr,
                       udatastr, 0);
            }
        }
        else
        {
            if ( facil->xd_len )
            {
                execlp(X25_ARGOTEL_PROC, X25_ARGOTEL_PROC,
                       portstr, lsnstr, localstr, remotestr, debugstr,
                       facilstr, 0);
            }
            else
            {
                execlp(X25_ARGOTEL_PROC, X25_ARGOTEL_PROC,
                       portstr, lsnstr, localstr, remotestr, debugstr, 0);
            }
        }


        /* if reached means that execlp failed */

        debug((1, "argotel(child) execlp failed errno %d\n", errno));

        errlog(INT_LOG, "%s : UNABLE TO EXECLP %s , ERRNO %d\n",
               pname, X25_ARGOTEL_PROC, errno);
        sleep(3);
        exit(FAILURE);

        break;

    default:

        if (pid_push(pid, NULL, NULL) == -1)
        {
            /* pid table is full */

            errlog(INT_LOG, "%s : PID TABLE IS FULL - CONTINUING %d",
                   pname, pid);
        }

        debug((3, "argotel(father) - x25delconn(%d)\n", c));

        if ( x25delconn(c) < 0 )
        {
            xerr = x25error(); x25errormsg(msg);
            errlog(X25_LOG, "%s : x25delconn error %d, %s\n", pname, xerr, msg);
            debug((1, "argotel(father) - x25delconn error %d %s\n", xerr, msg));
        }

        return(0);
        break;
    }
}
