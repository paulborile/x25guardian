/*
 * $Id: sm_enddebug.c,v 1.1.1.1 1998/11/18 15:03:09 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: sm_enddebug.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:09  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  10:02:52  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: sm_enddebug.c,v 1.1.1.1 1998/11/18 15:03:09 paul Exp $";

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
 * Procedure: sm_enddebug.c
 *
 * Parameters:
 *
 * Description:
 *
 * Return:
 */

int sm_enddebug()
{
    char *finger;
    char sm_host[BUFSIZ];
    static int *status;

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

    if (( status = end_debug_1((void *)NULL, cl)) != NULL )
    {
        return(*status);
    }
    else
    {
        return(-12);
    }
}
