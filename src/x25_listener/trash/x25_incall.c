/*
 * $Id: x25_incall.c,v 1.1.1.1 1998/11/18 15:03:27 paul Exp $
 * 
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_incall.c
 *
 * Contents: 
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_incall.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:27  paul
 * Guardian : x25 Pos router
 *
 *
 */

static char rcsid[] = "$Id: x25_incall.c,v 1.1.1.1 1998/11/18 15:03:27 paul Exp $";

#define  MAX_STR 32              /* useful for not including smp.h */

/*  System include files                        */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include	<string.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <x25.h>
#include <neterr.h>

/*  Project include files                       */
#include    "px25_globals.h"
#include		"sm.h"

/*  Module include files                        */
#include "debug.h"
#include "errlog.h"

/*  Extern functions used                       */
extern	struct NUA * rt_find();
extern	struct BEST_ROUTE * sm_get_best_route();

/*  Extern data used                            */

extern	int	sm_errno;
extern   int   rt_errno;

extern	int	port;
extern	int	info;
extern	int	lsn ;
extern 	struct   x25data  user_data;    /* user_data from listen */

/*  Local constants                             */

#define  HOSTNAME_LEN   32
#define  PROTOCOL "tcp"

/*  Local types                                 */

/*  Local macros                                */

#define  X25_ADLEN   (X25_ADDRLEN+X25_ADDREXT+2)

/*  Local data                                  */

int 		rc;
int 		cid1;
char     dest_addr[X25_ADLEN];
char     source_addr[X25_ADLEN];
char		*inbuf;
static   char     myhostname[HOSTNAME_LEN];
struct   NUA   *NUA_INFO;
struct	X25_INFO		X25_TO_GO;
struct	BEST_ROUTE	*BEST_ROUTE_INFO;
struct   hostent     *he;
struct   servent     *se;
struct   protoent 	*pe;
struct   sockaddr_in dest_sin;
int   	s;								/* socket number */
char  msg[ENETMSGLEN];

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

