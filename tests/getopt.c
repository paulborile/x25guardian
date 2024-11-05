From paul Tue Apr  4 17:30:59 1995
Received: by scubi.cusun.sublink.org id <4888>; Tue, 4 Apr 1995 17:30:46 +0200
Subject: getopt
From:	Paul Stephen Borile <paul>
To:	giorgio (Giorgio Priveato)
Date:	Tue, 4 Apr 1995 18:30:37 +0200
X-Mailer: ELM [version 2.3 PL0]
Message-Id: <95Apr4.173046met_dst.4888@scubi.cusun.sublink.org>
Status: OR




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


-- 
--------------------------------------------------------------------------
Paul Stephen Borile -

