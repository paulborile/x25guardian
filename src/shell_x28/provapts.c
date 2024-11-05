/*  System include files                        */

#include        <stdio.h>
#include        <errno.h>
#include        <fcntl.h>
#include        <signal.h>
#include        <sys/types.h>
#include        <sys/time.h>
#include        <termio.h>

/*  Project include files                       */

#include    "errlog.h"
#include "debug.h"

/*  Module include files                        */

/*  Extern functions used                       */

void    get_command_line();
void    sigterm();

/*  Extern data used                            */

/*  Local constants                             */

/*  Local functions used                        */

void     shell_x28_usage();
void     get_command_line();
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
    sprintf(debug_file, "/tmp/prova_pty%05d.debug", getpid());
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    signal(SIGCHLD, child_termination);
    signal(SIGTERM, (void (*)()) sigterm);

    debug((3, "Started shell_x28\n"));

    if ((ptym = open ("/dev/ptmx", O_RDWR)) < 0)
    {
        debug((1, "open /dev/ptmx failed: %s\n", strerror(errno)));
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

        /************
           if (grantpt(ptym) < 0)
           {
           debug((1,"cannot access slave device, %s\n", strerror(errno)));
            errlog(INT_LOG,"%s : cannot access slave device: %s\n",
                                pname, strerror(errno));

           close(ptym);

         #ifdef DEBUG
           enddebug();
         #endif
           exit();
           }
         **************/

        if (unlockpt(ptym) < 0)
        {
            debug((1, "cannot access slave device, %s\n", strerror(errno)));

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
            close(ptym);

#ifdef DEBUG
            enddebug();
#endif
            exit();
        }

        ioctl(ptys, TIOCSPGRP, &pid_slave);

    }

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
        execl("/usr/bin/telnet", "telnet", "localhost", 0);

        /* if reached means that execlp failed */

        debug((1, "main() CHILD: execlp failed errno %d\n", errno));
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

            dup2(ptys, 0);

            while (( read(ptys, &c, 1)) > 0 )
            {
                buffer[count] = c;
                count++;
            }
            debug((3, "reader: shell_read() read %d characters\n", src));

        }

        close(socket);
        close(ptym);
        close(ptys);
        kill(pid, SIGTERM);
        kill(getppid(), SIGTERM);
#ifdef DEBUG
        enddebug();
#endif
        exit();
        break;

    default:

        dup2(ptys, 1);

        /* read from input socket	*/
        /* here pid1 is the upper child	*/

        write(ptys,
              ibuffer + sizeof(struct PKT_HDR), c - sizeof(struct PKT_HDR));

        kill(pid, SIGTERM);
        kill(pid1, SIGTERM);
        close(socket);
        close(ptym);
        close(ptys);
#ifdef DEBUG
        enddebug();
#endif
        exit();
        break;

    }         /* end of second fork	*/

    /* here is father of telnet	*/
}

#ifdef DEBUG
enddebug();
#endif

exit();
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

    while (( rc = read(fd, &c, 1)) == 1 )
    {
        buffer[count] = c;
        count++;
    }

    return(rc);
}

void    sigterm()
{
    close(ptym);
    close(ptys);
    close(socket);
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

