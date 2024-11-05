/*
 * $Id: x25_enable.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_spv
 *
 * Contents: Enable tty port after their deletion with sm_delete_route
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_enable.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:14  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/09/29  14:33:49  giorgio
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_enable.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $";

/*  System include files                        */

#include    <stdio.h>
#include    <stdlib.h>
#include        <string.h>
#include    <rpc/rpc.h>
#include    "sm.h"

/*  Project include files                       */

#include    "px25_globals.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local functions used                        */

/*  Local macros                                */

/*  Local data                                  */


asy_enable()
{
    char host[32];
    char tty[32];
    int rc;

    printf("Hostname:");
    fflush(stdout);
    gets(host);
    printf("TTY:");
    fflush(stdout);
    gets(tty);

    rc = sm_create_route(host, tty, "px25asynmgr", 0, 1, ASY_NUA);
    if ( rc < 0 )
    {
        printf("Cannot enable tty line %s on host %s\n", tty, host);
        return(0);
    }
}
