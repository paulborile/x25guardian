/*
 * $Id: x25_argotel.c,v 1.1.1.1 1998/11/18 15:03:12 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: x25_argotel
 *
 * Contents: manage argotel calls to x25 network
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_argotel.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:12  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.3  1995/09/29  17:10:09  px25
 * Added mos_recv for ASY_CALL_{REJECT,ACCEPT}
 *
 * Revision 1.2  1995/09/18  13:09:26  px25
 * Added tty Group routing
 * Added "COM" answerback when calls go to asy.
 *
 * Revision 1.1  1995/07/14  09:10:45  px25
 * Better errlog display.
 *
 * Revision 1.0  1995/07/07  10:15:06  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_argotel.c,v 1.1.1.1 1998/11/18 15:03:12 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include        <sys/socket.h>
#include        <sys/signal.h>
#include        <netinet/in.h>
#include        <netdb.h>
#include        <string.h>
#include        <x25.h>
#include        <neterr.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "x25_caller.h"
#include        "rt.h"
#include        "group.h"
#include        "sm.h"
#include        "debug.h"
#include        "errlog.h"

/*  Module include files                        */

/*  Extern functions used                       */

void    get_command_line();
void    x25_argotel_usage();
void    hangup();
extern struct  NUA *rt_find();

/*  Extern data used                            */

extern int errno;
extern int rt_errno;

/*  Local constants                             */

#define OPTS                "d:p:l:x:r:u:f:"
#define  HOSTNAME_LEN   32

#ifdef  DEBUG
#define X25_INCALL          "x25_incall.d"
#else
#define X25_INCALL          "x25_incall"
#endif

#define  PROTOCOL               "tcp"

#define ARGOTEL_1ST_PKT_TIMEOUT     60
#define EXIT_TIMEOUT                    3

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static struct      hostent *he;
static struct      servent *se;
static struct      protoent *pe;
static struct      sockaddr_in dest_sin;
int argotel_1st_pkt_timeout = ARGOTEL_1ST_PKT_TIMEOUT;
char *recvbuf;

static char udata_str[BUFSIZ];
static char facil_str[BUFSIZ];

char myhostname[HOSTNAME_LEN];
static char buf[BUFSIZ];
static struct  GROUP *gr         = (struct GROUP *)
                                   (buf + sizeof(struct PKT_HDR));
static struct  PKT_HDR *ph         = (struct PKT_HDR *) (buf);
static struct  X25_INFO *x25_to_go  = (struct X25_INFO *)
                                      (buf + sizeof(struct PKT_HDR));
char pname[32];
char pid_str[32];
char debug_file[BUFSIZ];
int debuglevel = -1;

int cid;
int sock = -1;
int lsn = -1;
int port = -1;
static char *argotel_com_buf;
char local[X25_ADDRLEN+X25_ADDREXT+2];
char remote[X25_ADDRLEN+X25_ADDREXT+2];
static mem_alloc = 0;

/*
 *
 * Procedure: main
 *
 * Parameters: argc argv
 *
 * Description: receive call accept or reject and then x25accept call or not
 *
 * Return: none
 *
 */


