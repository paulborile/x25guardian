/*
 * $Id: x25_listener.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_listener
 *
 * Contents: listen for incoming x25 calls
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_listener.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:25  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.4  1995/09/29  17:08:21  px25
 * New Argotel procesdure. Check (if UD present) in routing tabel first
 * if nothing found, go t o normal argotel procedure, else like easyway
 *
 * Revision 1.3  1995/09/18  13:03:18  px25
 * No more getparam of transfer_udata and transfer_facilities.
 * New sub-nua routing of calls
 *
 * Revision 1.2  1995/07/14  09:17:39  px25
 * Moved tables and parameters load point inside x25listen loop.
 *
 * Revision 1.1  1995/07/12  13:52:38  px25
 * Added load of params : argotel_pid, easyway_pid, transfer_udata,
 * transfer_facilities
 * Restyling of errlogs.
 * Loading of local (mynua) nua from sync.tab
 * Call to security_check function to check out incoming nua.
 *
 * Revision 1.0  1995/07/07  10:21:16  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_listener.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <sys/types.h>
#include        <stdlib.h>
#include        <sys/socket.h>
#include        <string.h>
#include        <errno.h>
#include        <signal.h>

#include        <x25.h>
#include        <neterr.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "sm.h"
#include        "rt.h"
#include        "debug.h"
#include        "errlog.h"

/*  Module include files                        */

/*  Extern functions used                       */

extern int     easyway();
extern int     argotel();
extern char **lin_toks();
extern struct  NUA *rt_find();

/*  Local functions used                       */

void    get_command_line();
void    x25_listener_usage();
void  child_termination();
void  myexit();
void  terminate();
int load_tables();

/*  Extern data used                            */

/*  Local constants                             */

#define OPTS                "d:p:s:"
#define  HOSTNAME_LEN   32

/*  Local types                                 */

/*  Local macros                                */

#define X25_ADLEN   (X25_ADDRLEN+X25_ADDREXT+2)

/*  Local data                                  */

static struct  x25data facility;
static struct  x25data user_data;

char pname[BUFSIZ];                     /* Program name */
char myhostname[HOSTNAME_LEN];
static char remote_addr[X25_ADLEN];
static char local_addr[X25_ADLEN];
char mynua[X25_ADLEN];
static int cid;
static int info = 0;

int listenport  = -1;
char listenportstr[MAX_STR];
int security = 0;
static int xerr;

static char pid_str[20];
int debuglevel = -1;
static char debug_file[BUFSIZ];
int sigterm_received = 0;


/*
 *
 * Procedure: main
 *
 * Parameters: argc, argv
 *
 * Description: the main listener process
 *
 * Return:-
 *
 */


