/*
 * $Id: x25_incall_rec.c,v 1.1.1.1 1998/11/18 15:03:29 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:  x25_incall
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_incall_rec.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:29  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/18  13:51:55  px25
 * Added managment of QBIT
 * Better Diag on CLR commands.
 *
 * Revision 1.1  1995/07/14  09:14:25  px25
 * New get_param function used to retrieve general parameters
 * Datascope functions added.
 * Get_command_line() modified to accept local and remote nua.
 *
 * Revision 1.0  1995/07/07  10:19:48  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_incall_rec.c,v 1.1.1.1 1998/11/18 15:03:29 paul Exp $";

/*  System include files                        */

#include <stdio.h>
#include <signal.h>
#include <eicon/x25.h>
#include <eicon/neterr.h>

/*  Project include files                       */

#include    "px25_globals.h"

/*  Module include files                        */

#include "debug.h"
#include "errlog.h"

/*  Extern functions used                       */

extern int child_sigterm();

/*  Extern data used                            */

extern int info;
extern int port;
extern int lsn;
extern int sd;
extern char pname[];
extern char *outbuf;
extern int PID;
extern char remote[], local[];
extern char x25name[];
extern int datascope;

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

int cid1;
int xerr;
char buf[BUFSIZ];
char msg[ENETMSGLEN];                     /* x25 string. */

/*
 *
 * Procedure: x25_receiver
 *
 * Parameters:
 *
 * Description:
 *
 * Return: 0 if success, -1 otherwise
 *
 */

