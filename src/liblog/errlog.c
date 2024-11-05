/*
 * $Id: errlog.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $
 *
 *	$Log: errlog.c,v $
 *	Revision 1.1.1.1  1998/11/18 15:03:23  paul
 *	Guardian : x25 Pos router
 *
 * Revision 1.1  1995/07/12  14:26:09  px25
 * Pid is not printed anymore in X25 logfile
 *
 * Revision 1.0  1995/07/07  09:53:56  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: errlog.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $";

#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include        <time.h>
#include        <sys/stat.h>
#include        <varargs.h>

#include        "errlog.h"


/*VARARGS0*/
void    errlog(va_alist)
va_dcl
{
    struct  stat buf;
    struct  tm *t;
    struct  tm tsave;
    time_t clock;
    char *finger;
    char file[BUFSIZ];
    char filename[BUFSIZ];
    char open_mode[2];
    FILE *fp;
    char *fmt;
    int type;
    va_list dargs;

    va_start(dargs);
    type    = va_arg(dargs, int);
    fmt = va_arg(dargs, char *);

    time(&clock);
    t = localtime(&clock);
    tsave = *t;

    if (( finger = getenv(PX25_LOG_DIR)) != NULL )
    {
        strcpy(file, finger);

        switch (type)
        {
        case    X25_LOG:
            strcat(file, X25_FILE_NAME);
            break;
        case    ASY_LOG:
            strcat(file, ASY_FILE_NAME);
            break;
        case    SNA_LOG:
            strcat(file, SNA_FILE_NAME);
            break;
        case    INT_LOG:
            strcat(file, INT_FILE_NAME);
            break;
        default:
            strcpy(file, "/dev/tty");
            break;
        }
        sprintf(filename, "%s-%02d", file, tsave.tm_mday);

        if ( stat(filename, &buf) < 0 )
        {
            strcpy(open_mode, "a");
        }
        else
        {
            t = localtime(&buf.st_mtime);

            if ( t->tm_mon == tsave.tm_mon )
            {
                /* It was modified the same month so today so append */
                strcpy(open_mode, "a");
            }
            else
            {
                /* it was modified last month */
                strcpy(open_mode, "a");
                fp = fopen(filename, "w");
                fclose(fp);
            }
        }

        if (( fp = fopen(filename, open_mode)) == NULL )
        {
            fp = stdout;
        }
    }
    else
    {
        fp = stdout;
    }

    if ( type != X25_LOG )
    {
        fprintf(fp, "%02d:%02d:%02d %5d ",
                tsave.tm_hour, tsave.tm_min, tsave.tm_sec, getpid());
    }
    else
    {
        fprintf(fp, "%02d:%02d:%02d ",
                tsave.tm_hour, tsave.tm_min, tsave.tm_sec);
    }


    (void) vfprintf(fp, fmt, dargs);
    if ( fp != stdout )
    {
        (void) fclose(fp);
    }

    va_end(dargs);
}
