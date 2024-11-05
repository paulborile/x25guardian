/*
 * $Id: dscope.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $
 *
 *	$Log: dscope.c,v $
 *	Revision 1.1.1.1  1998/11/18 15:03:23  paul
 *	Guardian : x25 Pos router
 *
 * Revision 1.2  1995/07/14  09:04:17  px25
 * Debugging printouts take away.
 *
 * Revision 1.1  1995/07/12  14:26:39  px25
 * Clock is long instead of int
 *
 * Revision 1.0  1995/07/07  09:53:56  px25
 * Initial revision
 *
 *
 */

static char rcsid[] = "$Id: dscope.c,v 1.1.1.1 1998/11/18 15:03:23 paul Exp $";

#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include        <sys/times.h>
#include        <sys/stat.h>
#include        <string.h>

#include        "px25_globals.h"
#include        "errlog.h"


/* Use this one if char less than 0x20	*/

char *ascii_tab[] = { "nu", "sh", "sx", "ex", "et", "eq", "ak", "bl",
                      "bs", "ht", "nl", "vt", "ff", "cr", "so", "si",
                      "dl", "d1", "d2", "d3", "d4", "nk", "sy", "eb",
                      "ca", "em", "su", "es", "fs", "gs", "rs", "us",
                      "sp" };

/***
 | 00 nul| 01 soh| 02 stx| 03 etx| 04 eot| 05 enq| 06 ack| 07 bel|
 | 08 bs | 09 ht | 0a nl | 0b vt | 0c np | 0d cr | 0e so | 0f si |
 | 10 dle| 11 dc1| 12 dc2| 13 dc3| 14 dc4| 15 nak| 16 syn| 17 etb|
 | 18 can| 19 em | 1a sub| 1b esc| 1c fs | 1d gs | 1e rs | 1f us |
 | 20 sp | 21  ! | 22  " | 23  # | 24  $ | 25  % | 26  & | 27  ' |
 | 28  ( | 29  ) | 2a  * | 2b  + | 2c  , | 2d  - | 2e  . | 2f  / |
 | 30  0 | 31  1 | 32  2 | 33  3 | 34  4 | 35  5 | 36  6 | 37  7 |
 | 38  8 | 39  9 | 3a  : | 3b  ; | 3c  < | 3d  = | 3e  > | 3f  ? |
 | 40  @ | 41  A | 42  B | 43  C | 44  D | 45  E | 46  F | 47  G |
 | 48  H | 49  I | 4a  J | 4b  K | 4c  L | 4d  M | 4e  N | 4f  O |
 | 50  P | 51  Q | 52  R | 53  S | 54  T | 55  U | 56  V | 57  W |
 | 58  X | 59  Y | 5a  Z | 5b  [ | 5c  \ | 5d  ] | 5e  ^ | 5f  _ |
 | 60  ` | 61  a | 62  b | 63  c | 64  d | 65  e | 66  f | 67  g |
 | 68  h | 69  i | 6a  j | 6b  k | 6c  l | 6d  m | 6e  n | 6f  o |
 | 70  p | 71  q | 72  r | 73  s | 74  t | 75  u | 76  v | 77  w |
 | 78  x | 79  y | 7a  z | 7b  { | 7c  | | 7d  } | 7e  ~ | 7f del|
 */

static FILE *dscope_open();
static int dscope_write();

#define DSCOPE_LINE_SIZE    16

static char iline_buffer[8*BUFSIZ];
static int ibuffer_count = 0;
static int iline_count = 0;

static char oline_buffer[8*BUFSIZ];
static int obuffer_count = 0;
static int oline_count = 0;

/*
 *
 * Procedure: dscope_log
 *
 * Parameters: name of device logging refers to, buffer to log,
 *					len of buffer, mode ( 'i' for input, 'o' for output)
 *
 * Description: simulate a datascope for data contained in buffer
 *
 * Return: -1 if unable to write to dscope file
 *
 */


