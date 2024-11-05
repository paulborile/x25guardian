/*
 * $Id: sm_mgr.c,v 1.1.1.1 1998/11/18 15:03:10 paul Exp $
 *
 * Project : PX25 - Status Manager
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: sm_mgr.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:10  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.3  1995/09/18  10:57:05  giorgio
 * Async lines have been organized into tty groups so that in the routing tab
 * is possible to route incoming calls to different groups of ttys.
 * Management of direct call to a specific syncronous line on a specific
 * machine.
 *
 * Revision 1.2  1995/07/14  09:09:53  px25
 * Modified dump_1 function. Fopen and Fclose the file every time.
 *
 * Revision 1.1  1995/07/12  10:06:38  px25
 * Added TTY_NUA type in best function
 *
 * Revision 1.0  1995/07/07  10:14:23  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: sm_mgr.c,v 1.1.1.1 1998/11/18 15:03:10 paul Exp $";

/*  System include files                        */

#include <stdio.h>
#include <string.h>
#include    <signal.h>
#include <rpc/rpc.h>
#include    <ctype.h>

/*  Project include files                       */

#include        "px25_globals.h"

/*  Module include files                        */
#include "smp.h"
#include "debug.h"
#define  NAME "sm_mgr.d"
#define DUMP_FILE "/tmp/sm.txt"

/*  Extern functions used                       */

void    myexit();

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static char empty[2] = { 0, 0};
static char pid_str[20];
static int debuglevel;
static char debug_file[BUFSIZ];
extern char *optarg;
int fatal = 0;

#define MAX_LIST 200
#define BUFSIZE 150
#define PERMS 0666

struct nodo {
    char hostname[MAX_STR];
    char link[MAX_STR];
    char service[MAX_STR];
    int active;
    int max;
    int route_type;
} lista[MAX_LIST];

int *list_route_1()
{
    int i;
    static int status;

    debug((1, "list_route_1() - starting\n"));

    for (i=0; i< MAX_LIST; i++)
    {
        if ( lista[i].hostname[0] == '\0' )
        {
            continue;
        }

        debug((1, "%d: host %s, link %s, serv %s, act %d, max %d, typ(%d) %s\n",
               i, lista[i].hostname, lista[i].link, lista[i].service, lista[i].active,
               lista[i].max, lista[i].route_type,
               ( lista[i].route_type == ASY_NUA ) ?
               ASY_NUA_STRING : X25_NUA_STRING ));
    }

    debug((1, "list_route_1() - ending\n"));
    status =0;
    return(&status);
}

int *    dump_1(file)
char **file;
{
    FILE *fp = NULL;
    int i;
    static int status;
    char dummy[32];

    debug((1, "dump_1() - starting\n"));

    if ((fp = fopen(*file, "w")) == NULL)
    {
        debug((1, "dump_1 :cannot create %s\n", *file));
        status = -1;
        return (&status);
    }

    fprintf(fp, "Host\tLink\tService\t\tActive\tMax\tType\n");

    for (i=0; i< MAX_LIST; i++)
    {
        if ( lista[i].hostname[0] == '\0' )
        {
            continue;
        }

        switch (lista[i].route_type)
        {
        case    ASY_NUA:
            strcpy(dummy, ASY_NUA_STRING);
            break;

        case    X25_NUA:
            strcpy(dummy, X25_NUA_STRING);
            break;

        case    SNA_NUA:
            strcpy(dummy, SNA_NUA_STRING);
            break;

        case    TTY_NUA:
            strcpy(dummy, TTY_NUA_STRING);
            break;

        default:
            strcpy(dummy, "UNKNOWN");
            break;
        }

        fprintf(fp, "%s\t%s\t%s\t%d\t%d\t%s\n",
                lista[i].hostname, lista[i].link, lista[i].service, lista[i].active,
                lista[i].max, dummy);
    }

    debug((1, "dump_1() - ending\n"));
    status =0;
    fclose(fp);
    return(&status);
}

