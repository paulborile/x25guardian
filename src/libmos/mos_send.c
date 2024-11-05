/*
 * $Id: mos_send.c,v 1.1.1.1 1998/11/18 15:03:15 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: Messages on Sockets
 *
 * Contents: mos_send
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: mos_send.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:15  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  09:48:15  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: mos_send.c,v 1.1.1.1 1998/11/18 15:03:15 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <errno.h>

/*  Project include files                       */

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

#ifdef DEBUG
#   define print(a) printf a
#else
#   define print(a)
#endif

/*  Local data                                  */


/*
 *
 * Procedure: mos_send
 *
 * Parameters: socket, buffer, size
 *
 * Description: send data on sockets as messages
 *
 * Return: -2: null buffer passed, -3: wrong len passed
 *			  -4: bad file descriptor
 *
 */

int mos_send(sd, buf, len)
int sd;
char *buf;
int len;
{
    int written = 0;
    int tb_written = len;
    int chunks = 0;
    int wrc = 0;

    if ( buf == NULL )
    {
        /* null pointer passed	*/
        return(-2);
    }

    if ( len <= 0 )
    {
        return(-3);
    }

    if ( sd < 0 )
    {
        return(-4);
    }

    /*
     * Send first number of bytes and then the block
     */

    tb_written = htonl(len);

    if (( wrc = write(sd, &tb_written, sizeof(int))) == -1 )
    {
        /* nothing I can do here, probably socket is not connected */
        print(("mos_send() - wr size ret -1, errno %d\n", errno));
        return(-1);
    }

    /* Now try to send the big packet, all at once	*/

    while ( written < len )
    {
        if (( wrc = write(sd, &buf[written], len - written)) == -1)
        {
            print(("mos_send() - wr block size %d ret -1 errno %d\n",
                   tb_written, errno));
            return(-1);
        }
        written += wrc;
        chunks++;
    }

    /*
     * Ok correct termination
     */

    return(0);
}
