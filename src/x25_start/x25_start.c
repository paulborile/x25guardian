/*
 * $Id: x25_start.c,v 1.1.1.1 1998/11/18 15:03:35 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: supervisor! start the center
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: x25_start.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:35  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.3  1995/09/29  17:29:25  px25
 * Added read of 2nd field of synctab to now how many concurrent
 * x25_listeners to run
 *
 * Revision 1.2  1995/09/18  13:27:11  px25
 * Fixed a bug when starting sm_mgr
 * Better errlog display in output.
 *
 * Revision 1.1  1995/07/18  13:19:59  px25
 * Added setpgrp() to disassociate processes from x25_spv
 *
 * Revision 1.0  1995/07/14  09:20:02  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: x25_start.c,v 1.1.1.1 1998/11/18 15:03:35 paul Exp $";

/*  System include files                        */

#include        <stdio.h>
#include        <stdlib.h>
#include        <unistd.h>
#include        <signal.h>

/*  Project include files                       */

#include        "px25_globals.h"
#include        "errlog.h"
#include        "debug.h"

/*  Module include files                        */

/*  Extern functions used                       */

void    get_command_line();
void    child_termination();
void    terminate();
void    terminate1();
void    daemonize();

/*  Extern data used                            */

extern char **lin_toks();
extern int errno;

/*  Local constants                             */

#define     OPTS                    "d:s"
#define     FILENAME_LEN        128
#define     COMMENT             '#'
#define     SYNC_FILE_FIELDS    4
#define     SM_MANAGER_START_TIMEOUT    7

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

char utmp_file[FILENAME_LEN];
char asy_file[FILENAME_LEN];
char sync_file[FILENAME_LEN];
FILE *fp_utmp;
char myhostname[FILENAME_LEN];
char pname[FILENAME_LEN];
int debuglevel = -1;
int server = 0;
int sm_pid;
struct  sigaction act;
static int sigterm_received = 0;


