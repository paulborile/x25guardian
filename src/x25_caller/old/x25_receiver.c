
/*
 * $Id: x25_receiver.c,v 1.1.1.1 1998/11/18 15:03:32 paul Exp $
 * 
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: 
 *
 * Contents: 
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_receiver.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:32  paul
 * Guardian : x25 Pos router
 *
 *
 */

static char rcsid[] = "$Id: x25_receiver.c,v 1.1.1.1 1998/11/18 15:03:32 paul Exp $";

/*  System include files                        */

#include <stdio.h>
#include <signal.h>
#include <eicon/x25.h>
#include <eicon/neterr.h>

/*  Project include files                       */
#include    "px25_globals.h"

/*  Module include files                        */
#include "debug.h"
#include "errlog.h"

/*  Extern functions used                       */

/*  Extern data used                            */

extern	int	socket;
extern	int	cid;

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

int	port;
int	lsn;
char	inbuf[BUFSIZ];
char	buf[BUFSIZ];
int	info;
char 	msg[ENETMSGLEN];                  /* x25 string. */

/*
 *
 * Procedure: x25_receiver
 *
 * Parameters: 
 *
 * Description:
 *
 * Return: 0 if success, -1 otherwise
 *
 */

int	x25_receiver()
{
	struct	PKT_HDR	*h = ( struct PKT_HDR * ) outbuf;
	int		x25_pktlen;
	int		rc;
	int 		xrc;
	int		xerr;

	if (( rc = x25getconn(cid, &port, &lsn)) < 0 )
	{
		xerr = x25errormsg(msg);
		errlog(X25_LOG,"x25receiver - x25getconn error %d, %s\n", 
						x25error(), msg);
		debug((1,"x25receiver - x25getconn error %d, %s\n", x25error(), msg)); 
		return(-1);
	}

	if (( rc = x25mkconn(&cid, port, lsn,  X25WAIT)) < 0 )
	{
		xerr = x25errormsg(msg);
		errlog(X25_LOG,"x25receiver - x25mkconn error %d, %s\n", 
						x25error(), msg);
		debug((1,"x25receiver - x25mkconn error %d, %s\n", x25error(), msg));
		return(-1);
	}

	while (( x25_pktlen = x25recv(cid, inbuf, BUFSIZ, &info, X25NULLFN)) > 0 )
	{
		h->pkt_code = X25_DATA_PACKET;
		h->pkt_error= 0;
		h->pkt_len	= x25_pktlen;
		if (info == XI_MBIT)
		{
			/* if more bit in input set it 	*/
			h-> flags	= ASY_MORE_BIT; 
			debug((3,"x25receiver - x25recv() setting more bit to mos_send\n"));
		}

		memcpy(buf + sizeof(struct PKT_HDR), inbuf, x25_pktlen);

		if (( rc = mos_send(socket, buf,
									x25_pktlen + sizeof(struct PKT_HDR))) < 0 ) 
		{
			errlog(X25_LOG,"x25receiver - cannot mos_send on socket\n");
			return(-1);
		}
	}


	/* x25_recv has returned < 0 */
	/* LINK DOWN ? */

	if (x25hangup(cid, NULL , XH_IMM, X25NULLFN) < 0)
	{
		x25errormsg(msg);
		errlog(X25_LOG,"x25receiver - x25hangup error %d, %s\n",
						x25error(), msg);
	}

	x25errormsg(msg);
	errlog(X25_LOG,"x25receiver - x25recv error %d, %s\n", x25error(), msg);

	/* end connection	*/
	return(-1);
}
