/*
 * $Id: asy_x28.c,v 1.1.1.1 1998/11/18 15:03:11 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: asyn_x28
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: asy_x28.c,v $
 * Revision 1.1.1.1  1998/11/18 15:03:11  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.4  1995/09/29  17:18:44  px25
 * Added answer ASY_CALL_{ACCEPT,REJECT} after successfull or not
 * open of tty device.
 *
 * Revision 1.3  1995/09/18  10:53:39  giorgio
 * The asyn_x28 process shall not issue x28_write for X29 packet (with QBIT)
 * coming from syncronous lines
 * 
 *
 * Revision 1.2  1995/09/18  10:39:34  giorgio
 * Errlog message in case of tty configure error with ioctl() function and
 * packet error sent to x25_incall_rec module.
 * Deleted all reference to previous configuration file x3.tab.
 * Aggiunta la gestione dei file di tipo "GPC,3.X29" contenenti parametri X29,
 * che vengono inviati come pacchetti X29(con QBIT alzato).
 *
 * Revision 1.1  1995/07/14  09:08:40  px25
 * Added datascope functionalities
 *
 * Revision 1.0  1995/07/07  10:08:31  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: asy_x28.c,v 1.1.1.1 1998/11/18 15:03:11 paul Exp $";

/*  System include files                        */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include    <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include "sys/types.h"
#include "sys/time.h"
#include "sys/termio.h"

/*  Project include files                       */

#include        "px25_globals.h"
#include    "gp.h"

/*  Module include files                        */

#include "debug.h"
#include "errlog.h"

/*  Extern functions used                       */

extern char *getenv();

/*  Extern data used                            */

extern int gp_errno;

/*  Local constants                             */

#define     OPTS        "d:s:t:"
#define  HOSTNAME_LEN   32
#define MAX_STR 250
#define DEFAULT_BLOCKSIZE   128
#define DEFAULT_TIMEOUT     3
#define DEFAULT_DEV_OPEN_TIMEOUT    10

#define IOCTL_RETRIES       2
#define IOCTL_SLEEP         2

/*  Local functions used								*/

void        asy_x28_usage();
void        get_command_line();
int     x28_write();               /* parameters: device, buf, rc      */
extern int x28_read();               /* parameters: device               */
int   configure();             /* parameters: char globdevice[MAX_STR] */
int sigterm();
int sighup();
int sigalarm();

/*  Local types                                 */

/*  Local macros                                */

/*  Local data                                  */

static char pid_str[20];
static int debuglevel;
static char debug_file[BUFSIZ];

int socket = -1;      /* Parameters for signal processing */
int tty = -1;

int nbyte;          /* used to send x29 file associated with a tty	*/

int RC = -1;          /* Set if read X25_ASY_DATA_PACKET from asy */
int PID = -1;

int fd_x29;
static char myhostname[HOSTNAME_LEN];
char default_path[MAX_STR];
char def_asy_tab[MAX_STR];
char def_gen_path[MAX_STR];
char asy_table[BUFSIZ];
char x29_table[BUFSIZ];
char asy_init_script[BUFSIZ];
char *x29file;
char *genfile;
char *finger;
char *newline;
char *bslash;
char *position;
char *message;
char globdevice[MAX_STR];
char x29buffer[BUFSIZ];
char ttybuffer[BUFSIZ];
char socketbuffer[BUFSIZ];
char buftemp[BUFSIZ];
char temp[BUFSIZ];
char pname[BUFSIZ];
char group[MAX_STR];                /* per il nome del gruppo del tty    */
char line_number[MAX_STR];       /* per il numero della linea del tty */
struct  PKT_HDR *x25 = (struct PKT_HDR *) socketbuffer;
struct  PKT_HDR *x29 = (struct PKT_HDR *) buftemp;

/*	contiene il flag per attivare o meno la funzionalita' di datascope */
int datascope = 0;

/* definizioni per il settaggio della seriale */
int blocksize = -1;
int timeout = -1;
int dev_open_timeout;
int sigalarm_caught = 0;
struct  termio cb;

struct
{
    char *string;
    int speed;
} speeds[] = {
    "0", B0,
    "50", B50,
    "75", B75,
    "110", B110,
    "134", B134,
    "134.5", B134,
    "150", B150,
    "200", B200,
    "300", B300,
    "600", B600,
    "1200", B1200,
    "1800", B1800,
    "2400", B2400,
    "4800", B4800,
    "9600", B9600,
    "19200", B19200,
    "19.2", B19200,
    "38400", B38400,
    "38.4", B38400,
    0,
};
struct mds {
    char *string;
    int set;
    int reset;
};

