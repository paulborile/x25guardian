/*
 * $Id: x25_delete.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_spv
 *
 * Contents: delete init and x29 file
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_delete.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:14  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/09/29  17:25:47  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_delete.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $";

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

delete()
{
    char *finger;
    char file_name[BUFSIZ];
    char command[BUFSIZ];
    char file[BUFSIZ];

    if (( finger = getenv(PX25_X29_DIR)) == NULL )
    {
        printf("Environment variable %s not set\n", PX25_X29_DIR);
        return(0);
    }

    strcpy(file_name, finger);

    printf("init_asy, x29 : ");
    fflush(stdout);
    gets(file);

    if ( strcmp(file, "init_asy") == 0 )
    {
        sprintf(command, "cd %s; rm -i *.init", file_name);
    }
    else
    {
        sprintf(command, "cd %s; rm -i *.x29", file_name);
    }

    system(command);
    return(0);
}


