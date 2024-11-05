/*
 * $Id: bin_to_ea.c,v 1.1.1.1 1998/11/18 15:03:19 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: Routing
 *
 * Contents: binary to extended ascii
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: bin_to_ea.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:19  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/09/14  16:01:16  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: bin_to_ea.c,v 1.1.1.1 1998/11/18 15:03:19 paul Exp $";

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


#ifndef OK
#define OK  0
#endif

#ifndef ERR
#define ERR (-1)
#endif

/*
 *
 * Procedure: bin_to_ea
 *
 * Parameters: binary str, extended ascii str, binary string len
 *
 * Description: convert binary userdata to extended ascii
 *
 * Return: 0 if ok -1 if error
 *
 */

bin_to_ea(bin_str, ea_str, bin_len)
unsigned char *bin_str;
char *ea_str;
int bin_len;
{

    if ( bin_len > MAX_USER_DATA_LEN )
    {
        return(ERR);
    }

    while ( bin_len > 0 )
    {
        if ( *bin_str == '\\' )
        {
            *ea_str++ = '\\';
            *ea_str++ = '\\';
            bin_str++;
            bin_len--;
            continue;
        }
        if ( isprint(*bin_str) )
        {
            *ea_str++ = *bin_str++;
            bin_len--;
        }
        else
        {
            sprintf(ea_str, "\\%02x", *bin_str++);
            ea_str += 3;
            bin_len--;
        }
    }
    *ea_str++ = 0;
    return(OK);
}
