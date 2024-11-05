#include		<stdio.h>
#include		<fcntl.h>

main(argc, argv)
int	argc;
char	**argv;
{
	int	fd;

	printf("Open device %s\n", argv[1]);

	if (( fd = open(argv[1], O_RDWR)) < 0 )
	{
		perror(argv[1]);
		exit(1);
	}

	printf("Press enter to exit\n");
	getchar();
}
