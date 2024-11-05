/*
 * $Id: signal.c,v 1.1.1.1 1998/11/18 15:03:27 paul Exp $
 * 
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: POS simulator
 *
 * Contents: signal handling
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: signal.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:27  paul
 * Guardian : x25 Pos router
 *
 *
 */

static char rcsid[] = "$Id: signal.c,v 1.1.1.1 1998/11/18 15:03:27 paul Exp $";

/*  System include files                        */

#include	<stdio.h>
#include	<signal.h>

/*  Project include files                       */

/*  Module include files                        */

/*  Extern functions used                       */

/*  Local functions used                       */

int	handler();

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */


/*
 *
 * Procedure: set_signals
 *
 * Parameters: none
 *
 * Description: trap all signal with one sig handler
 *
 * Return: none
 *
 */

int	set_signals()
{
	signal(SIGHUP, (void (*)())handler);
	signal(SIGINT, (void (*)())handler);
	signal(SIGQUIT, (void (*)())handler);
	signal(SIGTERM, (void (*)())handler);
}


/*
 *
 * Procedure: handler
 *
 * Parameters:
 *
 * Description:
 *
 * Return:
 *
 */

int	handler(sig)
int	sig;
{
	printf("Signal %d received\n", sig);
	exit();
}
