/*
 * $Id: mos_recv.c,v 1.1.1.1 1998/11/18 15:03:18 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: Messages on Sockets
 *
 * Contents: mos_receive
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: mos_recv.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:18  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  09:48:15  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: mos_recv.c,v 1.1.1.1 1998/11/18 15:03:18 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <signal.h>
#include        <errno.h>

/*  Project include files                       */

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

#define MAX_SOCKETS     70

/*  Local types                                 */

struct  mos_id
{
    int size;           /* buffer where to read size	*/
    short s_tbread;     /* size bytes to be read	*/
    short s_bcur;       /* size buffer current position	*/
    int tbread;     /* bytes to be read	*/
    int bcur;           /* buffer current position	*/
};

/*  Local macros                                */

#ifdef DEBUG
#   define print(a) printf a
#else
#   define print(a)
#endif

/*  Local data                                  */

static struct  mos_id mid[MAX_SOCKETS];

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
    int rrc = 0;
    int tot_read = 0;
    int tb_read = 0;
    char *tr;

    if ( buf == NULL )
    {
        /* null pointer passed	*/
        return(-2);
    }

    if ( len <= 0 )
    {
        return(-3);
    }

    if (( sd < 0 ) || ( sd > MAX_SOCKETS))
    {
        return(-4);
    }

    tr = (char *) &(mid[sd].size);

    /*
     * Read first number of bytes and then the block
     */

    if ( mid[sd].tbread == 0 )
    {
        /* Was not interrupted	*/

        if ( mid[sd].s_tbread == 0 )
        {
            mid[sd].s_tbread = sizeof(int);
            mid[sd].s_bcur = 0;
            mid[sd].size = 0;
        }

        while ( mid[sd].s_tbread )
        {
            if (( rrc = read(sd, &tr[mid[sd].s_bcur], mid[sd].s_tbread )) == -1 )
            {
                if ( errno != EINTR )
                {
                    mid[sd].s_tbread = 0;
                    mid[sd].s_bcur = 0;
                }
                print(("mos_recv() - read of size ret %d, errno %d\n", rrc, errno));
                return(rrc);
            }

            if ( rrc == 0 )
            {
                /* connection has been closed - incomplete message !	*/
                print(("mos_recv() - Closed during size read\n"));
                mid[sd].s_tbread = 0;
                mid[sd].s_bcur = 0;
                return(0);
            }

            mid[sd].s_tbread -= rrc;
            mid[sd].s_bcur += rrc;
        }

        mid[sd].s_tbread    = 0;
        mid[sd].s_bcur      = 0;
        mid[sd].tbread      = ntohl(mid[sd].size);
        mid[sd].size        = 0;
    }

    /*
     * Check if message is bigger than available user buffer
     */

    if ( mid[sd].tbread > len )
    {
        /* User buffer is too small     */
        print(("mos_recv() - user buffer %d, to be read %d\n",
               len, mid[sd].tbread));
        return(-5);
    }

    /*
     * now read the big chunk
     */

    while ( mid[sd].tbread )
    {
        if (( red = read(sd, &buf[mid[sd].bcur], mid[sd].tbread)) == -1)
        {
            if ( errno != EINTR )
            {
                mid[sd].tbread = 0;
                mid[sd].bcur = 0;
            }
            print(("mos_recv() - rd block size %d ret -1 errno %d\n",
                   mid[sd].tbread, errno));
            return(red);
        }

        if ( red == 0 )
        {
            /* connection has been closed - incomplete message !	*/
            print(("mos_recv() - incomplete message\n"));
            mid[sd].tbread = 0;
            mid[sd].bcur = 0;
            return(0);
        }

        mid[sd].tbread -= red;
        mid[sd].bcur += red;
    }

    /* good termination	*/
    tot_read = mid[sd].bcur;
    mid[sd].bcur = 0;
    mid[sd].tbread = 0;
    return(tot_read);
}
