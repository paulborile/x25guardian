#include    <stdio.h>


#include    "errlog.h"

char buf[BUFSIZ];

main()
{
    int c;
    unsigned char i = 0;

    srand(getpid());

    for (c = 0; c < BUFSIZ; c++)
    {
        buf[c] = i;
        i++;
    }

    dscope_mark("input", "Starting log", 'i');
    dscope_mark("output", "Starting log", 'o');

    dscope_log("input", buf, BUFSIZ, 'i');
    dscope_log("output", buf, BUFSIZ, 'o');
    dscope_log("input", &buf[20], 10, 'i');
    dscope_log("output", &buf[30], 10, 'o');

    dscope_mark("input", "Terminating log", 'i');
    dscope_mark("output", "Terminating log", 'o');
}
