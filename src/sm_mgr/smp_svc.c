/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <stdio.h>
#include <signal.h>
#include <rpc/rpc.h>
#include <memory.h>
#include <stropts.h>
#include <netconfig.h>
#include <stropts.h>
#include <sys/resource.h>
#ifdef SYSLOG
#include <syslog.h>
#else
#define LOG_ERR 1
#define openlog(a, b, c)
#endif
#include "smp.h"

#ifdef DEBUG
#define RPC_SVC_FG
#endif

#define _RPCSVC_CLOSEDOWN 120

static void rdbprog_1();
static void msgout();
static void closedown();

static int _rpcpmstart;     /* Started by a port monitor ? */
static int _rpcfdtype;      /* Whether Stream or Datagram ? */
static int _rpcsvcdirty;    /* Still serving ? */

main()
{
    pid_t pid;
    int i;
    char mname[FMNAMESZ + 1];

    if (!ioctl(0, I_LOOK, mname) &&
        (!strcmp(mname, "sockmod") || !strcmp(mname, "timod")))
    {
        char *netid;
        struct netconfig *nconf = NULL;
        SVCXPRT *transp;
        int pmclose;
        extern char *getenv();

        _rpcpmstart = 1;
        if ((netid = getenv("NLSPROVIDER")) == NULL)
        {
            msgout("cannot get transport name");
        }
        else if ((nconf = getnetconfigent(netid)) == NULL)
        {
            msgout("cannot get transport info");
        }
        if (strcmp(mname, "sockmod") == 0)
        {
            if (ioctl(0, I_POP, 0) || ioctl(0, I_PUSH, "timod"))
            {
                msgout("could not get the right module");
                exit(1);
            }
        }
        pmclose = (t_getstate(0) != T_DATAXFER);
        if ((transp = svc_tli_create(0, nconf, NULL, 0, 0)) == NULL)
        {
            msgout("cannot create server handle");
            exit(1);
        }
        if (nconf)
        {
            freenetconfigent(nconf);
        }
        if (!svc_reg(transp, RDBPROG, RDBVERS, rdbprog_1, 0))
        {
            msgout("unable to register (RDBPROG, RDBVERS).");
            exit(1);
        }
        if (pmclose)
        {
            (void) signal(SIGALRM, closedown);
            (void) alarm(_RPCSVC_CLOSEDOWN);
        }
        svc_run();
        exit(1);
        /* NOTREACHED */
    }
#ifndef RPC_SVC_FG
    pid = fork();
    if (pid < 0)
    {
        perror("cannot fork");
        exit(1);
    }
    if (pid)
    {
        exit(0);
    }
    for (i = 0; i < 20; i++)
        (void) close(i);
    setsid();
    openlog("smp", LOG_PID, LOG_DAEMON);
#endif
    if (!svc_create(rdbprog_1, RDBPROG, RDBVERS, "netpath"))
    {
        msgout("unable to create (RDBPROG, RDBVERS) for netpath.");
        exit(1);
    }

    svc_run();
    msgout("svc_run returned");
    exit(1);
    /* NOTREACHED */
}

static void
rdbprog_1(rqstp, transp)
struct svc_req *rqstp;
register SVCXPRT *transp;
{
    union {
        node create_route_1_arg;
        node get_best_route_1_arg;
        node free_route_1_arg;
        node incr_route_1_arg;
        node delete_route_1_arg;
        int init_debug_1_arg;
        char *dump_1_arg;
    } argument;
    char *result;
    bool_t (*xdr_argument)(), (*xdr_result)();
    char *(*local)();

    _rpcsvcdirty = 1;
    switch (rqstp->rq_proc) {
    case NULLPROC:
        (void) svc_sendreply(transp, xdr_void, (char *)NULL);
        _rpcsvcdirty = 0;
        return;

    case CREATE_ROUTE:
        xdr_argument = xdr_node;
        xdr_result = xdr_int;
        local = (char *(*)()) create_route_1;
        break;

    case GET_BEST_ROUTE:
        xdr_argument = xdr_node;
        xdr_result = xdr_node;
        local = (char *(*)()) get_best_route_1;
        break;

    case FREE_ROUTE:
        xdr_argument = xdr_node;
        xdr_result = xdr_int;
        local = (char *(*)()) free_route_1;
        break;

    case INCR_ROUTE:
        xdr_argument = xdr_node;
        xdr_result = xdr_int;
        local = (char *(*)()) incr_route_1;
        break;

    case DELETE_ROUTE:
        xdr_argument = xdr_node;
        xdr_result = xdr_int;
        local = (char *(*)()) delete_route_1;
        break;

    case LIST_ROUTE:
        xdr_argument = xdr_void;
        xdr_result = xdr_int;
        local = (char *(*)()) list_route_1;
        break;

    case INIT_DEBUG:
        xdr_argument = xdr_int;
        xdr_result = xdr_int;
        local = (char *(*)()) init_debug_1;
        break;

    case END_DEBUG:
        xdr_argument = xdr_void;
        xdr_result = xdr_int;
        local = (char *(*)()) end_debug_1;
        break;

    case DUMP:
        xdr_argument = xdr_wrapstring;
        xdr_result = xdr_int;
        local = (char *(*)()) dump_1;
        break;

    case LOAD:
        xdr_argument = xdr_void;
        xdr_result = xdr_int;
        local = (char *(*)()) load_1;
        break;

    default:
        svcerr_noproc(transp);
        _rpcsvcdirty = 0;
        return;
    }
    (void) memset((char *)&argument, 0, sizeof (argument));
    if (!svc_getargs(transp, xdr_argument, &argument))
    {
        svcerr_decode(transp);
        _rpcsvcdirty = 0;
        return;
    }
    result = (*local)(&argument, rqstp);
    if (result != NULL && !svc_sendreply(transp, xdr_result, result))
    {
        svcerr_systemerr(transp);
    }
    if (!svc_freeargs(transp, xdr_argument, &argument))
    {
        msgout("unable to free arguments");
        exit(1);
    }
    _rpcsvcdirty = 0;
    return;
}

static void
msgout(msg)
char *msg;
{
#ifdef RPC_SVC_FG
    if (_rpcpmstart)
    {
        syslog(LOG_ERR, msg);
    }
    else{
        (void) fprintf(stderr, "%s\n", msg);
    }
#else
    syslog(LOG_ERR, msg);
#endif
}

static void
closedown()
{
    if (_rpcsvcdirty == 0)
    {
        extern fd_set svc_fdset;
        static int size;
        int i, openfd;
        struct t_info tinfo;

        if (!t_getinfo(0, &tinfo) && (tinfo.servtype == T_CLTS))
        {
            exit(0);
        }
        if (size == 0)
        {
            struct rlimit rl;

            rl.rlim_max = 0;
            getrlimit(RLIMIT_NOFILE, &rl);
            if ((size = rl.rlim_max) == 0)
            {
                return;
            }
        }
        for (i = 0, openfd = 0; i < size && openfd < 2; i++)
            if (FD_ISSET(i, &svc_fdset))
            {
                openfd++;
            }
        if (openfd <= 1)
        {
            exit(0);
        }
    }
    (void) alarm(_RPCSVC_CLOSEDOWN);
}