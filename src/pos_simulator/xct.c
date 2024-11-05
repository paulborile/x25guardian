/*
 *      Copyright (C) Eicon Technology Corporation, 1994.
 *
 *      A sample program for the file transfer (back-to-back configuration)
 *      which uses X.25 TOOLKIT functions.
 *
 *              The program demonstrates the following topics:
 *              - NOWAIT modes approaches for connection handling,
 *              - the technique of polling with x25done() function,
 *              - the use of qualified listen,
 *              - the management of error codes returned by X.25 TOOLKIT functions.
 */
#ifdef MSC6
#include        <io.h>
#endif
#include        <fcntl.h>
#include        <stdio.h>
#ifndef UNIX
#include        <conio.h>
#endif
#include        <stdlib.h>
#include        <string.h>
#include        <signal.h>

#include        "x25.h"
#include        "neterr.h"

#define OK      2
#define TRUE    1
#define FALSE   0
#define KO     -1

#define UPPER_CALL    'C'
#define LOWER_CALL    'c'
#define UPPER_LISTEN  'L'
#define LOWER_LISTEN  'l'

#define BUFSIZE         16
#define NB_MAX_ATTEMPT   5
#define NEW_LINE        13

#define END_SENTENCE    '#'
#define END_CONNECTION  '$'

#define SIG    -2



#define streq(s1, s2)   (strcmp(s1, s2)==0)

char remote[X25_ADDRLEN+X25_ADDREXT+2],    /* Remote DTE address */
     local[X25_ADDRLEN+X25_ADDREXT+2],     /* Local DTE address */
     listen_udata[XD_UDATALEN],
     *write_buffer,
     *read_buffer,                          /* Buffer for send */
     Cause,
     Diag;

struct  x25data udata,                     /* Call/Listen user data */
                *p_facility;               /* Call/Listen facility */

struct  x25doneinfo done_info;

struct  sigaction isas;

int info,                                  /* Call/Listen info parameter */
    port,                                  /* Call/Listen port parameter */
    port2,
    cid,                                   /* Connection identifier */
    writer,
    acpt_completed,
    lstn_completed,
    call_completed,
    send_completed,
    recv_completed,
    req_completed,
    send_end,
    recv_end,
    connected,
    connection_terminated,
    connection_established,
    Error,
    retcode;
short Method;

int call                      (short Method);
int listen                    (short Method);
int accept_hangup_connection  (short Method);
int recv_char                 (short Method);
int send_char                 (short Method);

void set_call_parameters   (void);
void set_listen_parameters (void);

void check_send      (struct x25doneinfo *done_info);
void check_recv      (struct x25doneinfo *done_info);
void check_call      (struct x25doneinfo *done_info);
void check_listen    (struct x25doneinfo *done_info);
void check_confirm   (struct x25doneinfo *done_info);

void sig_handler   (int sig);

int  done (short Timer,
           void (*post_handler) (struct x25doneinfo *done_info));


void get_side       (char *side);
void usage          (void);
void disp_error     (char *s);
void log_error      (char *s);
void log_cause_diag (char *s);


void sig_handler (int sig)
{
    req_completed = TRUE;
}

/*****************************************************************************
 */
