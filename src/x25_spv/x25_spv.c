/*
 * $Id: x25_spv.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: supervisor!
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_spv.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:14  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.3  1995/09/29  17:26:26  px25
 * Added asy_enable and delete functions
 *
 * Revision 1.2  1995/09/28  11:22:44  px25
 * Added control over dscope files
 * Added control over eicon working directory
 *
 * Revision 1.1  1995/09/18  13:55:54  px25
 * New commands
 *
 * Revision 1.0  1995/07/14  09:19:17  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_spv.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>

/*  Project include files                       */

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

#define HOSTNAME_LEN    32

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

char editor[32];
char pager[32];
char bineditor[32];
char eicon_dir[32];

main()
{
    long clock;
    char *finger;
    char myhostname[HOSTNAME_LEN];
    char command[BUFSIZ];
    char prevcmd[BUFSIZ];

    time(&clock);
    printf("PX25 - Router : %s", ctime(&clock));

    if ( gethostname(myhostname, HOSTNAME_LEN) < 0 )
    {
        printf("Unable to get hostname\n");
        strcpy(myhostname, "unknown");
    }

    if (( finger = getenv("EDITOR")) != NULL )
    {
        strcpy(editor, finger);
    }
    else
    {
        strcpy(editor, "vi");
    }

    if (( finger = getenv("PAGER")) != NULL )
    {
        strcpy(pager, finger);
    }
    else
    {
        strcpy(pager, "more");
    }
    if (( finger = getenv("BINEDITOR")) != NULL )
    {
        strcpy(bineditor, finger);
    }
    else
    {
        strcpy(bineditor, "beav");
    }
    if (( finger = getenv("EICON_DIR")) != NULL )
    {
        strcpy(eicon_dir, finger);
    }
    else
    {
        strcpy(eicon_dir, "/var/eicon");
    }

    while (1)
    {
        printf("%s -> ", myhostname);
        fflush(stdout);
        command[0] = '\0';
        gets(command);

        if ( command[0] == '\0' )
        {
            continue;
        }

        if ( strcmp(command, "!!") == 0 )
        {
            strcpy(command, prevcmd);
        }

        /*
         * Command search start here
         */

        if ( strcmp(command, "edit") == 0 )
        {
            edit();
        }
        else if ( strcmp(command, "status") == 0 )
        {
            status();
        }
        else if ( strcmp(command, "showlog") == 0 )
        {
            showlog();
        }
        else if ( strcmp(command, "taillog") == 0 )
        {
            taillog();
        }
        else if ( strcmp(command, "purgelog") == 0 )
        {
            purgelog();
        }
        else if ( strcmp(command, "board") == 0 )
        {
            board();
        }
        else if ( strcmp(command, "help") == 0 )
        {
            help();
        }
        else if ( strcmp(command, "asy_enable") == 0 )
        {
            asy_enable();
        }
        else if ( strcmp(command, "delete") == 0 )
        {
            delete();
        }
        else if ( strcmp(command, "exit") == 0 )
        {
            printf("Terminating\n");
            exit(0);
        }
        else
        {
            system(command);
        }

        if ( strcmp(command, prevcmd) != 0 )
        {
            strcpy(prevcmd, command);
        }
    }
}

help()
{
    printf("edit      {general,sync,async,nua-subrouting,routing,init-asy,x29}\n");
    printf("delete    {init_asy,x29}\n");
    printf("status    {route,hdlc,x25,boards,process}\n");
    printf("showlog   {x25,int,asy,dscope}\n");
    printf("taillog   {x25,int,asy,dscope}\n");
    printf("purgelog  {dscope,debug}\n");
    printf("board     {start,stop,config}\n");
    printf("asy_enable\n");
    printf("x25_start [-s]\n");
    printf("x25_stop\n");
    printf("help\n");
}
