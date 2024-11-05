
/*
 * $Id: sm_free_rt.c,v 1.1.1.1 1998/11/18 15:03:09 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: sm_free_rt.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:09  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  10:02:52  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: sm_free_rt.c,v 1.1.1.1 1998/11/18 15:03:09 paul Exp $";

#define _LIBSM

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <rpc/rpc.h>

/*  Project include files                       */

#include    "smp.h"

/*  Module include files                        */

#include        "sm.h"

/*  Extern functions used                       */

/*  Extern data used                            */

extern CLIENT *cl;

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static char dummy[2];


/*
 *
 * Procedure: sm_free_route
 *
 * Parameters: hostname, link
 *
 * Description:
 *
 * Return:
 *
 */

int sm_free_route(host, link)
char *host;
char *link;
{
    char *finger;
    char sm_host[BUFSIZ];
    static int *status;
    static node n;


    if ( cl == NULL )
    {
        if (( finger = getenv(SM_HOST)) == NULL )
        {
            sm_errno = ESM_NO_SM_HOST;
            return (-10);
        }

        strcpy(sm_host, finger);

        if ((cl = clnt_create(sm_host, RDBPROG, RDBVERS, "tcp")) == NULL)
        {
            /* il client handle non puo` essere creato il server non e` la`  */
            clnt_pcreateerror(sm_host);
            sm_errno = ESM_NO_SM;
            return(-11);
        }
    }

    n.machine   = host;
    n.link      = link;
    n.service   = dummy;
    n.active        = 0;
    n.max           = 0;
    n.route_type    = 0;

    if (( status = free_route_1(&n, cl)) != NULL )
    {
        sm_errno = 0;
        return(*status);
    }
    else
    {
        return(-12);
    }
}