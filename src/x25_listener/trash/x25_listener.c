/*
 * 
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_listener
 *
 * Contents: listen for incoming x25 calls
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_listener.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:27  paul
 * Guardian : x25 Pos router
 *
 *
 */

static char rcsid[] = "$Id: x25_listener.c,v 1.1.1.1 1998/11/18 15:03:27 paul Exp $";

/*  System include files                        */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>

#include	<x25.h>
#include	<neterr.h>

/*  Project include files                       */

#include	"px25_globals.h"
#include	"x25_caller.h"
#include "sm.h"
#include	"debug.h"
#include "errlog.h"

/*  Module include files                        */

/*  Extern functions used                       */
extern   struct NUA * rt_find();
extern   struct BEST_ROUTE * sm_get_best_route();

/*  Local functions used                       */

void	get_command_line();
void	x25_listener_usage();
void  child_termination();
void  myexit();
void  terminate();

/*  Extern data used                            */

extern   int   sm_errno;
extern   int   rt_errno;

/*  Local constants                             */

#define	OPTS	"d:p:"
#define  HOSTNAME_LEN   32
#define  PROTOCOL "tcp"

#ifdef   DEBUG
#define  X25_INCALL	"x25_incall.d"
#else
#define  X25_INCALL  "x25_incall"
#endif

/*  Local types                                 */

/*  Local macros                                */

#define	X25_ADLEN	(X25_ADDRLEN+X25_ADDREXT+2)

/*  Local data                                  */

struct	x25data	facility;
struct	x25data	user_data;

int      rc;
char		remote_addr[X25_ADLEN];
char		local_addr[X25_ADLEN];
char		*inbuf;
int		cid;
int		lsn;
int		info = 0;

int		easyway = 0;
int		listenport = -1;
int		port = -1;
char     myhostname[HOSTNAME_LEN];
struct   NUA   *NUA_INFO;
char		buf[BUFSIZ];
struct	PKT_HDR		*ph = (struct PKT_HDR *) (buf);
struct   X25_INFO   	*X25_TO_GO = (struct X25_INFO *)
												(buf + sizeof(struct PKT_HDR));
struct   BEST_ROUTE  *BEST_ROUTE_INFO;

/* For call to unix services	*/

struct	BEST_ROUTE	unix_ser;

struct   hostent     *he;
struct   servent     *se;
struct   protoent    *pe;
struct   sockaddr_in dest_sin;
int      s = -1;                      /* socket number */
int		alloc_err = 0;
int		xerr;

char     pname[BUFSIZ];             	/* Program name */
char		cmd[16];
char		cmdport[16];
char		cmdlsn[16];
char		cmddebug[16];

static 	char pid_str[20];
static 	int debuglevel = -1;
static 	char debug_file[BUFSIZ];