main(argc, argv)
int argc;
char **argv;
{
    int info = 0;
    int rc;
    int xerr;
    pid_t pid;
    pid_t dead;
    long status;
    char dummy[BUFSIZ];
    char msg[ENETMSGLEN];
    char link[MAX_STR];
    char sockstr[MAX_STR], portstr[MAX_STR], lsnstr[MAX_STR],
         localstr[MAX_STR], remotestr[MAX_STR], debugstr[MAX_STR];
    struct  x25doneinfo xdi;
    char to_route[MAX_USER_DATA_LEN];
    struct  NUA *r_nua;
    struct  BEST_ROUTE *br;
    struct  GROUP grp, *group;
    char *finger;
    struct  x25data udata, facil;
    char sync_link[MAX_STR], sync_host[MAX_STR];
    char pri_nua[NUA_LEN];

    signal(SIGPIPE, SIG_IGN);

    /*
     * Globalize name of the program
     */

    strcpy(pname, argv[0]);

    get_command_line(argc, argv);

#ifdef DEBUG
    sprintf(pid_str, "%d", getpid());
    sprintf(debug_file, "/tmp/x25_argotel%05d.debug", getpid());
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    debug((3, "main() - remote %s local %s\n", remote, local));

    if (( finger = getenv(PX25_GEN_TABLE)) != NULL )
    {
        if (( argotel_1st_pkt_timeout =
                  get_param(finger, "argotel_call_timeout")) == -1 )
        {
            errlog(INT_LOG,
                   "%s : Parameter \"argotel_call_timeout\" not found\n", pname);

            argotel_1st_pkt_timeout = ARGOTEL_1ST_PKT_TIMEOUT;
        }
    }
    else
    {
        errlog(INT_LOG, "%s: Environment var PX25_GEN_TABLE not set\n");
    }

    debug((3, "main() - argotel_1st_pkt_timeout = %d\n",
           argotel_1st_pkt_timeout));

    if ( gethostname(myhostname, HOSTNAME_LEN) < 0 )
    {
        debug((1, "main() - unable to gethostname() errno %d\n", errno));
        errlog(INT_LOG,
               "%s : unable to gethostname - errno %d\n", pname, errno);
        exit(FAILURE);
    }

    /*
     * First initialize x25 toolkit and the mkconn
     */

    if ( x25init(0) < 0 )
    {
        x25errormsg(msg);
        errlog(X25_LOG, "%s : INITIALIZATION ERROR %s\n", remote, msg);
        errlog(INT_LOG, "%s : x25init error %d, %s\n", pname, x25error(), msg);
#ifdef  DEBUG
        enddebug();
#endif
        exit(1);
    }

    x25version(msg);                             /* Get the toolkit version. */
    debug((1, "main() - %s", msg));

    debug((3, "main() - mkconn NOWAIT port %d lsn %d\n", port, lsn));

    if ( x25mkconn(&cid, port, lsn, X25NOWAIT) < 0 )
    {
        xerr = x25error();
        x25errormsg(msg);
        errlog(X25_LOG, "%s : INITIALIZATION ERROR %s\n", remote, msg);
        errlog(INT_LOG, "%s: x25mkconn error %d, %s\n", pname, xerr, msg);
        debug((1, "main() - x25mkconn error %d %s\n", xerr, msg));

#ifdef  DEBUG
        enddebug();
#endif
        exit(1);
    }

    debug((3, "main() - mkconn ok cid %d\n", cid));

    /*
     * Allocate buffer for receive
     */

    if (( recvbuf = x25alloc(BUFSIZ)) == NULL)
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s : UNABLE TO ALLOCATE RECEIVE BUFFER\n", remote);
        errlog(INT_LOG, "%s - x25alloc() inbuf error %d, %s\n", pname, xerr, msg);
        debug((1, "main() - x25alloc inbuf error %d %s\n", xerr, msg));
        hangup();
    }

    /*
     * Allocate buffer for argotel send of COM string
     */

    if (( argotel_com_buf = x25alloc(X25_DATA_PACKET_SIZE)) == NULL)
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s : UNABLE TO ALLOCATE ARGOTEL COM BUFFER\n", remote);
        errlog(INT_LOG, "%s - x25alloc() error %d, %s\n", pname, xerr, msg);
        debug((1, "main() - x25alloc error %d %s\n", xerr, msg));
        hangup();
    }

    mem_alloc = 1;

    /*
     * we want to receive data
     */

    debug((3, "main() - issuing x25recv in No wait mode\n"));

    if (( rc = x25recv(cid, recvbuf, X25_DATA_PACKET_SIZE,
                       NULL, X25NULLFN)) < 0)
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s : X25RECV ERROR, %s\n", remote, msg);
        errlog(INT_LOG, "%s - x25recv() error %d, %s\n", pname, xerr, msg);
        debug((1, "main() - x25recv error %d %s\n", xerr, msg));
        hangup();
    }

    /*
     * wait for completion of x25recv
     */

    debug((3, "main() - x25done with %d timeout\n", argotel_1st_pkt_timeout));

    if ( x25done(cid, argotel_1st_pkt_timeout, &xdi) < 0 )
    {
        xerr = x25error(); x25errormsg(msg);
        debug((1, "main() - x25done error %d %s\n", xerr, msg));

        if ( xerr == EX25DONETO )
        {
            /* receive timed out	, cancel it and hangup */

            errlog(X25_LOG,
                   "%s : TIMED OUT WAITING 1ST ARGOTEL PACKET\n", remote);

            if (( rc = x25cancel(cid, recvbuf)) == -1 )
            {
                debug((1, "main() - x25cancel error %d %s\n", xerr, msg));
            }

        }
        hangup();
    }

    if ( xdi.xi_retcode != 0 )
    {
        switch ( xdi.xi_retcode )
        {
        case    ENETCALLCLR:

            errlog(X25_LOG, "%s : CALL CLEARED WHILE RECEIVING 1ST PKT\n",
                   remote);
            break;
        }

        hangup();
    }

    recvbuf[xdi.xi_len] = '\0'; /* null terminate packet	*/

    debug((3, "main() - recv %d bytes User Data  %s\n", xdi.xi_len, recvbuf));

    /* copy userdata and null terminate it		*/

    strcpy(to_route, recvbuf);

    if (( finger = strchr(to_route, '\r')) != NULL )
    {
        *finger = '\0';
    }

    debug((3, "main() - going to route %s\n", to_route));

    if ((r_nua = rt_find(to_route, strlen(to_route))) == (struct NUA *) NULL)
    {
        debug((1, "main() - rt_find() error, rt_errno %d\n", rt_errno));

        if ( rt_errno == ERT_UD_NOTFOUND    )
        {
            errlog(X25_LOG, "%s : UD >%s< NOT IN ROUTING TABLE\n",
                   remote, to_route);
        }
        else
        {
            errlog(INT_LOG, "%s - rt_find() error %d\n", pname, rt_errno);
            errlog(X25_LOG, "%s : UD >%s< ROUTING ERROR %d\n",
                   remote, to_route, rt_errno);
        }
        hangup();
    }

    debug((3, "main() - rt_find ok: 1nua %s, 2nua %s, type %d\n",
           r_nua->primary_nua, r_nua->secondary_nua, r_nua->nua_type));

    /*
     * search now best route to access particular nua type
     */

    group = NULL;


    switch (r_nua->nua_type)
    {
    case    SNA_NUA:
    case    TTY_NUA:

        errlog(X25_LOG, "%s : DESTINATION NOT IMPLEMENTED\n", remote);
        hangup();
        break;

    case    ASY_NUA:
        /*
         * Take out group info from nua struct
         * Permit also :
         * .GPC..32                -  GPC,1 GPC ASY
         * \01\00\00\00=G=32       -  GPC,1 GPC ASY
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

        debug((1, "main() - Unknown nua type %d\n", r_nua->nua_type));
        errlog(X25_LOG, "%s : ROUTING ERROR - UNKNOWN NUA TYPE %d\n",
               remote, r_nua->nua_type);
        hangup();
        break;
    }

    if ( br == NULL )
    {
        debug((1, "main() - Best Route Not found sm_errno %d\n",
               sm_errno));

        switch (sm_errno)
        {
        case  ESM_NO_SM_HOST:

            errlog(X25_LOG,
                   "%s : UD >%s< STATUS MANAGER HOST NOT FOUND\n",
                   remote, to_route );

            break;

        case  ESM_NO_SM:

            errlog(X25_LOG,
                   "%s : UD >%s< UNABLE TO CONTACT STATUS MANAGER\n",
                   remote, to_route );

            break;

        case  ESM_NO_ROUTE:

            if ( r_nua->nua_type == ASY_NUA )
            {
                errlog(X25_LOG,
                       "%s : UD >%s< GROUP >%s< NO ASYNC LINES FREE\n",
                       remote, to_route, grp.grp_name);
            }
            else
            {
                errlog(X25_LOG, "%s : UD >%s< NO X25 VC FREE\n",
                       remote, to_route );
            }
            break;

        case  ESM_INVALID_RTTYPE:

            errlog(X25_LOG, "%s : UD >%s< INVALID ROUTE TYPE\n",
                   remote, to_route );
            break;
        }
        hangup();
    }

    debug((3, "main() - best route host %s, link %s, service %s\n",
           br->hostname, br->link, br->service));

    if ( r_nua->nua_type == ASY_NUA )
    {
        errlog(X25_LOG, "%s : UD >%s< TO ASYNC LINE >%s< ON HOST %s\n",
               remote, to_route, br->link, br->hostname);
    }
    else if ( r_nua->nua_type == X25_NUA )
    {
        errlog(X25_LOG, "%s : UD >%s< ROUTED TO X25 ON HOST %s LINK %s\n",
               remote, to_route, br->hostname, br->link);
    }

    /*
     * Put up socket with service returned by sm_get_best_route
     */

    /*
     * Select service and protocol
     */

    if (( se = getservbyname(br->service, PROTOCOL)) == NULL )
    {
        debug((1, "main() - unknown service %S for protocol %s\n",
               br->service, PROTOCOL));
        errlog(X25_LOG, "%s : CONF ERROR %s/%s NOT FOUND IN /etc/services\n",
               remote, br->service, PROTOCOL);

        sm_free_route(br->hostname, br->link);
        hangup();
    }

    dest_sin.sin_port =  se->s_port;

    /*
     * Get Host information
     */

    if (( he = gethostbyname(br->hostname)) == NULL )
    {
        debug((1, "main() - gethostbyname failed, %s unknown\n",
               br->hostname ));
        errlog(X25_LOG, "%s : HOST %s UNKNOWN\n", remote, br->hostname);
        sm_free_route(br->hostname, br->link);
        hangup();
    }

    dest_sin.sin_family = he->h_addrtype;
    memcpy((char *)&dest_sin.sin_addr, he->h_addr, he->h_length);

    /*
     * Get protocol number
     */

    if (( pe = getprotobyname(PROTOCOL)) == NULL )
    {
        debug((1, "main() - getprotobyname failed, %s unknown\n", PROTOCOL));
        errlog(X25_LOG, "%s : UNKNOWN PROTOCOL %s\n", remote, PROTOCOL);
        sm_free_route(br->hostname, br->link);
        hangup();
    }

    /*
     * Create Socket with all information collected
     */

    if (( sock = socket(he->h_addrtype, SOCK_STREAM, pe->p_proto)) < 0 )
    {
        debug((1, "main() - socket create failed, errno %d\n", errno));
        errlog(X25_LOG, "%s : UNABLE TO CREATE SOCKET\n", remote);
        errlog(INT_LOG, "%s : UNABLE TO CREATE SOCKET\n", pname);
        sm_free_route(br->hostname, br->link);
        hangup();
    }

    /*
     * connect to remote
     */

    if ( connect(sock, (char *)&dest_sin, sizeof(dest_sin)) < 0)
    {
        debug((1, "main() - connect failed, errno %d\n", errno));
        errlog(X25_LOG, "%s : UNABLE TO CONNECT TO SERVICE %s\n",
               remote, br->service);
        errlog(INT_LOG, "%s : UNABLE TO CONNECT TO %s\n", pname, br->service);
        sm_free_route(br->hostname, br->link);
        hangup();
    }

    debug((3, "main() - Connect OK socket %d\n", sock));

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

        debug((3, "main() - converting %s %s to bin\n", udata_str, facil_str));

        if (( r_nua->udata_tobe_replaced[0] == '=' ) &&
            ( r_nua->udata_tobe_replaced_len == 1 ))
        {
            /* Routing table has a `=` in ud_tobe_replaced	*/
            /* so copy orig ud                                      */

            if ( udata_str[0] != '\0' )
            {
                udata.xd_len = hex2bin(udata_str, udata.xd_data);
                x25_to_go->userdata = udata;
            }
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
         * TRansfer facilities based on prim.nua end flag
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

                if ( facil_str[0] != '\0' )
                {
                    facil.xd_len = hex2bin(facil_str, facil.xd_data);
                    x25_to_go->facility = facil;
                }
                *finger = '\0';
                break;

            default:
                /* wrong format	*/
                break;
            }
        }

        /*
         * copy remote nua (the caller) into source_nua for outgoing call
         */

        strcpy(x25_to_go->source_nua, remote);

        if (( finger = strchr(r_nua->primary_nua, '.')) != NULL )
        {
            strcpy(x25_to_go->NUA_X25.primary_nua, finger + 1 );
        }
        else
        {
            strcpy(x25_to_go->NUA_X25.primary_nua, r_nua->primary_nua);
        }

        strcpy(x25_to_go->NUA_X25.secondary_nua, r_nua->secondary_nua);

        if ((rc = mos_send(sock, buf, sizeof(struct PKT_HDR) +
                           sizeof(struct X25_INFO))) < 0)
        {
            errlog(X25_LOG, "%s : UNABLE TO GENERATE X25 OUTPUT CALL\n", pname);
            errlog(INT_LOG, "%s : mos_send error %d\n", pname, errno);
            sm_free_route(br->hostname, br->link);
            close(sock);
            hangup();
        }

        /* Call sent to x25_callmgr	*/

        /*
         * now receive from the socket
         * and wait for a X25_CALL_ACCEPT or REJECT
         */

        if (( rc = mos_recv(sock, buf, BUFSIZ)) <= 0 )
        {
            /*
             * socket was closed already so hangup the connection
             */

            debug((1, "main() - mos_recv ret %d, errno %d\n", rc, errno));
            errlog(X25_LOG, "%s : ERROR RECEIVING X25 CONFIRM\n", remote);
            errlog(INT_LOG, "%s : mos_recv error %d\n", pname, errno);
            sm_free_route(br->hostname, br->link);
            close(sock);
            hangup();
        }

        /* Receive ok	*/

        debug((3, "main() - mos_received %d bytes\n", rc));

        switch (ph->pkt_code)
        {
        case    X25_CALL_REJECT:

            /* call not possible on x25_caller side	*/

            debug((3, "main() - X25_CALL_REJECT received\n"));
            sm_free_route(br->hostname, br->link);
            close(sock);
            hangup();
            break;

        case    X25_CALL_ACCEPT:

            debug((3, "main() - X25_CALL_ACCEPT received\n"));
            break;

        default:

            debug((1, "main() - packet_code %x not foreseen\n", ph->pkt_code));
            sm_free_route(br->hostname, br->link);
            close(sock);
            hangup();
            break;
        }
    }
    else if ( r_nua->nua_type == ASY_NUA )
    {
        /*
         * Prepare ASY_CALL_REQ + GROUP infomation from br->link
         */

        ph->pkt_code    = ASY_CALL_REQ;
        ph->pkt_error   = 0;
        ph->flags       = 0;
        ph->pkt_len     = 0;

        /* extract now from link (GPC,3) group and num	*/

        strcpy(link, br->link);

        if (( finger = strchr(link, ',')) == NULL )
        {
            errlog(X25_LOG, "%s : UD >%s< INVALID GROUP %s\n",
                   remote, to_route, link);
            sm_free_route(br->hostname, br->link);
            close(sock);
            hangup();
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
            hangup();
        }

        /*
         * mos_recv ASY_CALL_ACCEPT or ASY_CALL_REJECT packet
         */

        if (( rc = mos_recv(sock, buf, BUFSIZ)) <= 0 )
        {
            errlog(X25_LOG, "%s : ERROR RECEIVING ASYNC CONFIRM\n", remote);
            errlog(INT_LOG, "%s : mos_recv error %d\n", pname, errno);
            sm_free_route(br->hostname, br->link);
            close(sock);
            hangup();
        }

        switch (ph->pkt_code)
        {
        case        ASY_CALL_REJECT:

            debug((3, "main() - ASY_CALL_REJECT received\n"));
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
            hangup();
            break;
        case        ASY_CALL_ACCEPT:

            debug((3, "main() - ASY_CALL_ACCEPT received\n"));
            break;

        default:

            debug((1, "main() - Unknown pkt_code received %d\n",
                   ph->pkt_code));
            errlog(INT_LOG, "%s : mos_recv pkt_code %d not foreseen\n",
                   pname, ph->pkt_code);
            break;
        }
    }

    /*
     * now send back "COM\r" string
     */

    sprintf(argotel_com_buf, "COM\r");
    if (( rc = x25send(cid, argotel_com_buf, strlen(argotel_com_buf),
                       0, X25NULLFN)) < 0)
    {
        xerr = x25error();
        x25errormsg(msg);
        errlog(X25_LOG, "%s : X25SEND \"COM\\r\" FAILED %d, %s\n",
               remote, xerr, msg);
    }

    /*
     * Wait for send termination now
     */

    if ( x25done(cid, argotel_1st_pkt_timeout/2, &xdi) < 0 )
    {
        xerr = x25error(); x25errormsg(msg);
        debug((1, "main() - x25done error %d %s\n", xerr, msg));

        if ( xerr == EX25DONETO )
        {
            /* receive timed out	, cancel it and hangup */

            errlog(X25_LOG,
                   "%s : TIMED OUT SENDING ARGOTEL COM\n", remote);

            if (( rc = x25cancel(cid, recvbuf)) == -1 )
            {
                debug((1, "main() - x25cancel error %d %s\n", xerr, msg));
            }
        }
    }

    debug((1, "main() - x25done() on COM x25send returned %d\n",
           xdi.xi_retcode));

    if ( xdi.xi_retcode != 0 )
    {
        switch ( xdi.xi_retcode )
        {
        case    ENETCALLCLR:

            errlog(X25_LOG, "%s : CALL CLEARED WHILE SENDING COM\n",
                   remote);
            break;
        }
        hangup();
    }

    /*
     * call was accepted so now getconn and fork real incall
     */

    if ( x25getconn(cid, &port, &lsn) < 0 )
    {
        xerr = x25error(); x25errormsg(msg);
        errlog(X25_LOG, "%s - x25getconn error %d, %s\n", pname, xerr, msg);
        debug((1, "main() - x25getconn error %d %s\n", xerr, msg));
        sm_free_route(br->hostname, br->link);
        close(sock);
        hangup();
    }

    /* fork exec now the standard X25_INCALL		*/

    pid = fork();
    switch (pid)
    {
    case    -1:

        /*
         * unable to fork more processes
         */

        debug((1, "main() - Unable to fork! errno = %d\n", errno));
        errlog(INT_LOG, "%s : Fork Failed, errno %d\n", pname, errno);
        sm_free_route(br->hostname, br->link);
        close(sock);
        hangup();
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
               "main(child) - execute %s socket %s port %s lsn %s  rem %s loc %s debug %s\n",
               X25_INCALL, sockstr, portstr, lsnstr, remotestr, localstr, debugstr));

