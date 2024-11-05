#include	<stdio.h>
#include	<time.h>

/*
 * $id:$
 *
 * $Log: c.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:02  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.5  1995/10/26  17:11:26  px25
 * Ancora modifiche
 *
 * Revision 1.4  1995/10/26  17:09:34  px25
 * Ulteriori modifiche
 *
 * Revision 1.3  1995/10/26  17:09:34  px25
 * Sardegna
 *
 * Revision 1.2  1995/10/26  17:08:09  px25
 * Aggiunto variabile d
 *
 * Revision 1.1  1995/10/26  17:07:49  px25
 * Aggiunto variabile b
 *
 * Revision 1.0  1995/10/26  17:07:30  px25
 * Initial revision
 *
 *
 */

char	c[2];
char	b[2];
char	d[2];

/* ALtra modifica 	*/



main()
{
	int	clock;
	int	rc;

	
	time(&clock);

	printf("%d\n", clock);
}

pippo()
{
	int	clock;
	int	rc;

	
	time(&clock);

	printf("%d\n", clock);
}
