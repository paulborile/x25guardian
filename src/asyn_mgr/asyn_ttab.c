/*
 * $Id: asyn_ttab.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: tty table
 *
 * Contents: adt tt, tt_init, tt_get_free, tt_set_pid, tt_free
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: asyn_ttab.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:14  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/29  17:11:51  px25
 * tt_free now frees up a given pid returning link string
 * for sm_delete_route.
 *
 * Revision 1.1  1995/09/18  13:29:32  px25
 * Changed adt of pid-table. Added also group nme and number.
 *
 * Revision 1.0  1995/07/07  10:04:46  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: asyn_ttab.c,v 1.1.1.1 1998/11/18 15:03:14 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <errno.h>
#include        <sys/types.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "debug.h"
#include        "errlog.h"

/*  Module include files                        */

/*  Extern functions used                       */

extern int sm_errno;

/*  Extern data used                            */

/*  Local constants                             */

#define DEV_LEN 32
#define GRP_LEN 14
#define TTAB_LEN    128
#define ASY_FILE_COMMENT        '#'
#define ASY_FILE_SEPARATOR  ':'

/*  Local types                                 */

struct  TTAB
{
    char grp_name[GRP_LEN];
    char device[DEV_LEN];
    pid_t asypid;
    int grp_num;
};

/*  Local macros                                */

/*  Local data                                  */

struct  TTAB ttab[TTAB_LEN];

/*
 *
 * Procedure: tt_init
 *
 * Parameters: path name
 *
 * Description: load asy.tab file in internal table
 *
 * Return: number of ttys inserted, -1 : error
 *
 */

int tt_init(file)
char *file;
{
    int i = 0;
    char *finger;
    char *cur;
    char buf[BUFSIZ];
    FILE *fp;

    debug((3, "tt_init() - starting to load %s\n", file));

    if (( fp = fopen(file, "r")) == NULL )
    {
        debug((1, "tt_init() - fopen error, errno = %d\n", errno));
        return(-1);
    }

    memset(ttab, '\0', sizeof(struct TTAB) * TTAB_LEN);

    while ( fgets(buf, BUFSIZ, fp) != NULL )
    {
        debug((5, "tt_init() - read %s", buf));

        if (( buf[0] == ASY_FILE_COMMENT ) || ( buf[0] == '\n' ))
        {
            continue;
        }

        if (( finger = strchr(buf, ASY_FILE_SEPARATOR)) == NULL )
        {
            continue;
        }

        *finger = '\0';
        strcpy(ttab[i].grp_name, buf);
        debug((5, "inserted entry grp_name %s\n", ttab[i].grp_name));
        cur = finger + 1;

        if (( finger = strchr(cur, ASY_FILE_SEPARATOR)) == NULL )
        {
            continue;
        }

        *finger = '\0';
        ttab[i].grp_num = atoi(cur);
        debug((5, "inserted entry grp_num %d\n", ttab[i].grp_num));
        cur = finger + 1;

        if (( finger = strchr(cur, ASY_FILE_SEPARATOR)) == NULL )
        {
            continue;
        }

        *finger = '\0';
        strcpy(ttab[i].device, cur);
        debug((5, "inserted entry device %s\n", ttab[i].device));

        ttab[i].asypid = 0;
        i++;
    }

    debug((5, "tt_init() - Finished processing of %s\n", file));
    fclose(fp);
    return(i);
}


/*
 *
 * Procedure: tt_get_device
 *
 * Parameters: none
 *
 * Description: Get device name for Group_name,group_num
 *
 * Return: char * device name; NULL if group not found or not free
 *
 */


char *tt_get_device(grp_name, grp_num)
char *grp_name;
int grp_num;
{
    int i;
    static char device[DEV_LEN];

    debug((3, "tt_get_device() - search device for %s,%d\n", grp_name, grp_num));

    for (i=0; i<TTAB_LEN; i++)
    {
        if ( ttab[i].device[0] == '\0' )
        {
            continue;
        }

        if (( strcmp(ttab[i].grp_name, grp_name) == 0 ) &&
            ( ttab[i].grp_num == grp_num))
        {
            /* found desired port	*/

            if ( ttab[i].asypid == 0 )
            {
                debug((5, "tt_get_device() - found device %s\n", ttab[i].device));
                strcpy(device, ttab[i].device);
                return(device);
            }
            else
            {
                debug((1, "tt_get_device() - FOUND %s,%d, but is BUSY pid %d\n",
                       ttab[i].asypid));
                return(NULL);
            }
        }
    }

    debug((1, "tt_get_device() - device for %s,%d not found\n",
           grp_name, grp_num));
    return(NULL);
}

/*
 *
 * Procedure: tt_set_pid
 *
 * Parameters: device pid
 *
 * Description: search the device entry and update pid
 *
 * Return: 0 if ok, -1 if device not found, -2: if slot found pid busy
 *
 */

