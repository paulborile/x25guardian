#include    <stdio.h>
#include    <errno.h>
#include    <fcntl.h>
#include    <signal.h>
#include    <sys/types.h>
#include    <sys/time.h>
#include    <termio.h>

main()
{
	int ptym, ptys;
	char     ptydev[32];

	ptym = open ("/dev/ptmx", O_RDWR);

	grantpt(ptym);

	unlockpt(ptym);

	strcpy(ptydev, ptsname(ptym));
   printf("ptyslave is %s\n", ptydev);
	
	ptys = open (ptydev, O_RDWR);

	sleep(60);

	close(ptym);
	close(ptys);
}
