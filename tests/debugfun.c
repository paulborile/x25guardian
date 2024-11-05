#include	<stdio.h>
#include	"debug.h" 	/* questo e' nella directory include di progetto */


main(argc, argv)
int		argc;
char	**argv;
{
	char	pid_str[20];
	char	debug_file[BUFSIZ];
	int		debuglevel; /* inizializzata da riga di comando	*/

	sprintf(pid_str, "%d", getpid());
	sprintf(debug_file, "/tmp/spvc%05d.debug", getpid());

	initdebug(debuglevel, debug_file, argv[0], pid_str);


	debug((1, "main() - starting\n"));
	debug((1, "main() - terminating\n"));
   enddebug();
}

