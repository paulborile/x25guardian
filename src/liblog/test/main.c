#include    <stdio.h>


char parfile[BUFSIZ];
char parname[BUFSIZ];
int parvalue;

main()
{
    int rc;
    unsigned int a;
    int i = 0;

    while (1)
    {
        printf("Parameter file : ");
        fflush(stdout);
        gets(parfile);

        printf("Parameter name : ");
        fflush(stdout);
        gets(parname);
        a = get_param(parfile, parname);
        printf("%s = %ud %x\n", parname, (unsigned int)    a, (unsigned int)    a);
    }
}
