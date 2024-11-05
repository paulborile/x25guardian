/*
 * $Id: getparam.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: getparam.c
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: getparam.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:23  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/12  14:25:11  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: getparam.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <errno.h>
#include    <ctype.h>
#include    "sys/types.h"
#include    "sys/time.h"

/*  Project include files                       */

#include        "gp.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

#define MAX_STR 250
#define SEPARATOR   '='

/*  Local types                                 */

/*  Local macros                                */

#ifdef DEBUG
#   define print(a) printf a
#else
#   define print(a)
#endif

/*  Local data                                  */

int gp_errno;

/*
 *
 * Procedure: get_param
 *
 * Parameters: file , parameter
 *
 * Description: get the value of parameter from the file specified
 *
 * Return: -1 if param not found (or error ) or the value ( >=0 ) of parameter
 *
 */

int get_param(par_file, par_name)
char *par_file, *par_name;
{
    FILE *fp;
    char line[MAX_STR];
    char *finger;
    char *valuestr;
    int value;
    int param_found = 0;

    print(("get_param() - called for file %s and param %s\n",
           par_file, par_name));

    if (( fp = fopen(par_file, "r")) == NULL )
    {
        /* cannot open file  */
        gp_errno = EGP_CANNOT_OPEN;
        print(("get_param() - cannot open file %s\n", par_file));
        return(-1);
    }

    while (fgets(line, MAX_STR, fp) != NULL)
    {

        if ((line[0] == '#') || (line[0] == '\n'))
        {
            continue;
        }

        if ( !isalpha(line[0]))
        {
            continue;
        }

        if (( finger = strchr(line, '\n')) != NULL )
        {
            *finger = '\0';
        }
        if (( finger = strchr(line, SEPARATOR)) == NULL )
        {
            continue;
        }

        *finger = '\0';

        if (strcmp(line, par_name) == 0)
        {
            print(("get_param() - parameter %s found!! value %s\n",
                   line, finger+1 ));

            if ( strlen(finger+1) != 0 )
            {
                value = atoi(finger+1);
                print(("get_param() - value %d\n", value));
                fclose(fp);
                return(value);
            }
            else
            {
                gp_errno = EGP_NO_VALUE;
                print(("get_param() - value not found\n"));
                fclose(fp);
                return(-1);
            }
        }
    }

    print(("get_param() - parameter %s not found\n", par_name));
    gp_errno = EGP_NO_PARAM;
    print(("get_param() - ending...\n"));
    fclose(fp);
    return (-1);
}
