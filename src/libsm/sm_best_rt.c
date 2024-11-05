/*
 * $Id: sm_best_rt.c,v 1.1.1.1 1998/11/18 15:03:09 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: sm_best_rt.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:09  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/18  13:07:52  px25
 * Modified prototype of sm_get_best_route to take into account
 * field link too.
 *
 * Revision 1.1  1995/07/12  10:06:08  px25
 * Added TTY_NUA rt_type as possible nua types to route
 *
 * Revision 1.0  1995/07/07  10:02:52  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: sm_best_rt.c,v 1.1.1.1 1998/11/18 15:03:09 paul Exp $";

#define _LIBSM

/*  System include files                        */
#include    <stdio.h>
#include    <stdlib.h>
#include    <rpc/rpc.h>

/*  Project include files                       */

#include    "px25_globals.h"
#include "smp.h"

/*  Module include files                        */

#include    "sm.h"
#include        "group.h"

/*  Extern functions used                       */

/*  Extern data used                            */

extern CLIENT *cl;

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static char dummy[2] = { 0, 0 };

/*
 *
 * Procedure:  sm_get_best_route
 *
 * Parameters: hostname, route_type, group for ASY
 *
 * Description:
 *
 * Return:   struct BEST_ROUTE *
 *
 */

struct  BEST_ROUTE *sm_get_best_route(host, rt_type, ad)
char *host;
int rt_type;
void *ad;
{
    char *finger;
    char sm_host[BUFSIZ];
    static char link[BUFSIZ];
    static node n, *nP;
    static struct  BEST_ROUTE rt;
    struct  GROUP *grp = (struct GROUP *) ad;
    char *sync_line = (char *) ad;

    sm_errno = 0;

    if ( rt_type == SNA_NUA )
    {
        sm_errno = ESM_INVALID_RTTYPE;
        return((struct BEST_ROUTE *) NULL);
    }

    if ( cl == NULL )
    {
        if (( finger = getenv(SM_HOST)) == NULL )
        {
            sm_errno = ESM_NO_SM_HOST;
            return((struct BEST_ROUTE *) NULL);
        }

        strcpy(sm_host, finger);

        if ((cl = clnt_create(sm_host, RDBPROG, RDBVERS, "tcp")) == NULL)
        {
            /* il client handle non puo` essere creato il server non e` la`  */
            clnt_pcreateerror(sm_host);
            sm_errno = ESM_NO_SM;
            return((struct BEST_ROUTE *) NULL);
        }
    }

    /*
     * For new Group routing on ASY
     */

    if (( rt_type == ASY_NUA ) && ( grp == NULL ))
    {
        /* grp parameter is needed	*/
        sm_errno = ESM_NO_GROUP_NAME;
        return((struct BEST_ROUTE *) NULL);
    }


    n.machine   = host;
    n.route_type= rt_type;
    n.service = dummy;

    if ( rt_type == ASY_NUA )
    {
        if ( grp->grp_num == -1 )
        {
            sprintf(link, "%s", grp->grp_name);
        }
        else
        {
            sprintf(link, "%s,%d", grp->grp_name, grp->grp_num);
        }
        n.link = link;
    }
    else if ( rt_type == X25_NUA )
    {
        if ( sync_line != NULL )
        {
            strcpy(link, sync_line);
            n.link = link;
        }
        else
        {
            n.link = dummy;
        }
    }
    else
    {
        n.link = dummy;
    }

    n.active = 0;
    n.max = 0;

    nP = get_best_route_1(&n, cl);

    if ( nP->machine[0] != '\0')
    {
        strcpy(rt.hostname, nP->machine);
        strcpy(rt.service, nP->service);
        strcpy(rt.link, nP->link);
        return (&rt);
    }
    else
    {
        sm_errno = ESM_NO_ROUTE;
        return((struct BEST_ROUTE *) NULL);
    }
}