#ifdef  DEBUG
        execlp(X25_INCALL, X25_INCALL, sockstr, portstr, lsnstr, remotestr, localstr, debugstr, 0);
#else
        execlp(X25_INCALL, X25_INCALL, sockstr, portstr, lsnstr, remotestr, localstr, 0);
#endif

        /* if reached means that execlp failed */

        debug((1, "main(child) execlp failed errno %d\n", errno));

        errlog(INT_LOG,
               "%s : UNABLE TO EXECLP %s , ERRNO %d\n", pname, X25_INCALL, errno);
        sleep(3);
        exit(FAILURE);

        break;

    default:

        debug((3, "main(father) - closing socket %d\n", sock));

        close(sock);

        debug((3, "main(father) - x25delconn(%d)\n", cid));

        if ( x25delconn(cid) < 0 )
        {
            x25errormsg(msg);
            errlog(X25_LOG, "%s : x25delconn error %d, %s\n",
                   pname, x25error(), msg);
            debug((1, "main(father) - x25delconn error %d %s\n",
                   x25error(), msg));
        }

        /* Has to wait till termination of incall to do sm_free_route */

        dead = wait(&status);

        debug((3, "main(father) - pid %d terminated\n", dead));

        if (( rc = sm_free_route(br->hostname, br->link)) < 0 )
        {
            errlog(INT_LOG, "%s : sm_free_route() rc %d  sm_errno %d\n",
                   pname, rc, sm_errno);
            debug((1, "main(father) - sm_free_route() rc %d sm_errno %d\n",
                   rc, sm_errno));
        }

        debug((3, "main(father) - sm_free_route(%s, %s) ok\n",
               br->hostname, br->link ));

        rc = x25free(recvbuf);
        debug((3, "main() - x25free rc %d\n", rc));

        rc = x25free(argotel_com_buf);
        debug((3, "main() - x25free rc %d\n", rc));

        x25exit();
        exit(0);
        break;
    }
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

    local[0] = remote[0] = '\0';
    udata_str[0] = facil_str[0] = '\0';

    while ((c = getopt(argc, argv, OPTS)) !=EOF)
    {
        switch (c)
        {
        case    'u':

            strcpy(udata_str, optarg);
            break;

        case    'f':

            strcpy(facil_str, optarg);
            break;

        case    'd':

            debuglevel  =   atoi(optarg);
            break;

        case    'p':

            port = atoi(optarg);
            break;

        case    'l':

            lsn =   atoi(optarg);
            break;

        case    'x':

            strcpy(local, optarg);
            break;

        case    'r':

            strcpy(remote, optarg);
            break;

        default:
            printf("%s : invalid flag.\n", argv[0]);
            x25_argotel_usage(argc, argv);
            exit(FAILURE);
        }
    }

