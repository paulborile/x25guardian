/*
 * $Id: shell_x28.c,v 1.1.1.1 1998/11/18 15:03:32 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: telnet server
 *
 * Contents: provide a shell to x25 input connection
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: shell_x28.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:32  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/09/29  17:23:29  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: shell_x28.c,v 1.1.1.1 1998/11/18 15:03:32 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <errno.h>
#include        <sys/fcntl.h>
#include        <signal.h>
#include        <sys/types.h>
#include        <sys/time.h>
#include        <termio.h>
#include <sys/stream.h>
#include <sys/stropts.h>

/*  Project include files                       */

#include    "px25_globals.h"
#include    "errlog.h"
#include "debug.h"

/*  Module include files                        */

/*  Extern functions used                       */

void    get_command_line();
void    sigterm();

/*  Extern data used                            */

/*  Local constants                             */

#define OPTS        "s:d:"

/*  Local functions used                        */

void     shell_x28_usage();
void     get_command_line();
int     shell_read();
void    child_termination();

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static char pid_str[20];
static int debuglevel;
static char debug_file[BUFSIZ];

int socket = -1;

char pname[32];
char ibuffer[BUFSIZ];
char obuffer[BUFSIZ];
struct  PKT_HDR *o = (struct PKT_HDR *) (obuffer);
char ptydev[32];
int ptym, ptys;
pid_t pid_slave;
struct  termio t;

main(argc, argv)
int argc;
char **argv;
{
    int src;
    int pid, pid1;
    int c, b;
    int i = 0;

    strcpy(pname, argv[0]);

    get_command_line(argc, argv);

#ifdef DEBUG
    sprintf(pid_str, "%d", getpid());
    sprintf(debug_file, "/tmp/shell_x28%05d.debug", getpid());
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    signal(SIGTERM, (void (*)()) sigterm);

    debug((3, "Started shell_x28\n"));

    if ((ptym = open ("/dev/ptmx", O_RDWR)) < 0)
    {
        debug((1, "open /dev/ptmx failed: %s\n", strerror(errno)));
        errlog(INT_LOG, "%s : open /dev/ptmx failed: %s\n",
               pname, strerror(errno));
#ifdef DEBUG
        enddebug();
#endif
        exit();
    }
    else
    {
        debug((3, "open /dev/ptmx with file des.%d\n", ptym));

        pid_slave = setpgrp();
        debug((3, "pid returned from setpgrp() is %d\n", pid_slave));

        if (grantpt(ptym) < 0)
        {
            debug((1, "cannot access slave device, %s\n", strerror(errno)));
            errlog(INT_LOG, "%s : cannot access slave device: %s\n",
                   pname, strerror(errno));

            close(ptym);

#ifdef DEBUG
            enddebug();
#endif
            exit();
        }

        if (unlockpt(ptym) < 0)
        {
            debug((1, "cannot access slave device, %s\n", strerror(errno)));
            errlog(INT_LOG, "%s : cannot access slave device: %s\n",
                   pname, strerror(errno));

            close(ptym);

#ifdef DEBUG
            enddebug();
#endif
            exit();
        }

        strcpy(ptydev, ptsname(ptym));
        debug((3, "ptyslave is %s\n", ptydev));

        if ((ptys = open (ptydev, O_RDWR)) < 0)
        {
            debug((1, "open of slave %s failed errno %d\n", ptydev, errno));
            errlog(INT_LOG, "%s : open %s failed: %s\n",
                   pname, ptydev, strerror(errno));
            close(ptym);

#ifdef DEBUG
            enddebug();
#endif
            exit();
        }

        ioctl(ptys, TIOCSPGRP, &pid_slave);

        if (ioctl (ptys, I_PUSH, "ptem") < 0)
        {
            debug((1, "ptys ioctl() for ptem failed: %s\n", strerror(errno)));
            errlog(INT_LOG, "ioctl() for ptem failed: %s\n", strerror(errno));
        }
        if (ioctl (ptys, I_PUSH, "ldterm") < 0)
        {
            debug((1, "ptys ioctl() for ldterm failed: %s\n", strerror(errno)));
            errlog(INT_LOG, "ioctl() for ldterm failed: %s\n", strerror(errno));
        }
    }

    signal(SIGCHLD, child_termination);

    pid = fork();

    switch (pid)
    {
    case    -1:

        errlog(INT_LOG, "%s : Fork error\n", argv[0]);
        close(ptym);
        close(ptys);
        close(socket);
#ifdef DEBUG
        enddebug();
#endif
        exit();
        break;

    case    0:

        close(socket);
        dup2(ptys, 0);
        dup2(ptys, 1);
        dup2(ptys, 2);

        if ( ioctl(ptys, TCGETA, &t) < 0 )
        {
            debug((1, "reader: ioctl TCGETA for ptys failed errno %s\n",
                   strerror(errno)));
            errlog(INT_LOG, "ioctl() TCGETA for ptys failed: %s\n",
                   strerror(errno));
        }

        t.c_cflag &= ~CBAUD;
        t.c_cflag |= 9600&CBAUD;

        if ( ioctl(ptys, TCSETA, &t) < 0 )
        {
            debug((1, "reader: ioctl TCSETA for ptys failed errno %s\n",
                   strerror(errno)));
            errlog(INT_LOG, "ioctl() TCSETA for ptys failed: %s\n",
                   strerror(errno));
        }

        execl("/usr/bin/telnet", "telnet", "localhost", 0);

        /* if reached means that execlp failed */

        debug((1, "main() CHILD: execlp failed errno %d\n", errno));
        errlog(INT_LOG,
               "%s : Unable to execlp %s , errno %d\n", pname, "telnet", errno);
        sleep(3);
        exit(FAILURE);
        break;

    default:

        signal(SIGTERM, (void (*)()) sigterm);

        /* here pid is telnet process	so we use pid1 */

        pid1 = fork();

        switch (pid1)
        {
        case    0:             /* child */

            close(ptys);

            while (( src = shell_read(ptym, obuffer + 4)) > 0 )
            {
                debug((3, "reader: shell_read() read %d characters\n", src));

                o->pkt_code = ASY_DATA_PACKET;
                o->flags = 0;
                o->pkt_error = 0;
                o->pkt_len = src;
                if ( mos_send(socket, obuffer,
                              sizeof(struct PKT_HDR) + o->pkt_len) == -1 )
                {
                    /* mos_send failed	*/
                    close(ptym);
                    break;
                }
                debug((3, "mos_sent %s\n", obuffer + sizeof(struct PKT_HDR)));
            }

            debug((1, "reader: shell_read() returned %d\n", src));
            close(socket);
            close(ptym);
            kill(pid, SIGTERM);
            kill(getppid(), SIGTERM);

            debug((1, "reader: exiting\n"));

#ifdef DEBUG
            enddebug();
#endif

            exit();
            break;

        default:

            /* read from input socket	*/
            /* here pid1 is the upper child	*/

            close(ptys);

            while (( c = mos_recv(socket, ibuffer, BUFSIZ)) > 0 )
            {
                debug((3, "mos_recv() %d byte\n", c));

                write(ptym,
                      ibuffer + sizeof(struct PKT_HDR), c - sizeof(struct PKT_HDR));
            }

            kill(pid, SIGTERM);
            kill(pid1, SIGTERM);
            close(socket);
            close(ptym);

            debug((1, "mos_recv() returned %d. Exiting\n", c));

#ifdef DEBUG
            enddebug();
#endif
            exit();
            break;

        }     /* end of second fork	*/

        /* here is father of telnet	*/
    }

#ifdef DEBUG
    enddebug();
#endif

    exit();
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
        case  'd':
            debuglevel  =   atoi(optarg);
            break;

        case 's':
            socket  =   atoi(optarg);
            break;

        default:
            errlog(INT_LOG, "%s : invalid flag %c\n", pname, c);
            shell_x28_usage(argc, argv);
            exit(FAILURE);
        }
    }

