/*
 * $Id: x28_read.c,v 1.1.1.1 1998/11/18 15:03:34 paul Exp $
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
 * Revision 1.1.1.1  1998/11/18 15:03:34  paul
 * Guardian : x25 Pos router
 *
 *
 */

static char rcsid[] = "$Id: x28_read.c,v 1.1.1.1 1998/11/18 15:03:34 paul Exp $";

/*  System include files                        */

#include		<stdio.h>
#include		<sys/types.h>
#include		<sys/termio.h>
#include		<fcntl.h>

/*  Project include files                       */

#include		"px25_globals.h"
#include		"debug.h"

/*  Module include files                        */

/*  Extern functions used                       */

void			set_vmin();
void			set_ondelay();


/*  Extern data used                            */

int		blocksize = 128;

/*  Local constants                             */

#define		X28_VMIN		2

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

main()
{
	char	buf[BUFSIZ];
	int	rc;

	while (( rc = x28_read(0, buf)) > 0 )
	{
		printf("x28_read ret %d bytes\n", rc);
		printf("%s\n", buf);
	}
	printf("Rc = %d\n", rc);
	exit();
}

#define	TRUE	1
#define	FALSE	0

int	x28_read(dev, buf)
int	dev;
char	*buf;
{
	char	c;
	int	rc;
	int	count = 0;
	int	ndelay = FALSE;


	while (1)
	{
		set_ondelay(dev, ndelay);
		rc = read(dev, &c, 1);

		putchar('.') ; fflush(stdout);

		if ( rc == 0 )
		{
			if ( ndelay )
			{
				/* return packet	*/
				return(count);
			}
			else
			{
				/* connection closed	*/
				return(0);
			}
		}
		buf[count] = c;
		count++;
		if ( count == 128 )
		{
			return(count);
		}
		ndelay = TRUE;
	}
}


int x28_ttyread(device, buf)
int device;
char	*buf;
{
	struct	PKT_HDR	*h = (struct PKT_HDR *) buf;
	int					rc;
	static	int		cur_buf = 0;
	static	int		cur_point = 0;
	static	char	lbuf[2][BUFSIZ];
	static	int	vmin = 2;
	int		size;

	while (1)
	{
		debug((3,"x28_read started\n"));

		set_vmin(device, vmin);

		if ( cur_point == blocksize )
		{
			/* we need to switch blocks	*/

			cur_point = 0;
			if ( cur_buf == 0 ) cur_buf = 1;
			else	cur_buf = 0;

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

				debug((3,"x28_read() - NO MORE BIT\n"));

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

				debug((3,"x28_read() - MORE BIT\n"));

				return(blocksize);
			}
		}

		rc = read(device, &lbuf[cur_buf][cur_point], 1);

		if ( rc == -1 )
		{
			debug((3,"Read ret -1 \n"));
			return(0);
		}

		if ( rc == 0 )
		{
			/* if device is in blocking io the terminal was switched off */
			/* device was closed, terminal switched off	*/

			if ( vmin == X28_VMIN )
			{
				h->pkt_code = ASY_CONNECTION_END;
				h->flags		= 0;
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


void	set_vmin(dev, min)
int	dev;
int	min;
{
	struct	termio	s;

	ioctl(dev, TCGETA, &s);

	s.c_cc[VMIN] = min;

	ioctl(dev, TCSETA, &s);
}

void	set_ondelay(dev, flag)
int	dev;
int	flag;
{
	int		status;
	struct	termio	s;

	if (( status = fcntl(dev, F_GETFL)) < 0 )
	{
		perror("F_GETFL");
		return;
	}

	if ( flag )
	{
		status |= O_NDELAY;
		if ( fcntl(dev, F_SETFL, status) < 0 )
		{
			perror("F_SETFL");
			return;
		}
	}
	else
	{
		status &= ~O_NDELAY;
		if ( fcntl(dev, F_SETFL, status) < 0 )
		{
			perror("F_SETFL");
			return;
		}
	}
}
