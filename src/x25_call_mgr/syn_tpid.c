/*
 * $Id: syn_tpid.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: process table for x25_call_mgr
 *
 * Contents: set_pid, pid_find, pid_kill, pid_push, pid_pop
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: syn_tpid.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:25  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  10:16:07  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: syn_tpid.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <errno.h>
#include        <sys/types.h>

/*  Project include files                       */

#include        "debug.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

#define X25TAB_LEN  512

/*  Local types                                 */

pid_t pidtab[X25TAB_LEN];

/*  Local macros                                */

/*  Local data                                  */


/*
 *
 * Procedure: set_pid
 *
 * Parameters: none
 *
 * Description: set all the table pid to -1
 *
 * Return: none
 *
 */

void    set_pid()
{
    int i;

    debug((3, "set_pid() - starting\n" ));

    memset(pidtab, -1, sizeof(pidtab));
}

/*
 *
 * Procedure: pid_find
 *
 * Parameters:  pid
 *
 * Description: search the pid entry and free pid
 *
 * Return: 0 if ok, -1 if pid not found
 *
 */


int pid_find(pid)
pid_t pid;
{
    int i;

    if (pid == -1)
    {
        debug((3, "pid_find() - searching a place in table ...\n"));
    }
    else{
        debug((3, "pid_find() - starting for pid %d\n", pid));
    }

    for (i=0; i<X25TAB_LEN; i++)
    {
        if ( pidtab[i] == pid )
        {
            return i;
        }

    }

    /* if reached means no pid in table	*/

    if (pid == -1)
    {
        debug((1, "pid_find() - no place in table!\n"));
    }
    else{
        debug((1, "pid_find() - pid %d not found\n", pid));
    }
    return(-1);
}

/*
 *
 * Procedure: pid_kill
 *
 * Parameters: sig
 *
 * Description: send all process signal sig
 *
 * Return: none
 *
 */


void    pid_kill(sig)
int sig;
{
    int i;

    debug((3, "pid_kill() - sending sig %d\n", sig));

    for (i=0; i<X25TAB_LEN; i++)
    {
        if ( pidtab[i] != -1 )
        {
            kill(pidtab[i], sig);
            pidtab[i] = -1;
        }
    }
    return;
}

/*
 *
 * Procedure: pid_push
 *
 * Parameters: pid
 *
 * Description: push a pid in the table
 *
 * Return: 0 if success, -1 if the table is full
 *
 */

int  pid_push(pid)
pid_t pid;
{
    int i;

    if ((i = pid_find(-1)) >= 0)
    {
        debug((3, "pid_push() - starting to insert pid %d \n", pid));
        pidtab[i] = pid;
        return 0;
    }
    else
    {
        debug((1, "pid_push() - pid table is full\n"));
        return -1;
    }
}

/*
 *
 * Procedure: pid_pop
 *
 * Parameters: pid
 *
 * Description: pop a pid from the table
 *
 * Return: 0 if success, -1 if pid not find
 *
 */

int  pid_pop(pid)
pid_t pid;
{
    int i;

    debug((3, "pid_pop() - starting to cancel pid %d \n", pid));

    if ((i = pid_find(pid)) >= 0)
    {
        pidtab[i] = -1;
        return 0;
    }
    else{
        debug((3, "pid_pop() - pid not find\n"));
        return -1;
    }
}