/* Control Modes */
struct mds cmodes[] = {
    "-parity", CS8, PARENB|CSIZE,
    "-evenp", CS8, PARENB|CSIZE,
    "-oddp", CS8, PARENB|PARODD|CSIZE,
    "parity", PARENB|CS7, PARODD|CSIZE,
    "evenp", PARENB|CS7, PARODD|CSIZE,
    "oddp", PARENB|PARODD|CS7, CSIZE,
    "parenb", PARENB, 0,
    "-parenb", 0, PARENB,
    "parodd", PARODD, 0,
    "-parodd", 0, PARODD,
    "cs8", CS8, CSIZE,
    "cs7", CS7, CSIZE,
    "cs6", CS6, CSIZE,
    "cs5", CS5, CSIZE,
    "cstopb", CSTOPB, 0,
    "-cstopb", 0, CSTOPB,
    "hupcl", HUPCL, 0,
    "hup", HUPCL, 0,
    "-hupcl", 0, HUPCL,
    "-hup", 0, HUPCL,
    "clocal", CLOCAL, 0,
    "-clocal", 0, CLOCAL,
    "loblk", LOBLK, 0,
    "-loblk", 0, LOBLK,
    "cread", CREAD, 0,
    "-cread", 0, CREAD,
    "raw", CS8, (CSIZE|PARENB),
    "-raw", (CS7|PARENB), CSIZE,
    "cooked", (CS7|PARENB), CSIZE,
    "sane", (CS7|PARENB|CREAD), (CSIZE|PARODD|CLOCAL),
    0
};

/* Input Modes */
struct mds imodes[] = {
    "ignbrk", IGNBRK, 0,
    "-ignbrk", 0, IGNBRK,
    "brkint", BRKINT, 0,
    "-brkint", 0, BRKINT,
    "ignpar", IGNPAR, 0,
    "-ignpar", 0, IGNPAR,
    "parmrk", PARMRK, 0,
    "-parmrk", 0, PARMRK,
    "inpck", INPCK, 0,
    "-inpck", 0, INPCK,
    "istrip", ISTRIP, 0,
    "-istrip", 0, ISTRIP,
    "inlcr", INLCR, 0,
    "-inlcr", 0, INLCR,
    "igncr", IGNCR, 0,
    "-igncr", 0, IGNCR,
    "icrnl", ICRNL, 0,
    "-icrnl", 0, ICRNL,
    "-nl", ICRNL, (INLCR|IGNCR),
    "nl", 0, ICRNL,
    "iuclc", IUCLC, 0,
    "-iuclc", 0, IUCLC,
    "lcase", IUCLC, 0,
    "-lcase", 0, IUCLC,
    "LCASE", IUCLC, 0,
    "-LCASE", 0, IUCLC,
    "ixon", IXON, 0,
    "-ixon", 0, IXON,
    "ixany", IXANY, 0,
    "-ixany", 0, IXANY,
    "ixoff", IXOFF, 0,
    "-ixoff", 0, IXOFF,
    "raw", 0, -1,
    "-raw", (BRKINT|IGNPAR|ISTRIP|ICRNL|IXON), 0,
    "cooked", (BRKINT|IGNPAR|ISTRIP|ICRNL|IXON), 0,
    "sane", (BRKINT|IGNPAR|ISTRIP|ICRNL|IXON),
    (IGNBRK|PARMRK|INPCK|INLCR|IGNCR|IUCLC|IXOFF),
    0
};

/* Local Modes */
struct mds lmodes[] = {
    "isig", ISIG, 0,
    "-isig", 0, ISIG,
    "icanon", ICANON, 0,
    "-icanon", 0, ICANON,
    "xcase", XCASE, 0,
    "-xcase", 0, XCASE,
    "lcase", XCASE, 0,
    "-lcase", 0, XCASE,
    "LCASE", XCASE, 0,
    "-LCASE", 0, XCASE,
    "echo", ECHO, 0,
    "-echo", 0, ECHO,
    "echoe", ECHOE, 0,
    "-echoe", 0, ECHOE,
    "echok", ECHOK, 0,
    "-echok", 0, ECHOK,
    "lfkc", ECHOK, 0,
    "-lfkc", 0, ECHOK,
    "echonl", ECHONL, 0,
    "-echonl", 0, ECHONL,
    "noflsh", NOFLSH, 0,
    "-noflsh", 0, NOFLSH,
    "raw", 0, (ISIG|ICANON|XCASE),
    "-raw", (ISIG|ICANON), 0,
    "cooked", (ISIG|ICANON), 0,
    "sane", (ISIG|ICANON|ECHO|ECHOK),
    (XCASE|ECHOE|ECHONL|NOFLSH),
    0,
};