int tt_set_pid(dev, pid)
char *dev;
pid_t pid;
{
    int i;

    debug((3, "tt_set_pid() - starting for dev %s pid %d\n", dev, pid));

    for (i=0; i<TTAB_LEN; i++)
    {
        if ( ttab[i].device[0] == '\0' )
        {
            continue;
        }

        if ( strcmp(ttab[i].device, dev) == 0 )
        {
            /* found device	*/

            debug((5, "tt_set_pid() - found device %s\n", ttab[i].device));

            /* check if pid is free	*/

            if ( ttab[i].asypid == 0 )
            {
                debug((5, "tt_set_pid() - setting pid %d in slot %d\n", pid, i));
                ttab[i].asypid = pid;
                return(0);
            }
            else
            {
                /* this slot is busy	*/

                debug((1, "tt_set_pid() - slot %d busy with pid %s\n",
                       i, ttab[i].asypid));
                return(-2);
            }
        }
    }

    /* if reached means not found	*/

    debug((1, "tt_set_pid() - device %s not found\n", dev));
    return(-1);
}

/*
 *
 * Procedure: tt_free
 *
 * Parameters:  pid
 *
 * Description: search the pid entry and free pid
 *
 * Return: 0 if ok, -1 if pid not found
 *
 */


int tt_free(pid, link)
pid_t pid;
char *link;
{
    int i;

    debug((3, "tt_free() - starting for pid %d\n", pid));

    for (i=0; i<TTAB_LEN; i++)
    {
        if ( ttab[i].device[0] == '\0' )
        {
            continue;
        }

        if ( ttab[i].asypid == pid )
        {
            debug((5, "tt_free() - found pid %d for device %s\n",
                   ttab[i].asypid, ttab[i].device));
            sprintf(link, "%s,%d", ttab[i].grp_name, ttab[i].grp_num);
            ttab[i].asypid = 0;
            return(0);
        }
    }

    /* if reached means no pid in table	*/

    debug((1, "tt_free() - pid %d not found\n", pid));
    return(-1);
}

/*
 *
 * Procedure: tt_kill
 *
 * Parameters: sig
 *
 * Description: send all process signal sig
 *
 * Return: none
 *
 */


void    tt_kill(sig)
int sig;
{
    int i;

    debug((3, "tt_kill() - starting to send sig %d\n", sig));

    for (i=0; i<TTAB_LEN; i++)
    {
        if ( ttab[i].device[0] == '\0' )
        {
            continue;
        }

        if ( ttab[i].asypid != 0 )
        {
            kill(ttab[i].asypid, sig);
        }
    }
    return;
}


/*
 *
 * Procedure: tt_create_route
 *
 * Parameters: my hostname, my service, name of program for errlog
 *
 * Description: create route scanning ttab
 *
 * Return: none
 *
 */


void    tt_create_route(myhost, myserv, myname)
char *myhost;
char *myserv;
char *myname;
{
    char link[DEV_LEN];
    int i;
    int rc;

    debug((3, "tt_create_route() - creating routes for host %s\n", myhost));

    for (i=0; i<TTAB_LEN; i++)
    {
        if ( ttab[i].device[0] == '\0' )
        {
            continue;
        }

        sprintf(link, "%s,%d", ttab[i].grp_name, ttab[i].grp_num);

        debug((3, "tt_create_route() - going to create route link %s\n", link));
        rc = sm_create_route(myhost, link, myserv, 0, 1, ASY_NUA);
        if ( rc < 0 )
        {
            debug((1, "tt_create_route() - sm_create_route() returned %d\n", rc));
            errlog(INT_LOG,
                   "%s : unable to sm_create_route, sm_errno = %d\n", myname, sm_errno);
        }
    }
    return;
}

/*
 *
 * Procedure: tt_delete_route
 *
 * Parameters: my hostname, name of program for errlog
 *
 * Description: delete routes scanning ttab
 *
 * Return: none
 *
 */

void    tt_delete_route(myhost, myname)
char *myhost;
char *myname;
{
    char link[DEV_LEN];
    int i;
    int rc;

    for (i=0; i<TTAB_LEN; i++)
    {
        if ( ttab[i].device[0] == '\0' )
        {
            continue;
        }

        sprintf(link, "%s,%d", ttab[i].grp_name, ttab[i].grp_num);

        debug((3, "tt_delete_route() - deleting route %s\n", link));

        if (( rc = sm_delete_route(myhost, link)) < 0)
        {
            errlog(INT_LOG, "%s : sm_delete_route failed, sm_errno %d\n",
                   myname, sm_errno);
            debug((1,
                   "tt_delete_route() - sm_delete_route() failed with sm_errno %d\n",
                   sm_errno));
        }
        else
        {
            debug((3, "tt_delete_route() - sm_delete_route() returned %d\n", rc));
        }
    }
    return;
}
