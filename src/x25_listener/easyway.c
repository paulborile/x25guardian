/*
 * $Id: easyway.c,v 1.1.1.1 1998/11/18 15:03:26 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: easyway.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:26  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.4  1995/09/29  17:09:32  px25
 * Added mos_recv for ASY_CALL_{REJECT,ACCEPT}
 *
 * Revision 1.3  1995/09/18  13:01:48  px25
 * Added new tty Groups management
 * Added new Direct Sync link routing
 *
 * Revision 1.2  1995/07/14  09:15:49  px25
 * Parameters transfer udata and facil.
 *
 * Revision 1.1  1995/07/12  13:48:03  px25
 * Added TTY routing and connect to x25shell process
 * Restyling of errlog stuff
 * Added check oin transfer_udata,facilities before actually
 * tranferring them
 *
 * Revision 1.0  1995/07/07  10:21:16  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: easyway.c,v 1.1.1.1 1998/11/18 15:03:26 paul Exp $";

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
#include        "x25_caller.h"
#include        "rt.h"
#include        "sm.h"
#include        "errlog.h"
#include        "group.h"

/*  Module include files                        */

/*  Extern functions used                       */

extern struct  NUA *rt_find();
extern struct  BEST_ROUTE *sm_get_best_route();

/*  Extern data used                            */

extern int rt_errno;
extern int sm_errno;
extern int errno;
extern int debuglevel;
extern char myhostname[];
extern char pname[];

/*  Local constants                             */

#define X25_ADLEN   (X25_ADDRLEN+X25_ADDREXT+2)
#define  PROTOCOL               "tcp"

#ifdef  DEBUG
#define X25_EASYWAY_PROC    "x25_easyway.d"
#define X25_INCALL          "x25_incall.d"
#else
#define X25_EASYWAY_PROC    "x25_easyway"
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
static struct      GROUP *gr = (struct GROUP *)
                               (buf + sizeof(struct PKT_HDR));
static struct      PKT_HDR *ph         = (struct PKT_HDR *) (buf);
static struct      X25_INFO *x25_to_go  = (struct X25_INFO *)
                                          (buf + sizeof(struct PKT_HDR));

char acc_remote[X25_ADLEN], acc_local[X25_ADLEN];
char remote[X25_ADLEN], local[X25_ADLEN];


/*
 *
 * Procedure: easyway
 *
 * Parameters: cid, user data, facillity of the incoming call
 *
 * Description: manage easyway calls
 *
 * Return: -1 if errors, 0 if OK
 *
 */

