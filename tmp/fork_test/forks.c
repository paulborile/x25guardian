#include		<stdio.h>

long	dummy[100000];
long	status;

main(argc, argv)
int	argc;
char	**argv;
{
	int	start, end;
	int	tot, count = 0;
	double	tot_time;

	if ( argc != 2 )
	{
		printf("Usage : %s <count>\n", argv[0]);
		exit(1);
	}	

	tot = count = atoi(argv[1]);

	time(&start);

	while (count--)
	{
		switch (fork())
		{
			case -1	:
				perror("fork");
				exit(1);
				break;
			case	0	:
				/* the child	*/

				execlp("getpid", "getpid", NULL);

				/* if reached means no exec possible	*/
				printf("Exec failed\n");
				exit(1);
				break;
			default	:
				/* the father	*/
				wait(&status);
				break;
		}
	}							

	time(&end);

	printf("Elapsed time %d secs\n", end - start);

	tot_time = (double) (end - start);

	printf("Fork/s %f\n", (double) (tot_time / (double) tot));
}
