/*
 * $Id: x25_edit.c,v 1.1.1.1 1998/11/18 15:03:13 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_spv
 *
 * Contents: edit functions for
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_edit.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:13  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.3  1995/09/29  17:27:19  px25
 * Added chmod +x on init files
 *
 * Revision 1.2  1995/09/28  15:42:32  giorgio
 * Added editing on asy-init files and x29 files
 *
 * Revision 1.1  1995/09/18  13:54:27  px25
 * Added editing of sun-nua. No more x3 parameters.
 *
 * Revision 1.0  1995/07/14  09:19:17  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_edit.c,v 1.1.1.1 1998/11/18 15:03:13 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include    <unistd.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "group.h"

/*  Module include files                        */

/*  Extern functions used                       */

extern char editor[];
extern char bineditor[];

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

edit()
{
    char *finger;
    char env[BUFSIZ];
    char file_name[BUFSIZ];
    char command[BUFSIZ];
    char file[BUFSIZ];
    char group[BUFSIZ];
    char line[BUFSIZ];
    int choice = -1;

    printf("general, sync, async, nua-subrouting, routing, init-asy, x29 : ");
    fflush(stdout);
    gets(file);

    switch (file[0])
    {
    case    'n':
        strcpy(env, PX25_SUB_RT_TABLE_PATH);
        break;
    case    'g':
        strcpy(env, PX25_GEN_TABLE);
        break;
    case    's':
        strcpy(env, PX25_SYN_TABLE_PATH);
        break;
    case    'r':
        strcpy(env, PX25_RT_TABLE_PATH);
        break;
    case    'a':
        strcpy(env, PX25_ASY_TABLE_PATH);
        break;
    case  'i':
        strcpy(env, PX25_X29_DIR);
        choice = 0;
        break;
    case  'x':
        strcpy(env, PX25_X29_DIR);
        choice = 1;
        break;
    default:

        printf("What ? %s ?\n", file);
        return(0);
    }

    if (( finger = getenv(env)) != NULL )
    {
        strcpy(file_name, finger);
    }
    else
    {
        printf("Environment variable %s not found\n", env);
        return(0);
    }

    if (choice == 0)
    {
        printf("Group name:");
        fflush(stdout);
        gets(group);
        printf("Line number:");
        fflush(stdout);
        gets(line);
        strcat(file_name, group);
        strcat(file_name, ",");
        strcat(file_name, line);
        strcat(file_name, ".init");
        if ( access(file_name, F_OK) != 0 )
        {
            printf("File %s not found!\n", file_name);
            printf("Going to create it....\n");
            sleep(3);
        }
        sprintf(command, "%s %s", editor, file_name);
    }
    else if (choice == 1)
    {
        printf("Group name:");
        fflush(stdout);
        gets(group);
        printf("Line number:");
        fflush(stdout);
        gets(line);
        strcat(file_name, group);
        strcat(file_name, ",");
        strcat(file_name, line);
        strcat(file_name, ".x29");
        if ( access(file_name, F_OK) != 0 )
        {
            printf("File %s not found!\n", file_name);
            printf("Going to create it....\n");
            sleep(3);
        }
        sprintf(command, "%s %s", bineditor, file_name);
    }
    else{
        sprintf(command, "%s %s", editor, file_name);
    }

    system(command);

    if ( access(file_name, F_OK) == 0 )
    {
        sprintf(command, "chmod +x %s", file_name);
        system(command);
    }

    return(0);
}


