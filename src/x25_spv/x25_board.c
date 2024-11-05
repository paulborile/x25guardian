/*
 * $Id: x25_board.c,v 1.1.1.1 1998/11/18 15:03:13 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_spv
 *
 * Contents: get status information on center
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_board.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:13  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/28  15:41:06  giorgio
 * Added possibility to choose card to start
 *
 * Revision 1.1  1995/09/18  13:53:40  px25
 * Added config option for board command
 *
 * Revision 1.0  1995/07/14  09:19:17  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_board.c,v 1.1.1.1 1998/11/18 15:03:13 paul Exp $";

/*  System include files                        */

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

/*  Project include files                       */

#include    "px25_globals.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

extern char eicon_dir[];

/*  Local constants                             */

/*  Local types                                 */

struct lines
{
    int number_of_lines;
    int lines[50];
};
typedef  struct lines lines;

/*  Local functions used                        */

lines *   board_number();

/*  Local macros                                */

/*  Local data                                  */


board ()
{
    char show[BUFSIZ];
    char board[BUFSIZ];
    char command[BUFSIZ];
    lines *sync_lines;
    int i;

    printf("start,stop,config : ");
    fflush(stdout);
    gets(show);

    if (strcmp(show, "start") ==0)
    {
        sync_lines = board_number();

        if (sync_lines->number_of_lines == 0)
        {
            printf("Errors or No sync lines in configuration file \"sync.tab\"\n");
            return(0);;
        }
        else
        {
            for (i=0; i<sync_lines->number_of_lines; ++i)
            {
                printf("%d) port %d\t",
                       sync_lines->lines[i], sync_lines->lines[i]);
            }
            printf("a) all\t");
            printf(":");
        }

        fflush(stdout);
        gets(board);

        if (board[0] == 'a')
        {
            if (strlen(board) == 1)
            {
                sprintf(command, "eccard start");
            }
            else if (strcmp(board, "all") != 0)
            {
                printf("Don't know how to make %s\n", board);
                return(0);
            }
            else{
                sprintf(command, "eccard start");
            }
        }
        else{
            sprintf(command, "eccard start -e %d", atoi(board));
        }

        system(command);
        return(0);
    }
    else if (strcmp(show, "stop") ==0)
    {
        system("eccard stop");
        return(0);
    }
    else if (strcmp(show, "config") ==0)
    {
        sprintf(command, "cd %s ; eccfg", eicon_dir);
        system(command);
        return(0);
    }
    else
    {
        printf("What ? %s ?\n", show);
        return(0);
    }
}

/*
 *
 * Procedure: board_number
 *
 * Parameters: none
 *
 * Description: get the number of sync line in sync.tab
 *
 * Return: sync lines number or -1 if unsuccesful
 *
 */

lines *   board_number()
{
    char *finger;
    FILE *fp;
    char *tok;
    char buf[BUFSIZ];
    lines line_def;
    int i = 0;

    line_def.number_of_lines = 0;

    /*
     * Load the sync table
     */

    if (( finger = getenv(PX25_SYN_TABLE_PATH)) == NULL )
    {
        return &line_def;
    }
    if (( fp = fopen(finger, "r")) == NULL )
    {
        return &line_def;
    }

    while ( fgets(buf, BUFSIZ, fp) != NULL )
    {
        if (( buf[0] == '#' ) || ( buf[0] == '\n' ))
        {
            continue;
        }
        else
        {
            if (strchr(buf, ':') != 0)
            {
                if ((tok = strtok(buf, ":")) != NULL )
                {
                    ++line_def.number_of_lines;
                    line_def.lines[i] = atoi(tok);
                    ++i;
                }
            }
        }
    }

    fclose(fp);
    return &line_def;
}