main(argc, argv)
int argc;
char *argv[];
{
    char cmdname;
    int rc, sig;



    send_end = recv_end = FALSE;
    connection_established = connection_terminated = FALSE;
    port = 0xff;


    if ( argc != 2 )
    {
        usage ();
        exit  (1);
    }


    printf ("\n\n XCT Sample program  :  ");
    if ( streq ( argv[1], "SIG") || streq (argv[1], "sig"))
    {
        Method = SIG;

#ifdef UNIXWARE
        sig = SIGPOLL;
#else
        sig = SIGUSR1;
#endif

        printf ("Signal Handler Mode\n");
        sigemptyset (&isas.sa_mask);
        isas.sa_flags = 0;
        isas.sa_handler = sig_handler;
        sigaction (sig, &isas, NULL);
    }
    else
    if ( streq ( argv[1], "TONOW") || streq (argv[1], "tonow"))
    {
        Method = XD_TONOW;
        printf ("Polling mode with No Timeout\n");
    }
    else
    {
        Method = XD_NOTO;
        printf ("Polling mode with Infinte Timeout\n");
    }


    printf ("\n\n\ttype '#' to stop sending character and start receiving\n");
    printf ("\ttype '$' to end the session\n\n");


    /* Initialize the X.25 library. */
    if (Method == SIG)
    {
        rc = x25init (sig);
    }
    else{
        rc = x25init (0);
    }

    if (rc < 0)
    {
        log_error("\nx25init");
        exit(1);
    }

/* Establishment of the connection: The writer "calls" the reader */

    get_side(&cmdname);

    if ( (cmdname == UPPER_CALL) || (cmdname == LOWER_CALL) )
    {
        req_completed = FALSE;
        if ( call (Method) == 0 )
        {
            if (Method == SIG)
            {
                while (req_completed == FALSE)
                {
                    sleep (1);
                }
            }

            while (call_completed == FALSE)
                done (Method, check_call);

            if (call_completed == OK)
            {
                connection_established = OK;
            }
            else{
                disp_error ("call");
            }

            writer = TRUE;
        }
    }


    else if ( (cmdname == UPPER_LISTEN) || (cmdname == LOWER_LISTEN) )
    {
        req_completed = FALSE;
        if ( listen(Method) == 0 )
        {

            if (Method == SIG)
            {
                while (req_completed == FALSE)
                {
                    sleep (1);
                }
            }

            while (lstn_completed == FALSE)
                done (Method, check_listen);

            if ( lstn_completed == OK)
            {
                req_completed = FALSE;
                if ( accept_hangup_connection (Method) == 0)
                {

                    if (Method == SIG)
                    {
                        while (req_completed == FALSE)
                        {
                            sleep (1);
                        }
                    }
                    while (acpt_completed == FALSE)
                        done (Method, check_confirm);

                    if (acpt_completed == KO)
                    {
                        disp_error ("confirm");
                    }

                    writer = FALSE;
                }
            }
            else{
                disp_error ("listen");
            }
        }
    }


    if (connection_established == OK)
    {
        printf("Connection established\n");
    }
    else
    {
        printf("Connection failed .... End of program\n");
        x25exit();
        exit (1);
    }


    /* Communication phase : Alternative exchange of message between both
     *  sides of the connection.
     *  Exchanged messages are sent character after character.
     */

    /* Allocate buffers. */
    if ((write_buffer=x25alloc(BUFSIZE)) == NULL)

    {
        log_error("\nx25alloc");
        exit(1);
    }

    if ((read_buffer=x25alloc(BUFSIZE)) == NULL)
    {
        log_error("\nx25alloc");
        exit(1);
    }

    while ( connection_terminated == FALSE )
    {
        if (writer)
        {
            send_end = FALSE;
            printf ("\n\n =>"); fflush (stdout);
            while ( !send_end )
            {
                req_completed = FALSE;
                if (send_char(Method) == 0)
                {
                    if (Method == SIG)
                    {
                        while (req_completed == FALSE)
                        {
                            sleep (1);
                        }

                        while (done (Method, check_send ) == 0)
                            if (send_completed == KO)
                            {
                                disp_error ("send");
                            }
                    }
                    else
                    {
                        while (send_completed == FALSE)
                            done (Method, check_send);
                        if (send_completed == KO)
                        {
                            disp_error ("send");
                        }
                    }
                }
            }
        }
        else
        {
            recv_end = FALSE;
            printf ("\n\n"); fflush (stdout);
            while ( !recv_end )
            {
                req_completed = FALSE;
                if (recv_char(Method) == 0)
                {
                    if (Method == SIG)
                    {
                        while (req_completed == FALSE)
                        {
                            sleep (1);
                        }
                        while (done (Method, check_recv ) == 0)
                            if (recv_completed == KO)
                            {
                                disp_error ("recv");
                            }
                    }
                    else
                    {
                        while (recv_completed == FALSE)
                            done (Method, check_recv);
                        if (recv_completed == KO)
                        {
                            disp_error ("send");
                        }
                    }
                }
            }
        }
    }
    x25exit ();
    exit(0);
}


/*
 ************************************************************************
 *   This function issues a x25done (waiting until a request completes),
 *  and then call the handler procedure, the application gave as parameter,
 *  with as parameter, the done_info structure filled by the x25done call.
 */
int done (short Timer, void (*post_handler) (struct x25doneinfo *done_info) )
{
    short rc;

    if (( rc = x25done(cid, Timer, &done_info)) != 0)
    {
        Error = x25error();

        if (Method == XD_TONOW)
        {
            if (Error == ENETPEND)
            {
                return (0);
            }
            if (Error == EX25NOPEND)
            {
                return (-1);
            }
        }

        if (Method == XD_NOTO)
        {
            if (Error == EX25NOPEND)
            {
                return (-1);
            }
        }

        if ( (Method == SIG) || (Method == XD_NOTO) )
        {
            if ( (Error == ENETPEND) || (Error == EX25NOPEND) )
            {
                return (-1);
            }
        }

        log_error("\nx25done");
        return (-1);
    }

    (*post_handler) (&done_info);

    return(0);
}


/*****************************************************************************
 *   This function issues a x25xcall (extended call)
 */
