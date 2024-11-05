#include		<stdio.h>
#include		<errno.h>
#include		<signal.h>

void	hand();
char	buf[BUFSIZ];

main()
{
	int	rc;
	struct	sigaction	act, oact;

	act.sa_handler	= hand;
	act.sa_flags	= SA_RESTART;
	sigemptyset(&act.sa_mask);

	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);


	while (1)
	{
		printf("reading\n");
		rc = read(0, buf, BUFSIZ);
		if (( rc == -1 ) && ( errno == EINTR))
		{
			printf("Read interrupted by signal\n");
		}
	}
}


void	hand(sig)
int	sig;
{
	printf("Signal %d received\n", sig);
	return;
}

/*******

struct sigaction {
	int sa_flags;		/* flags indicating desired action
	void (*sa_handler)();	/* address of signal handling routine
	sigset_t sa_mask;	/* signal mask for this action
	int sa_resv[2];		/* reserved
};

#define SA_ONSTACK	0x00000001
#define SA_RESETHAND	0x00000002
#define SA_RESTART	0x00000004
#define SA_SIGINFO	0x00000008
#define SA_NODEFER	0x00000010
************/
