/*
 * $Id: x25_status.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_spv
 *
 * Contents: get status information on center
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_status.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:14  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/28  15:46:22  giorgio
 * Delete control over asy-lines with "sp_watch" (going to panic the machine !)
 * Added control on single x25 lines
 *
 * Revision 1.1  1995/09/18  13:56:50  px25
 * Added monitoring of async lines.
 * Added check of how many boards are installed.
 *
 * Revision 1.0  1995/07/14  09:19:17  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_status.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $";

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

struct lines
{
    int number_of_lines;
    int lines[50];
};
typedef struct lines lines;

/*  Local functions used                        */

lines *  line_number();

/*  Local macros                                */

/*  Local data                                  */


status ()
{
    char show[BUFSIZ];
    char board[BUFSIZ];
    char command[50];
    FILE *fp;
    char buffer[BUFSIZ];
    char status[BUFSIZ];
    lines *sync_lines;
    int number, i;

    printf("route, hdlc, x25, boards, process : ");
    fflush(stdout);
    gets(show);

    switch (show[0])
    {
    case    'r':
        sprintf(buffer, "/tmp/sm%d", getpid());

        if ( sm_dump_route(buffer) < 0 )
        {
            printf("Unable to get status\n");
            break;
        }

        if (( fp = fopen(buffer, "r")) == NULL )
        {
            perror(buffer);
            break;
        }
        while ( fgets(status, BUFSIZ, fp) != NULL )
        {
            printf("%s", status);
        }

        fclose(fp);
        unlink(buffer);
        break;

    case    'h':

        sync_lines = line_number();

        if (sync_lines->number_of_lines == 0)
        {
            printf("Errors or No sync lines in configuration file \"sync.tab\"\n");
            break;
        }
        else
        {
            for (i=0; i<sync_lines->number_of_lines; ++i)
            {
                printf("%d) hdlc port %d\t",
                       sync_lines->lines[i], sync_lines->lines[i]);
            }
            printf(":");
        }

        fflush(stdout);
        gets(board);

        sprintf(command, "ecmodule status hdlc -p %d", atoi(board));
        system(command);

        break;

    case    'x':

        sync_lines = line_number();

        if (sync_lines->number_of_lines == 0)
        {
            printf("Errors or No sync lines in configuration file \"sync.tab\"\n");
            break;
        }
        else
        {
            for (i=0; i<sync_lines->number_of_lines; ++i)
            {
                printf("%d) port %d\t",
                       sync_lines->lines[i], sync_lines->lines[i]);
            }
            printf(":");
        }

        fflush(stdout);
        gets(board);

        sprintf(command, "ecmodule status x25 -p %d", atoi(board));
        system(command);

        break;

    case    'b':

        system("eccard status");
        break;

    case    'p':

        system("ps -fu px25");
        break;

    default:

        printf("What ? %s ?\n", show);
        return(0);
    }
    return(0);
}

/*
 *
 * Procedure: line_number
 *
 * Parameters: none
 *
 * Description: get the number of sync line in sync.tab
 *
 * Return: sync lines number or -1 if unsuccesful
 *
 */

lines *  line_number()
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
