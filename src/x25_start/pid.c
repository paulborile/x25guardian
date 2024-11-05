/*
 * $Id: pid.c,v 1.1.1.1 1998/11/18 15:03:35 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: process table for supervisor
 *
 * Contents: set_pid, pid_kill, pid_push, pid_pop, pid_scan
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: pid.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:35  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/14  09:20:02  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: pid.c,v 1.1.1.1 1998/11/18 15:03:35 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <errno.h>
#include        <sys/types.h>
#include        <sys/signal.h>
#include        <sys/procset.h>

/*  Project include files                       */

#include        "debug.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

#define X25TAB_LEN  512

/*  Local types                                 */

struct  process
{
    pid_t pid;
    char command[BUFSIZ];
};

struct  process pidtab[X25TAB_LEN];

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

    debug((3, "set_pid() - starting\n"));

    for (i=0; i < X25TAB_LEN; i++)
    {
        pidtab[i].pid = -1;
        memset(pidtab[i].command, '\0', BUFSIZ);
    }

    debug((3, "set_pid() - terminating\n"));
    return;
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


static int pid_find(pid)
pid_t pid;
{
    int i;

    debug((3, "pid_find() - starting for pid %d\n", pid));

    for (i=0; i<X25TAB_LEN; i++)
    {
        if ( pidtab[i].pid == pid )
        {
            debug((3, "pid_find() - found entry %d\n", i));
            return(i);
        }
    }

    /* if reached means no pid in table	*/

    debug((1, "pid_find() - pid %d not found\n", pid));
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

    debug((3, "pid_kill() - Going to send sig %d\n", sig));

    for (i=0; i<X25TAB_LEN; i++)
    {
        if ( pidtab[i].pid != -1 )
        {
            debug((3, "pid_kill() - killing %d\n", pidtab[i].pid));
            sigsend(P_PID, pidtab[i].pid, sig);
            sleep(1);
        }
    }
    debug((3, "pid_kill() - terminating\n"));
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

int pid_push(pid, command)
pid_t pid;
char *command;
{
    int i;

    debug((3, "pid_push() - starting for pid %d, command %s\n",
           pid, command ));

    if ((i = pid_find(-1L)) >= 0)
    {
        debug((3, "pid_push() - inserting pid %d com %s\n", pid, command));
        pidtab[i].pid = pid;
        strcpy(pidtab[i].command, command);
        return 0;
    }

    debug((1, "pid_push() - pid table is full returning -1\n"));
    return -1;
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

int pid_pop(pid, command)
pid_t pid;
char *command;
{
    int i;

    debug((3, "pid_pop() - starting to cancel pid %d \n", pid));

    if ((i = pid_find(pid)) >= 0)
    {
        pidtab[i].pid = -1;
        strcpy(command, pidtab[i].command);
        memset(pidtab[i].command, '\0', BUFSIZ);
        debug((3, "pid_pop() - found pid %d. returning\n", pid));
        return 0;
    }

    debug((1, "pid_pop() - pid %d not found\n", pid));
    return -1;
}


/*
 *
 * Procedure: pid_scan
 *
 * Parameters: link
 *
 * Description: for each entry in pidtab return pid and fill link
 *
 * Return: pid or -1 if table is finished
 *
 */

int pid_scan(command)
char *command;
{
    static int i = 0;

    debug((3, "pid_scan() - starting i = %d\n", i));

    while ( i < X25TAB_LEN )
    {
        if ( pidtab[i].pid == -1 )
        {
            i++;
            continue;
        }

        strcpy(command, pidtab[i].command);
        debug((3, "pid_scan() - returning i = %d\n", i));
        return(pidtab[i++].pid);
    }

    /* end of table	*/

    i = 0;
    debug((3, "pid_scan() - end of table - ret -1\n"));
    return(-1);
}