#ifdef DEBUG
    if ( debuglevel == -1 )
    {
        fatal++;
    }
#endif

    if ( port == -1 )
    {
        fatal++;
    }
    if ( lsn == -1 )
    {
        fatal++;
    }
    if ( local[0] == '\0' )
    {
        fatal++;
    }
    if ( remote[0] == '\0' )
    {
        fatal++;
    }

    if ( fatal )
    {
        x25_argotel_usage(argc, argv);
        exit(FAILURE);
    }
}

/*
 *
 *  Procedure: x25_argotel_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void    x25_argotel_usage(argc, argv)
int argc;
char **argv;
{
#ifdef  DEBUG
    printf("Usage: %s -d<debuglevel> -s<socket> -p<port> -l<lsn> -x<local_x25_addr> -r<remote_x25_addr>\n", argv[0]);
#else
    printf("Usage: %s -s<socket> -p<port> -l<lsn> -x<local_x25_addr> -r<remote_x25_addr>\n", argv[0]);
#endif
}

/*
 *
 *  Procedure: hangup
 *
 *  Parameters:  none
 *
 *  Description: hanguip connection and exit
 *
 *  Return: none
 *
 */

void    hangup()
{
    int rc;
    struct  x25doneinfo xdi;

    /* No wait mode hangup needs x25done	*/


    rc = x25hangup(cid, NULL, XH_IMM, X25NULLFN);

    debug((1, "hangup() - x25hangup rc %d\n", rc));

    if ((( rc = x25done(cid, EXIT_TIMEOUT, &xdi)) == 0 ) && (x25error() == 0 ))
    {
        errlog(X25_LOG, "%s : INCOMING CALL CLEARED\n", remote);
    }

    debug((1, "hangup() - x25done rc %d\n", rc));

    if ( mem_alloc )
    {
        rc = x25free(recvbuf);
        debug((1, "hangup() - x25free rc %d\n", rc));

        rc = x25free(argotel_com_buf);
        debug((1, "hangup() - x25free rc %d\n", rc));
    }

    x25exit();

#ifdef  DEBUG
    enddebug();
#endif

    exit(FAILURE);
}
