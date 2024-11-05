#include    <stdio.h>
#include    "px25_globals.h"
#include    "group.h"


struct  GROUP *g;

extern int sub_rt_errno;


main()
{
    char buf[BUFSIZ];

again:

    printf("sub-nua : ");
    fflush(stdout);
    gets(buf);

    g = sub_rt_find(buf);

    if ( g != NULL )
    {
        printf("Group name %s, group num %d\n", g->grp_name, g->grp_num);
    }
    else
    {
        printf("Not found, sub_rt_errno %d\n", sub_rt_errno);
    }
    goto again;
}
