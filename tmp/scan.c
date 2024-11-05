#include	<stdio.h>


char	buf[BUFSIZ];
FILE	*fp;


main(argc, argv)
int	argc;
char	**argv;
{
	char	str1[20], str2[20], str3[20], str4[20];

	if (( fp = fopen(argv[1], "r")) == NULL )
	{
		perror(argv[1]);
		exit();
	}

	while ( fgets(buf, BUFSIZ, fp) != NULL )
	{
		if (( buf[0] == '#' ) || ( buf[0] == '\n' )) continue;

		sscanf(buf, "%s %s,%s\n", str1, str2, str3);

		printf(">%s< >%s< >%s<\n", str1, str2, str3);
	}

	fclose(fp);
}