int call(short Method)
{


    /* Set call parameters. */
    set_call_parameters();


    call_completed = FALSE;


    printf("Call for connection.\n");

    /* Establish the X.25 connection on NO WAIT mode. */
    if (x25xcall(&cid, X25NOWAIT, port, info, p_facility, &udata,
                 remote, local, X25NULLFN) < 0)
    {
        log_error("\nx25xcall");
        printf ("Unable to call for a connection.....\n");
        return (-1);
    }
    return (0);
}


/*****************************************************************************
 */
int listen(short Method)
{



    /* Set listen parameters. */
    set_listen_parameters();


    lstn_completed = FALSE;


    printf ("listen waiting for a call for a connection....\n");
    /* Listen to incomming calls. Establish the connection on NOWAIT mode. */
    if (x25xlisten(&cid, X25NOWAIT, port, info, p_facility, &udata,
                   remote, local, X25NULLFN) < 0)
    {
        log_error("\nx25xlisten");
        printf ("Unable to listen to a connection.....\n");
        return (-1);
    }

    return (0);

}


/*
 ************************************************************************
 *   This function checks the parameters of the incoming call, and the
 *  accept the connection upon successful checking (x25accept call) or
 *  reject the connection otherwise (x25hangup call).
 */
int accept_hangup_connection (short Method)
{
    struct  x25data *p_inc_udata;
    int accept;
    char rep;

    /* At this point the incoming call has been received.
     * Check the parameters (ex. udata parameter) before accepting or
     * rejecting it.
     */

    acpt_completed = FALSE;

    p_inc_udata = (struct x25data *)&udata;

    printf ("Check the call parameters before accepting it.\n");
    if (strcmp (p_inc_udata->xd_data, listen_udata) == 0)
    {
        accept = TRUE;
    }
    else
    {
        printf ("\n\tExpected User Data : %s\n", listen_udata);
        printf ("\tReceived User Data : %s\n", p_inc_udata->xd_data);

        printf ("\n\tAccept the Call (y/n) ?"); fflush(stdout);

        fflush (stdin);
        /* scanf ("%c\n", &rep); */
        rep = getc(stdin);
        if ( (rep == 'y') || (rep == 'Y') )
        {
            accept = TRUE;
        }
        else{ accept = FALSE;}
        printf ("\n\n");
    }

    if (accept == TRUE)
    {
        printf("Parameters OK => Accept the call.\n");

        info = 0;

        if (x25accept(&cid, info, NULL, NULL, NULL, NULL, X25NULLFN) < 0)
        {
            log_error("\nx25accept");
            printf ("Unable to accept the connection.....\n");
            return (-1);
        }
    }
    else
    /* Check failed. */
    {
        printf("Parameters are not those expected => Reject the call.\n");

        if (x25hangup(cid, NULL, XH_IMM, X25NULLFN) < 0)
        {
            log_error("\nx25hangup-lstn");
            printf ("Unable to reject the connection.....\n");
            return (-1);
        }
    }
    return (0);
}


/*
 ************************************************************************
 *   This function reads a character from the standard input (Keyboard),
 *  sends it to the reader (x25send) in NOWAIT mode and
 *  Checks if it has a special meaning (# or $). If so Application's protocol
 *  variables are correctly updated.
 */
int send_char (short Method)
{
    char cc;


    /* Read a character and send it. */
    cc = getc(stdin);
    write_buffer[0]=cc;
    write_buffer[1]=0;
    if (cc == NEW_LINE)
    {
        printf ("\n =>");
    }
    info = 0;

    send_completed = FALSE;

    if (x25send(cid, write_buffer, BUFSIZE, info, X25NULLFN) < 0)
    {
        log_error("\nx25send");
        printf ("Unable to send data.....\n");
        send_end = TRUE;
        writer = FALSE;
        connection_terminated = KO;

        return (-1);
    }

    if (write_buffer[0] == END_SENTENCE)
    {
        send_end = TRUE;
        writer = FALSE;
    }

    if (write_buffer[0] == END_CONNECTION)
    {
        send_end = TRUE;
        writer = FALSE;
        connection_terminated = OK;
    }

    return (0);

}

/*
 ************************************************************************
 */
int recv_char (short Method)
{

    recv_completed = FALSE;

    if (x25recv(cid, read_buffer, BUFSIZE, &info, X25NULLFN) < 0)
    {
        log_error("\nx25recv");
        printf ("Unable to receive data.....\n");
        return (-1);
    }

    return (0);
}

/*****************************************************************************
 */
void set_call_parameters(void)
{

    printf ("\nCALL Parameters :\n");
    printf ("\tPort Number     = "); fflush(stdout); scanf ("%x", &port);
    printf ("\tLocal  Address  = "); fflush(stdout); scanf ("%s", local);
    printf ("\tRemote Address  = "); fflush(stdout); scanf ("%s", remote);
    printf ("\tUser Data       = "); fflush(stdout); scanf ("%s", udata.xd_data);

    udata.xd_len = strlen (udata.xd_data);

    info = 0;
    p_facility = NULL;

}

