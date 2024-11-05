/*
 * $Id: router.c,v 1.1.1.1 1998/11/18 15:03:19 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: router.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:19  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/09/14  15:58:44  px25
 * Added User data to be replaced in table
 * Fixed a bug in user data matching. Partial matching was implemented
 * instead of complete matching of input user data.
 *
 * Revision 1.0  1995/07/07  10:01:01  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: router.c,v 1.1.1.1 1998/11/18 15:03:19 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <fcntl.h>
#include        <sys/param.h>
#include        <sys/types.h>
#include        <sys/stat.h>

/*  Project include files                       */

#include        "px25_globals.h"

/*  Module include files                        */

#include        "rt.h"

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

#define MAX_RT                  1024
#define RT_FILE_COMMENT     '#'

/*  Local types                                 */

/*
 * Internal routing table
 */

struct  RT_TABLE
{
    char user_data[4*MAX_USER_DATA_LEN];
    struct  NUA nua;
};

/*  Local macros                                */

#ifdef DEBUG
#   define print(a) printf a
#else
#   define print(a)
#endif

/*  Local data                                  */

int rt_errno;
static time_t last_loaded = 0;
static FILE *rt_tab_fp = NULL;
static struct  RT_TABLE rt_tab[MAX_RT];
static struct  NUA nua_found;
static char rt_table_path[MAXPATHLEN];

/*
 *
 * Procedure: rt_find()
 *
 * Parameters: call user string
 *
 * Description: find call user string in table
 *
 * Return: nua, nua type
 *
 */

struct  NUA *rt_find(user_data, user_data_len)
unsigned    char *user_data;
int user_data_len;
{
    unsigned char bin_ud[BUFSIZ];
    char ea_ud[BUFSIZ];
    int ud_len;
    int i;
    char *finger;

    bin_to_ea(user_data, ea_ud, user_data_len);

    print(("rt_find() - called with user_data %s\n", ea_ud));

    /*
     * Check first if table is already in memory
     */

    if ( !last_loaded )
    {
        if (( finger = getenv(PX25_RT_TABLE_PATH)) == NULL )
        {
            /* No routing table variable set	*/
            print(("rt_find() - %s not set\n", PX25_RT_TABLE_PATH));
            rt_errno = ERT_NO_ENV_VAR;
            return((struct NUA *) NULL);
        }

        strcpy(rt_table_path, finger);

        memset(rt_tab, '\0', sizeof(struct RT_TABLE) * MAX_RT);

        if ( !rt_load(rt_table_path, rt_tab))
        {
            return((struct NUA *) NULL);
        }
    }

#ifdef  DYNAMIC_RELOAD

    if ( rt_needs_reload(rt_table_path, last_loaded) )
    {
        memset(rt_tab, '\0', sizeof(struct RT_TABLE) * MAX_RT);

        print(("rt_find() - reloading table, was modified\n"));

        if ( !rt_load(rt_table_path, rt_tab))
        {
            return((struct NUA *) NULL);
        }
    }

#endif

    /*
     * Do the real search
     */

    for (i=0; i<MAX_RT; i++)
    {
        if ( rt_tab[i].user_data[0] == '\0' )
        {
            continue;
        }

        /*
         * Convert User Data in table to binary one
         */

        if (( ud_len = ea_to_bin(rt_tab[i].user_data, bin_ud)) == -1 )
        {
            continue;
        }

        print(("rt_find() - comparing %s, %s\n", rt_tab[i].user_data, ea_ud));

        if (( memcmp(bin_ud, user_data, ud_len) == 0 ) &&
            ( ud_len == user_data_len))
        {
            /* Ok! found match, compile NUA struct and return	*/

            print(("rt_find() - Found!\n"));
            nua_found = rt_tab[i].nua;
            rt_errno = 0;
            return(&nua_found);
        }
    }

    /*
     * If reached means no user_data matches
     */

    rt_errno = ERT_UD_NOTFOUND;
    return((struct NUA *) NULL);
}



/*
 *
 * Procedure: rt_load
 *
 * Parameters: filename, table
 *
 * Description: load from file into internal table. set last loaded times
 *
 * Return: 0 if ok
 *
 */

static int rt_load(file, tab)
char *file;
struct  RT_TABLE tab[];
{
    char udata[BUFSIZ/4];
    char udata_tobe_replaced[BUFSIZ/4];
    char nua1[BUFSIZ/4];
    char nua2[BUFSIZ/4];
    char type[BUFSIZ/4];
    int i = 0;
    char buf[BUFSIZ];

    print(("rt_load() - loading file %s\n", file));

    if ( rt_tab_fp == NULL )
    {
        if (( rt_tab_fp = fopen(file, "r")) == NULL )
        {
            /* cannot open file	*/
            rt_errno = ERT_CANNOT_OPEN;
            return(0);
        }
    }
    else
    {
        rewind(rt_tab_fp);
    }

    /*
     * load phase
     */

    while ( fgets(buf, BUFSIZ, rt_tab_fp) != NULL )
    {
        if (( buf[0] == RT_FILE_COMMENT ) || ( buf[0] == '\n' ))
        {
            continue;
        }

        /* read fields and load into array	*/

        sscanf(buf, "%s %s %s %s %s\n",
               udata, udata_tobe_replaced, nua1, nua2, type);

        print(("rt_load() - %s %s %s %s %s\n",
               udata, udata_tobe_replaced, nua1, nua2, type));

        if (  strcmp(type, ASY_NUA_STRING) == 0 )
        {
            tab[i].nua.nua_type = ASY_NUA;
        }
        else if ( strcmp(type, X25_NUA_STRING) == 0 )
        {
            tab[i].nua.nua_type = X25_NUA;
        }
        else if ( strcmp(type, SNA_NUA_STRING) == 0 )
        {
            tab[i].nua.nua_type = SNA_NUA;
        }
        else if ( strcmp(type, TTY_NUA_STRING) == 0 )
        {
            tab[i].nua.nua_type = TTY_NUA;
        }
        else
        {
            /* unknown tag in routing table	*/
            /* skip the hole line				*/
            print(("rt_load() - Unknown nua_type %s\n", type));
            continue;
        }

        strcpy(tab[i].user_data, udata);
        tab[i].nua.udata_tobe_replaced_len =
            ea_to_bin(udata_tobe_replaced, tab[i].nua.udata_tobe_replaced);
        strcpy(tab[i].nua.primary_nua, nua1);
        strcpy(tab[i].nua.secondary_nua, nua2);

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

static int rt_needs_reload(file, last_loaded)
char *file;
time_t last_loaded;
{
    int fd;
    struct  stat s;

    print(("rt_needs_reload() - starting\n"));

    if ( rt_tab_fp == NULL )
    {
        /* Yes, was never loaded	*/
        print(("rt_needs_reload() - rt_tab_fp == NULL"));
        return(1);
    }

    fd = fileno(rt_tab_fp);

    if ( fstat(fd, &s) < 0 )
    {
        /* Unable to stat fd	*/
        /* reload				*/
        print(("rt_needs_reload() - fstat < 0\n"));
        return(1);
    }

    if ( s.st_mtime > last_loaded )
    {
        /* has to be reloaded	*/
        print(("rt_needs_reload() - TRUE times %d %d\n",
               s.st_mtime, last_loaded));
        return(1);
    }
    else
    {
        print(("rt_needs_reload() - FALSE times %d %d\n",
               s.st_mtime, last_loaded));
        return(0);
    }
}