main(argc, argv)
int argc;
char **argv;
{
    int i;
    char **fields;
    FILE *fp;
    int pid;
    char pid_str[10];
    char debug_file[FILENAME_LEN];
    char buffer[BUFSIZ];
    char command[BUFSIZ];
    char *finger;
    int parallel_listeners = 0;
    int svc;

#ifdef DEBUG
    sprintf(pid_str, "%d", getpid());
    sprintf(debug_file, "/tmp/x25_start%05d.debug", getpid());
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    strcpy(pname, argv[0]);

    get_command_line(argc, argv);

    /*
     * init pid table
     */

    set_pid();

    act.sa_handler      = child_termination;
    act.sa_flags        = SA_RESTART;
    sigemptyset(&act.sa_mask);

    sigaction(SIGCHLD, &act, NULL);

    act.sa_handler      = terminate1;
    act.sa_flags        = 0;

    sigaction(SIGTERM, &act, NULL);

    act.sa_handler      = SIG_IGN;

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);

    if ( gethostname(myhostname, FILENAME_LEN) < 0 )
    {

        printf("UNABLE TO GET NAME OF MY HOST\n");
        exit(99);
    }

    /*
     * check if center is already running
     */

    if (( finger = getenv(PX25_UTMP_FILE)) == NULL )
    {
        printf("VARIABLE PX25_UTMP_FILE NOT SET. TERMINATING\n");
        exit(99);
    }

    setpgrp();

    strcpy(utmp_file, finger);

    /* check for existence	*/

    if ( access(utmp_file, F_OK) == 0 )
    {
        /* the file exists so maybe process are already running */

        if (( fp_utmp = fopen(utmp_file, "r")) == NULL )
        {
            printf("CANNOT OPEN UTMP FILE %s FOR READING\n", utmp_file);
        }
        else
        {
            while (fscanf(fp_utmp, "%05d\n", &pid) == 1);

            if ((kill(pid, 0)) < 0)
            {
                /*process doesn't exist anymore */
                unlink(utmp_file);
            }
            else
            {
                printf("PX25 SOFTWARE FOR MACHINE %s IS ALREADY RUNNING\n", myhostname);
                printf("STOP SOFTWARE WITH x25_stop COMMAND\n");
                exit(99);
            }
        }
    }

    daemonize();

    if (( fp_utmp = fopen(utmp_file, "w")) == NULL )
    {
        printf("CANNOT OPEN UTMP FILE %s FOR WRITING\n", utmp_file);
        exit(99);
    }

    fprintf(fp_utmp, "%05d\n", getpid());
    fclose(fp_utmp);

    /*
     * Start status manager
     */

    if ( server )
    {
        printf("MACHINE IS SERVER SO START STATUS MANAGER\n");

        if ( debuglevel != -1 )
        {
            sprintf(command, "sm_mgr.d -d%d", debuglevel);
        }
        else{
            sprintf(command, "sm_mgr");
        }

        /* spawn process	*/

        if (( sm_pid = spawn_proc(command)) == -1 )
        {
            printf("UNABLE TO START PROCESS %s errno %d\n", command, errno);
            exit(99);
        }

        sleep(SM_MANAGER_START_TIMEOUT);
    }

    /*
     * Now start the asynchronous part
     */

    if (( finger = getenv(PX25_ASY_TABLE_PATH)) == NULL )
    {
        printf("VARIABLE PX25_ASY_TABLE_PATH NOT SET. TERMINATING\n");
        exit(99);
    }

    strcpy(asy_file, finger);

    if ( access(asy_file, F_OK) == 0 )
    {
        printf("ASYNC TABLE PRESENT SO START ASYN_MGR\n");

        if ( debuglevel != -1 )
        {
            sprintf(command, "asyn_mgr.d -d%d", debuglevel);
        }
        else{
            sprintf(command, "asyn_mgr", debuglevel);
        }

        /* spawn process	*/

        if (( pid = spawn_proc(command)) == -1 )
        {
            printf("UNABLE TO START PROCESS %s errno %d\n", command, errno);
            exit(99);
        }

        if ( pid_push(pid, command) == -1 )
        {
            printf("PID TABLE IS FULL FOR %s\n", command);
        }
    }
    else
    {
        printf("NO ASYNCHRONOUS COMMUNICATION SERVER ON %s\n", myhostname);
    }

    /*
     * Start the x25_shell manager only on server machine
     */


    if ( server )
    {
        printf("X25_SHELL MANAGER\n");

        if ( debuglevel != -1 )
        {
            sprintf(command, "x25_shmgr.d -d%d", debuglevel);
        }
        else{
            sprintf(command, "x25_shmgr");
        }

        /* spawn process	*/

        if (( pid = spawn_proc(command)) == -1 )
        {
            printf("UNABLE TO START PROCESS %s errno %d\n", command, errno);
            exit(99);
        }

        if ( pid_push(pid, command) == -1 )
        {
            printf("PID TABLE IS FULL FOR %s\n", command);
        }
    }

    /*
     * Now the SYNC part
     */

    if (( finger = getenv(PX25_SYN_TABLE_PATH)) == NULL )
    {
        printf("VARIABLE PX25_SYN_TABLE_PATH NOT SET\n");
        exit(99);
    }

    strcpy(sync_file, finger);

    if (( fp = fopen(sync_file, "r")) != NULL )
    {
        while ( fgets(buffer, BUFSIZ, fp) != NULL )
        {
            if (( buffer[0] == COMMENT ) || ( buffer[0] == '\n' ))
            {
                continue;
            }

            if (( fields = lin_toks(buffer, SYNC_FILE_FIELDS)) != NULL )
            {
                /* start one listener for each port	*/

                if ( debuglevel != -1 )
                {
                    sprintf(command, "x25_listener.d -d%d -p%s",
                            debuglevel, fields[0]);
                }
                else{
                    sprintf(command, "x25_listener -p%s", fields[0]);
                }

                svc = atoi(fields[2]);
                parallel_listeners = atoi(fields[1]);
                if ( parallel_listeners == 0 )
                {
                    parallel_listeners = 2;
                }
                if ( parallel_listeners > svc )
                {
                    parallel_listeners = svc;
                }

                for (i=0; i<parallel_listeners; i++)
                {
                    if (( pid = spawn_proc(command)) == -1 )
                    {
                        printf("UNABLE TO START PROCESS %s errno %d\n",
                               command, errno);
                        fclose(fp);
                        exit(99);
                    }

                    if ( pid_push(pid, command) == -1 )
                    {
                        printf("PID TABLE IS FULL FOR %s\n", command);
                    }

                    printf("STARTING X25_LISTENER ON PORT %s\n", fields[0]);
                }
            }
        }
        fclose(fp);

        /*
         * now start call manager
         */

        if ( debuglevel != -1 )
        {
            sprintf(command, "x25_call_mgr.d -d%d", debuglevel);
        }
        else{
            sprintf(command, "x25_call_mgr");
        }

        /* spawn process	*/

        if (( pid = spawn_proc(command)) == -1 )
        {
            printf("UNABLE TO START PROCESS %s errno %d\n", command, errno);
            exit(99);
        }

        if ( pid_push(pid, command) == -1 )
        {
            printf("PID TABLE IS FULL FOR %s\n", command);
        }

        printf("STARTING X25_CALL_MGR\n");
    }
    else
    {
        printf("NO SYNCHRONOUS COMMUNICATIONS ON MACHINE %s\n", myhostname);
    }


    printf("Running processes : \n");

    while (( pid = pid_scan(command)) != -1 )
    {
        printf("%s\t: %d\n", command, pid);
    }

    printf("Pausing\n");

    while (1)
    {
        pause();
        if ( sigterm_received )
        {
            terminate();
        }
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

    while ((c = getopt(argc, argv, OPTS)) !=EOF)
    {
        switch (c)
        {
        case    'd':
            debuglevel  =   atoi(optarg);
            break;

        case    's':
            server = 1;
            break;

        default:
            printf("%s : invalid flag.\n", argv[0]);
            printf("Usage : %s -d<debuglevel> [-s]\n", argv[0]);
            exit(FAILURE);
        }
    }

#ifdef DEBUG
    if ( debuglevel == -1 )
    {
        fatal++;
    }
#endif

    if ( fatal )
    {
        printf("Usage : %s -d<debuglevel> [-s]\n", argv[0]);
        exit(FAILURE);
    }
}

