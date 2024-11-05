/*
 * $Id: debug.c,v 1.1.1.1 1998/11/18 15:03:09 paul Exp $
 *
 *	$Log: debug.c,v $
 *	Revision 1.1.1.1  1998/11/18 15:03:09  paul
 *	Guardian : x25 Pos router
 *
 * Revision 1.1  1995/07/07  09:50:41  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: debug.c,v 1.1.1.1 1998/11/18 15:03:09 paul Exp $";

# include <sys/types.h>
# include <sys/times.h>
# include <fcntl.h>
# include <stdio.h>
# include <varargs.h>
# include <debug.h>

static int _d_level = 0;
static char *_d_mid = NULL, *_d_uid = NULL;
static FILE *_d_fp;
static struct  tms tmb;
static long origin;

initdebug(level, file, mid, uid)
char *file, *mid, *uid;
{
    _d_level = level;
    _d_mid = mid;
    _d_uid = uid;
    if ( file == NULL )
    {
        _d_fp = stderr;
    }
    else
    {
        int dfd;

        if ( (dfd = open(file, O_WRONLY|O_CREAT|O_TRUNC|O_APPEND|O_NDELAY,
                         0666)) == -1 )
        {
            return(0);
        }
        _d_fp = fdopen(dfd, "a");
# ifdef UNBUF
        setbuf(_d_fp, NULL);
# endif
    }

    origin = times(&tmb);
    return(1);
}

void    enddebug()
{
    if ( _d_fp != stderr )
    {
        fclose(_d_fp);
    }
}

/*VARARGS0*/
void    _debug(va_alist)
va_dcl
{
    char *fmt;
    int r_level;
    va_list dargs;

    va_start(dargs);
    r_level = va_arg(dargs, int);
    fmt = va_arg(dargs, char *);

    if ( (r_level & ~D_FLAGS) > _d_level )
    {
        va_end(dargs);
        return;
    }

    if ( _d_fp == NULL )
    {
        va_end(dargs);
        return;
    }

    if ( !(r_level&D_SIL) )
    {
        long t, time();
        char *ctime();

        if ( r_level & D_DATE )
        {
            t = time((long *)0);
            (void)fprintf(_d_fp, "%.24s - ", ctime(&t));
        }

        if ( r_level & D_TIME )
        {
            (void)fprintf(_d_fp, "%ld - ", times(&tmb) - origin);
        }

        (void) fprintf(_d_fp, "%s:%s: ", _d_mid, _d_uid);
    }
    (void) vfprintf(_d_fp, fmt, dargs);

    (void)fflush(_d_fp);
    va_end(dargs);
}