main(argc, argv)
int	argc;
char	**argv;
{
	pid_t pid;

	char  msg[ENETMSGLEN];  
	char  message[ENETMSGLEN];

	char  *finger;
	char  *p;

	/*
	 * Globalize name of the program
	 */

	strcpy(pname, argv[0]);

	get_command_line(argc, argv);

#ifdef DEBUG
	sprintf(pid_str, "%d", getpid());
	sprintf(debug_file, "/tmp/x25_listener%05d.debug", getpid());
	initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif
	
	/*
	* set signals
	*/

	set_signals();

	/*
	* set pid table
	*/

	set_pid();

	/*
	 * Initialize x25 toolkit
	 */
	if ( x25init(0) < 0 )
	{
		x25errormsg(msg);
		errlog(X25_LOG,"%s : x25init error %d, %s\n", 
						pname, x25error(), msg);
		exit(1);
	}
		
	x25version(msg);
	debug((1,"%s\n", msg));

	/*
	 *		Allocate a buffer for ARGOTEL call
	 */
	if (( inbuf = x25alloc(X25_DATA_PACKET_SIZE)) == NULL)
   {
		x25errormsg(msg);
      errlog(X25_LOG,"%s - x25alloc() error %d, %s\n",
                                pname, x25error(), msg);
      debug((1,"main() - x25alloc error %d %s\n", x25error(), msg));
		alloc_err = 1;
   }

	/*
	 * now LISTEN on the specified port
	 */

again1:

	debug((3,"Listen Port %d, Going to listen ....\n", listenport));

	strcpy(local_addr, "");
	strcpy(remote_addr, "");
	info = XI_QBIT;

	memset(&user_data, '\0', sizeof(struct x25data));
	memset(&facility, '\0', sizeof(struct x25data));

	if ( x25xlisten(&cid, X25WAIT, listenport, info, &facility, &user_data,
						 remote_addr, local_addr, X25NULLFN) < 0)
	{
		xerr = x25error();
		x25errormsg(msg);

		debug((1,"x25listen x25error %d %s\n", xerr, msg));

		if ( xerr == EX25INTR)
		{
			debug((1, "main() - EX25INTR received\n"));
			goto again1;
		}
		
		errlog(X25_LOG,"%s : x25listen error %d, %s\n", 
					pname, xerr, msg);
		debug((1,"x25listen x25error %d %s\n", xerr, msg));
		myexit(FAILURE);
	}

	errlog(X25_LOG,"%s : CALL RECEIVED FROM %s\n", pname, remote_addr);

	debug((3,"main() - CALL RECEIVED FROM %s TO %s cid %d\n",
												remote_addr, local_addr, cid));
	debug((3,"USERDATA len = %d\n", user_data.xd_len));
	debug((3,"FACILITY len = %d\n", facility.xd_len));
	debug((3,"FACILITY %x %x %x %x\n", facility.xd_data[0], facility.xd_data[1],
				facility.xd_data[2], facility.xd_data[3]));

	if ( user_data.xd_len <= 0 )
	{
		/* empty user data : sending a CLEAR */

		debug((3,"main() - empty user data: sending a clear..\n"));

		if(x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
		{
			xerr = x25error();
			x25errormsg(msg);
			debug((1, "main() - x25hangup error %d %s\n", xerr, msg));

			errlog(X25_LOG,"%s - x25hangup() failed with error %d, %s\n",
												pname, xerr, msg);
		}
		else
		{
			debug((3, "main() - x25hangup sent\n"));
		}
		goto again1;
	}

	p = user_data.xd_data;

	if ((p[0] == 1) && (p[1] == 0) && (p[2]  == 0) && (p[3] == 0))
	{
		/* EASYWAY */
		easyway = 1;
		
		debug((3,"main() - EASYWAY Call User Data %s\n", &p[4]));

		if ((NUA_INFO = rt_find(&p[4])) == (struct NUA *)NULL)
		{
			debug((1,
				"main() - rt_find() error, rt_errno %d\n", rt_errno));
			errlog(X25_LOG,"%s - rt_find() error %d\n", pname, rt_errno);

			if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
			{
				x25errormsg(msg);
				debug((1,
					"main() - x25hangup() failed error %d %s\n",
					x25error(), msg));
				errlog(X25_LOG,"%s - x25hangup() failed with error %d, %s\n",
						pname, x25error(), msg);
			}
			else
			{
				debug((1, "main() - x25hangup sent\n"));
			}
			goto again1;
		}
	}
	else
	{
		/*	ARGOTEL	*/

		easyway = 0;

		debug((3,"main() - ARGOTEL call\n"));

		if (alloc_err == 1)
			myexit(FAILURE);

		/*
		 * Accept the call now because we need to read argotel buffer
		 */


		if ( x25accept(&cid, info, NULL, NULL,
						remote_addr, local_addr, X25NULLFN) < 0)
		{
			x25errormsg(msg);
			errlog(X25_LOG,"%s - x25accept() error %d, %s\n", 
						pname, x25error(), msg);
			debug((1,"main() - x25accept() error %d %s\n", x25error(), msg));
			myexit(FAILURE);
		}

		debug((3, "main() - x25accept() OK cid %d\n", cid));

		if (( rc = 
			x25recv(cid, inbuf, X25_DATA_PACKET_SIZE, NULL, X25NULLFN)) < 0) 
		{
			x25errormsg(msg);
			errlog(X25_LOG,"%s - x25recv() error %d, %s\n", 
								pname, x25error(), msg);
			debug((1,"main() - x25recv error %d %s\n", x25error(), msg));
			myexit(FAILURE);
		}

		debug((3,"main() - ARGOTEL recv %d bytes User Data  %s\n", 
						rc, inbuf));

		/*
		 * Take out newline from user data string
		 */

		if (( finger = strchr(inbuf, '\r')) != NULL) 
		{
			*finger = '\0';
		}

		if ((NUA_INFO = rt_find(inbuf)) == (struct NUA *)NULL)
		{
			debug((1,"main() - rt_find() rt_errno %d\n", rt_errno));
			errlog(X25_LOG,"%s - rt_find() error %d\n", pname, rt_errno);

			if(x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
			{
				x25errormsg(msg);
				debug((1, "main() - x25hangup error %d %s\n", x25error(), msg));

				errlog(X25_LOG,"%s - x25hangup() failed with error %d, %s\n",
									pname, x25error(), msg);
			}
			else
			{
				debug((3, "main() - x25hangup sent\n"));
			}
			goto again1;
		}
	}


	/*
	 * get my hostname
	 */

	if ( gethostname(myhostname, HOSTNAME_LEN) < 0 )
	{
		debug((1, "main() - unable to gethostname() errno %d\n", errno));
		errlog(INT_LOG,
			  "%s : unable to gethostname - errno %d\n", pname, errno);
		myexit(FAILURE);
	}

	if (( NUA_INFO->nua_type != TTY_NUA ) &&( NUA_INFO->nua_type != SNA_NUA ))
	{
		if ((BEST_ROUTE_INFO = sm_get_best_route(myhostname, NUA_INFO->nua_type))
							== ((struct BEST_ROUTE*) NULL))
		{
			debug((1, "main() - Best Route Not found sm_errno %d\n", sm_errno));
			errlog(X25_LOG,"%s - CANNOT ROUTE CONNECTION - NO FREE CHANNELS\n",
							pname );

			if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
			{
				xerr = x25error();
				x25errormsg(msg);
				debug((1, 
					"main() - x25hangup failed error %d %s\n", xerr, msg));
				errlog(X25_LOG,"%s - x25hangup() failed with error %d, %s\n",
							pname, xerr, msg);
			}
			else
			{
				debug((1, "main() - x25hangup sent\n"));
			}
			goto again1;
		}
	}
	else
	{
		/* Unknown nua for the moment	*/

		debug((1,"main() - nuatype %d unknown\n", NUA_INFO->nua_type));

		errlog(X25_LOG,"%s - UNKNOWN NUA TYPE %d\n", NUA_INFO->nua_type);

		if(x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
		{
			x25errormsg(msg);
			debug((1, "main() - x25hangup error %d %s\n", x25error(), msg));

			errlog(X25_LOG,"%s - x25hangup() failed with error %d, %s\n",
								pname, x25error(), msg);
		}
		else
		{
			debug((3, "main() - x25hangup sent\n"));
		}
		goto again1;

		/***************** for later use
		gethostname(unix_ser.hostname, MAX_STR);
		strcpy(unix_ser.service, "telnet");
		strcpy(unix_ser.link, "pty");
		BEST_ROUTE_INFO = &unix_ser;
		******************/
	}

	
	debug((3, "main() - Best route : host %s, serv %s, link %s\n",
					BEST_ROUTE_INFO->hostname, BEST_ROUTE_INFO->service,
					BEST_ROUTE_INFO->link ));

	/*
	* Select service and protocol
	*/

	if (( se = getservbyname(BEST_ROUTE_INFO->service, PROTOCOL)) == NULL ) {
		errlog(INT_LOG,"%s : UNKNOWN SERVICE %s FOR PROTOCOL %s\n",
								   pname, BEST_ROUTE_INFO->service, PROTOCOL);
		exit(1);
	}

	dest_sin.sin_port =  se->s_port;
	
	/*
	 * Get Host information
	*/

	if (( he = gethostbyname(BEST_ROUTE_INFO->hostname)) == NULL )
	{
		debug((1, "main() gethostbyname failed, %s unknown\n",
			BEST_ROUTE_INFO->hostname ));
		errlog(INT_LOG, "%s : host %s unknown\n", 
											pname, BEST_ROUTE_INFO->hostname);
		myexit(FAILURE);
	}
	
	dest_sin.sin_family = he->h_addrtype;
	memcpy((char *)&dest_sin.sin_addr, he->h_addr, he->h_length);

	/*
	 * Get protocol number
	*/

	if (( pe = getprotobyname(PROTOCOL)) == NULL )
	{
		debug((1, "main() getprotobyname failed, %s unknown\n", PROTOCOL));
		errlog(INT_LOG, "%s : unknown protocol %s\n", pname, PROTOCOL);
		myexit(FAILURE);
	}

	/*
	 * Create Socket with all information collected
	*/

	if (( s = socket(he->h_addrtype, SOCK_STREAM, pe->p_proto)) < 0 ) {
		debug((1, "main() socket failed, errno %d\n", errno));
		errlog(INT_LOG,"%s : unable to create socket\n", pname);
		myexit(FAILURE);
	}

	/*
	 * connect to remote
	*/

	if ( connect(s, (char *)&dest_sin, sizeof(dest_sin)) < 0) {
		debug((1, "main() - connect failed, errno %d\n", errno));
		errlog(INT_LOG, "%s : unable to connect to %s\n", 
						pname, BEST_ROUTE_INFO->service);
		myexit(FAILURE);
	}

	debug((3, "main() - Connect OK socket %d \n", s));

	/* getconn now for later fork	*/

	if (( x25getconn(cid, &port, &lsn)) < 0 )
	{
		x25errormsg(msg);
		errlog(X25_LOG,"%s - x25getconn error %d, %s\n",
							pname, x25error(), msg);
		debug((1,"main() - x25getconn error %d %s\n", x25error(), msg));
		myexit(FAILURE);
	}

	debug((3,"x25getconn() port = %d lsn %d\n", port, lsn));

	
	/* IMPORTANT */
	/* NEED TO mos_send a X25_INFO to x25_out_call */

	if ((NUA_INFO->nua_type == X25_NUA))
	{
		memset(buf, '\0', BUFSIZ);
		ph->pkt_code	= X25_CALL_REQ;
		X25_TO_GO->port =  atoi(BEST_ROUTE_INFO->link);
		X25_TO_GO->NUA_X25.nua_type = NUA_INFO->nua_type;
		strcpy(X25_TO_GO->NUA_X25.primary_nua, NUA_INFO->primary_nua);
		strcpy(X25_TO_GO->NUA_X25.secondary_nua, NUA_INFO->secondary_nua);

		if ((rc = mos_send(s, buf, sizeof(struct PKT_HDR) +
											sizeof(struct X25_INFO))) < 0)
		{
			errlog(X25_LOG, "%s : UNABLE TO GENERATE X25 OUTPUT CALL\n", pname);
			myexit(FAILURE);
		}
	}

	/* ACCEPT the call now only if easy-way and then fork	*/
	
	if ( easyway )
	{
		if ( x25accept(&cid, info, NULL, NULL,
			remote_addr, local_addr, X25NULLFN) < 0)
		{
			x25errormsg(msg);
			errlog(X25_LOG,"%s - x25accept() error %d, %s\n", 
						pname, x25error(), msg);
			debug((1,"main() - x25accept() error %d %s\n", x25error(), msg));
			myexit(FAILURE);
		}
		debug((3, "main() - x25accept() OK cid %d\n", cid));
	}


	
	debug((3, "main() - going to fork\n"));

	pid   = fork();

	switch ( pid )
	{
		case  -1 :

			debug((1, "%s : main() - Unable to fork! errno = %d\n", pname, errno));
			errlog(INT_LOG, "%s : Fork Failed, errno %d\n", pname, errno);
			x25hangup(cid, NULL, XH_IMM, X25NULLFN);
			myexit(FAILURE);
			break;

  		case  0  :  /* child: reader */

			/* exec new process to manage call  */

			debug((3,"main(child) : socket %d, port %d, lsn %d\n",
														s, port, lsn));

			sprintf(cmd, "-s%d", s);
			sprintf(cmdport, "-p%d", port);
			sprintf(cmdlsn, "-l%d", lsn);
			sprintf(cmddebug, "-d%d", debuglevel);

			debug((3, 
			"main(child) - going to execute %s socket %s port %s lsn %s debug%s\n",
								  X25_INCALL, cmd, cmdport, cmdlsn , cmddebug));

			/* use execlp instead   */

#ifdef	DEBUG
			execlp(X25_INCALL, X25_INCALL, cmd, cmdport, cmdlsn, cmddebug, 0);
#else
			execlp(X25_INCALL, X25_INCALL, cmd, cmdport, cmdlsn, 0);
#endif

			/* if reached means that execlp failed */

			debug((1, "main(child) execlp failed errno %d\n", errno));
			errlog(INT_LOG,
			  "%s : Unable to execlp %s , errno %d\n", pname, X25_INCALL, errno);
			sleep(3);
			exit(FAILURE);

			break;


		default  :   /* father continue to listen after deletion of cid */

			if (pid_push(pid, BEST_ROUTE_INFO->link,
										BEST_ROUTE_INFO->hostname) == -1)
			{
				/* pid table is full */

				errlog(INT_LOG,
				"%s : Pid table is full - going to SIGTERM process %d",
					pname, pid);

				kill(SIGTERM, pid);
			}

			debug((3, "main(father) - closing socket %d\n", s));

			close(s);

			debug((3, "main(father) - x25delconn(%d)\n", cid));

			if (( x25delconn(cid)) < 0 )
			{
				x25errormsg(msg);
				errlog(X25_LOG,"%s : x25delconn error %d, %s\n", 
								pname, x25error(), msg);
				debug((1, "x25delconn error %d %s\n", x25error(), msg));
			}

			goto again1;
	}
}

/*
 *
 *  Procedure: get_command_line
 *
 *  Parameters: argc, argv
 *
 *  Description: parse command line
 *
 *  Return: none
 *
 */

void	get_command_line(argc, argv)
int	argc;
char	**argv;
{
	int	fatal = 0;
	int	c;

	extern char *optarg;

	while ((c = getopt(argc, argv, OPTS)) !=EOF)
	{
		switch(c)
		{
			case	'd':
				debuglevel  =   atoi(optarg);
				break;

			case	'p':
				listenport = atoi(optarg);
				break;

			default:
				printf("%s : invalid flag.\n", argv[0]);
				x25_listener_usage(argc, argv);
				exit(FAILURE);
		}
	}

#ifdef DEBUG
	if ( debuglevel == -1 ) fatal++;
#endif

	if ( listenport == -1 ) fatal++;

	if ( fatal )
	{
		x25_listener_usage(argc, argv);
		exit(FAILURE);
	}
}


/*
 *
 *  Procedure: x25_listener_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void	x25_listener_usage(argc, argv)
int	argc;
char	**argv;
{
#ifdef	DEBUG
	printf("Usage: %s -d<debuglevel> -p<port>\n",argv[0]);
#else
	printf("Usage: %s -p<port>\n",argv[0]);
#endif
}


/*
 *
 *  Procedure: myexit
 *
 *  Parameters: none
 *
 *  Description: close file descriptors, 
 *						free route associated with X25_listener,
 *						send SIGTERM to childs and exit
 *
 *  Return:
 *
 */

void  myexit(code)
int   code;
{
	char	link[MAX_STR];
	int	ret;
	char	msg[ENETMSGLEN];
	int	pid;

	debug((3, "myexit() - terminating code %d\n", code));

	while (( pid = pid_scan(link)) != -1 )
	{
		/* for each link execute sm_free_route	*/

		if (( ret = sm_free_route(myhostname, link)) < 0)
		{
			errlog(INT_LOG, "%s : sm_free_route() failed with sm_errno %d\n",
					pname, sm_errno);
			debug((1, "myexit() - sm_free_route() failed with sm_errno %d\n",
					sm_errno));
		}
		else
		{
			debug((3, "myexit() - sm_free_route() returned %d\n", ret));
		}
	}

	if ( s != -1 ) close(s);

	/* kill all processes	*/

	pid_kill(SIGTERM);

	if (x25free(inbuf) < 0)
   {
   	x25errormsg(msg);
      errlog(X25_LOG,"%s - x25free error %d, %s\n",
                             pname, x25error(), msg);
   }
	else
		debug((3," x25free() - deallocating ARGOTEL buffer\n"));

  errlog(INT_LOG, "%s : Terminating execution\n", pname);

  debug((3, "myexit() - Terminating\n"));

#ifdef DEBUG
	enddebug();
#endif

	x25exit();
	exit(code);
}
