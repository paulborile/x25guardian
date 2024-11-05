/*
 *
 *	Procedure: get_command_line
 *
 *	Parameters: argc, argv
 *
 *	Description: parse command line options
 *
 *	Return:
 *
 */


void	get_command_line(argc, argv)
int		argc;
char	**argv;
{
	int		fatal = 0;
	int		c;
	extern char *optarg;

	debug((3, "get_command_line() - Starting.\n"));

#define		OPTS		"M:d:r:"

	while ((c = getopt(argc, argv, OPTS)) !=EOF)
	{
		switch(c)
		{

			case 'M':
				mymachine	=	atoi(optarg);
				break;

			case 'd':
				debuglevel	=	atoi(optarg);
				break;

			case 'r':
				cidin_request_interval = atoi(optarg);
				break;

			default:
				printf("%s : invalid flag.\n", argv[0]);
				usage(argc, argv);
				exit(FAILURE);
		}	
	}

	if ((cidin_request_interval < 2) || (cidin_request_interval > 58))
	{
		printf("%s : Invalid cidin_request_interval(%d). Using %d\n",
							argv[0], cidin_request_interval,
							CIDIN_REQUEST_INTERVAL);
		cidin_request_interval = CIDIN_REQUEST_INTERVAL;
	}

	if ( mymachine == -1	)  fatal++;

#ifdef DEBUG
	if ( debuglevel == -1 ) fatal++;
#endif

	if ( fatal )
	{
		usage(argc, argv);
		exit(FAILURE);
	}
}

