#include    <stdio.h>
#include    <signal.h>

void    del();

main()
{
    int i;

    signal(SIGINT, del);

    while (1) i++;
}

void    del()
{
    signal(SIGINT, del);
    printf("SIGINT\n");
}
