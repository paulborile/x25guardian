#include    <stdio.h>

#define PX25_GEN_TABLE  "PX25_GEN_TABLE"
#define CALL_TIMEOUT    "output_call_timeout"
#define CLR_TIMEOUT     "hangup_timeout"

extern char *getenv();
extern int gp_errno;

main(argc, argv)
int argc;
char *argv[];
{
    char *genfile;
    int call_timeout, clr_timeout;

    if (( genfile = getenv(PX25_GEN_TABLE)) == NULL )
    {
        printf("%s: defaulting PX25_GEN_TABLE to %s\n",
               argv[0], "/usr3/px25/conf/gen.tab");
        genfile = "/usr3/px25/conf/gen.tab";
    }

    if ( (call_timeout = get_param( genfile, CALL_TIMEOUT )) < 0 )
    {
        printf("%s : get_param() gp_errno %d\n", argv[0], gp_errno);
        printf("%s : defaulting CALL_TIMEOUT to %d\n",
               argv[0], 20);
    }
    printf ("%s: call_timeout = %d\n", argv[0], call_timeout);

    if ( (clr_timeout = get_param( genfile, CLR_TIMEOUT )) < 0 )
    {
        printf("%s : get_param() gp_errno %d\n", argv[0], gp_errno);
        printf("%s : defaulting CLR_TIMEOUT to %d\n",
               argv[0], 10);
    }
    printf("%s : clr_timeout %d\n", argv[0], clr_timeout);
}