main(argc, argv)
int argc;
char **argv;
{
    pid_t pid;
    char msg[ENETMSGLEN];
    char *finger;
    char sub_nua[X25_ADLEN];
    char ea_ud[MAX_USER_DATA_LEN * 3];
    struct  NUA *nua;

    /*
     * Globalize name of the program
     */

    strcpy(pname, argv[0]);

    get_command_line(argc, argv);

#ifdef DEBUG
    sprintf(pid_str, "%d", getpid());
    sprintf(debug_file, "/tmp/x25_listener%05d.debug", getpid());
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    sprintf(listenportstr, "%d", listenport);

    /*
     * set signals
     */

    set_signals();

    /*
     * set pid table
     */

    set_pid();

    /*
     * get my host name
     */

    if ( gethostname(myhostname, HOSTNAME_LEN) < 0 )
    {
        debug((1, "main() - unable to gethostname() errno %d\n", errno));
        errlog(INT_LOG,
               "%s : gethostname - errno %d\n", pname, errno);
        exit(FAILURE);
    }

    /*
     * Initialize x25 toolkit
     */

    if ( x25init(0) < 0 )
    {
        x25errormsg(msg);
        errlog(INT_LOG, "%s : x25init error %d, %s\n", pname, x25error(), msg);
        exit(FAILURE);
    }

    x25version(msg);
    debug((1, "main() - %s\n", msg));

    /*
     * load tables and parameters
     */

    mynua[0] = '\0';

    /*
     * now LISTEN on the specified port
     */

    while (1)
    {
        /*
         * Reload parameters before each x25listen
         */

        load_tables(listenport);

        strcpy(local_addr, "");
        strcpy(remote_addr, "");

        memset(&user_data, '\0', sizeof(struct x25data));
        memset(&facility, '\0', sizeof(struct x25data));

        /* Qualified listen	*/

        info = XI_QBIT;

        debug((3, "main() - Port %d, Going to listen ....\n", listenport));

        if ( x25xlisten(&cid, X25WAIT, listenport, info, &facility, &user_data,
                        remote_addr, local_addr, X25NULLFN) < 0)
        {
            xerr = x25error();
            x25errormsg(msg);

            debug((1, "x25listen x25error %d %s\n", xerr, msg));

            if ( xerr == EX25INTR)
            {
                debug((1, "main() - EX25INTR received\n"));
                if ( sigterm_received )
                {
                    myexit(SUCCESS);
                }
                continue;
            }

            errlog(X25_LOG, "%s : x25listen error %d, %s\n",
                   pname, xerr, msg);
            debug((1, "x25listen x25error %d %s\n", xerr, msg));
            myexit(FAILURE);
        }

        debug((3, "main() - CALL RECEIVED FROM %s TO %s cid %d\n",
               remote_addr, local_addr, cid));
        debug((3, "USERDATA len = %d\n", user_data.xd_len));
        debug((3, "USERDATA PID %x %x %x %x\n", user_data.xd_data[0],
               user_data.xd_data[1], user_data.xd_data[2], user_data.xd_data[3]));

        debug((3, "FACILITY len = %d\n", facility.xd_len));
        debug((3, "FACILITY %x %x %x %x\n", facility.xd_data[0],
               facility.xd_data[1], facility.xd_data[2], facility.xd_data[3]));

        /*
         * Check if remote nua is empty
         */

        if ( strlen(remote_addr) == 0 )
        {
            strcpy(remote_addr, "unknown-nua");
        }

        /*
         * check some security issues
         */

        if ( security )
        {
            if ( security_check(remote_addr, local_addr,
                                &facility, &user_data) == -1 )
            {
                if ( x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
                {
                    xerr = x25error(); x25errormsg(msg);
                    debug((1, "main() - x25hangup error %d %s\n", xerr, msg));
                    errlog(INT_LOG, "%s - x25hangup() failed with error %d, %s\n",
                           pname, xerr, msg);
                    continue;
                }

                errlog(X25_LOG, "%s : CALL CLEARED FOR SECURITY PROBLEMS\n",
                       remote_addr);

                debug((3, "main() - x25hangup sent\n"));
                continue;
            }
        }

        debug((3, "main() - Incrementing Input active channels for port %d\n",
               listenport ));

        sm_incr_route(myhostname, listenportstr);

        /*
         * Check first if it is SUB_NUA routing
         */

        if ( strlen(local_addr) > strlen(mynua) )
        {
            /* it is sub_nua routing	*/

            strcpy(sub_nua, &local_addr[strlen(mynua)]);

            debug((3, "main() - Routing by sub-nua %s\n", sub_nua));

            errlog(X25_LOG, "%s : %s:%d INCOMING CALL TO %s SUB NUA %s\n",
                   remote_addr, myhostname, listenport, local_addr, sub_nua);

            if ( subnua(cid, &user_data, &facility,
                        remote_addr, local_addr, info, sub_nua) == -1 )
            {
                if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
                {
                    x25errormsg(msg);
                    debug((1, "main() - x25hangup() failed error %d %s\n",
                           x25error(), msg));
                    errlog(INT_LOG, "%s - x25hangup() failed with error %d, %s\n",
                           pname, x25error(), msg);
                    continue;
                }
                sm_free_route(myhostname, listenportstr);
                debug((3, "main() - x25hangup sent\n"));

                errlog(X25_LOG, "%s : %s:%d INCOMING CALL CLEARED\n",
                       remote_addr, myhostname, listenport );
            }
            continue;
        }

        debug((3, "main() - checking if user data is routable\n"));

        if ( user_data.xd_len > 0 )
        {
            bin_to_ea(user_data.xd_data, ea_ud, user_data.xd_len);

            errlog(X25_LOG, "%s : %s:%d INC CALL TO %s UD %s\n",
                   remote_addr, myhostname, listenport, local_addr,
                   ea_ud);
            /*
             * We must check if it is routable by means of the normal
             * routing table. If yes process as normal old-easyway call
             */

            if ((nua = rt_find(user_data.xd_data,
                               user_data.xd_len)) != (struct NUA *) NULL)
            {
                if ( easyway(cid, &user_data, &facility,
                             remote_addr, local_addr, info) == -1 )
                {
                    if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
                    {
                        x25errormsg(msg);
                        debug((1, "main() - x25hangup() failed error %d %s\n",
                               x25error(), msg));
                        errlog(INT_LOG, "%s - x25hangup() failed with error %d, %s\n",
                               pname, x25error(), msg);
                    }
                    sm_free_route(myhostname, listenportstr);
                    debug((3, "main() - x25hangup sent\n"));

                    errlog(X25_LOG, "%s : %s:%d INCOMING CALL CLEARED\n",
                           remote_addr, myhostname, listenport);
                }
                continue;
            }

            /* It is not routable so use normal procedure */

            if ( argotel(cid, &user_data, &facility,
                         remote_addr, local_addr, info) == -1 )
            {
                if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
                {
                    xerr = x25error(); x25errormsg(msg);
                    debug((1, "main() - x25hangup error %d %s\n", xerr, msg));

                    errlog(INT_LOG, "%s - x25hangup() failed with error %d, %s\n",
                           pname, xerr, msg);
                }
                sm_free_route(myhostname, listenportstr);
                debug((3, "main() - x25hangup sent\n"));

                errlog(X25_LOG, "%s : %s:%d INCOMING CALL CLEARED\n",
                       remote_addr, myhostname, listenport);
            }
            continue;
        }

        debug((3, "main() - Call with no user data\n"));

        if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
        {
            x25errormsg(msg);
            debug((1, "main() - x25hangup() failed error %d %s\n",
                   x25error(), msg));
            errlog(INT_LOG, "%s - x25hangup() failed with error %d, %s\n",
                   pname, x25error(), msg);
            continue;
        }
        sm_free_route(myhostname, listenportstr);
        debug((3, "main() - x25hangup sent\n"));

        errlog(X25_LOG, "%s : %s:%d INCOMING CALL WITH NO PID.CLEARED.\n",
               remote_addr, myhostname, listenport);
    } /* end while(1)	*/
}


/*
 *
 *  Procedure: get_command_line
 *
 *  Parameters: argc, argv
 *
 *  Description: parse command line
 *
 *  Return: none
 *
 */

void    get_command_line(argc, argv)
int argc;
char **argv;
{
    int fatal = 0;
    int c;

    extern char *optarg;

    while ((c = getopt(argc, argv, OPTS)) !=EOF)
    {
        switch (c)
        {
        case    'd':
            debuglevel  =   atoi(optarg);
            break;

        case    'p':
            listenport = atoi(optarg);
            break;

        case    's':
            security = atoi(optarg);
            break;

        default:
            printf("%s : invalid flag.\n", argv[0]);
            x25_listener_usage(argc, argv);
            exit(FAILURE);
        }
    }

#ifdef DEBUG
    if ( debuglevel == -1 )
    {
        fatal++;
    }
#endif

    if ( listenport == -1 )
    {
        fatal++;
    }

    if ( fatal )
    {
        x25_listener_usage(argc, argv);
        exit(FAILURE);
    }
}


/*
 *
 *  Procedure: x25_listener_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void    x25_listener_usage(argc, argv)
int argc;
char **argv;
{
#ifdef  DEBUG
    printf("Usage: %s -d<debuglevel> -p<port> [-s<security_level>]\n", argv[0]);
#else
    printf("Usage: %s -p<port> [-s<security_level>]\n", argv[0]);
#endif
}


/*
 *
 *  Procedure: myexit
 *
 *  Parameters: none
 *
 *  Description: close file descriptors,
 *						free route associated with X25_listener,
 *						send SIGTERM to childs and exit
 *
 *  Return:
 *
 */

void  myexit(code)
int code;
{
    char link[MAX_STR];
    char host[MAX_STR];
    int ret;
    char msg[ENETMSGLEN];
    int pid;

    debug((3, "myexit() - terminating code %d\n", code));

    while (( pid = pid_scan(link, host)) != -1 )
    {
        /* for each link execute sm_free_route	*/

        if (( host[0] != '\0' ) && ( link[0] != '\0' ))
        {
            if (( ret = sm_free_route(host, link)) < 0)
            {
                errlog(INT_LOG, "%s : sm_free_route() failed with sm_errno %d\n",
                       pname, sm_errno);
                debug((1, "myexit() - sm_free_route() failed with sm_errno %d\n",
                       sm_errno));
            }
            else
            {
                debug((3, "myexit() - sm_free_route() returned %d\n", ret));
            }
        }
        else
        {
            /* host and link are 0 so it was argotel call	*/
            debug((3, "myexit() - argotel call; no need for sm_free\n"));
        }
    }

    /* kill all processes	*/

    pid_kill(SIGTERM);

    x25cancel(cid, NULL);

    debug((3, "myexit() - Terminating\n"));
#ifdef DEBUG
    enddebug();
#endif

    /****
       errlog(INT_LOG, "%s : PORT %d LISTENER TERMINATING.\n", pname,listenport);
    ****/

    x25exit();
    exit(code);
}

/*
 *
 *  Procedure: load_params
 *
 *  Parameters: none
 *
 *  Description:
 *
 *  Return:
 */

int load_tables(port)
int port;
{
    int len;
    int found;
    char **fields;
    char buffer[BUFSIZ];
    FILE *fp;
    char *finger;
    char portstr[MAX_STR];

#define COMMENT '#'
#define SYNC_FILE_FIELDS        4

    sprintf(portstr, "%d", port);

    debug((3, "load_tables() - starting port = %d\n", port));

    /*
     * Get env var to point to sync file
     */

    if (( finger = getenv(PX25_SYN_TABLE_PATH)) == NULL )
    {
        errlog(INT_LOG, "%s : PX25_SYN_TABLE_PATH not set !\n", pname);
        security = 0;
        strcpy(mynua, "");
        errlog(INT_LOG, "%s : RELAXING SECURITY LEV TO 0\n", pname);
        return(-1);
    }

    /*
     * opene it to find out my nua
     */

    if (( fp = fopen(finger, "r")) == NULL )
    {
        errlog(INT_LOG, "%s : CANNOT OPEN %s !\n", pname, finger);
        security = 0;
        strcpy(mynua, "");
        errlog(INT_LOG, "%s : RELAXING SECURITY LEV TO 0\n", pname);
        return(-1);
    }

    while ( fgets(buffer, BUFSIZ, fp) != NULL )
    {
        if (( buffer[0] == COMMENT ) || ( buffer[0] == '\n' ))
        {
            continue;
        }

        if (( fields = lin_toks(buffer, SYNC_FILE_FIELDS)) != NULL )
        {
            if ( strcmp(portstr, fields[0]) == 0 )
            {
                /* found	correct port	*/

                strcpy(mynua, fields[SYNC_FILE_FIELDS-1]);
                found = 1;
                break;
            }
        }
    }

    /*
     * end of file or port found
     */

    fclose(fp);

    debug((3, "load_tables() - end of synctab mynua = %s\n", mynua));

    /*
     * now read security level from general parameters file
     */

    if (( finger = getenv(PX25_GEN_TABLE)) != NULL )
    {
        if (( security = get_param(finger, "security_level")) == -1 )
        {
            errlog(INT_LOG,
                   "%s : Parameter \"security_level\" not found\n", pname);

            security = 1;
        }
    }
    else
    {
        errlog(INT_LOG, "%s: Environment var PX25_GEN_TABLE not set\n");
    }

    debug((3, "load_tables() - security_level %d\n", security));

    return(0);
}
