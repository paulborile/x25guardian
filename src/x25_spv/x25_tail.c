/*
 * $Id: x25_tail.c,v 1.1.1.1 1998/11/18 15:03:13 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_spv
 *
 * Contents: tail functions for spv
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_tail.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:13  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/09/28  15:48:18  giorgio
 * Added tail logging of datascope files
 *
 * Revision 1.0  1995/07/14  09:19:17  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_tail.c,v 1.1.1.1 1998/11/18 15:03:13 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <time.h>

/*  Project include files                       */

#include    "px25_globals.h"

/*  Module include files                        */

/*  Extern functions used                       */

extern char editor[];
extern char pager[];

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

taillog()
{
    char *finger;
    char env[BUFSIZ];
    char file_name[BUFSIZ];
    char dummy[BUFSIZ];
    char day[20];
    char command[BUFSIZ];
    char command2[BUFSIZ];
    char dscope_name[BUFSIZ];
    char dscope_file[BUFSIZ];
    char file[BUFSIZ];
    long clock;
    int choice = -1;
    struct  tm *t;

    day[0] = 0;

    if (( finger = getenv(PX25_LOG_DIR)) == NULL )
    {
        printf("Environment variable %s not set\n", PX25_LOG_DIR);
        return(0);
    }
    strcpy(file_name, finger);

    if (( finger = getenv(PX25_DSCOPE_DIR)) == NULL )
    {
        printf("Environment variable %s not set\n", PX25_LOG_DIR);
        return(0);
    }
    strcpy(dscope_name, finger);

    printf("x25, int, asy, dscope : ");
    fflush(stdout);
    gets(file);

    switch (file[0])
    {
    case    'x':
        strcpy(dummy, "/x25.log-");
        break;
    case    'i':
        strcpy(dummy, "/int.log-");
        break;
    case    'a':
        strcpy(dummy, "/asy.log-");
        break;
    case    'd':
        choice = 0;
        break;
    default:

        printf("What ? %s ?\n", file);
        return(0);
    }

    if (choice == 0)
    {
        printf("Datascope files available ...\n");
        sleep(1);
        sprintf(command2, "cd %s; ls -l *-i *-o", dscope_name);
        system(command2);
        printf("Which file ? : ");
        fflush(stdout);
        gets(dscope_file);
        sprintf(command, "tail -f %s/%s", dscope_name, dscope_file);
    }
    else
    {
        time(&clock);
        t = localtime(&clock);

        printf("which day (enter for today) : ");
        fflush(stdout);
        gets(day);

        if ( day[0] == '\0' )
        {
            sprintf(day, "%02d", t->tm_mday);
        }

        sprintf(command, "tail -f %s%s%s", file_name, dummy, day);
    }

    system(command);
    return(0);
}


