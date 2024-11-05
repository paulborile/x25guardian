/*
 * $Id: subnua.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: subnua.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:25  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/09/29  17:09:06  px25
 * Added mos_recv for ASY_CALL_{REJECT,ACCEPT}
 *
 * Revision 1.0  1995/09/18  12:39:30  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: subnua.c,v 1.1.1.1 1998/11/18 15:03:25 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <sys/types.h>
#include        <signal.h>
#include        <string.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <netdb.h>
#include        <x25.h>
#include        <neterr.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "debug.h"
#include        "rt.h"
#include        "group.h"
#include        "sm.h"
#include        "errlog.h"

/*  Module include files                        */

/*  Extern functions used                       */

extern struct  GROUP *sub_rt_find();
extern struct  BEST_ROUTE *sm_get_best_route();

/*  Extern data used                            */

extern int sub_rt_errno;
extern int sm_errno;
extern int errno;
extern int debuglevel;
extern char myhostname[];
extern char pname[];

/*  Local constants                             */

#define X25_ADLEN   (X25_ADDRLEN+X25_ADDREXT+2)
#define  PROTOCOL               "tcp"

#ifdef  DEBUG
#define X25_EASYWAY_PROC    "x25_subnua.d"
#define X25_INCALL          "x25_incall.d"
#else
#define X25_EASYWAY_PROC    "x25_subnua"
#define X25_INCALL          "x25_incall"
#endif

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static int sock;
static struct      hostent *he;
static struct      servent *se;
static struct      protoent *pe;
static struct      sockaddr_in dest_sin;
static int port, lsn;
static char msg[ENETMSGLEN];
static char buf[BUFSIZ];

char acc_remote[X25_ADLEN], acc_local[X25_ADLEN];
static char remote[X25_ADLEN], local[X25_ADLEN];


/*
 *
 * Procedure: subnua
 *
 * Parameters: cid, user data, facillity of the incoming call
 *
 * Description: manage calls with subnua
 *
 * Return: -1 if errors, 0 if OK
 *
 */

