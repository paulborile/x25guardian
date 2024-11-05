#include        <stdio.h>

main()
{
    int i;
    int len;
    char dummy[BUFSIZ];
    unsigned char blk[BUFSIZ];

    printf("Enter Ascii User Data to Match :\n");
    gets(dummy);

    len = ea_to_bin(blk, dummy);

    for (i=0; i<len; i++)
    {
        printf("%02x\t", blk[i]);
    }

    printf("\n len is %d\n", len);
    exit(0);
}