int *    load_1()
{
    FILE *fd;

    static int status;
    char line[MAX_STR];
    char *tok;
    char *newtok;
    int i = -1;

    if ((fd = fopen(DUMP_FILE, "r")) == NULL)
    {
        debug((1, "Unable to open file %d\n", DUMP_FILE));
        status = -1;
        return &status;
    }

    while (fgets(line, MAX_STR, fd) != NULL)
    {
        if (line[0] == '#' || line[0] == '\n')
        {
            continue;
        }
        else
        {
            if ( i < MAX_LIST )
            {
                ++i;
            }
            else{
                status = -1;
                debug((1, "The list is full"));
                return &status;
            }
            if (isalpha(line[0]) != 0)
            {
                tok = strtok(line, "\n");
                if ((tok = strtok(tok, " ")) != NULL )
                {
                    strcpy(lista[i].hostname, tok);
                }
                if ((newtok = strtok(NULL, " ")) != NULL)
                {
                    strcpy(lista[i].link, newtok);
                }
                if ((newtok = strtok(NULL, " ")) != NULL)
                {
                    strcpy(lista[i].service, newtok);
                }
                if ((newtok = strtok(NULL, " ")) != NULL)
                {
                    lista[i].active = atoi(newtok);
                }
                if ((newtok = strtok(NULL, " ")) != NULL)
                {
                    lista[i].max = atoi(newtok);
                }
                if ((newtok = strtok(NULL, " ")) != NULL)
                {
                    lista[i].route_type = atoi(newtok);
                }
                else{
                    status = -1;
                    return &status;
                }

            }
        }
    }
    status = 0;
    return &status;
}

int * free_route_1(askfree)
node *askfree;
{
    int i = 0;
    static int status;

    debug((1, "free_route_1() - starting\n"));

    for (i=0; i<MAX_LIST; i++)
    {
        if (lista[i].hostname[0] == '\0')
        {
            continue;
        }
        if (lista[i].link[0] == '\0')
        {
            continue;
        }

        if (strcmp(lista[i].hostname, askfree->machine) == 0)
        {
            if (strcmp(lista[i].link, askfree->link) == 0)
            {
                status = 0;
                if ( lista[i].active > 0 )
                {
                    lista[i].active -= 1;
                    return (&status);
                }
            }
        }
    }

    debug((1, "free_route_1() - ending\n"));

    status = -1;
    return (&status);
}

int * incr_route_1(askincr)
node *askincr;
{
    int i = 0;
    static int status;

    debug((1, "incr_route_1() - starting\n"));

    for (i=0; i<MAX_LIST; i++)
    {
        if (lista[i].hostname[0] == '\0')
        {
            continue;
        }
        if (lista[i].link[0] == '\0')
        {
            continue;
        }

        if (strcmp(lista[i].hostname, askincr->machine) == 0)
        {
            if (strcmp(lista[i].link, askincr->link) == 0)
            {
                status = 0;
                if ( lista[i].active < lista[i].max )
                {
                    lista[i].active += 1;
                    return (&status);
                }
            }
        }
    }

    debug((1, "incr_route_1() - ending\n"));

    status = -1;
    return (&status);
}

int * delete_route_1(askdel)
node *askdel;
{
    int i = 0;
    static int status;

    debug((1, "delete_route_1() - starting\n"));

    for (i=0; i<MAX_LIST; i++)
    {
        if (lista[i].hostname[0] == '\0')
        {
            continue;
        }
        if (lista[i].link[0] == '\0')
        {
            continue;
        }
        if (strcmp(lista[i].hostname, askdel->machine) == 0)
        {
            if (strcmp(lista[i].link, askdel->link) == 0)
            {
                status = 0;
                lista[i].hostname[0] = '\0';
                return (&status);
            }
        }
    }
    debug((1, "delete_route_1() - ending\n"));
    status = -1;
    return (&status);
}

