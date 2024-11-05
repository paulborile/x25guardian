#include	<stdio.h>

int		sock;


main(argc, argv)
int		argc;
char	**argv;
{
	char	c;

	if ( argc != 2 )
	{
		printf("Usage: %s socket#\n", argv[0]);
		exit();
	}

	printf("argv[0] %s, argv[1] %s\n", argv[0], argv[1]);

	sock = atoi(argv[1]);

	while (read(sock, &c, 1) == 1)
	{
		write(1, &c, 1);
	}

	printf("Closing ..\n");
	close(sock);
}

