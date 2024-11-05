/*
 * $Id: errlog.h,v 1.1.1.1 1998/11/18 15:03:05 paul Exp $
 * 
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: errlog
 *
 * Contents: Logging functions
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: errlog.h,v $
 * Revision 1.1.1.1  1998/11/18 15:03:05  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  10:12:06  px25
 * Initial revision
 *
 *
 */

/*
 * Possible errlog types
 */

#define		X25_LOG		0
#define		ASY_LOG		1
#define		SNA_LOG		2
#define		INT_LOG		3

#define		PX25_LOG_DIR	"PX25_LOG_DIR"

#define		X25_FILE_NAME	"/x25.log"
#define		ASY_FILE_NAME	"/asy.log"
#define		SNA_FILE_NAME	"/sna.log"
#define		INT_FILE_NAME	"/int.log"
