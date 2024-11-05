/*
 * $Id: binhex.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: binhex
 *
 * Contents: convert binary stuff to hex
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: binhex.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:23  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/07/12  14:25:41  px25
 * Added check for even length of input string
 *
 * Revision 1.0  1995/07/07  09:53:56  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: binhex.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $";

/*  System include files                        */

#include    <stdio.h>

/*  Project include files                       */

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

/*
 *
 * Procedure: bin2hex
 *
 * Parameters: binary buffer of data, len, string
 *
 * Description: convert binary data to hex into string
 *
 * Return: len of cenvreted string
 *
 */


int bin2hex(buf, len, str)
unsigned char *buf;
int len;
char *str;
{
    int j = 0;
    int i = 0;

    for (i=0; i<len; i++)
    {
        sprintf(str+j, "%02x", buf[i]);
        j   += 2;
    }
    return(strlen(str));
}


/*
 *
 * Procedure: hex2bin
 *
 * Parameters: hex string, binary buffer of data
 *
 * Description: convert hex string into binary data
 *
 * Return: len of binary data obtained
 *
 */


int hex2bin(str, buf)
char *str;
unsigned char *buf;
{
    int j = 0;
    int i = 0;
    int len = strlen(str);

    if ((len % 2) != 0 )
    {
        /* some error, the string has to be even len	*/
        return(0);
    }

    for (j=0; j<len; j+= 2)
    {
        sscanf(str+j, "%02x", &buf[i]);
        i++;
    }
    return(i);
}