#ifdef DEBUG
    if ( debuglevel == -1 )
    {
        fatal++;
    }
#endif

    if ( socket == -1 )
    {
        fatal++;
    }

    if ( fatal )
    {
        shell_x28_usage(argc, argv);

        exit(FAILURE);
    }
}

/*
 *
 *  Procedure: shell_x28_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void  shell_x28_usage(argc, argv)
int argc;
char **argv;
{
#ifdef   DEBUG
    printf("Usage: %s -d<debuglevel> -s<socket>\n", argv[0]);
#else
    printf("Usage: %s -s<socket>\n", argv[0]);
#endif
}

/*
 * read up to newline
 */

int shell_read(fd, buffer)
int fd;
char *buffer;
{
    int count = 0;
    char c;
    int rc;

    fcntl(fd, F_SETFL, O_NDELAY);

again:

    while (( rc = read(fd, &c, 1)) == 1 )
    {
        debug((3, "shell_read() - rc = %d, char = %c\n", rc, c));
        buffer[count] = c;
        count++;

        if (count == 128)
        {
            break;
        }
        if (c == '\n')
        {
            break;
        }
    }

    if ( count != 0 )
    {
        return(count);
    }
    else
    {
        debug((3, "Sleep(1)\n"));
        sleep(1);
        goto again;
    }
}

void    sigterm()
{
    close(ptym);
    close(ptys);
    close(socket);

#ifdef  DEBUG
    enddebug();
#endif

    exit();
}

void  child_termination()
{
    long status = 0;
    pid_t pid;

    debug((3, "child_termination() - Starting.\n"));

    signal(SIGCHLD, SIG_DFL);

    pid = wait(&status);

    signal(SIGCHLD, (void (*)()) child_termination);
    debug((3, "child_termination() - terminated %d with %d\n", pid, status));
}

