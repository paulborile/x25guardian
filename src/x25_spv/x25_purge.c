/*
 * $Id: x25_purge.c,v 1.1.1.1 1998/11/18 15:03:13 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_spv
 *
 * Contents: purge functions for spv
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_purge.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:13  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/29  17:26:56  px25
 * Restyling
 *
 * Revision 1.1  1995/09/28  15:43:37  giorgio
 * Addedd management of PX25_DSCOPE_DIR
 *
 * Revision 1.0  1995/07/14  09:19:17  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_purge.c,v 1.1.1.1 1998/11/18 15:03:13 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>

/*  Project include files                       */

#include    "px25_globals.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

purgelog()
{
    char *finger;
    char file_name[BUFSIZ];
    char command[BUFSIZ];
    char file[BUFSIZ];

    if (( finger = getenv(PX25_DSCOPE_DIR)) == NULL )
    {
        printf("Environment variable %s not set\n", PX25_LOG_DIR);
        return(0);
    }

    strcpy(file_name, finger);

    printf("dscope, debug : ");
    fflush(stdout);
    gets(file);

    if ( strcmp(file, "dscope") == 0 )
    {
        sprintf(command, "cd %s; rm -i *-i *-o", file_name);
    }
    else
    {
        sprintf(command, "rm -i /tmp/*.debug");
    }

    system(command);
    return(0);
}


