/*
 * $Id: sm_create_rt.c,v 1.1.1.1 1998/11/18 15:03:08 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: sm_create_rt.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:08  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  10:02:52  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: sm_create_rt.c,v 1.1.1.1 1998/11/18 15:03:08 paul Exp $";

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


/*
 *
 * Procedure: sm_create_route
 *
 * Parameters: hostanme, link, service, active, max, route_type
 *
 * Description:
 *
 * Return: -10: no SM_HOST environment variable set,
 *        -11: create clnt handle failed
 *         -12: create_route_1 return NULL
 *      status: return value from create_route_1
 */

int sm_create_route(host, link, service, active, max, rt_type)
char *host;
char *link;
char *service;
int active;
int max;
int rt_type;
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
            return(-10);
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
    n.service   = service;
    n.active        = active;
    n.max           = max;
    n.route_type= rt_type;

    if (( status = create_route_1(&n, cl)) != NULL )
    {
        return(*status);
    }
    else
    {
        return(-12);
    }
}