/*****************************************************************************
 */
void set_listen_parameters (void)
{
    printf ("\nLISTEN Parameters :\n");
    printf ("\tPort Number     = "); fflush(stdout); scanf ("%x", &port);
    printf ("\tLocal  Address  = "); fflush(stdout); scanf ("%s", local);
    printf ("\tRemote Address  = "); fflush(stdout); scanf ("%s", remote);
    printf ("\tUser Data       = "); fflush(stdout); scanf ("%s", udata.xd_data);

    udata.xd_len = strlen (udata.xd_data);
    strncpy (listen_udata, udata.xd_data, udata.xd_len);

    p_facility = NULL;

    /* Set info parameter for qualified LISTEN. */
    info = XI_QBIT;

}


/*
 ************************************************************************
 */
void check_call (done_info)
struct x25doneinfo *done_info;

{

    switch (done_info->xi_retcode)
    {
    case 0:
        call_completed = OK;
        break;

    default:
        call_completed = KO;
        break;
    }
}

/*****************************************************************************
 */
void check_listen (done_info)
struct x25doneinfo *done_info;
{



    switch (done_info->xi_retcode)
    {
    case 0:
        lstn_completed = OK;
        break;

    default:
        lstn_completed = KO;
        break;
    }
}

/*
 ************************************************************************
 */
void check_confirm (done_info)
struct x25doneinfo *done_info;
{



    switch (done_info->xi_retcode)
    {
    case 0:
        acpt_completed = OK;
        if (done_info->xi_cmd == XC_HANGUP)
        {
            connection_established = KO;
        }
        else
        {
            connection_established = OK;
        }
        break;
    case ENETPEND:
        break;
    default:
        acpt_completed = KO;
    }
}

/*
 ************************************************************************
 */
void check_send (done_info)
struct x25doneinfo *done_info;
{


    switch (done_info->xi_retcode)
    {
    case 0:
        send_completed = OK;
        break;
    case ENETPEND:   /* N/A IN POST_ROUTINE MODE */
        break;
    case ENETCALLCLR:
        send_completed = KO;
        send_end = TRUE;
        connection_terminated = KO;
        break;
    default:
        send_completed = KO;
        send_end = TRUE;
        connection_terminated = KO;
    }
}


void check_recv (done_info)
struct x25doneinfo *done_info;
{

    switch (done_info->xi_retcode)
    {
    case 0:
        switch (read_buffer[0])
        {
        case END_SENTENCE:
            writer = TRUE; recv_end = TRUE;
            break;
        case END_CONNECTION:
            printf ("\n\nSession Closed by remote site\n");
            recv_end = TRUE;
            connection_terminated = OK;
            break;
        case NEW_LINE:
            printf ("\n");
        default:
            printf ("%c", read_buffer[0]); fflush(stdout);
        }
        recv_completed = OK;
        break;
    case ENETPEND:
        break;
    default:
        recv_completed = KO;
        recv_end = TRUE;
        connection_terminated = KO;
    }
}



/*****************************************************************************
 */

void get_side (char *side)
{
    printf ("Choose :    (L) LISTEN => To receive first\n");
    printf ("         or (C) CALL   => To send first\n");
    *side = getc(stdin);
}

void usage(void)
{
    fprintf(stderr, "Usage: xct [ SIG | NOTO | TONOW ]\n");
    exit(0);
}

/*****************************************************************************
 */
void disp_error (command)
char *command;
{
    if (Method == SIG)
    {
        printf("\n%s  error: %02X\n", command, Error);
        printf("%s  cause: %02X\n", command, Cause);
        printf("%s  diag : %02X\n\n", command, Diag);
    }
    else
    {
        Error = x25error();
        log_error(command);
        log_cause_diag (command);
    }
    switch (Error)
    {
    case ENETCALLCLR:
        printf ("Request unexpectidly Cleared by remote site\n");
        break;
    default:
        if (connection_established == OK)
        {
            printf ("We will close the session and exit the application\n");
        }
        break;
    }
}
/*****************************************************************************
 */
void log_cause_diag(s)
char *s;
{
#if defined (OS2)
    printf("%s  cause: %02X\n", s, x25cause());
    printf("%s  diag:  %02X\n", s, x25diag());
#else
    x25pcause(s);
    x25pdiag(s);
    printf ("\n");
#endif
}

/*****************************************************************************
 */
void log_error(s)
char *s;
{
#if defined (OS2)
    printf("%s  error: %02X\n", s, x25error());
#else
    printf("\n");
    x25perror(s);
#endif
}
/*****************************************************************************
 */