int x25incall_rec()
{
    char dummy[BUFSIZ];
    struct  PKT_HDR *h = ( struct PKT_HDR * ) buf;
    int rc;
    int xrc;
    int drc;

    debug((3, "%s_rec() - port %d lsn %d\n", pname, port, lsn ));

    signal(SIGTERM, (void (*)()) child_sigterm);

    if (( rc = x25mkconn(&cid1, port, lsn, X25WAIT)) < 0 )
    {
        xerr = x25error();
        x25errormsg(msg);
        errlog(X25_LOG, "%s_rec - x25mkconn() error %d, %s\n",
               pname, xerr, msg);
        debug((1, "%s_rec() - x25mkconn() error %d, %s\n", pname, xerr, msg));
        close(sd);
        return(-1);
    }

    debug((3, "%s_rec() - x25mkconn OK with cid %d, port %d, lsn %d\n",
           pname, cid1, port, lsn));

    while (( rc = mos_recv(sd, buf, BUFSIZ)) > 0 )
    {
        debug((3, "%s_rec() : mos_recv OK, read %d\n", pname, rc));

        switch (h->pkt_code)
        {
        case    ASY_DATA_PACKET:

            info = 0;

#ifdef  CHECKS

            if (((rc - sizeof(struct PKT_HDR)) > X25_DATA_PACKET_SIZE ) ||
                ( h->pkt_len > X25_DATA_PACKET_SIZE ))
            {
                errlog(INT_LOG, "%s : PACKET TOO BIG (%d) RECVD PKT_LEN %d\n",
                       pname, rc - sizeof(struct PKT_HDR), h->pkt_len);

                continue;
            }
#endif

            if (h->flags == MORE_BIT)
            {
                info = info | XI_MBIT;
                debug((3, "%s_rec() : ASY_DATA_PACKET - MORE BIT %d\n",
                       pname, rc));
            }
            if (h->flags == QBIT)
            {
                info = info | XI_QBIT;
                debug((3, "%s_rec() : ASY_DATA_PACKET - QBIT %d\n",
                       pname, rc));
            }
            if ( (h->flags != MORE_BIT) && (h->flags != QBIT) )
            {
                info = 0;
                debug((3, "%s_rec() : ASY_DATA_PACKET %d\n", pname, rc));
            }

            memcpy(outbuf, buf + sizeof(struct PKT_HDR),
                   rc - sizeof(struct PKT_HDR));

            debug((3, "%s_rec() : x25sending %d bytes\n",
                   pname, rc - sizeof(struct PKT_HDR)));

            if (( xrc = x25send(cid1, outbuf, rc - sizeof(struct PKT_HDR),
                                info, X25NULLFN)) < 0 )
            {
                xerr = x25error();
                x25errormsg(msg);
                errlog(X25_LOG, "%s_rec : x25send() error %d, %s\n",
                       pname, xerr, msg);
                debug((1, "%s_rec() - x25send() error %d, %s\n",
                       pname, xerr, msg));
                drc = x25delconn(cid1);
                debug((1, "%s_rec() - x25delconn() ret %d\n", pname, drc));
                close(sd);
                return(-1);
            }
            if (datascope == 1)
            {
                if (dscope_log(x25name, outbuf, rc - sizeof(struct PKT_HDR), 'o') == -1)
                {
                    errlog(INT_LOG, "%s : unable to open dscope file\n",
                           pname);
                    debug((1, "Unable to open dscope file\n"));
                }
            }

            debug((3, "%s_rec() -  x25send() OK rc = %d\n", pname, xrc));
            break;

        case  X25_DATA_PACKET:

            info = 0;

            if ( h->flags == MORE_BIT )
            {
                info = info | XI_MBIT;
                debug((3, "%s_rec : mos_recv() - setting more bit\n", pname));
            }
            if ( h->flags == QBIT )
            {
                info = info | XI_QBIT;
                debug((3, "%s_rec : mos_recv() - setting qbit\n", pname));
            }
            if ( h->flags != MORE_BIT && h->flags != QBIT )
            {
                info = 0;
                debug((3, "%s_rec() : X25_DATA_PACKET %d\n", pname, rc));
            }

            memcpy(outbuf, buf + sizeof(struct PKT_HDR),
                   rc - sizeof(struct PKT_HDR));

            if ((xrc = x25send(cid1, outbuf, rc - sizeof(struct PKT_HDR),
                               info, X25NULLFN)) < 0)
            {
                xerr = x25error();
                x25errormsg(msg);

                errlog(X25_LOG, "%s : ERROR SENDING PACKET : %s\n", remote, msg);

                errlog(INT_LOG, "%s_rec : x25send() error %d, %s\n",
                       pname, xerr, msg);
                debug((1, "%s_rec : x25send() error %d, %s\n",
                       pname, xerr, msg));
                drc = x25delconn(cid1);
                debug((1, "%s_rec() - x25delconn() ret %d\n", pname, drc));
                close(sd);
                return(-1);
            }
            if (datascope == 1)
            {
                if (dscope_log(x25name, outbuf, rc - sizeof(struct PKT_HDR), 'o') == -1)
                {
                    errlog(INT_LOG, "%s : unable to open dscope file\n",
                           pname);
                    debug((1, "Unable to open dscope file\n"));
                }
            }

            debug((3, "%s_rec : x25send() OK rc = %d\n", pname, xrc));
            break;

        case    ASY_ERROR:

            memcpy(dummy, buf + sizeof(struct PKT_HDR),
                   rc - sizeof(struct PKT_HDR));

            dummy[rc - sizeof(struct PKT_HDR)] = '\0';
            errlog(X25_LOG, "%s : ASYNC ERROR : %s \n", remote, dummy);

        case    ASY_CONNECTION_END:

            debug((3, "%s_rec() : ASY_CONNECTION_END\n", pname));

            /* errlog(X25_LOG, "%s : ASYNC CONNECTION DOWN\n", remote); */

            if (x25hangup(cid1, NULL, XH_IMM, X25NULLFN) < 0)
            {
                x25errormsg(msg);
                errlog(INT_LOG, "%s_rec - x25hangup error %d, %s\n",
                       pname, x25error(), msg);

                debug((1, "%s_rec - x25hangup failed %d, %s\n",
                       pname, x25error(), msg));
            }

            drc = x25delconn(cid1);
            debug((1, "%s_rec() - x25delconn() ret %d\n", pname, drc));

            close(sd);
            return(-1);

            break;

        case  X25_ERROR:

            /* close outgoing connection on X25 SVC   */
            /* Da verificare con riccardo se fare una chiusura diversa
                da end_connection                      */

            break;

        case  X25_CONNECTION_END:

            /* close outgoing connection on X25 SVC   */

            break;

        default:

            debug((1, "%s_rec- Unknown packet %02x received\n",
                   pname, h->pkt_code));
            errlog(INT_LOG, "%s_rec : UNKNOWN INTERNAL PKT %02x RECEIVED\n",
                   pname, h->pkt_code);
        }
    }


    debug((1, "%s_rec() -  mos_recv rc = %d\n", pname, rc));

    /* mos_recv has returned < 0 */
    /* LINK DOWN ? */

    if (x25hangup(cid1, NULL, XH_IMM, X25NULLFN) < 0)
    {
        x25errormsg(msg);
        errlog(INT_LOG, "%s_rec - x25hangup error %d, %s\n",
               pname, x25error(), msg);

        debug((1, "%s_rec - x25hangup failed %d, %s\n",
               pname, x25error(), msg));
    }

    /* end connection	*/
    close(sd);

    debug((3, "%s_rec() - return -1\n", pname));
    return(-1);
}

/*
 *
 *  Procedure: child_sigterm
 *
 *  Parameters: none
 *
 *  Description: close sd and exit
 *
 *  Return:
 *
 */

child_sigterm(sig)
int sig;
{
    int rc;
    debug((1, "child_sigterm() - %s terminating\n",
           (PID == 0) ? "child" : "father"));

    if ( sd != -1 )
    {
        close(sd);
    }

    if ( PID == 0 )
    {
        debug((1, "child_sigterm() - child executing delconn\n"));
        rc = x25hangup(cid1, NULL, XH_IMM, X25NULLFN);
        debug((1, "child_sigterm() - child hangup ret %d\n", rc));
        rc = x25delconn(cid1);
        debug((1, "child_sigterm() - child delconn ret %d\n", rc));
    }

    if (datascope)
    {
        if ( PID == 0 )
        {
            dscope_mark(x25name, "end write", 'o');
        }
        else
        {
            dscope_mark(x25name, "end read", 'i');
        }
    }

    exit(0);
}

