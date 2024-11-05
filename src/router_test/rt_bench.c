#include    <stdio.h>

#include    "px25_globals.h"
#include    "rt.h"

struct  NUA *pippo, *rt_find();
extern int rt_errno;
char buf[BUFSIZ];
int start, end;
double tot_time;

main(argc, argv)
int argc;
char **argv;
{
    int tot, count = 0;
    int len;

    if ( argc != 2 )
    {
        printf("Usage : %s <count>\n");
        exit(1);
    }

    tot = count = atoi(argv[1]);
    strcpy(buf, "lkajsdlkjalkdfjlakdjfladk");
    len = strlen(buf);

    time(&start);
    while (count--)
    {
        pippo = rt_find(buf, len);
    }
    time(&end);

    printf("Elapsed time %d\n", end-start);
    tot_time = (double) (end-start);
    printf("rt_find/s %f\n", (double) (tot_time/(double)tot));
}
