/*
 * $Id: sub_router.c,v 1.1.1.1 1998/11/18 15:03:19 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: sub_router.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:19  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/14  16:00:23  px25
 * Allow also routing to Group with no number.
 *
 * Revision 1.1  1995/09/05  12:42:04  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: sub_router.c,v 1.1.1.1 1998/11/18 15:03:19 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <fcntl.h>
#include        <string.h>
#include        <sys/param.h>
#include        <sys/types.h>
#include        <sys/stat.h>

/*  Project include files                       */

#include        "px25_globals.h"

/*  Module include files                        */

#include        "group.h"

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

#define MAX_RT                  100
#define RT_FILE_COMMENT     '#'
#define GRP_SEPARATOR           ','

/*  Local types                                 */

/*
 * Internal routing table
 */

struct  SUB_RT_TABLE
{
    char sub_nua[MAX_USER_DATA_LEN];
    struct  GROUP grp;
};

/*  Local macros                                */

#ifdef DEBUG
#   define print(a) printf a
#else
#   define print(a)
#endif

/*  Local data                                  */

int sub_rt_errno;
static time_t last_loaded = 0;
static FILE *sub_rt_tab_fp = NULL;
static struct  SUB_RT_TABLE sub_rt_tab[MAX_RT];
static char sub_rt_table_path[MAXPATHLEN];
static struct  GROUP grp_found;

/*
 *
 * Procedure: sub_rt_find()
 *
 * Parameters: sub_nua string
 *
 * Description: find sub nua in table
 *
 * Return:
 *
 */

struct  GROUP *sub_rt_find(sub_nua)
char *sub_nua;
{
    int i = 0;
    char *finger;

    print(("sub_rt_find() - called with sub_nua %s\n", sub_nua));

    /*
     * Check first if table is already in memory
     */

    if ( !last_loaded )
    {
        if (( finger = getenv(PX25_SUB_RT_TABLE_PATH)) == NULL )
        {
            /* No routing table variable set	*/
            print(("sub_rt_find() - %s not set\n", PX25_SUB_RT_TABLE_PATH));
            sub_rt_errno = ESUB_RT_NO_ENV_VAR;
            return((struct GROUP *) NULL);
        }

        strcpy(sub_rt_table_path, finger);

        memset(sub_rt_tab, '\0', sizeof(struct SUB_RT_TABLE) * MAX_RT);

        if ( !sub_rt_load(sub_rt_table_path))
        {
            return((struct GROUP *) NULL);
        }
    }

#ifdef  DYNAMIC_RELOAD

    if ( sub_rt_needs_reload(sub_rt_table_path, last_loaded) )
    {
        memset(sub_rt_tab, '\0', sizeof(struct SUB_RT_TABLE) * MAX_RT);

        print(("sub_rt_find() - reloading table, was modified\n"));

        if ( !sub_rt_load(sub_rt_table_path))
        {
            return((struct GROUP *) NULL);
        }
    }

#endif

    /*
     * Do the real search
     */

    for (i=0; i<MAX_RT; i++)
    {
        if ( sub_rt_tab[i].sub_nua[0] == '\0' )
        {
            continue;
        }

        print(("sub_rt_find() - comparing %s, %s\n",
               sub_rt_tab[i].sub_nua, sub_nua));

        if ( strcmp(sub_rt_tab[i].sub_nua, sub_nua) == 0 )
        {
            /* Ok! found match, compile GROUP struct and return	*/

            print(("sub_rt_find() - Found!\n"));
            grp_found = sub_rt_tab[i].grp;
            sub_rt_errno = 0;
            return(&grp_found);
        }
    }

    /*
     * If reached means no user_data matches
     */

    sub_rt_errno = ESUB_RT_UD_NOTFOUND;
    return((struct GROUP *) NULL);
}



/*
 *
 * Procedure: sub_rt_load
 *
 * Parameters: filename
 *
 * Description: load from file into internal table. set last loaded times
 *
 * Return: 0 if ok
 *
 */

static int sub_rt_load(file)
char *file;
{
    int i = 0;
    char sn[MAX_USER_DATA_LEN], gr[MAX_USER_DATA_LEN];
    char *finger;
    char buf[BUFSIZ];

    print(("sub_rt_load() - loading file %s\n", file));

    if ( sub_rt_tab_fp == NULL )
    {
        if (( sub_rt_tab_fp = fopen(file, "r")) == NULL )
        {
            /* cannot open file	*/
            sub_rt_errno = ESUB_RT_CANNOT_OPEN;
            return(0);
        }
    }
    else
    {
        rewind(sub_rt_tab_fp);
    }

    /*
     * load phase
     */

    while ( fgets(buf, BUFSIZ, sub_rt_tab_fp) != NULL )
    {
        if (( buf[0] == RT_FILE_COMMENT ) || ( buf[0] == '\n' ))
        {
            continue;
        }

        /* read fields and load into array	*/

        sscanf(buf, "%s %s\n", sn, gr);

        print(("sub_rt_load() - %s %s\n", sn, gr));

        if (( finger = strchr(gr, GRP_SEPARATOR)) == NULL )
        {
            /* No number associated to group		*/

            strcpy(sub_rt_tab[i].sub_nua, sn);
            strcpy(sub_rt_tab[i].grp.grp_name, gr);
            sub_rt_tab[i].grp.grp_num = -1;

        }
        else
        {

            *finger = '\0';

            strcpy(sub_rt_tab[i].sub_nua, sn);
            strcpy(sub_rt_tab[i].grp.grp_name, gr);
            sub_rt_tab[i].grp.grp_num = atoi(finger + 1);
        }

        i++;
    }

    /* do not close */

    last_loaded = time(0);
    return(1);
}


/*
 *
 * Procedure: rt_needs_reload
 *
 * Parameters:
 *
 * Description:
 *
 * Return:
 *
 */

static int sub_rt_needs_reload(file, last_loaded)
char *file;
time_t last_loaded;
{
    int fd;
    struct  stat s;

    print(("sub_rt_needs_reload() - starting\n"));

    if ( sub_rt_tab_fp == NULL )
    {
        /* Yes, was never loaded	*/
        print(("sub_rt_needs_reload() - rt_tab_fp == NULL"));
        return(1);
    }

    fd = fileno(sub_rt_tab_fp);

    if ( fstat(fd, &s) < 0 )
    {
        /* Unable to stat fd	*/
        /* reload				*/

        print(("sub_rt_needs_reload() - fstat < 0\n"));
        return(1);
    }

    if ( s.st_mtime > last_loaded )
    {
        /* has to be reloaded	*/
        print(("sub_rt_needs_reload() - TRUE times %d %d\n",
               s.st_mtime, last_loaded));
        return(1);
    }
    else
    {
        print(("sub_rt_needs_reload() - FALSE times %d %d\n",
               s.st_mtime, last_loaded));
        return(0);
    }
}
