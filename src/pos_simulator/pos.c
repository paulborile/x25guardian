/*
 * $Id: pos.c,v 1.1.1.1 1998/11/18 15:03:27 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: POS simulator
 *
 * Contents: simulate EASYWAY and ARGOTEL calls
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: pos.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:27  paul
 * Guardian : x25 Pos router
 *
 *
 */

static char rcsid[] = "$Id: pos.c,v 1.1.1.1 1998/11/18 15:03:27 paul Exp $";

/*  System include files                        */

#include    <stdio.h>
#include    <string.h>
#include    <eicon/x25.h>

/*  Project include files                       */

/*  Module include files                        */

/*  Extern functions used                       */

/*  Extern data used                            */

/*  Local constants                             */

/*  Local types                                 */

/*  Local macros                                */

#define X25_ADLEN   (X25_ADDRLEN+X25_ADDREXT+2)

/*  Local data                                  */

char dest_addr[X25_ADLEN];
char source_addr[X25_ADLEN];
char *outbuf;
int port;
struct  x25data user_data;
struct  x25data *facility;
int cid;
int info = 0;
char *outbuf;
char msg[BUFSIZ];


main(argc, argv)
int argc;
char **argv;
{
    char msg[BUFSIZ];
    int rc;
    FILE *fp;
    char *finger;
    char dummy[BUFSIZ];

    set_signals();

    printf("POS Simulator.\n");

    printf("Enter source address : ");
    fflush(stdout);
    gets(source_addr);

    /* Take away newline	*/

    if ((finger = strchr(source_addr, '\n')) != NULL )
    {
        *finger = '\0';
    }

    printf("Enter destination address : ");
    fflush(stdout);
    gets(dest_addr);

    if ((finger = strchr(dest_addr, '\n')) != NULL )
    {
        *finger = '\0';
    }

    printf("Output port : ");
    fflush(stdout);
    gets(dummy);
    port = atoi(dummy);

    printf("Call user data string : ");
    fflush(stdout);
    gets(dummy);

    user_data.xd_data[0] = 1;
    user_data.xd_data[1] = 0;
    user_data.xd_data[2] = 0;
    user_data.xd_data[3] = 0;
    strcpy(&(user_data.xd_data[4]), dummy);

    if ((finger = strchr(user_data.xd_data, '\n')) != NULL )
    {
        *finger = '\0';
    }
    user_data.xd_len = strlen(&(user_data.xd_data[4])) + 4;

    printf("OK: going to place call\n");
    printf("Source: %s, Destination: %s, Port: %d, user_data: %s\n",
           source_addr, dest_addr, port, user_data.xd_data);


    printf("Going to initialize x25 ..");
    fflush(stdout);

    if ( x25init(0) < 0 )
    {
        x25perror("x25init()");
        exit();
    }

    printf(" .. done.\nNow calling ...");
    fflush(stdout);

    /*
     * Allocate buffer
     */

    if (( outbuf = x25alloc(1024)) == NULL)
    {
        x25perror("\nx25alloc");
        exit(1);
    }

    /* no facilities	*/
    facility = NULL;

    /* info	*/
    info = 0;

    if ( x25xcall(&cid, X25WAIT, port, info, facility, &user_data,
                  dest_addr, source_addr, X25NULLFN) < 0)
    {
        x25errormsg(msg);
        printf("x25xcall() - Error %s\n", msg);
        x25pdiag("x25xcall");
        x25pcause("x25xcall");
        x25exit();
        exit();
    }

    printf("... call done.\n press enter to continue");

    getchar();

    if ((outbuf=x25alloc(128)) == NULL)
    {
        x25errormsg(msg);
        printf("x25incall_rec - x25alloc error %d %s\n", x25error(), msg);
        x25exit();
        exit(1);
    }

    if (( fp = fopen("/etc/passwd", "r")) == NULL )
    {
        perror("/etc/passwd");
        exit();
    }

    memset(dummy, '\0', BUFSIZ);
    while ( fgets(dummy, BUFSIZ, fp) != NULL )
    {
        dummy[strlen(dummy)] = '\r';
        memcpy(outbuf, dummy, strlen(dummy));

        if (( rc = x25send(cid, outbuf,
                           strlen(dummy), info, X25NULLFN)) < 0 )
        {
            x25errormsg(msg);
            printf("x25send error %d %s\n", x25error(), msg);
        }

        printf("Sent %d bytes\n", strlen(dummy));
        memset(dummy, '\0', BUFSIZ);
    }

    fclose(fp);
    printf("Close connection\n");

    sleep(2);
    x25hangup(cid, NULL, XH_NOTIMM, X25NULLFN);

    x25exit();
    exit(1);
}