node * get_best_route_1(askbest)
node *askbest;
{
    int i = 0;
    int diff = 0;
    int mydiff;
    int namediff;
    int index = -1;
    int nameindex;
    static node best;
    char copia[MAX_STR];
    char *comma;

    best.machine = empty;
    best.link = empty;
    best.service = empty;
    best.active = 0;
    best.max = 0;
    best.route_type = -1;

    debug((1, "get_best_route_1() - starting: machine %s, rt_type(%d) %s\n",
           askbest->machine, askbest->route_type,
           (askbest->route_type == ASY_NUA) ? ASY_NUA_STRING : X25_NUA_STRING ));

    switch (askbest->route_type)
    {
    case TTY_NUA:
        for (i=0; i<MAX_LIST; i++)
        {
            if (lista[i].hostname[0] == '\0')
            {
                continue;
            }
            if (lista[i].route_type == askbest->route_type)
            {
                if (lista[i].active < lista[i].max)
                {
                    mydiff = lista[i].max - lista[i].active;
                    if (mydiff > diff)
                    {
                        best.machine = lista[i].hostname;
                        best.service = lista[i].service;
                        best.link = lista[i].link;
                        diff = mydiff;
                        index = i;
                    }
                    if (strcmp(lista[i].hostname, askbest->machine) == 0)
                    {
                        best.machine = lista[i].hostname;
                        best.service = lista[i].service;
                        best.link = lista[i].link;
                        index = i;
                        break;
                    }
                }
            }
            else{ continue;}
        }
        if (best.machine[0] == '\0')
        {
            return &best;
        }
        else {
            debug((1, "get_best_route() - incrementing active..\n"));
            lista[index].active +=1;
            debug((1, "Best route %s, %s, %s\n",
                   best.machine, best.service, best.link));
            return &best;
        }
        break;

    case ASY_NUA:
        for (i=0; i<MAX_LIST; i++)
        {
            if (lista[i].hostname[0] == '\0')
            {
                continue;
            }
            if (lista[i].route_type == askbest->route_type)
            {
                if ((strchr(askbest->link, ',')) == NULL)
                {       /* se non c'e la virgola */

                    strcpy(copia, lista[i].link);

                    if ((comma = strchr(copia, ',')) != NULL)
                    {
                        *comma = '\0';

                        if (strcmp(copia, askbest->link) == 0)
                        {
                            if ((mydiff = lista[i].max - lista[i].active) > 0)
                            {
                                best.machine = lista[i].hostname;
                                best.service = lista[i].service;
                                best.link = lista[i].link;
                                index = i;
                                if (strcmp(lista[i].hostname, askbest->machine) == 0)
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
                else               /* we've got "," */
                {
                    if (strcmp(lista[i].link, askbest->link) == 0)
                    {
                        if ((mydiff = lista[i].max - lista[i].active) > 0)
                        {
                            best.machine = lista[i].hostname;
                            best.service = lista[i].service;
                            best.link = lista[i].link;
                            index = i;
                            break;
                        }
                    }
                }
            }
            else{ continue;}
        }
        if (best.machine[0] == '\0')
        {
            return &best;
        }
        else
        {
            debug((1, "get_best_route() - incrementing active..\n"));
            lista[index].active +=1;
            debug((1, "Best route %s, %s, %s\n",
                   best.machine, best.service, best.link));
            return &best;
        }
        break;

    case X25_NUA:

        if (askbest->link[0] != '\0')
        {
            for (i=0; i<MAX_LIST; i++)
            {
                if (lista[i].hostname[0] == '\0')
                {
                    continue;
                }
                if (lista[i].route_type == askbest->route_type)
                {
                    if ( (strcmp(lista[i].hostname, askbest->machine) == 0 ) && (strcmp(lista[i].link, askbest->link) == 0 ) )
                    {
                        if (lista[i].active < lista[i].max)
                        {
                            best.machine = lista[i].hostname;
                            best.service = lista[i].service;
                            best.link = lista[i].link;
                            lista[i].active += 1;
                            debug((1, "Best route %s, %s, %s\n",
                                   best.machine, best.service, best.link));
                            return &best;
                        }
#ifdef  FIND_ONLY_DIRECT
                        else
                        {
                            return &best;
                        }
#endif
                    }
                }
                else{ continue;}
            }
        }
#ifdef  FIND_ONLY_DIRECT
        else
#endif
        {
            for (i=0; i<MAX_LIST; i++)
            {
                if (lista[i].hostname[0] == '\0')
                {
                    continue;
                }
                if (lista[i].route_type == askbest->route_type)
                {
                    if (lista[i].active < lista[i].max)
                    {
                        mydiff = lista[i].max - lista[i].active;
                        if (mydiff >= diff)
                        {
                            diff = mydiff;
                            index = i;
                            if (strcmp(lista[i].hostname, askbest->machine) == 0)
                            {
                                namediff = mydiff;
                                nameindex = i;
                            }
                        }
                    }
                }
                else{ continue;}
            }

            if (index != -1)
            {
                if (namediff == diff)
                {
                    best.machine = lista[nameindex].hostname;
                    best.service = lista[nameindex].service;
                    best.link = lista[nameindex].link;
                    lista[nameindex].active += 1;
                }
                else
                {
                    best.machine = lista[index].hostname;
                    best.service = lista[index].service;
                    best.link = lista[index].link;
                    lista[index].active += 1;
                }
                debug((1, "Best route %s, %s, %s\n", best.machine, best.service, best.link));
                return &best;
            }
            else{return &best;}
        }

    default:
        debug((1, "get_best_route_1() - Unknown route type %d\n",
               askbest->route_type));
        return &best;
    }
}

int *    create_route_1(data)
node *data;
{
    int i = 0;
    static int status;

    debug((1, "create_route_1() - starting\n"));

    signal(SIGTERM, myexit);

    for (i=0; i<MAX_LIST; i++)
    {
        if ( lista[i].hostname[0] == '\0')
        {
            continue;
        }
        if (strcmp(lista[i].hostname, data->machine) == 0)
        {
            if (strcmp(lista[i].link, data->link) == 0)
            {
                status = -1;
                return((int *) &status);
            }
        }
    }

    for (i=0; i<MAX_LIST; i++)
    {
        if ( lista[i].hostname[0] == '\0')
        {
            break;
        }
    }

    if (i == MAX_LIST)
    {
        status = -1;
        return((int *) &status);
    }


    debug((1, "Inserting %s, %s, %s in slot %d\n",
           data->machine, data->link, data->service, i));
    strcpy(lista[i].hostname, data->machine);
    strcpy(lista[i].link, data->link);
    strcpy(lista[i].service, data->service);
    lista[i].active = data->active;
    lista[i].max = data->max;
    lista[i].route_type = data->route_type;

    debug((1, "create_route_1() - ending\n"));

    status = 0;
    return &status;
}

/* int* exit_1()
   {
    static int status = 0;

    debug((1,"exit_1()\n"));

 #ifdef DEBUG
   enddebug();
 #endif

    exit (&status);
   }
 */

int * init_debug_1(debug_level)
int *debug_level;
{
    static int status;
#ifdef DEBUG
    sprintf(pid_str, "%d", getpid());
    sprintf(debug_file, "/tmp/sm_mgr%05d.debug", getpid());

    initdebug(*debug_level, debug_file, NAME, pid_str);
#endif
    debug((0, "init_debug_1() - init at level %d\n", *debug_level));
    status = 0;
    return &status;
}

int * end_debug_1()
{
    static int status;
#ifdef DEBUG
    enddebug();
#endif
    status=0;
    return &status;
}

void    myexit()
{
    exit(SUCCESS);
}
