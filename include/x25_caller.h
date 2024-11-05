/*
 * $Id: x25_caller.h,v 1.1.1.1 1998/11/18 15:03:05 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_caller.h
 *
 * Contents: communication info structure for x25_caller
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: x25_caller.h,v $
 * Revision 1.1.1.1  1998/11/18 15:03:05  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  10:12:06  px25
 * Initial revision
 *
 *
 */

struct   X25_INFO
{
	int      port;
	struct   x25data  facility;
	struct   x25data  userdata;
	struct   NUA   NUA_X25;
	char		source_nua[NUA_LEN];
};
