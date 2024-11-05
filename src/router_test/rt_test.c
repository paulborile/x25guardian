#include    <stdio.h>

#include    "px25_globals.h"
#include    "rt.h"

struct  NUA *pippo, *rt_find();
extern int rt_errno;

main()
{
    char buf[BUFSIZ];

again:
    printf("Call user data: ");
    fflush(stdout);

    gets(buf);

    pippo = rt_find(buf);

    if ( pippo != NULL )
    {
        printf("nua1 %s, nua2 %s, type(%d) %s\n",
               pippo->primary_nua, pippo->secondary_nua, pippo->nua_type,
               (pippo->nua_type == ASY_NUA) ? "ASY_NUA" : "X25_NUA");
    }
    else
    {
        printf("rt_find() returned NULL, rt_errno = %d\n", rt_errno);
    }

    goto again;
}