int	x25_incall()
{
	char	*finger;
	char	*p;

	debug((1,"x25_incall - Beginning ... \n"));

	strcpy(source_addr, "");
	strcpy(dest_addr, "");

	/*
	 * get my hostname
	*/

	if ( gethostname(myhostname, HOSTNAME_LEN) < 0 )
	{
		debug((1, "x25_incall - unable to gethostname() errno %d\n", errno));
		errlog(INT_LOG,
			  "x25_incall : unable to gethostname - errno %d\n", errno);
		return(-1);
	}

	if (( x25mkconn(&cid1, port, lsn,  X25WAIT)) < 0 )
	{
		x25errormsg(msg);
		errlog(X25_LOG,"x25incall - x25mkconn error %d, %s\n", 
					x25error(), msg);
		return(-1);
	}

	printf("mkconn cid1 = %d\n",cid1);

	/* accept side */
	
	if ( x25accept(&cid1, info, NULL, NULL,
			dest_addr, source_addr, X25NULLFN) < 0)
	{
		x25errormsg(msg);
		errlog(X25_LOG,"x25incall - x25accept() error %d, %s\n", 
					x25error(), msg);
		return(-1);
	}

	if ( user_data.xd_len > 0 )
	{
		p = user_data.xd_data;

		if ((p[0] == 1) && (p[1] == 0) && (p[2]  == 0) && (p[3] == 0)){

			/* EASYWAY */
			
			debug((1,"EASYWAY call\n"));
			debug((1,"EASYWAY Call User Data with no PID is: %s\n", &p[4]));

			if ((NUA_INFO = rt_find(&p[4])) == (struct NUA *)NULL){
				errlog(X25_LOG,"x25incall - rt_find() error %d\n",rt_errno);
				return(-1);
			}
		}
		else
		{
			/*	ARGOTEL	*/

			debug((1,"ARGOTEL call\n"));

			if (( inbuf = x25alloc(X25_DATA_PACKET_SIZE)) == NULL)
			{
				x25perror("\nx25alloc");
				exit(1);
			}

			if (( rc = 
				x25recv(cid1, inbuf, X25_DATA_PACKET_SIZE, NULL, X25NULLFN)) < 0) 
			{
				errlog(X25_LOG,"x25incall - x25recv() error %d\n", x25error());
				return(-1);
			}
			else {
				debug((1,"x25recv() OK!\n"));
			}

			debug((1,"ARGOTEL Call User Data is: %s, byte %d\n", inbuf, rc));

			if (( finger = strchr(inbuf, '\n')) != NULL) 
			{
				*finger = '\0';
			}

			if ((NUA_INFO = rt_find(inbuf)) == (struct NUA *)NULL) {
				errlog(X25_LOG,"x25incall - rt_find() error %d\n",rt_errno);
				return(-1);
			}
		}
	}
	else
	{
		/* TODO
		 * CLEAR ?
		 */

		/*		die? */
	}

	if ((BEST_ROUTE_INFO =
		sm_get_best_route(myhostname, NUA_INFO->nua_type))
							== ((struct BEST_ROUTE*) NULL))
	{
		errlog(X25_LOG,"Best Route Not found sm_errno = %d\n", sm_errno);
		return(-1);
	}
	else {
		debug((1,"x25incall - Found Best Route for %s %s\n",
					myhostname, (NUA_INFO->nua_type == ASY_NUA) ?
												ASY_NUA_STRING : X25_NUA_STRING ));
	}
	
	debug((1, "x25incall() - Best route : mach %s, serv %s\n", link %s\n",
							BEST_ROUTE_INFO->hostname,
							BEST_ROUTE_INFO->service,
							BEST_ROUTE_INFO->link ));
	/*
	* Select service and protocol
	*/

	if (( se = getservbyname(BEST_ROUTE_INFO->service, PROTOCOL)) == NULL ) {
		errlog(INT_LOG,"x25_incall : unknown service %s for protocol %s\n",
								   BEST_ROUTE_INFO->service, PROTOCOL);
		return(-1);
	}

	dest_sin.sin_port =  se->s_port;
	
	/*
	 * Get Host information
	*/

	if (( he = gethostbyname(BEST_ROUTE_INFO->hostname)) == NULL ) {
		errlog(INT_LOG, "x25_incall : host %s unknown\n", 
											BEST_ROUTE_INFO->hostname);
		return(-1);
	}
	
	dest_sin.sin_family = he->h_addrtype;
	memcpy((char *)&dest_sin.sin_addr, he->h_addr, he->h_length);

	/*
	 * Get protocol number
	*/

	if (( pe = getprotobyname(PROTOCOL)) == NULL ) {
		errlog(INT_LOG, "x25_incall : unknown protocol %s\n", PROTOCOL);
		return(-1);
	}

	/*
	 * Create Socket with all information collected
	*/

	if (( s = socket(he->h_addrtype, SOCK_STREAM, pe->p_proto)) < 0 ) {
		errlog(INT_LOG,"x25_incall : unable to create socket\n");
		return(-1);
	}

	/*
	 * connect to remote
	*/

	if ( connect(s, (char *)&dest_sin, sizeof(dest_sin)) < 0) {
		errlog(INT_LOG, "x25_incall : unable to connect to %s\n", service);
		return (-1);
	}

	debug((1, "x25_incall : Connect OK ...\n"));
	
	if ((NUA_INFO->nua_type == X25_NUA))
	{
			/* x25 connection  */
		/* Need to mos_send a X25_INFO to x25_out_call */

		X25_TO_GO.port =  atoi(BEST_ROUTE_INFO->link);
		X25_TO_GO.NUA_X25.nua_type = NUA_INFO->nua_type;
		strcpy(X25_TO_GO.NUA_X25.primary_nua, NUA_INFO->primary_nua);
		strcpy(X25_TO_GO.NUA_X25.secondary_nua, NUA_INFO->secondary_nua);

		if ((rc = mos_send(s, (char *)&X25_TO_GO, sizeof(X25_TO_GO))) < 0)
		{
			errlog(X25_LOG, "x25_incall : Unable to mos_send on socket\n");
		}
	}
	else {

		/* ASY	connection	*/

		mos_send(s, user_data.xd_data , user_data.xd_len);
	}

	if (( x25delconn(cid1)) < 0 )
	
	{
		x25errormsg(msg);
		errlog(X25_LOG,"x25incall - x25delconn error %d, %s\n", 
					x25error(), msg);
	}
}
