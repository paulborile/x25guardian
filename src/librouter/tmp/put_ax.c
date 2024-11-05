#include <ctype.h>

#include <defines.h>

#ifndef OK
#define OK  0
#endif

#ifndef ERR
#define ERR (-1)
#endif

/*
** Converts CIDIN Center code into a MAX_STR_AX long char array (1st param),
** getting it from an unsigned char string (2nd param) of specified length
** (3rd param). Returns OK (0) upon success or ERR (-1) otherwise.
** -------------------------------------------------------------------------
** $Log: put_ax.c,v $
** Revision 1.1.1.1  1998/11/18 15:03:21  paul
** Guardian : x25 Pos router
**
* Revision 4.1  95/04/07  16:27:58  af2
* AeroNet APR 1995 - Ready for acceptance tests
*
* Revision 1.0  94/05/18  12:50:34  af2
* Initial revision
*
*/

static char *rcsid[] = "$Header: /u1/rcs/guardian/src/librouter/tmp/put_ax.c,v 1.1.1.1 1998/11/18 15:03:21 paul Exp $";

put_cidin_code(ret_str, inp_str, inp_len)
char *ret_str;
unsigned char *inp_str;
int inp_len;
{

    if ( inp_len > MAX_BIN_AX )
    {
        return(ERR);
    }

    while ( inp_len > 0 )
    {
        if ( *inp_str == '\\' )
        {
            *ret_str++ = '\\';
            *ret_str++ = '\\';
            inp_str++;
            inp_len--;
            continue;
        }
        if ( isprint(*inp_str) )
        {
            *ret_str++ = *inp_str++;
            inp_len--;
        }
        else
        {
            sprintf(ret_str, "\\%02x", *inp_str++);
            ret_str += 3;
            inp_len--;
        }
    }
    *ret_str++ = 0;
    return(OK);
}
