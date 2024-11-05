/*
 * $Id: mos_send.c,v 1.1.1.1 1998/11/18 15:03:18 paul Exp $
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
 * Revision 1.1.1.1  1998/11/18 15:03:18  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  09:48:15  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: mos_send.c,v 1.1.1.1 1998/11/18 15:03:18 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <errno.h>
#include        <sys/uio.h>

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
    struct  iovec iov[2];

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

    /*
     * Set up iovec for writev
     * First chunk
     */

    iov[0].iov_base = &tb_written;
    iov[0].iov_len = sizeof(int);

    /*
     * Second chunk
     */

    iov[1].iov_base = buf;
    iov[1].iov_len = len;

    /* Now try to send the big packet, all at once	*/

    wrc = Writev(sd, &iov, 2);

    if ( wrc == -1 )
    {
        print(("mos_send() - writev ret -1 errno %d\n", errno));
        return(-1);
    }
    /*
     * Ok correct termination
     */
    return(0);
}

#ifdef  MYWRITEV

Writev(fd, iovec, iovlen)
int fd;
struct iovec iovec[];
int iovlen;
{
    int i;
    int bwr = 0;

    for (i = 0; i < iovlen; ++i)
    {
        register int ret;

        if (iovec[i].iov_len <= 0)
        {
            continue;
        }
        if ((ret = send(fd, (char *)(iovec[i].iov_base),
                        (unsigned)(iovec[i].iov_len), 0)) == -1)
        {
            if ( errno == EINTR )
            {
                print(("Writev() - write ret -1 EINTR\n"));
            }
            return(bwr?bwr:-1);
        }
        else if (ret == 0)
        {
            return(bwr);
        }
        else if (ret != iovec[i].iov_len)
        {
            return(bwr+ret);
        }
        bwr += ret;
    }
    return(bwr);
}
# endif
