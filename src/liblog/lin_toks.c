/*
 * $Id: lin_toks.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $
 *
 * Project : PX25
 *
 * Module: Logh library
 *
 * Contents: colon separated strings management functions
 *
 * Author(s): C.U. S.r.l - P.S.Borile
 *
 * $Log: lin_toks.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:23  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/11  10:13:53  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: lin_toks.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $";

/* System includes	*/

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

/* Local includes	*/

#define     MAX_FIELDS_PER_LINE     10
#define     SEPARATOR                   ':'

static char *fields[MAX_FIELDS_PER_LINE];

/*
 *
 * Procedure: lin_toks
 *
 * Parameters: colon separated buffer of fields
 *
 * Description: return list of string fields in buffer
 *
 * Return: (char ** ) 0 on error
 *
 */

char **lin_toks(buffer, num_of_fields)
char *buffer;
int num_of_fields;
{
    int i;
    int fields_count = 0;
    char *cur, *prev;
    char lbuf[BUFSIZ];


    for (i = 0; i < MAX_FIELDS_PER_LINE; i++ )
    {
        if ( fields[i] != NULL )
        {
            free(fields[i]);
            fields[i] = NULL;
        }
    }

    strcpy(lbuf, buffer);

    cur = prev = lbuf;

    while ( *cur != '\0' )
    {
        if (( *cur == SEPARATOR ) || ( *cur == '\n' ))
        {
            /* found field : fields goes from prev -> cur-1	*/

            if (( cur - prev ) == 0 )
            {
                /* found 0 len field	*/

                if (( fields[fields_count] = malloc(1)) == NULL )
                {
                    return((char **) 0 );
                }
                fields[fields_count][0] = '\0';
            }
            else
            {
                if (( fields[fields_count] = malloc(cur - prev + 1)) == NULL )
                {
                    return((char **) 0 );
                }

                strncpy(fields[fields_count], prev, cur - prev);
                fields[fields_count][cur - prev] = '\0';
            }

            cur++;
            prev = cur;
            fields_count++;
        }
        else
        {
            cur++;
        }
    }

    /*
     * check if fields_count matches with num_of_fields
     */

    if ( fields_count == num_of_fields )
    {
        /* Perfect match	*/
        return(fields);
    }
    else
    {
        return((char **) 0);
    }
}
