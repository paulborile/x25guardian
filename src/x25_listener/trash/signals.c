/*
 * $Id: signals.c,v 1.1.1.1 1998/11/18 15:03:27 paul Exp $
 * 
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: 
 *
 * Contents: 
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: signals.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:27  paul
 * Guardian : x25 Pos router
 *
 *
 */

static char rcsid[] = "$Id: signals.c,v 1.1.1.1 1998/11/18 15:03:27 paul Exp $";

/*  System include files                        */

#include		<stdio.h>
#include		<signal.h>
#include		<x25.h>

/*  Project include files                       */

#include		"px25_globals.h"
#include		"sm.h"
#include		"errlog.h"
#include		"debug.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

extern	char	pname[];

/*  Local constants                             */

/*  Local types                                 */

void	child_termination();
void	terminate();

/*  Local macros                                */

/*  Local data                                  */

static	struct	sigaction	act;

/*
 *
 *  Procedure: set_signals
 *
 *  Parameters: none
 *
 *  Description: set initial signals using sigaction
 *
 *  Return:  none
 *
 */

void	set_signals()
{
	debug((3, "set_signals() - setting signals\n"));

	act.sa_handler		= child_termination;
	act.sa_flags		= SA_RESTART;
	sigemptyset(&act.sa_mask);

	sigaction(SIGCHLD, &act, NULL);

	act.sa_handler		= terminate;

	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);

	debug((3, "set_signals() - done\n"));
}

/*
 *
 *  Procedure: child_termination
 *
 *  Parameters: standard signal
 *
 *  Description: freeing route and checkout ttab for pid terminated
 *
 *  Return:  none
 *
 */

void  child_termination()
{
	int		ret;
	long     status = 0;
	pid_t pid;
	char		link[MAX_STR];
	char		host[MAX_STR];

	debug((3, "child_termination() - Starting.\n"));

	pid = wait(&status);

	if (pid_pop(pid, link, host) == -1)
	{
		debug((1,"child_termination() - pid_pop() did not find pid %d\n", pid));
		errlog(INT_LOG, "%s : pid_pop() - process %d not found\n", pname, pid);
	}

	debug((3, "child_termination() - sm_free_route(%s,%s)\n", 
													host, link));

	if (( link[0] != '\0' ) && ( host[0] != '\0' ))
	{
		if (( ret = sm_free_route(host, link)) < 0)
		{
			errlog(INT_LOG,
				"%s : child_termination() - sm_free_route() ret %d  sm_errno %d\n",
																pname, ret, sm_errno);
			debug((1,
				"child_termination() - sm_free_route() ret %d sm_errno %d\n",
																ret, sm_errno));
		}
		else
		{
			debug((3, "child_termination() - sm_free_route() returned %d\n", ret));
		}
	}
	else
	{
		/*
		 * do not execute sm_free_route - it was an argiotel call
		 * which is managed by x25_argotel process as far as 
		 * sm_free_route stuff is concerned
		 */

		debug((3, "child_termination() - Argotel call, no host, link\n"));
	}


	debug((3, "child_termination() - terminated pid %d with status %x\n", 
		pid, status));

	return;
}

/*
 *
 *  Procedure: terminate
 *
 *  Parameters: none
 *
 *  Description: call myexit() 
 *
 *  Return:
 *
 */

void  terminate(sig)
int   sig;
{
	debug((3, "terminate() - terminating on sig  %d\n", sig));
	myexit(SUCCESS);
}
