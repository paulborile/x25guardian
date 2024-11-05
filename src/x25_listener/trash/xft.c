/*
 *      Copyright (C) Eicon Technology Corporation, 1994.
 *
 *      A sample program for the file transfer (back-to-back configuration)
 *      which uses X.25 TOOLKIT functions.
 *
 *      Usage :
 *        The program is invoked with three parameters.
 *        The first is the port number (hex).
 *        The second having one of the two values:
 *        - recv  which causes the following actions : listen, receive file
 *              from remote DTE, hangup,
 *        - send  which causes the following actions : call, send file
 *              to remote DTE, hangup.
 *        The third is the file name which will be send or received.
 *
 *      The program demonstrates the use of qualified listen and
 *      management of error codes returned by X.25 TOOLKIT functions.
 *
 */
#include        <stdio.h>
#include        <unistd.h>
#include        <fcntl.h>
#include        <string.h>

#include        "x25.h"
#include        "neterr.h"

#define streq(s1, s2)   (strcmp(s1, s2)==0)

#define SENDBUFSIZE         512
#define RECVBUFSIZE         512

char    remote[X25_ADDRLEN+X25_ADDREXT+2]; /* Remote DTE address */
char    local[X25_ADDRLEN+X25_ADDREXT+2];  /* Local DTE address */
struct  x25data udata;                     /* Call/Listen user data */
struct	x25data facil;
struct  x25data *facility_p;               /* Call/Listen facility */
int     info;                              /* Call/Listen info parameter */
char    *sendbuf;                          /* Buffer for send */
char    *recvbuf;                          /* Buffer for receive */
int     port;                              /* Call/Listen port parameter */
int     cid;                               /* Connection identifier */

static void    log_cause_diag();
static void    log_error();
static void    usage();

/*****************************************************************************
*/
main(argc, argv)
int     argc;
char    *argv[];
{

        char    msg[ENETMSGLEN];
        char    *file;
        char    *cmdname;

        if (argc != 4)
                usage();
	if (1!=sscanf(argv[1],"%x",&port))
		exit(1);
        cmdname = argv[2];
        file = argv[3];
        /* Initialize the X.25 library. */
        if (x25init(0) < 0) {
                log_error("\nx25init");
                exit(1);
        }
        /* Print the current TOOLKIT version */
        x25version(msg);
        printf("%s\n", msg);
        /* Call the function selected on the command line */
        if (streq(cmdname, "send"))
                send(file);
        else if (streq(cmdname, "recv"))
                recv(file);
        else
                usage();
        exit(0);
}

/*****************************************************************************
*/
send(file)
char    *file;
{
        int     fd;
        int     cc;
        char    rep;

        /* Open file to be sent. */
        if ((fd = open(file, O_RDONLY)) < 0) {
                perror(file);
                exit(1);
        }
        /* Allocate buffers. */
        if ((sendbuf=x25alloc(SENDBUFSIZE)) == NULL) {
                log_error("\nx25alloc");
                exit(1);
        }
        if ((recvbuf=x25alloc(RECVBUFSIZE)) == NULL) {
                log_error("\nx25alloc");
                exit(1);
        }
        /* Set parameters. */
        info = 0;
        facility_p = NULL;
        /* Initialize the user data that the LISTEN side will check before
         * accepting or rejecting the call.
         */
        udata.xd_data[0] = 1;
	udata.xd_data[1] = 0;
	udata.xd_data[2] = 0;
	udata.xd_data[3] = 0;
        udata.xd_len = 4;
        /* Initialize the remote and local DTE addresses */
        strcpy(remote, "123");
        strcpy(local, "");
        /* Establish the X.25 connection in WAIT mode. */
        if (x25xcall(&cid, X25WAIT, port, info, facility_p, &udata,
                     remote, local, X25NULLFN) < 0) {
                log_error("\nx25xcall");
                log_cause_diag("x25xcall");
                x25exit();
                exit(1);
        }
        printf("Connection established.\n");
        /* Read file and send data. */
        while ((cc=read(fd, sendbuf, SENDBUFSIZE)) > 0) {
                static  long size = 0;

                info = 0;
                if (x25send(cid, sendbuf, cc, info, X25NULLFN) < 0) {
                        log_error("\nx25send");
                        switch (x25error()) {

                        case ENETSRESET:
                        /* Call x25recv() to get the cause and diagnostic. */
                                if (x25recv(cid, recvbuf, RECVBUFSIZE, &info,
                                            X25NULLFN) > 0)
                                        log_cause_diag("x25send-recv");
                                x25exit();
                                break;
                        case ENETCALLCLR:
                        /* Call x25hangup() and get the cause and diagnostic. */
                                if (x25hangup(cid, NULL, XH_IMM,X25NULLFN) < 0)
                                        log_error("\nx25hangup");
                                else
                                        log_cause_diag("x25send-hang");
                                x25exit();
                                break;
                        default:
                                x25exit();
                        }
                        exit(1);
                }
                size += cc;
                printf("\r%ld bytes sent ", size);
                fflush(stdout);
        }
        if (cc < 0) {
                perror("read");
                x25exit();
                exit(1);
        }
        printf("\nDone.");
        sleep (5);
      
        /* Hangup the active connection and clean the x25 context. */
        x25exit();
        printf("Connection cleared.\n");
}

