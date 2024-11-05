#include    <stdio.h>

int sock;

#define MAXMSG  (1024*64)

struct  GROG
{
    int size;
    char s[MAXMSG];     /* character string used to send info	*/
};


main(argc, argv)
int argc;
char **argv;
{
    char buf[MAXMSG];
    struct  GROG *str = (struct GROG *) buf;
    int sz;
    int rc;

    if ( argc != 3 )
    {
        printf("Usage: %s socket# <tty>\n", argv[0]);
        exit();
    }

    printf("argv[0] %s, argv[1] %s\n", argv[0], argv[1]);

    sock = atoi(argv[1]);

    while ((rc = mos_recv(sock, buf, MAXMSG)) > 0)
    {
        sz = ntohl(str->size);
        if ( sz != rc )
        {
            printf("rc = %d , sz = %d !!!!\n", rc, sz);
        }
        else
        {
            putchar('.');
            fflush(stdout);
        }
    }

    printf("Closing ..\n");
    close(sock);
}

