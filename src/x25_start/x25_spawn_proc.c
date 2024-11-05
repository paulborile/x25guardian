/*
 * $Id: x25_spawn_proc.c,v 1.1.1.1 1998/11/18 15:03:35 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_start
 *
 * Contents: spwan_proc
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: x25_spawn_proc.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:35  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/09/18  13:27:58  px25
 * Added errlog when starting processes.
 *
 * Revision 1.0  1995/07/14  09:20:02  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_spawn_proc.c,v 1.1.1.1 1998/11/18 15:03:35 paul Exp $";

/*  System include files                        */

#include        <stdio.h>

/*  Project include files                       */

#include        "debug.h"
#include        "errlog.h"

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

/*
 *
 *  Procedure: spawn_proc
 *
 *  Parameters: command string
 *
 *  Description: fork,exec a new process
 *
 *  Return: the Process Pid
 *
 */

int spawn_proc(command)
char *command;
{
    int newpid;
    char exec_string[2048];

    sprintf(exec_string, "exec %s", command);

    debug((3, "spawn_proc() - %s\n", exec_string));

    newpid = fork();

    switch (newpid)
    {
    case 0:
        execl("/bin/sh", "sh", "-c", exec_string, 0);
    case -1:
        return(-1);
    default:

        errlog(INT_LOG, "PROCESS %s STARTED, PID %d\n", command, newpid );
        return(newpid);
    }
}