/*****************************************************************************
*/
recv(file)
char    *file;
{
        int     fd;
        int     cc;

        /* Open/creat file that will be filled with received data. */
        if ((fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
                perror(file);
                exit(1);
        }
        /* Allocate a buffer. */
        if ((recvbuf=x25alloc(RECVBUFSIZE)) == NULL) {
                log_error("\nx25alloc");
                exit(1);
        }
        /* Set info for qualified LISTEN. */
        info = XI_QBIT; 
        /* Set parameters. */
        facility_p = NULL;
        udata.xd_len = 4;
	udata.xd_data[0] = 1;
	udata.xd_data[1] = 0;
	udata.xd_data[2] = 0;
	udata.xd_data[3] = 0;
        facil.xd_len = 0;
	/* Set remote and local addresses */
        strcpy(remote, "");
        strcpy(local, "");
        /* Listen to incoming calls - Establish the connection in WAIT mode. */
        if (x25xlisten(&cid, X25WAIT, port, info, &facil, &udata,
                       remote, local, X25NULLFN) < 0) {
                log_error("\nx25xlisten");
                log_cause_diag("x25xlisten");
                x25exit();
                exit(1);
        }
        /* Check the incoming call (ex. udata parameter) before accepting or
         * rejecting it.
         */
        if (info & XI_QBIT) {
                if (udata.xd_len > 0 && udata.xd_data[0] == 1) {
                        info = 0;
                        if (x25accept(&cid, info, NULL, NULL, remote, local, X25NULLFN) < 0) {
                                log_error("\nx25accept");
                                exit(1);
                        }
                        printf("Call accepted.\n");
                }
                else {
                        if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0) {
                                log_error("\nx25hangup-lstn");
                        }
                        printf("Call rejected.\n");
                        x25exit();
                        exit(1);
                }
	}
        printf("Connection established.\n");
        /* Receive data and write into file. */
        while ((cc=x25recv(cid, recvbuf, RECVBUFSIZE, &info, X25NULLFN)) > 0) {
                static  long size = 0;

                size += cc;
                printf("\r%d bytes received ", size);
                fflush(stdout);
                if (write(fd, recvbuf, cc) < 0) {
                        perror("write");
                        x25exit();
                        exit(1);
                }
        }
        if (cc < 0) {
                log_error("\nx25recv");
                switch (x25error()) {

                case ENETSRESET:
                        if (info == 0 || info == XI_MBIT) {
                        /* Call x25recv() to get the cause and diagnostic. */
                                if (x25recv(cid, recvbuf, RECVBUFSIZE, &info,
                                            X25NULLFN) > 0)
                                        log_cause_diag("x25recv-recv");
                                else
                                        log_error("\nx25recv-recv");
                        }
                        else if (info == XC_RRESETIND)
                        /* Get the cause and diagnostic */
                                log_cause_diag("x25recv");
                        x25exit();
                        break;
                case ENETCALLCLR:
                        /* Call x25hangup() and get the cause and diagnostic. */
                        if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
                                log_error("\nx25hangup-recv");
                        else
                                log_cause_diag("x25recv-hang");
                        x25exit();
                        break;
                default:
                        x25exit();
                }
	        printf("\nDone.\n");
	        printf("Connection cleared.\n");
                exit(1);
        }
        printf("\nDone.\n");
        /* Hangup the active connection and clean the x25 context. */
        x25exit();
        printf("Connection cleared.\n");
}

/*****************************************************************************
*/
static void usage()
{
        fprintf(stderr, "Usage: xft port [recv|send] file\n");
        exit(0);
}

/*****************************************************************************
*/
static void log_cause_diag(s)
char    *s;
{
        x25pcause(s);
        x25pdiag(s);
}

/*****************************************************************************
*/
static void log_error(s)
char    *s;
{
        x25perror(s);
}
