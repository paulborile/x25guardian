/*
 * $Id: ea_to_bin.c,v 1.1.1.1 1998/11/18 15:03:19 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: Routing table
 *
 * Contents: convert ascii non printable stuff to binary
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: ea_to_bin.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:19  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/09/14  16:01:44  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: ea_to_bin.c,v 1.1.1.1 1998/11/18 15:03:19 paul Exp $";

/*  System include files                        */

#include <ctype.h>

/*  Project include files                       */

#include        "px25_globals.h"

/*  Module include files                        */

#include        "rt.h"

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */


#ifndef ERR
#define ERR (-1)
#endif

/*
 *
 * Procedure: ea_to_bin
 *
 * Parameters: binary array, ascii array
 *
 * Description: convert extended ascii array in the form of ABC\02
 *               to binary 41424302
 *
 * Return: len of string
 *
 */


ea_to_bin(ascii_str, bin_str)
char *ascii_str;
unsigned char *bin_str;
{
    int xx, a_len=0;

    while ( *ascii_str )
    {
        switch ( *ascii_str )
        {
        case '\\':
            if ( *(ascii_str+1) && *(ascii_str+1) == '\\' )
            {
                *bin_str++ = *ascii_str++;
                *ascii_str++;
                a_len++;
                break;
            }
            if ( *(ascii_str+1) && *(ascii_str+2) && isxdigit(*(ascii_str+1)) && isxdigit(*(ascii_str+2)) )
            {
                sscanf(ascii_str+1, "%2x", &xx);
                *bin_str++ = (unsigned char)xx;
                ascii_str += 3;
                a_len++;
            }
            else{
                return(ERR);
            }
            break;

        default:
            *bin_str++ = *ascii_str++;
            a_len++;
            break;
        }
    }
    return((a_len > MAX_USER_DATA_LEN ? ERR : a_len));
}