/*
 *
 *  Procedure: child_termination
 *
 *  Parameters: standard signal
 *
 *  Description: checkout pidtab for pid terminated
 *
 *  Return:  none
 *
 */

void  child_termination()
{
    int ret;
    long ret_code = 0;
    char command[BUFSIZ];
    pid_t pid;
    int respawn_it = 0;

#define     STATUSLOW   0x000000ff
#define     STATUSHIGH  0x0000ff00

    debug((3, "child_termination() - Starting.\n"));

    pid = wait(&ret_code);

    if ( pid != sm_pid )
    {
        if (pid_pop(pid, command) == -1)
        {
            debug((1, "child_termination() - pid_pop() did not find pid %d\n",
                   pid));
            errlog(INT_LOG, "%s : pid_pop() - process %d not found\n",
                   pname, pid);
        }
    }
    else
    {
        strcpy(command, "STATUS MANAGER");
    }

    if ( ret_code != 0 )
    {
        if ((ret_code & STATUSLOW) == 0)
        {
            debug((1, "child_termination() - Process %d Exits with code %d\n",
                   pid, (ret_code >> 8) & 0x000000ff ));

            errlog(INT_LOG, "PROCESS %d, %s TERMINATED. EXIT CODE %x\n",
                   pid, command, (ret_code >> 8) & 0x000000ff );

            switch ((ret_code >> 8) & 0x000000ff)
            {
            case        FAILURE:
                break;
            case        RESPAWN:
            default:
                respawn_it = 1;
                break;
            }
        }
        else
        {
            if ((ret_code & STATUSHIGH) == 0)
            {
                debug((1, "child_termination() - Process %d Killed by signal %d\n",
                       pid, (ret_code  & 0x0000007f)));

                errlog(INT_LOG, "PROCESS %d, %s ABNORMAL TERMINATION. STATUS %x\n",
                       pid, command, (ret_code & 0x0000007f));

                respawn_it = 1;
            }
        }
    }
    else
    {
        debug((1, "child_termination() - Process died ret_code %d\n", ret_code));
        errlog(INT_LOG, "PROCESS %d, %s TERMINATED. STATUS %x\n",
               pid, command, ret_code);
    }

    if (( respawn_it ) && ( pid != sm_pid))
    {
        if (( pid = spawn_proc(command)) == -1 )
        {
            errlog(INT_LOG, "UNABLE TO RESPAWN %s\n", command);
        }
        else
        {
            pid_push(pid, command);
        }
    }
    return;
}

/*
 * Terminate
 */


void    terminate1()
{
    sigterm_received = 1;
}

void    terminate()
{
    char buffer[BUFSIZ];
    int count = 0;
    int pid;

    /*
     * send SIGTERM to all processes
     */

    pid_kill(SIGTERM);

    sleep(SM_MANAGER_START_TIMEOUT);

again:
    count = 0;
    while (( pid = pid_scan(buffer)) != -1 )
        count++;

    if ( count )
    {
        printf("STILL %d PROCESSES TO TERMINATE\n", count);
        sleep(1);
        goto again;
    }

    kill(sm_pid, SIGTERM);
    sleep(1);

    printf("PX25 SOFTWARE IS DOWN.\n");
    exit(0);
}

void    daemonize()
{
    /*
     * spawn a child and let parent exit
     */
    switch (fork())
    {
    case    -1:      /* error */
        printf("ERROR DAEMONIZING MYSELF\n");
        exit(99);
        break;
    case    0:      /* child    */
        break;
    default:        /* parent   */
        exit(99);
        break;
    }
    return;
}

