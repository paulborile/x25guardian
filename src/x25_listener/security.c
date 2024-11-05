/*
 * $Id: security.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: security.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:25  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/09/18  12:50:44  px25
 * Check is done now on source nua only on partial matching.
 * Longer nuas are accepted because of sub-nua routing.
 *
 * Revision 1.0  1995/07/12  13:52:01  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: security.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <x25.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "errlog.h"
#include        "debug.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

extern char mynua[];
extern int listenport;
extern char myhostname[];

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

/*
 *
 *  Procedure: security_check
 *
 *  Parameters: remote nua, local nua, facilities, userdata
 *
 *  Description: do some security checks
 *
 *  Return: TRUE FALSE
 *
 */

int security_check(remote, local, facil, userd)
char *remote;
char *local;
struct  x25data *facil;
struct  x25data *userd;
{
    debug((3, "security_check() - rem %s loc %s mynua %s\n",
           remote, local, mynua));
    /*
     * check if nua called is really our nua
     */

    if ( mynua[0] != '\0' )
    {
        if ( strncmp(mynua, local, strlen(mynua)) != 0 )
        {
            /* called nua and my nua do not match	*/

            errlog(X25_LOG, "%s : %s:%d INCOMING CALL TO NUA (%s) NOT PERMITTED\n",
                   remote, myhostname, listenport, local);
            return(-1);
        }
    }

    /*
     * TODO : check out black lists
     */

    return(1);
}