int     subnua(c, udata, facil, r, l, info, subnua)
int c;
struct  x25data *udata, *facil;
char *r, *l;
int info;
char *subnua;
{
    int rc;
    char *finger;
    int xerr;
    char dummy[BUFSIZ];
    char sockstr[MAX_STR], portstr[MAX_STR], lsnstr[MAX_STR],
         localstr[MAX_STR], remotestr[MAX_STR], debugstr[MAX_STR];
    int pid;
    static struct  BEST_ROUTE tr;
    struct  BEST_ROUTE *br;
    struct  GROUP *grp;
    char link[MAX_STR];
    struct  PKT_HDR *ph = (struct PKT_HDR *) (buf);
    struct  GROUP *gr = (struct GROUP *) (buf + sizeof(struct PKT_HDR));

    debug((3,
           "subnua() - Start, c %d, ud.len %d, fac.len %d remote %s local %s sub_nua %s\n",
           c, udata->xd_len, facil->xd_len, r, l, subnua));

    /* Make local copy of r and l pointers cause x25accept modifies */

    strcpy(remote, r);
    strcpy(local, l);

    debug((3, "subnua() - going to route %s\n", subnua));

    if (( grp = sub_rt_find(subnua)) == (struct GROUP *) NULL)
    {
        debug((1,
               "subnua() - sub_rt_find() error, sub_rt_errno %d\n", sub_rt_errno));

        if ( sub_rt_errno == ESUB_RT_UD_NOTFOUND    )
        {
            errlog(X25_LOG, "%s : SUB NUA >%s< NOT IN ROUTING TABLE\n",
                   remote, subnua);
        }
        else
        {
            errlog(INT_LOG, "%s - rt_find() error %d\n", pname, sub_rt_errno);
            errlog(X25_LOG, "%s : SUB NUA >%s< ROUTING ERROR %d\n",
                   remote, subnua, sub_rt_errno);
        }
        return(-1);
    }

    debug((3, "subnua() - sub_rt_find ok : grp_name %s, grp_num %d\n",
           grp->grp_name, grp->grp_num));

    if ( grp->grp_num != -1 )
    {
        errlog(X25_LOG, "%s : SUB NUA >%s< GROUP %s,%d\n",
               remote, subnua, grp->grp_name, grp->grp_num );
    }
    else
    {
        errlog(X25_LOG, "%s : SUB NUA >%s< GROUP %s\n",
               remote, subnua, grp->grp_name );
    }

    /*
     * search now best route to access particular nua type
     */

    if ((br = sm_get_best_route(myhostname, ASY_NUA, grp))
        == (struct BEST_ROUTE *) NULL)
    {
        debug((1, "subnua() - Best Route Not found sm_errno %d\n",
               sm_errno));

        switch (sm_errno)
        {
        case    ESM_NO_SM_HOST:

            errlog(X25_LOG,
                   "%s : SUB NUA >%s< STATUS MANAGER HOST NOT FOUND\n",
                   remote, subnua );

            break;

        case    ESM_NO_SM:

            errlog(X25_LOG,
                   "%s : SUB NUA >%s< UNABLE TO CONTACT STATUS MANAGER\n",
                   remote, subnua );

            break;

        case    ESM_NO_ROUTE:

            if ( grp->grp_num != -1 )
            {
                errlog(X25_LOG,
                       "%s : SUB NUA >%s< GROUP >%s,%d< LINE NOT FREE\n",
                       remote, subnua, grp->grp_name, grp->grp_num );
            }
            else
            {
                errlog(X25_LOG,
                       "%s : SUB NUA >%s< GROUP >%s< NO LINE FREE\n",
                       remote, subnua, grp->grp_name);
            }

            break;

        case    ESM_INVALID_RTTYPE:

            errlog(X25_LOG, "%s : SUB NUA >%s< INVALID ROUTE TYPE\n",
                   remote, subnua );

            break;
        }
        return(-1);
    }

    debug((3, "subnua() - best route host %s, link %s, service %s\n",
           br->hostname, br->link, br->service));

    errlog(X25_LOG, "%s : SUB NUA >%s< TO ASYNC LINE >%s< ON %s\n",
           remote, subnua, br->link, br->hostname);

    /*
     * Put up socket with service returned by sm_get_best_route
     */

    /*
     * Select service and protocol
     */

    if (( se = getservbyname(br->service, PROTOCOL)) == NULL )
    {
        debug((1, "subnua() - unknown service %s for protocol %s\n",
               br->service, PROTOCOL));

        errlog(X25_LOG, "%s : CONF ERROR %s/%s NOT FOUND IN /etc/services\n",
               remote, br->service, PROTOCOL);

        sm_free_route(br->hostname, br->link);
        return(-1);
    }

    dest_sin.sin_port =  se->s_port;

    /*
     * Get Host information
     */

    if (( he = gethostbyname(br->hostname)) == NULL )
    {
        debug((1, "subnua() - gethostbyname failed, %s unknown\n",
               br->hostname ));
        errlog(X25_LOG, "%s : HOST %s UNKNOWN\n", remote, br->hostname);
        sm_free_route(br->hostname, br->link);
        return(-1);
    }

    dest_sin.sin_family = he->h_addrtype;
    memcpy((char *)&dest_sin.sin_addr, he->h_addr, he->h_length);

    /*
     * Get protocol number
     */

    if (( pe = getprotobyname(PROTOCOL)) == NULL )
    {
        debug((1, "subnua() - getprotobyname failed, %s unknown\n", PROTOCOL));
        errlog(X25_LOG, "%s : UNKNOWN PROTOCOL %s\n", remote, PROTOCOL);
        sm_free_route(br->hostname, br->link);
        return(-1);
    }

    /*
     * Create Socket with all information collected
     */

    if (( sock = socket(he->h_addrtype, SOCK_STREAM, pe->p_proto)) < 0 )
    {
        debug((1, "subnua() - socket create failed, errno %d\n", errno));
        errlog(X25_LOG, "%s : UNABLE TO CREATE SOCKET\n", remote);
        errlog(INT_LOG, "%s : UNABLE TO CREATE SOCKET\n", pname);
        sm_free_route(br->hostname, br->link);
        return(-1);
    }

    /*
     * connect to remote
     */

    if ( connect(sock, (char *)&dest_sin, sizeof(dest_sin)) < 0)
    {
        debug((1, "subnua() - connect failed, errno %d\n", errno));
        errlog(X25_LOG, "%s : UNABLE TO CONNECT TO SERVICE %s\n",
               remote, br->service);
        errlog(INT_LOG, "%s : UNABLE TO CONNECT TO %s\n", pname, br->service);
        sm_free_route(br->hostname, br->link);
        return(-1);
    }

    debug((3, "subnua() - Connect OK socket %d\n", sock));
    debug((3, "subnua() - remote %s local %s\n", remote, local));

    if ( x25getconn(c, &port, &lsn) < 0 )
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s - x25getconn error %d, %s\n", remote, xerr, msg);
        errlog(INT_LOG, "%s - x25getconn error %d, %s\n", pname, xerr, msg);
        debug((1, "subnua() - x25getconn error %d %s\n", xerr, msg));
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
    }

    debug((3, "subnua() - remote %s local %s\n", remote, local));

    /*
     * if here then nua type was not x25 so go for normal asy
     * and fork exec x25_incall
     */

    debug((3, "subnua() - remote %s local %s\n", remote, local));

    /* First accept the call	*/

    strcpy(acc_remote, remote);
    strcpy(acc_local, local);

    /*
     * Create PKT_HDR + GROUP to send to asyn_mgr
     */

    ph->pkt_code    = ASY_CALL_REQ;
    ph->pkt_error   = 0;
    ph->flags       = 0;
    ph->pkt_len     = 0;

    /* extract now from link (GPC,3) group and num	*/

    strcpy(link, br->link);

    if (( finger = strchr(link, ',')) == NULL )
    {
        errlog(X25_LOG, "%s : SUB NUA >%s< INVALID GROUP %s\n",
               remote, subnua, link);
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
    }

    *finger = '\0';

    strcpy(gr->grp_name, link);
    gr->grp_num = atoi(finger + 1);

    if ((rc = mos_send(sock, buf, sizeof(struct PKT_HDR) +
                       sizeof(struct GROUP))) < 0)
    {
        errlog(X25_LOG, "%s : UNABLE TO TRANSFER CALL TO ASYNC\n", remote);
        errlog(INT_LOG, "%s : mos_send to asyn_mgr error %d\n",
               pname, errno);
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
    }

    /*
     * Insert here receive section for ASY_CALL_ACCEPT or ASY_CALL_REJECT
     */

    if (( rc = mos_recv(sock, buf, BUFSIZ)) <= 0 )
    {
        errlog(X25_LOG, "%s : ERROR RECEIVING ASYNC CONFIRM\n", remote);
        errlog(INT_LOG, "%s : mos_recv error %d\n", pname, errno);
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
    }

    switch (ph->pkt_code)
    {
    case        ASY_CALL_REJECT:

        debug((3, "subnua() - ASY_CALL_REJECT received\n"));
        memcpy(dummy, buf + sizeof(struct PKT_HDR),
               rc - sizeof(struct PKT_HDR));

        dummy[rc - sizeof(struct PKT_HDR)] = '\0';
        errlog(X25_LOG, "%s : ASYNC ERROR : %s \n", remote, dummy);

        if ( ph->pkt_error == PORT_DISABLED )
        {
            errlog(X25_LOG, "%s : PORT %s DISABLED !\n",
                   remote, br->link);
        }

        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
        break;
    case        ASY_CALL_ACCEPT:

        debug((3, "subnua() - ASY_CALL_ACCEPT received\n"));
        break;

    default:

        debug((1, "subnua() - Unknown pkt_code received %d\n",
               ph->pkt_code));
        errlog(INT_LOG, "%s : mos_recv pkt_code %d not foreseen\n",
               pname, ph->pkt_code);
        errlog(X25_LOG, "%s : ASYNC CONFIRM %d NOT FORESEEN\n",
               remote, ph->pkt_code);
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
        break;
    }

    if ( x25accept(&c, info, NULL, NULL, acc_remote, acc_local, X25NULLFN) < 0)
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s : CALL ACCEPT ERROR %s\n", remote, msg);
        errlog(INT_LOG, "%s - x25accept() error %d, %s\n", pname, xerr, msg);
        debug((1, "subnua() - x25accept() error %d %s\n", xerr, msg));
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
    }

    debug((3, "subnua() - asy connection.x25accept() OK cid %d\n", c));
    debug((3, "subnua() - remote %s local %s\n", remote, local));

    /* fork now the standard X25_INCALL		*/

    pid = fork();
    switch (pid)
    {
    case    -1:

        /*
         * unable to fork more processes
         */

        debug((1, "subnua() - Unable to fork! errno = %d\n", errno));
        errlog(X25_LOG, "%s : UNABLE TO START I/O PROCESS\n", remote);
        errlog(INT_LOG, "%s : Fork Failed, errno %d\n", pname, errno);
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
        break;

    case    0:

        /* exec new process to manage call  */

        sprintf(sockstr, "-s%d", sock);
        sprintf(portstr, "-p%d", port);
        sprintf(lsnstr, "-l%d", lsn);
        sprintf(debugstr, "-d%d", debuglevel);
        sprintf(localstr, "-x%s", local);
        sprintf(remotestr, "-r%s", remote);


        debug((3,
               "subnua(child) - execute %s socket %s port %s lsn %s local %s remote %s debug %s\n",
               X25_INCALL, sockstr, portstr, lsnstr, localstr, remotestr, debugstr));

#ifdef  DEBUG
        execlp(X25_INCALL, X25_INCALL, sockstr, portstr, lsnstr, remotestr, localstr, debugstr, 0);
#else
        execlp(X25_INCALL, X25_INCALL, sockstr, portstr, lsnstr, remotestr, localstr, 0);
#endif

        /* if reached means that execlp failed */

        debug((1, "subnua(child) execlp failed errno %d\n", errno));

        errlog(X25_LOG, "%s : UNABLE TO START I/O PROCESS\n", remote);

        errlog(INT_LOG,
               "%s : UNABLE TO EXECLP %s , ERRNO %d\n", pname, X25_INCALL, errno);
        sleep(3);
        close(sock);
        exit(FAILURE);

        break;

    default:

        if (pid_push(pid, br->link, br->hostname) == -1)
        {
            /* pid table is full */

            errlog(X25_LOG, "%s : INTERNAL ERROR - PID TABLE IS FULL\n",
                   remote);
            errlog(INT_LOG,
                   "%s : PID TABLE IS FULL - PROCESS %d NOT INSERTED\n",
                   pname, pid);

            kill(SIGTERM, pid);
        }

        debug((3, "subnua(father) - closing socket %d\n", sock));

        close(sock);

        debug((3, "subnua(father) - x25delconn(%d)\n", c));

        if ( x25delconn(c) < 0 )
        {
            x25errormsg(msg);
            errlog(INT_LOG, "%s : x25delconn error %d, %s\n",
                   pname, x25error(), msg);
            debug((1, "subnua(father) - x25delconn error %d %s\n",
                   x25error(), msg));
        }

        return(0);
        break;
    }
}