/* Output Modes */
struct mds omodes[] = {
    "opost", OPOST, 0,
    "-opost", 0, OPOST,
    "olcuc", OLCUC, 0,
    "-olcuc", 0, OLCUC,
    "lcase", OLCUC, 0,
    "-lcase", 0, OLCUC,
    "LCASE", OLCUC, 0,
    "-LCASE", 0, OLCUC,
    "onlcr", ONLCR, 0,
    "-onlcr", 0, ONLCR,
    "-nl", ONLCR, (OCRNL|ONLRET),
    "nl", 0, ONLCR,
    "ocrnl", OCRNL, 0,
    "-ocrnl", 0, OCRNL,
    "onocr", ONOCR, 0,
    "-onocr", 0, ONOCR,
    "onlret", ONLRET, 0,
    "-onlret", 0, ONLRET,
    "fill", OFILL, OFDEL,
    "-fill", 0, OFILL|OFDEL,
    "nul-fill", OFILL, OFDEL,
    "del-fill", OFILL|OFDEL, 0,
    "ofill", OFILL, 0,
    "-ofill", 0, OFILL,
    "ofdel", OFDEL, 0,
    "-ofdel", 0, OFDEL,
    "cr0", CR0, CRDLY,
    "cr1", CR1, CRDLY,
    "cr2", CR2, CRDLY,
    "cr3", CR3, CRDLY,
    "tab0", TAB0, TABDLY,
    "tabs", TAB0, TABDLY,
    "tab1", TAB1, TABDLY,
    "tab2", TAB2, TABDLY,
    "tab3", TAB3, TABDLY,
    "-tabs", TAB3, TABDLY,
    "nl0", NL0, NLDLY,
    "nl1", NL1, NLDLY,
    "ff0", FF0, FFDLY,
    "ff1", FF1, FFDLY,
    "vt0", VT0, VTDLY,
    "vt1", VT1, VTDLY,
    "bs0", BS0, BSDLY,
    "bs1", BS1, BSDLY,
    "raw", 0, OPOST,
    "-raw", OPOST, 0,
    "cooked", OPOST, 0,
    "tty33", CR1, (CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
    "tn300", CR1, (CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
    "ti700", CR2, (CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
    "vt05", NL1, (CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
    "tek", FF1, (CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
    "tty37", (FF1|VT1|CR2|TAB1|NL1), (NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY),
    "sane", (OPOST|ONLCR), (OLCUC|OCRNL|ONOCR|ONLRET|OFILL|OFDEL|
                            NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY),
    0,
};

main(argc, argv)
int argc;
char *argv[];
{
    int dev;           /* device descriptor returned by configure */
    int rc;            /* int returned by select */

    get_command_line(argc, argv);

#ifdef DEBUG
    sprintf(pid_str, "%d", getpid());
    sprintf(debug_file, "/tmp/asy_x28%05d.debug", getpid());
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    /*
     * Globalize program name
     */

    strcpy(pname, argv[0]);

    /*
     *	Setting default paths for configuration files
     */

    strcpy(default_path, "/usr3/px25/conf/");

    if ( gethostname(myhostname, HOSTNAME_LEN) < 0 )
    {
        debug((1, "main() - unable to gethostname() errno %d\n", errno));
        errlog(INT_LOG,
               "%s : unable to gethostname - errno %d\n", pname, errno);
    }
    else
    {
        strcat(default_path, myhostname);
        strcat(default_path, "/");
    }

    /*
     * Set signals
     */

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, (void (*)()) sigterm);
    signal(SIGALRM, (void (*)()) sigalarm);

    if (( finger = getenv(PX25_ASY_TABLE_PATH)) == NULL )
    {
        strcpy(def_asy_tab, default_path);
        strcat(def_asy_tab, "asy.table");
        errlog(INT_LOG, "%s : defaulting PX25_ASY_TABLE_PATH to %s\n",
               pname, def_asy_tab);
        finger = def_asy_tab;
    }
    strcpy(asy_table, finger);

    debug((3, "main() - asy_table is %s\n", asy_table));

    /* get the configuration files */

    if (( genfile = getenv(PX25_GEN_TABLE)) == NULL )
    {
        strcpy(def_gen_path, default_path);
        strcat(def_gen_path, "gen.tab");
        errlog(INT_LOG, "%s: defaulting PX25_GEN_TABLE to %s\n",
               pname, def_gen_path);
        genfile = def_gen_path;
    }

    debug((3, "main() - gen.tab is %s\n", genfile));

    /*	datascope function will be active ? */

    if ( (datascope = get_param( genfile, "asy_datascope" )) < 0 )
    {
        errlog(INT_LOG, "%s : get_param() gp_errno %d\n", pname, gp_errno);
        errlog(INT_LOG, "%s : defaulting DATASCOPE FUNCTION TO OFF to %d\n",
               pname);
        datascope = 0;
    }
    if ( (datascope != 1) && (datascope !=0) )
    {
        errlog(INT_LOG, "%s : datascope value not valid in file %s: forced to 0\n",
               pname, genfile);
        datascope = 0;
    }

    debug((3, "datascope value = %d\n", datascope));

    if (datascope == 1)
    {
        if ((position = strrchr(globdevice, '/')) == NULL)
        {
            debug((3, "error with strrchr\n"));
            datascope = 0;
        }
        else
        {
            debug((3, "position = %s\n", position));
            debug((3, "Doing datascope ...\n"));
            dscope_mark(position, "begin read", 'i');
            dscope_mark(position, "begin write", 'o');
        }
    }

    /*	configure */

    if ((dev = configure(globdevice)) < 0)
    {
        errlog(ASY_LOG, "%s : UNABLE TO CONFIGURE ASY DEVICE\n", argv[0]);
        close(socket);

#ifdef DEBUG
        enddebug();
#endif

        if (datascope == 1)
        {
            dscope_mark(position, "end read", 'i');
            dscope_mark(position, "end write", 'o');
        }

        if ( dev == -2 )
        {
            exit(EXIT_AND_DELETE_ROUTE);
        }
        else{
            exit(FAILURE);
        }
    }

    tty = dev;

    /*
     *		Get the X29 file and send it to remote
     */

    if (( x29file = getenv(PX25_X29_DIR)) == NULL )
    {
        errlog(INT_LOG, "%s: defaulting PX25_X29_DIR to %s\n",
               pname, default_path);
        x29file = default_path;
    }

    sprintf(x29_table, "%s/%s,%s.x29", x29file, group, line_number);
    debug((3, "x29_table is %s\n", x29_table));

    if ((fd_x29 = open(x29_table, O_RDONLY)) >= 0)
    {
        debug((3, "Opening file %s ...\n", x29_table));

        nbyte = read(fd_x29, x29buffer, BUFSIZ);

        x29->pkt_code = ASY_DATA_PACKET;
        if ( nbyte <= X25_DATA_PACKET_SIZE )
        {
            x29->pkt_len = nbyte;
        }
        else
        {
            x29->pkt_len = X25_DATA_PACKET_SIZE;
            errlog(INT_LOG, "%s : X29 FILE TOO LARGE (%d)\n", pname, nbyte);
        }

        x29->pkt_error = 0;

        x29->flags = QBIT;

        memcpy(buftemp + sizeof(struct PKT_HDR), x29buffer, x29->pkt_len);
again3:
        if (( mos_send(socket, buftemp,
                       x29->pkt_len + sizeof(struct PKT_HDR))) < 0)
        {
            if ( errno == EINTR )
            {
                goto again3;
            }
            else
            {
                /* unable to write on socket */
                errlog(INT_LOG, "%s: UNABLE TO MOS_SEND %s, errno %d\n",
                       pname, x29_table, errno);
            }
        }

        close(fd_x29);
    }

    /*
     * Check if there is a tty init file
     */

    sprintf(asy_init_script, "%s/%s,%s.init", x29file, group, line_number);
    debug((3, "main() - checking if %s file exists\n", asy_init_script));

    if ( access(asy_init_script, F_OK) == 0 )
    {
        debug((3, "main() - Executing %s\n", asy_init_script));
        system(asy_init_script);
    }

    PID = fork();

    switch (PID)
    {
    case    -1:

        errlog(INT_LOG, "%s : cannot fork, errno %d\n", pname, errno);
        close(socket);
        close(tty);
#ifdef DEBUG
        enddebug();
#endif

        if (datascope == 1)
        {
            dscope_mark(position, "end read", 'i');
            dscope_mark(position, "end write", 'o');
        }

        exit(0);
        break;

    case    0:

        /* This is child process: reader	*/

        signal(SIGTERM, (void (*)()) sigterm);

        while (( rc = x28_read(dev, ttybuffer)) != 0 )
        {
            if (( rc == -1 ) && ( errno == EINTR ))
            {
                continue;
            }

            debug((3, "%s - x28_read returned %d\n",
                   (PID == 0) ? "reader" : "writer", rc));

            if (datascope == 1)
            {
                if (dscope_log(position, ttybuffer + sizeof(struct PKT_HDR), rc, 'i') == -1)
                {
                    errlog(INT_LOG, "%s : unable to open dscope file\n",
                           pname);
                    debug((1, "Unable to open dscope file\n"));
                }
            }

again1:
            if ((RC = mos_send(socket, ttybuffer, rc + sizeof(struct PKT_HDR)))< 0)
            {
                if ( errno == EINTR )
                {
                    goto again1;
                }
                else
                {
                    /* unable to write on socket, close and exit	*/
                    errlog(INT_LOG,
                           "%s: %s - unable to mos_send to syn_x25, errno %d\n",
                           pname, (PID == 0) ? "reader" : "writer", errno);
                    close(socket);
                    close(tty);
                    kill( getppid(), SIGTERM);
#ifdef DEBUG
                    enddebug();
#endif
                    if (datascope == 1)
                    {
                        dscope_mark(position, "end read", 'i');
                    }

                    exit(0);
                }
            }
        }

        /* check EINTR	if LINK DOWN */
again2:
        if ((RC = mos_send(socket, ttybuffer, sizeof(struct PKT_HDR))) < 0)
        {
            if ( errno == EINTR )
            {
                goto again2;
            }
            else{
                debug((1, "%s - Unable to mos_send on socket\n",
                       (PID == 0) ? "reader" : "writer"));
            }
        }
        close(socket);
        close(tty);
        kill( getppid(), SIGTERM);
#ifdef DEBUG
        enddebug();
#endif
        if (datascope == 1)
        {
            dscope_mark(position, "end read", 'i');
        }

        exit(0);
        break;

    default:

        debug((3, "%s - starting to mos_recv\n",
               (PID == 0) ? "reader" : "writer"));
        while (( rc = mos_recv(socket, socketbuffer, BUFSIZ)) > 0 )
        {
            if ( x25->flags != QBIT )
            {
                if ( x28_write(dev, socketbuffer + sizeof(struct PKT_HDR),
                               rc - sizeof(struct PKT_HDR)) == -1)
                {
                    debug((1, "%s - End connection\n",
                           (PID == 0) ? "reader" : "writer"));
                    close(socket);
                    close(tty);
                    kill(PID, SIGTERM);
#ifdef DEBUG
                    enddebug();
#endif
                    if (datascope == 1)
                    {
                        dscope_mark(position, "end write", 'o');
                    }

                    exit(0);
                }
            }
            else
            {
                debug((3, "%s - received packet with QBIT, going to refuse it\n",
                       (PID == 0) ? "reader" : "writer"));
            }
        }

        debug((1, "%s - End connection\n",
               (PID == 0) ? "reader" : "writer"));
        close(socket);
        close(tty);
        kill(PID, SIGTERM);
#ifdef DEBUG
        enddebug();
#endif
        if (datascope == 1)
        {
            dscope_mark(position, "end write", 'o');
        }

        exit(0);
        break;

    }
#ifdef DEBUG
    enddebug();
#endif
    exit(0);
}

/*
 *
 *  Procedure: sigterm
 *
 *  Parameters: none
 *
 *  Description: close socket and tty and exit
 *
 *  Return:
 *
 */

sigterm(sig)
{
    close(socket);
    close(tty);
    debug((3, "%s - terminating.\n",
           (PID == 0) ? "reader" : "writer"));

#ifdef DEBUG
    enddebug();
#endif

    if ( datascope )
    {
        if ( PID == 0 )
        {
            /* child process	*/
            dscope_mark(position, "End read", 'i');
        }
        else
        {
            dscope_mark(position, "End write", 'o');
        }
    }
    exit(0);
}

/*
 *
 *  Procedure: sighup
 *
 *  Parameters: none
 *
 *  Description: close tty and exit
 *
 *  Return:
 *
 */

sighup(sig)
{
    close(tty);
    errlog(ASY_LOG, "%s : SIGHUP received from %s\n", pname, globdevice);

#ifdef DEBUG
    enddebug();
#endif

    if (datascope == 1)
    {
        dscope_mark(position, "end read", 'i');
        dscope_mark(position, "end write", 'o');
    }

    exit(0);
}


/*
 *  Procedure: sigalarm
 *
 *  Parameters: none
 *
 *  Description: close tty and exit
 *
 *  Return:
 *
 */

sigalarm(sig)
{
    sigalarm_caught = 1;
}

/*
 *  Procedure: configure
 *
 *  Parameters: device
 *
 *  Description: configure the serial port
 *
 *  Return: the device descriptor or -1 if unable to configure
 *
 */

int     configure(device)
char device[MAX_STR];
{
    register int i;

/* definizioni per l'ottenimento della configurazione della seriale */
    FILE *fd;
    char line[MAX_STR];
    char *newtok;
    char *param;
    char *finger, *cur, *Line, *linea;
    char buffer[BUFSIZ];
    char tomatch[MAX_STR];         /* per il nome del device dal file asyn.tab */
    int dev_found = 0;
    int ioctl_count = 0;

    int dev;                 /* device descriptor */

/* configurazione della seriale */

    if ((fd = fopen(asy_table, "r")) == NULL)
    {
        debug((1, "Unable to open file %d\n", asy_table));
        return -1;
    }
    else
    {
        while (fgets(line, MAX_STR, fd) != NULL)
        {
            if (line[0] == '#' || line[0] == '\n')
            {
                continue;
            }
            if (( finger = strchr(line, ':')) != NULL )
            {
                *finger = '\0';
                strcpy(group, line);
                debug((3, "Group type %s\n", group));

                /*	estrai il nome del device che e` prima dell'ultimo ":"  */
                cur = finger + 1;
                if (( finger = strchr(cur, ':')) != NULL )
                {
                    *finger = '\0';
                    strcpy(line_number, cur);
                    debug((3, "Line number %s\n", line_number));

                    Line = finger + 1;
                }
                else
                {
                    fclose(fd);
                    return -1;
                }

                if ((finger = strchr(Line, ':')) != NULL)
                {
                    linea = finger +1;  /* punta al char dopo l'ultimo : */
                    strcpy(buffer, linea);
                    *finger = '\0';
                    strcpy(tomatch, Line);
                }
                else
                {
                    fclose(fd);
                    return -1;
                }
            }
            else{ continue;}

            if (strcmp(device, tomatch) == 0)
            {
                dev_found = 1;

                do
                {
                    if (( bslash = strchr(buffer, '\\')) != NULL )
                    {
                        *bslash = '\0';
                    }
                    else{break;}

                    fgets(line, MAX_STR, fd);
                    strcat(buffer, line);

                }while (strchr(line, '\\') != NULL);

                if ((newline = strchr(buffer, '\n')) != NULL )
                {
                    *newline = '\0';
                }

                break;
            }
            else{continue;}

        }   /* fine ciclo per estrarre la stringa da parsare */

        if (!dev_found)
        {
            debug((1, "Device not found\n"));
            fclose(fd);
            return -1;
        }

        if ((dev_open_timeout =
                 get_param( genfile, "serial_open_timeout" )) < 0 )
        {
            errlog(INT_LOG, "%s : get_param() gp_errno %d\n",
                   pname, gp_errno);
            errlog(INT_LOG, "%s : defaulting DEV_OPEN_TIMEOUT to %d\n",
                   pname, DEFAULT_DEV_OPEN_TIMEOUT);
            dev_open_timeout = DEFAULT_DEV_OPEN_TIMEOUT;
        }
        debug((3, "dev_open_timeout %d\n", dev_open_timeout));

        alarm(dev_open_timeout);

        if ((dev = open(tomatch, O_RDWR)) < 0)
        {
            if ( (errno == EINTR) && (sigalarm_caught == 1) )
            {
                debug((1, "TIMEOUT : Unable to open %s\n", tomatch));
                debug((1, "mos_sending TIMEOUT ASY_CALL_REJECT packet\n"));
                sprintf(temp, "%s OPEN TIMEOUT", device);
                x25->pkt_code = ASY_CALL_REJECT;
                x25->pkt_error = PORT_DISABLED;
                strcpy(socketbuffer + sizeof(struct PKT_HDR), temp);
                if ((mos_send(socket, socketbuffer, strlen(temp) + sizeof(struct PKT_HDR))) < 0 )
                {
                    debug((1,
                           "Unable to mos_sending ASY_CALL_REJECT on socket %d\n",
                           socket));
                    errlog(INT_LOG,
                           "%s : cannot mos_sending ASY_CALL_REJECT on socket %d\n",
                           pname, socket);
                }
            }
            else
            {
                debug((1, "Unable to open %s\n", tomatch));
                debug((1, "mos_sending ASY_CALL_REJECT packet\n"));
                message = strerror(errno);
                sprintf(temp, "%s %s", device, message);
                x25->pkt_code = ASY_CALL_REJECT;
                x25->pkt_error = PORT_DISABLED;
                strcpy(socketbuffer + sizeof(struct PKT_HDR), temp);
                if ((mos_send(socket, socketbuffer, strlen(temp) + sizeof(struct PKT_HDR)))< 0 )
                {
                    debug((1,
                           "Unable to mos_sending ASY_CALL_REJECT on socket %d\n",
                           socket));
                    errlog(INT_LOG,
                           "%s : cannot mos_sending ASY_CALL_REJECT on socket %d\n",
                           pname, socket);
                }
            }
            fclose(fd);
            return(-2);
        }
        alarm(0);

        debug((1, "Aperto il device %s\n", tomatch));

tcgeta:

        if (ioctl(dev, TCGETA, &cb) == -1)
        {
            debug((1, "TCGETA error: errno %d\n", errno));
            debug((1, "mos_sending ASY_ERROR packet\n"));
            errlog(INT_LOG, "%s : TCGETA error on %s: errno %d\n",
                   pname, device, errno);
            message = strerror(errno);
            sprintf(temp, "TCGETA %s %s", device, message);
            x25->pkt_code = ASY_CALL_REJECT;
            x25->pkt_error = 0;
            strcpy(socketbuffer + sizeof(struct PKT_HDR), temp);
            if ((mos_send(socket, socketbuffer, strlen(temp) + sizeof(struct PKT_HDR)))< 0 )
            {
                debug((1,
                       "Unable to mos_sending ASY_ERROR on socket %d\n",
                       socket));
                errlog(INT_LOG,
                       "%s : cannot mos_sending ASY_ERROR on socket %d\n",
                       pname, socket);
            }

            fclose(fd);
            return -1;
        }

        if ((newtok = strtok(buffer, " ")) != NULL)
        {
            debug((3, "found token for %s\n", tomatch));

            do
            {
                if ((param = strrchr(newtok, '=')) != NULL)
                {
                    *param = '\0';
                    if (strcmp(newtok, "blocksize") == 0)
                    {
                        blocksize = atoi(param + 1);
                        cb.c_cc[VMIN] = blocksize + 1;
                    }
                    if (strcmp(newtok, "timeout") == 0)
                    {
                        timeout = atoi(param + 1);
                        cb.c_cc[VTIME] = timeout;
                    }
                }

                for (i=0; speeds[i].string; i++)
                    if (strcmp(speeds[i].string, newtok) == 0)
                    {
                        cb.c_cflag &= ~CBAUD;
                        cb.c_cflag |= speeds[i].speed&CBAUD;
                        break;
                    }
                for (i=0; imodes[i].string; i++)
                    if (strcmp(imodes[i].string, newtok) == 0)
                    {
                        cb.c_iflag &= ~imodes[i].reset;
                        cb.c_iflag |= imodes[i].set;
                        break;
                    }
                for (i=0; omodes[i].string; i++)
                    if (strcmp(omodes[i].string, newtok) == 0)
                    {
                        cb.c_oflag &= ~omodes[i].reset;
                        cb.c_oflag |= omodes[i].set;
                        break;
                    }
                for (i=0; cmodes[i].string; i++)
                    if (strcmp(cmodes[i].string, newtok) == 0)
                    {
                        cb.c_cflag &= ~cmodes[i].reset;
                        cb.c_cflag |= cmodes[i].set;
                        break;
                    }
                for (i=0; lmodes[i].string; i++)
                    if (strcmp(lmodes[i].string, newtok) == 0)
                    {
                        cb.c_lflag &= ~lmodes[i].reset;
                        cb.c_lflag |= lmodes[i].set;
                        break;
                    }
                debug((1, "%s\n", newtok));
            } while ((newtok = strtok(NULL, " ")) != NULL);
        }
        else
        {
            debug((1, "strtok == NULL\n"));
            fclose(fd);
            return -1;
        }

        ioctl_count = 0;
tcsetaw:
        if (ioctl(dev, TCSETAW, &cb) == -1)
        {
            if ( ioctl_count <= dev_open_timeout )
            {
                ioctl_count += IOCTL_SLEEP;
                sleep(IOCTL_SLEEP);
                goto tcsetaw;
            }

            debug((1, "TCSETAW error: errno %d\n", errno));
            debug((1, "mos_sending ASY_ERROR packet\n"));
            errlog(INT_LOG, "%s : TCSETAW error on %s: errno %d\n",
                   pname, device, errno);
            message = strerror(errno);
            sprintf(temp, "TCSETAW %s %s", device, message);
            x25->pkt_code = ASY_CALL_REJECT;
            x25->pkt_error = 0;
            strcpy(socketbuffer + sizeof(struct PKT_HDR), temp);
            if ((mos_send(socket, socketbuffer, strlen(temp) + sizeof(struct PKT_HDR)))< 0 )
            {
                debug((1,
                       "Unable to mos_sending ASY_ERROR on socket %d\n",
                       socket));
                errlog(INT_LOG,
                       "%s : cannot mos_sending ASY_ERROR on socket %d\n",
                       pname, socket);
            }

            fclose(fd);
            return -1;
        }

        /*
         * Send ASY_CALL_ACCEPT
         */

        x25->pkt_code   = ASY_CALL_ACCEPT;
        x25->pkt_error  = 0;
        x25->flags      = 0;
        x25->pkt_len    = 0;

        if (mos_send(socket, socketbuffer, sizeof(struct PKT_HDR)) < 0 )
        {
            debug((1,
                   "Unable to mos_send ASY_CALL_ACCEPT on socket %d\n",
                   socket));
            errlog(INT_LOG,
                   "%s : cannot mos_send ASY_CALL_ACCEPT on socket %d\n",
                   pname, socket);

            fclose(fd);
            return(-1);
        }

        /* if we didn't find default values for timeout and           */
        /* blocksize assign'em default values                             */

        if (blocksize == -1)
        {
            blocksize = DEFAULT_BLOCKSIZE;
        }
        if (timeout == -1)
        {
            timeout = DEFAULT_TIMEOUT;
        }

        debug((3, "blocksize value %d\n", blocksize));
        debug((3, "timeout value %d\n", timeout));

        debug((1, "Fine configurazione %s\n", tomatch));

        fclose(fd);
        return dev;
    }
}

/*
 *
 *  Procedure: x28_write
 *
 *  Parameters: device, buf, len
 *
 *  Description: write x28 packet on serial
 *
 *  Return: 0 if succeded, -1 if error
 *
 */

int x28_write(device, buf, len)
int device;
char *buf;
int len;
{
    if ((write(device, buf, len)) != -1)
    {
        debug((1, "x28_write() - writing %d bytes of %c\n",
               len, buf[0]));

        if ( datascope == 1)
        {
            if (dscope_log(position, buf, len, 'o') == -1)
            {
                errlog(INT_LOG, "%s : unable to open dscope file\n",
                       pname);
                debug((1, "Unable to open dscope file\n"));
            }
        }

        return 0;
    }
    else
    {
        debug((1, "x28_write: errno %d\n", errno));
        errlog(ASY_LOG, "%s : Error %d from x28_write() on asy\n", pname, errno);
        return -1;
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

    globdevice[0] = '\0';

    while ((c = getopt(argc, argv, OPTS)) !=EOF)
    {
        switch (c)
        {
        case    'd':
            debuglevel  =   atoi(optarg);
            break;

        case    's':

            socket  =   atoi(optarg);
            break;

        case    't':

            strcpy(globdevice, optarg);
            break;

        default:
            printf("%s : invalid flag.\n", argv[0]);
            asy_x28_usage(argc, argv);
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

    if ( globdevice[0] == '\0' )
    {
        fatal++;
    }

    if ( fatal )
    {
        asy_x28_usage(argc, argv);
        exit(FAILURE);
    }
}



/*
 *
 *  Procedure: asy_x28_usage
 *
 *  Parameters: argc, argv
 *
 *  Description: obvious ?
 *
 *  Return: ah!
 *
 */

void    asy_x28_usage(argc, argv)
int argc;
char **argv;
{
#ifdef  DEBUG
    printf("Usage: %s -d<debuglevel> -s<socket> -t<tty>\n", argv[0]);
#else
    printf("Usage: %s -s<socket> -t<tty>\n", argv[0]);
#endif
}