int     easyway(c, udata, facil, r, l, info)
int c;
struct  x25data *udata, *facil;
char *r, *l;
int info;
{
    static struct  BEST_ROUTE tr;
    char sockstr[MAX_STR], portstr[MAX_STR], lsnstr[MAX_STR],
         localstr[MAX_STR], remotestr[MAX_STR], debugstr[MAX_STR];
    char dummy[BUFSIZ];
    int rc, xerr;
    int pid;
    struct  NUA *r_nua;
    struct  BEST_ROUTE *br;
    struct  GROUP grp, *group = NULL;
    unsigned char bin_to_route[MAX_USER_DATA_LEN];
    char ea_to_route[3*MAX_USER_DATA_LEN];
    char *finger;
    char sync_link[MAX_STR], sync_host[MAX_STR];
    char link[MAX_STR];
    char pri_nua[NUA_LEN];


    debug((3,
           "easyway() - Start, c %d, ud.len %d, fac.len %d remote %s local %s\n",
           c, udata->xd_len, facil->xd_len, remote, local));

    /* Make local copy of r and l pointers cause x25accept modifies */

    strcpy(remote, r);
    strcpy(local, l);

    /* copy userdata */

    memcpy(bin_to_route, &(udata->xd_data[0]), udata->xd_len);
    bin_to_ea(bin_to_route, ea_to_route, udata->xd_len);

    debug((3, "easyway() - going to route %s\n", ea_to_route));

    if ((r_nua = rt_find(bin_to_route, udata->xd_len)) == (struct NUA *) NULL)
    {
        debug((1, "easyway() - rt_find() error, rt_errno %d\n", rt_errno));

        if ( rt_errno == ERT_UD_NOTFOUND    )
        {
            errlog(X25_LOG, "%s : UD >%s< NOT IN ROUTING TABLE\n",
                   remote, ea_to_route);
        }
        else
        {
            errlog(INT_LOG, "%s - rt_find() error %d\n", pname, rt_errno);
            errlog(X25_LOG, "%s : UD >%s< ROUTING ERROR %d\n",
                   remote, ea_to_route, rt_errno);
        }
        return(-1);
    }

    debug((3,
           "easyway() - rt_find ok: 1nua %s, 2nua %s, type %d ud_len %d ud %s\n",
           r_nua->primary_nua, r_nua->secondary_nua, r_nua->nua_type,
           r_nua->udata_tobe_replaced_len, r_nua->udata_tobe_replaced));

    /*
     * search now best route to access particular nua type
     */

    group = NULL;

    switch (r_nua->nua_type)
    {
    case    SNA_NUA:

        errlog(X25_LOG, "%s : SNA DESTINATION NOT IMPLEMENTED\n", remote);
        return(-1);
        break;

    case    ASY_NUA:

        /*
         * Entries managed
         * Normal entry :
         * .GPC..32                -  GPC   GPC   ASY
         * \01\00\00\00=G=32       -  GPC   GPC   ASY
         *
         * Permit also :
         * .GPC..32                -  GPC,1 GPC,2 ASY
         * \01\00\00\00=G=32       -  GPC,1 GPC,2 ASY
         */

        strcpy(pri_nua, r_nua->primary_nua);
        if (( finger = strchr(pri_nua, ',')) == NULL )
        {
            strcpy(grp.grp_name, r_nua->primary_nua);
            grp.grp_num = -1;
        }
        else
        {
            *finger = '\0';
            strcpy(grp.grp_name, pri_nua);
            grp.grp_num = atoi(finger + 1);
        }

        group = &grp;

        br = sm_get_best_route(myhostname, r_nua->nua_type, group);
        break;

    case    TTY_NUA:

        br = sm_get_best_route(myhostname, r_nua->nua_type, NULL);
        break;

    case    X25_NUA:

        /*
         * Analize pri-nua field returned by routing
         * It could normal : 2394905488[,f] or
         * indicating link : <host>:<link>.2394905488[,f]
         */

        strcpy(pri_nua, r_nua->primary_nua);

        if (( finger = strchr(pri_nua, '.')) != NULL )
        {
            *finger = '\0';

            if (( finger = strchr(pri_nua, ':')) != NULL )
            {
                *finger = '\0';
                strcpy(sync_host, pri_nua);
                strcpy(sync_link, finger + 1);

                br = sm_get_best_route(sync_host, r_nua->nua_type, sync_link);
            }
        }
        else
        {
            br = sm_get_best_route(myhostname, r_nua->nua_type, NULL);
        }
        break;

    default:

        debug((1, "easyway() - Unknown nua type %d\n", r_nua->nua_type));
        errlog(X25_LOG, "%s : ROUTING ERROR - UNKNOWN NUA TYPE %d\n",
               remote, r_nua->nua_type);
        return(-1);
        break;
    }

    if ( br == NULL )
    {
        debug((1, "easyway() - Best Route Not found sm_errno %d\n",
               sm_errno));

        switch (sm_errno)
        {
        case    ESM_NO_SM_HOST:

            errlog(X25_LOG,
                   "%s : UD >%s< STATUS MANAGER HOST NOT FOUND\n",
                   remote, ea_to_route );

            break;

        case    ESM_NO_SM:

            errlog(X25_LOG,
                   "%s : UD >%s< UNABLE TO CONTACT STATUS MANAGER\n",
                   remote, ea_to_route );

            break;

        case    ESM_NO_ROUTE:

            if ( r_nua->nua_type == ASY_NUA )
            {
                errlog(X25_LOG,
                       "%s : UD >%s< GROUP >%s< NO ASYNC LINES FREE\n",
                       remote, ea_to_route, grp.grp_name );
            }
            else if ( r_nua->nua_type == X25_NUA )
            {
                errlog(X25_LOG, "%s : UD >%s< NO X25 VC FREE\n",
                       remote, ea_to_route );
            }
            else if ( r_nua->nua_type == TTY_NUA )
            {
                errlog(X25_LOG, "%s : UD >%s< NO SHELLS FREE\n",
                       remote, ea_to_route );
            }

            break;

        case    ESM_INVALID_RTTYPE:

            errlog(X25_LOG, "%s : UD >%s< INVALID ROUTE TYPE\n",
                   remote, ea_to_route );

            break;
        }
        return(-1);
    }

    debug((3, "easyway() - best route host %s, link %s, service %s\n",
           br->hostname, br->link, br->service));

    if ( r_nua->nua_type == ASY_NUA )
    {
        errlog(X25_LOG, "%s : UD >%s< TO ASYNC LINE >%s< ON %s\n",
               remote, ea_to_route, br->link, br->hostname);
    }
    else if ( r_nua->nua_type == X25_NUA )
    {
        errlog(X25_LOG, "%s : UD >%s< TO X25 ON %s LINK %s\n",
               remote, ea_to_route, br->hostname, br->link);
    }
    else if ( r_nua->nua_type == TTY_NUA )
    {
        errlog(X25_LOG, "%s : UD >%s< TO SHELL ON %s\n",
               remote, ea_to_route, br->hostname);
    }

    /*
     * Put up socket with service returned by sm_get_best_route
     */

    /*
     * Select service and protocol
     */

    if (( se = getservbyname(br->service, PROTOCOL)) == NULL )
    {
        debug((1, "easyway() - unknown service %s for protocol %s\n",
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
        debug((1, "easyway() - gethostbyname failed, %s unknown\n",
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
        debug((1, "easyway() - getprotobyname failed, %s unknown\n", PROTOCOL));
        errlog(X25_LOG, "%s : UNKNOWN PROTOCOL %s\n", remote, PROTOCOL);
        sm_free_route(br->hostname, br->link);
        return(-1);
    }

    /*
     * Create Socket with all information collected
     */

    if (( sock = socket(he->h_addrtype, SOCK_STREAM, pe->p_proto)) < 0 )
    {
        debug((1, "easyway() - socket create failed, errno %d\n", errno));
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
        debug((1, "easyway() - connect failed, errno %d\n", errno));
        errlog(X25_LOG, "%s : UNABLE TO CONNECT TO SERVICE %s\n",
               remote, br->service);
        errlog(INT_LOG, "%s : UNABLE TO CONNECT TO %s\n", pname, br->service);
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
    }

    debug((3, "easyway() - Connect OK socket %d\n", sock));
    debug((3, "easyway() - remote %s local %s\n", remote, local));

    if ( x25getconn(c, &port, &lsn) < 0 )
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s - x25getconn error %d, %s\n", remote, xerr, msg);
        errlog(INT_LOG, "%s - x25getconn error %d, %s\n", pname, xerr, msg);
        debug((1, "easyway() - x25getconn error %d %s\n", xerr, msg));
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
    }

    debug((3, "easyway() - remote %s local %s\n", remote, local));

    if ( r_nua->nua_type == X25_NUA )
    {
        /*
         * X25 to X25 routing :
         * transfer the call to x25_caller, fork a new process
         * which will wait for accept or reject packets
         * and then x25accept or x25hangup the call
         */

        memset(buf, '\0', BUFSIZ);

        ph->pkt_code    = X25_CALL_REQ;

        x25_to_go->port =  atoi(br->link);
        x25_to_go->NUA_X25.nua_type = r_nua->nua_type;

        /*
         * Copy userdata to remote x25_callmgr
         */

        if (( r_nua->udata_tobe_replaced[0] == '=' ) &&
            ( r_nua->udata_tobe_replaced_len == 1 ))
        {
            /* Routing table has a `=` in ud_tobe_replaced	*/
            /* so copy orig ud                                      */

            x25_to_go->userdata = *udata;
        }
        else if (( r_nua->udata_tobe_replaced[0] == '-' ) &&
                 ( r_nua->udata_tobe_replaced_len == 1 ))
        {
            /* Routing table contains - so clear output user_data	*/

            x25_to_go->userdata.xd_len = 0;
        }
        else
        {
            /* routing table contains a valid user data to be replaced */

            x25_to_go->userdata.xd_len = r_nua->udata_tobe_replaced_len;
            memcpy(x25_to_go->userdata.xd_data, r_nua->udata_tobe_replaced,
                   r_nua->udata_tobe_replaced_len);
        }

        /*
         * copy remote nua (the caller) into source_nua for outgoing call
         */

        strcpy(x25_to_go->source_nua, remote);

        /*
         * Transfer facilities based on prim.nua end flag
         * Ex:  22600234,f means call nua 22600234 using input facilities
         */

        if (( finger = strrchr(r_nua->primary_nua, ',')) == NULL )
        {
            x25_to_go->facility.xd_len = 0;
        }
        else
        {
            switch (*(finger+1))
            {
            case    'f':

                /* transfer facilities	*/

                x25_to_go->facility = *facil;
                *finger = '\0';
                break;
            default:
                /* this is wrong format	*/
                break;
            }
        }

        if (( finger = strchr(r_nua->primary_nua, '.')) != NULL )
        {
            strcpy(x25_to_go->NUA_X25.primary_nua, finger + 1 );
        }
        else
        {
            strcpy(x25_to_go->NUA_X25.primary_nua, r_nua->primary_nua);
        }

        strcpy(x25_to_go->NUA_X25.secondary_nua, r_nua->secondary_nua);

        debug((3, "easyway() - X25_CALL_REQ pri %s, sec %s, port %d\n",
               x25_to_go->NUA_X25.primary_nua, x25_to_go->NUA_X25.secondary_nua,
               x25_to_go->port ));

        debug((3, "easyway() - Sending %d bytes\n", sizeof(struct PKT_HDR) +
               sizeof(struct X25_INFO) ));

        if ((rc = mos_send(sock, buf, sizeof(struct PKT_HDR) +
                           sizeof(struct X25_INFO))) < 0)
        {
            errlog(X25_LOG, "%s : UNABLE TO TRANSFER X25 OUTPUT CALL\n", remote);
            errlog(INT_LOG, "%s : mos_send to x25_caller error %d\n",
                   pname, errno);
            sm_free_route(br->hostname, br->link);
            close(sock);
            return(-1);
        }

        /* Call sent to x25_callmgr	*/

        /*
         * Now fork and exec
         * "x25_easyway -s<socket> -p<port> -l<lsn>
         * -x<local_x25_addr> -r<remote_x25_addr>
         */

        pid = fork();
        switch (pid)
        {
        case    -1:

            /*
             * unable to fork more processes
             */

            debug((1, "easyway() - Unable to fork! errno = %d\n", errno));
            errlog(X25_LOG, "%s : UNABLE TO START EASYWAY PROCESS\n", remote);
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
            sprintf(localstr, "-x%s", local);
            sprintf(remotestr, "-r%s", remote);
            sprintf(debugstr, "-d%d", debuglevel);


            debug((3,
                   "easyway(child) - execute %s socket %s port %s lsn %s loc %s rem %s debug %s\n",
                   X25_EASYWAY_PROC, sockstr, portstr, lsnstr,
                   localstr, remotestr, debugstr));

#ifdef  DEBUG
            execlp(X25_EASYWAY_PROC, X25_EASYWAY_PROC,
                   sockstr, portstr, lsnstr, localstr, remotestr, debugstr, 0);
#else
            execlp(X25_EASYWAY_PROC, X25_EASYWAY_PROC,
                   sockstr, portstr, lsnstr, localstr, remotestr, 0);
#endif

            /* if reached means that execlp failed */

            debug((1, "easyway(child) execlp failed errno %d\n", errno));

            errlog(X25_LOG, "%s : UNABLE TO START EASYWAY PROCESS\n", remote);
            errlog(INT_LOG, "%s : UNABLE TO EXECLP %s , ERRNO %d\n",
                   pname, X25_EASYWAY_PROC, errno);
            sleep(3);
            close(sock);
            exit(FAILURE);

            break;

        default:

            if (pid_push(pid, br->link, br->hostname) == -1)
            {
                /* pid table is full */

                errlog(X25_LOG, "%s : INTERNAL ERROR - PID TABLE IS FULL",
                       remote);

                errlog(INT_LOG,
                       "%s : PID TABLE IS FULL - PROCESS %d NOT INSERTED\n",
                       pname, pid);

            }

            debug((3, "easyway(father) - closing socket %d\n", sock));

            close(sock);

            debug((3, "easyway(father) - x25delconn(%d)\n", c));

            if ( x25delconn(c) < 0 )
            {
                x25errormsg(msg);
                errlog(INT_LOG, "%s : x25delconn error %d, %s\n",
                       pname, x25error(), msg);
                debug((1, "easyway(father) - x25delconn error %d %s\n",
                       x25error(), msg));
            }

            return(0);
            break;
        }
    }

    /*
     * if here then nua type was not x25 so go for normal asy
     * and fork exec x25_incall
     */

    debug((3, "easyway() - remote %s local %s\n", remote, local));

    /* First accept the call	*/

    strcpy(acc_remote, remote);
    strcpy(acc_local, local);

    /*
     * Create PKT_HDR + GROUP to send to asyn_mgr
     */

    if ( r_nua->nua_type == ASY_NUA )
    {
        ph->pkt_code    = ASY_CALL_REQ;
        ph->pkt_error   = 0;
        ph->flags       = 0;
        ph->pkt_len     = 0;

        /* extract now from link (GPC,3) group and num	*/

        strcpy(link, br->link);

        if (( finger = strchr(link, ',')) == NULL )
        {
            errlog(X25_LOG, "%s : UD >%s< INVALID GROUP %s\n",
                   remote, ea_to_route, link);
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

            debug((3, "easyway() - ASY_CALL_REJECT received\n"));
            memcpy(dummy, buf + sizeof(struct PKT_HDR),
                   rc - sizeof(struct PKT_HDR));

            dummy[rc - sizeof(struct PKT_HDR)] = '\0';
            errlog(X25_LOG, "%s : ASYNC ERROR : %s\n", remote, dummy);

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

            debug((3, "easyway() - ASY_CALL_ACCEPT received\n"));
            break;

        default:

            debug((1, "easyway() - Unknown pkt_code received %d\n",
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
    }

    if ( x25accept(&c, info, NULL, NULL, acc_remote, acc_local, X25NULLFN) < 0)
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s : CALL ACCEPT ERROR %s\n", remote, msg);
        errlog(INT_LOG, "%s - x25accept() error %d, %s\n", pname, xerr, msg);
        debug((1, "easyway() - x25accept() error %d %s\n", xerr, msg));
        sm_free_route(br->hostname, br->link);
        close(sock);
        return(-1);
    }

    debug((3, "easyway() - asy connection.x25accept() OK cid %d\n", c));
    debug((3, "easyway() - remote %s local %s\n", remote, local));

    /* fork now the standard X25_INCALL		*/

    pid = fork();
    switch (pid)
    {
    case    -1:

        /*
         * unable to fork more processes
         */

        debug((1, "easyway() - Unable to fork! errno = %d\n", errno));
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
               "easyway(child) - execute %s socket %s port %s lsn %s local %s remote %s debug %s\n",
               X25_INCALL, sockstr, portstr, lsnstr, localstr, remotestr, debugstr));

#ifdef  DEBUG
        execlp(X25_INCALL, X25_INCALL, sockstr, portstr, lsnstr, remotestr, localstr, debugstr, 0);
#else
        execlp(X25_INCALL, X25_INCALL, sockstr, portstr, lsnstr, remotestr, localstr, 0);
#endif

        /* if reached means that execlp failed */

        debug((1, "easyway(child) execlp failed errno %d\n", errno));

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

        debug((3, "easyway(father) - closing socket %d\n", sock));

        close(sock);

        debug((3, "easyway(father) - x25delconn(%d)\n", c));

        if ( x25delconn(c) < 0 )
        {
            x25errormsg(msg);
            errlog(INT_LOG, "%s : x25delconn error %d, %s\n",
                   pname, x25error(), msg);
            debug((1, "easyway(father) - x25delconn error %d %s\n",
                   x25error(), msg));
        }

        return(0);
        break;
    }
}
