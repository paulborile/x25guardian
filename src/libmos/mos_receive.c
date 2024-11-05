/*
 * $Id: mos_receive.c,v 1.1.1.1 1998/11/18 15:03:15 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: Messages on Sockets
 *
 * Contents: mos_receive
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: mos_receive.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:15  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  09:48:15  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: mos_receive.c,v 1.1.1.1 1998/11/18 15:03:15 paul Exp $";

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
 * Procedure: mos_receive
 *
 * Parameters: socket, buffer, size of local buffer
 *
 * Description: receive data from sockets as messages
 *
 * Return: -2: null buffer passed, -3: invalid len, -4: invalid sd. -5
 * buffer to small
 *
 */

int mos_recv(sd, buf, len)
int sd;
char *buf;
int len;
{
    int red = 0;
    int tb_read = 0;
    int chunks = 0;
    int rrc = 0;
    int tot_read = 0;

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
     * Read first number of bytes and then the block
     */

    if (( rrc = read(sd, &tb_read, sizeof(int))) != sizeof(int) )
    {
        /* ah! couldn't even read 4 bytes   */

        print(("mos_recv() - read of size part ret %d, errno %d\n", rrc, errno));

        if ( rrc == 0 )
        {
            /* the connection was closed	*/
            return(0);
        }
    }

    tot_read = tb_read = ntohl(tb_read);

    /*
     * Check if message is bigger than available user buffer
     */

    if ( tb_read > len )
    {
        /* User buffer is too small     */
        print(("mos_recv() - user buffer %d, to be read %d\n", len, tb_read));
        return(-5);
    }

    /*
     * now read the big chunk
     */

    while ( tb_read )
    {
        if (( red = read(sd, &buf[red], tb_read)) == -1)
        {
            print(("mos_recv() - rd block size %d ret -1 errno %d\n",
                   tb_read, errno));
            return(-1);
        }

        if ( red == 0 )
        {
            /* connection has been closed - incomplete message !	*/
            print(("mos_send() - incomplete message\n"));
            return(0);
        }

        tb_read -= red;
        chunks++;
    }

    /* good termination	*/
    return(tot_read);
}
