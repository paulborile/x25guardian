/*
 * $Id: x28_read.c,v 1.1.1.1 1998/11/18 15:03:11 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x28_read.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:11  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/07/14  09:08:17  px25
 * No change.
 *
 * Revision 1.0  1995/07/07  10:08:31  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x28_read.c,v 1.1.1.1 1998/11/18 15:03:11 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <sys/types.h>
#include        <sys/termio.h>

/*  Project include files                       */

#include        "px25_globals.h"

/*  Module include files                        */

/*  Extern functions used                       */

void            set_vmin();

/*  Extern data used                            */

extern int blocksize;

/*  Local constants                             */

#define     X28_VMIN        2

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */


/*
 *
 *  Procedure: x28_read
 *
 *  Parameters: device, buffer
 *
 *  Description: read from serial and create x28 packets
 *
 *  Return: number of read character , -1 if error ,
 *		         0 when connection ends
 *
 */

int x28_read(device, buf)
int device;
char *buf;
{
    struct  PKT_HDR *h = (struct PKT_HDR *) buf;
    int rc;
    static int cur_buf = 0;
    static int cur_point = 0;
    static char lbuf[2][BUFSIZ];
    static int vmin = 2;
    int size;


    while (1)
    {
        set_vmin(device, vmin);

        if ( cur_point == blocksize )
        {
            /* we need to switch blocks	*/

            cur_point = 0;
            if ( cur_buf == 0 )
            {
                cur_buf = 1;
            }
            else{cur_buf = 0;}

            /*
             * we need now to switch to non blocking io
             * to see if there is more data, only if
             * blocksize == X25_DATA_PACKET_SIZE
             */

            set_vmin(device, 0);

            if (( rc = read(device, &lbuf[cur_buf][cur_point], 1)) == 0 )
            {

                /* send previous packet with NO MORE BIT */

                h->flags = 0;
                h->pkt_code = ASY_DATA_PACKET;

                memcpy(buf + sizeof(struct PKT_HDR),
                       (cur_buf == 0) ? lbuf[1] : lbuf[0], blocksize);

                vmin = X28_VMIN;

                return(blocksize);
            }
            else
            {
                /*
                 * set More Bit on previous packet
                 * only if blocksize == X25_DATA_PACKET_SIZE
                 */

                h->pkt_code = ASY_DATA_PACKET;

                if ( blocksize == X25_DATA_PACKET_SIZE )
                {
                    h->flags = MORE_BIT;
                }
                else
                {
                    h->flags = 0;
                }

                /* copy lbuf to buf  */

                memcpy(buf + sizeof(struct PKT_HDR),
                       (cur_buf == 0) ? lbuf[1] : lbuf[0], blocksize);

                cur_point++;
                vmin = 0;

                return(blocksize);
            }
        }

        rc = read(device, &lbuf[cur_buf][cur_point], 1);

        if ( rc == -1 )
        {
            return(0);
        }

        if ( rc == 0 )
        {
            /* if device is in blocking io the terminal was switched off */
            /* device was closed, terminal switched off	*/

            if ( vmin == X28_VMIN )
            {
                h->pkt_code = ASY_CONNECTION_END;
                h->flags        = 0;
                cur_point = 0;
                vmin = X28_VMIN;
                return(0);
            }

            memcpy(buf + sizeof(struct PKT_HDR), lbuf[cur_buf], cur_point);

            h->flags = 0;
            h->pkt_code = ASY_DATA_PACKET;

            size = cur_point;
            cur_point = 0;
            vmin = X28_VMIN;
            return(size);
        }

        cur_point++;
        vmin = 0;
    }
}


void    set_vmin(dev, min)
int dev;
int min;
{
    struct  termio s;

    ioctl(dev, TCGETA, &s);

    s.c_cc[VMIN] = min;

    ioctl(dev, TCSETA, &s);
}