int dscope_log(device, buf, len, mode)
char *device;
unsigned char *buf;
int len;
char mode;
{
    int rc;
    int *buffer_count;
    char *line_buffer;
    int buf_count = 0;

    if ( mode == 'i' )
    {
        buffer_count = &ibuffer_count;
        line_buffer = iline_buffer;
    }
    else
    {
        buffer_count = &obuffer_count;
        line_buffer = oline_buffer;
    }

    while ((len + *buffer_count ) >= DSCOPE_LINE_SIZE )
    {
        /* copy buffer up to DSCOPE_LINE_SIZE and print	*/

        /* printf("copying and wr line_buf[%d], buf + %d, size %d\n", */
        /*	*buffer_count, buf_count, DSCOPE_LINE_SIZE - *buffer_count);*/

        memcpy(&line_buffer[*buffer_count], buf + buf_count,
               DSCOPE_LINE_SIZE - *buffer_count);
        len -= DSCOPE_LINE_SIZE - *buffer_count;
        buf_count += (DSCOPE_LINE_SIZE - *buffer_count);
        *buffer_count += (DSCOPE_LINE_SIZE - *buffer_count);
        if (( rc = dscope_write(device, line_buffer, mode)) == -1 )
        {
            return(rc);
        }
        *buffer_count = 0;
    }

    if (( len + *buffer_count ) < DSCOPE_LINE_SIZE )
    {
        /* printf("Only copying line_buf[%d], buf + %d, size %d\n",  */
        /*			buffer_count, buf_count, len);    */
        memcpy(&line_buffer[*buffer_count], buf + buf_count, len);
        *buffer_count += len;
        rc = 0;
    }
    return(rc);
}

/*
 *
 * Procedure: dscope_mark
 *
 * Parameters: device name, mark string, mode ('i' or 'o')
 *
 * Description: append a user string in the dscope file and
 *					 previously not written data.
 *
 * Return: none
 *
 */


int dscope_mark(device, str, mode)
char *device;
char *str;
{
    FILE *fp;
    struct  tm *t;
    char *line_buffer;
    int *buffer_count;
    int *line_count;
    long clock;


    time(&clock);
    t = localtime(&clock);

    if ( mode == 'i' )
    {
        buffer_count = &ibuffer_count;
        line_buffer = iline_buffer;
        line_count = &iline_count;
    }
    else
    {
        buffer_count = &obuffer_count;
        line_buffer = oline_buffer;
        line_count = &oline_count;
    }

    dscope_write(device, line_buffer, mode);

    *buffer_count = 0;

    if (( fp = dscope_open(device, mode)) == NULL )
    {
        return(-1);
    }

    *line_count = 0;

    fprintf(fp, "**** %02d:%02d:%02d - %02d/%02d/%04d : %s\n",
            t->tm_hour, t->tm_min, t->tm_sec,
            t->tm_mday, t->tm_mon+1, t->tm_year + 1900,
            str);

    fclose(fp);
    return(0);
}

/*
 *
 * Procedure: dscope_write
 *
 * Parameters: device, buffer to write, mode
 *
 * Description: really flush buffer to file
 *
 * Return: -1 if unable to open dcsope file, 0 if OK
 *
 */

static int dscope_write(device, buf, mode)
char *device;
unsigned char *buf;
char mode;
{
    int *buffer_count;
    int *line_count;
    int i;
    FILE *fp;

    if (( fp = dscope_open(device, mode)) == NULL )
    {
        return(-1);
    }

    if ( mode == 'i' )
    {
        buffer_count = &ibuffer_count;
        line_count = &iline_count;
    }
    else
    {
        buffer_count = &obuffer_count;
        line_count = &oline_count;
    }

    if ( *buffer_count != 0 )
    {
        fprintf(fp, "%04x ", *line_count);
    }

    for (i=0; i<*buffer_count; i++)
    {
        (*line_count)++;
        if ( buf[i] <= 0x20 )
        {
            fprintf(fp, "%2.2s ", ascii_tab[buf[i]]);
        }
        else if ( buf[i] < 0x7F )
        {
            fprintf(fp, " %c ", buf[i]);
        }
        else
        {
            fprintf(fp, "%02X ", buf[i]);
        }
    }

    fprintf(fp, "\n");
    fclose(fp);
    return(0);
}

/*
 *
 * Procedure: dscope_open
 *
 * Parameters: device, mode
 *
 * Description: open datascope file (internal function)
 *
 * Return: FILE pointer retunred by fopen
 *
 */

static FILE *dscope_open(dev, mode)
char *dev;
char mode;
{
    FILE *fp;
    char device_name[BUFSIZ];
    char *finger;

    if (( finger = getenv(PX25_DSCOPE_DIR)) == NULL )
    {
        strcpy(device_name, "/tmp");
    }
    else
    {
        strcpy(device_name, finger);
    }

    strcat(device_name, "/");

    if (( finger = strrchr(dev, '/')) != NULL )
    {
        strcat(device_name, finger +1);
    }
    else
    {
        strcat(device_name, dev);
    }

    if ( mode == 'i' )
    {
        strcat(device_name, "-i");
    }
    else
    {
        strcat(device_name, "-o");
    }

    if (( fp = fopen(device_name, "a")) == NULL )
    {
        errlog(INT_LOG, "dscope_log : cannot open %s\n", device_name);
        return(NULL);
    }
    return(fp);
}
