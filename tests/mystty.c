	register i;

	if(ioctl(0, TCGETA, &cb) == -1) {
		perror(STTY);
		exit(2);
	}
	else {
		cb.c_cc[7] = (unsigned)stio.tab;
		cb.c_lflag = stio.lmode;
		cb.c_oflag = stio.omode;
		cb.c_iflag = stio.imode;
	}

	while(--argc > 0) {		/* set terminal modes for supplied options */

		arg = *++argv;
		match = 0;
		if (term == ASYNC) {
			if (eq("erase") && --argc)
				cb.c_cc[VERASE] = gct(*++argv);
			else if (eq("intr") && --argc)
				cb.c_cc[VINTR] = gct(*++argv);
			else if (eq("quit") && --argc)
				cb.c_cc[VQUIT] = gct(*++argv);
			else if (eq("eof") && --argc)
				cb.c_cc[VEOF] = gct(*++argv);
			else if (eq("min") && --argc)
				cb.c_cc[VMIN] = gct(*++argv);
			else if (eq("eol") && --argc)
				cb.c_cc[VEOL] = gct(*++argv);
			else if (eq("time") && --argc)
				cb.c_cc[VTIME] = gct(*++argv);
			else if (eq("kill") && --argc)
				cb.c_cc[VKILL] = gct(*++argv);
			else if (eq("swtch") && --argc)
				cb.c_cc[VSWTCH] = gct(*++argv);
			else if (eq("ek")) {
				cb.c_cc[VERASE] = CERASE;
				cb.c_cc[VKILL] = CKILL;
			}
			else if (eq("line") && --argc)
				cb.c_line = atoi(*++argv);
			else if (eq("raw")) {
				cb.c_cc[VMIN] = 1;
				cb.c_cc[VTIME] = 1;
			}
			else if (eq("-raw") | eq("cooked")) {
				cb.c_cc[VEOF] = CEOF;
				cb.c_cc[VEOL] = CNUL;
			}
			else if(eq("sane")) {
				cb.c_cc[VERASE] = CERASE;
				cb.c_cc[VKILL] = CKILL;
				cb.c_cc[VQUIT] = CQUIT;
				cb.c_cc[VINTR] = CINTR;
				cb.c_cc[VEOF] = CEOF;
				cb.c_cc[VEOL] = CNUL;
							   /* SWTCH purposely not set */
			}
			for(i=0; speeds[i].string; i++)
				if(eq(speeds[i].string)) {
					cb.c_cflag &= ~CBAUD;
					cb.c_cflag |= speeds[i].speed&CBAUD;
				}
		}
		if (term == SYNC && eq("ctab") && --argc)
			cb.c_cc[7] = gct(*++argv);
		for(i=0; imodes[i].string; i++)
			if(eq(imodes[i].string)) {
				cb.c_iflag &= ~imodes[i].reset;
				cb.c_iflag |= imodes[i].set;
			}
		for(i=0; omodes[i].string; i++)
			if(eq(omodes[i].string)) {
				cb.c_oflag &= ~omodes[i].reset;
				cb.c_oflag |= omodes[i].set;
			}
		if(term == SYNC && eq("sane"))
			cb.c_oflag |= TAB3;
		for(i=0; cmodes[i].string; i++)
			if(eq(cmodes[i].string)) {
				cb.c_cflag &= ~cmodes[i].reset;
				cb.c_cflag |= cmodes[i].set;
			}
		for(i=0; lmodes[i].string; i++)
			if(eq(lmodes[i].string)) {
				cb.c_lflag &= ~lmodes[i].reset;
				cb.c_lflag |= lmodes[i].set;
			}
		if(!match)
			if(!encode()) {
				fprintf(stderr, "unknown mode: %s\n", arg);
				exit(2);
			}
	}
	if (term == ASYNC) {
		if(ioctl(0, TCSETAW, &cb) == -1) {
			perror(STTY);
			exit(2);
		}
	} else {
		stio.imode = cb.c_iflag;
		stio.omode = cb.c_oflag;
		stio.lmode = cb.c_lflag;
		stio.tab = cb.c_cc[7];
		if (ioctl(0, STSET, &stio) == -1) {
			perror(STTY);
			exit(2);
		}
	}
	exit(0);	/*NOTREACHED*/
}

eq(string)
char *string;
{
	register i;

	if(!arg)
		return(0);
	i = 0;
loop:
	if(arg[i] != string[i])
		return(0);
	if(arg[i++] != '\0')
		goto loop;
	match++;
	return(1);
}

				/* get pseudo control characters from terminal */
gct(cp)				/* and convert to internal representation      */
register char *cp;
{
	register c;

	c = *cp++;
	if (c == '^') {
		c = *cp;
		if (c == '?')
			c = CINTR;		/* map '^?' to DEL */
		else if (c == '-')
			c = 0377;		/* map '^-' to 0377, i.e. undefined */
		else
			c &= 037;
	}
	return(c);
}

delay(m, s)
char *s;
{
	if(m)
		(void) printf("%s%d ", s, m);
}

long	speed[] = {
	0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400
};

prspeed(c, s)
char *c;
int s;
{

	(void) printf("%s%d baud; ", c, speed[s]);
}

					/* print current settings for use with  */
prencode()				/* another stty cmd, used for -g option */
{
	(void) printf("%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
	cb.c_iflag,cb.c_oflag,cb.c_cflag,cb.c_lflag,cb.c_cc[0],
	cb.c_cc[1],cb.c_cc[2],cb.c_cc[3],cb.c_cc[4],cb.c_cc[5],
	cb.c_cc[6],cb.c_cc[7]);
}

encode()
{
	int grab[12], i;
	i = sscanf(arg, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
	&grab[0],&grab[1],&grab[2],&grab[3],&grab[4],&grab[5],&grab[6],
	&grab[7],&grab[8],&grab[9],&grab[10],&grab[11]);

	if(i != 12) return(0);

	cb.c_iflag = (ushort) grab[0];
	cb.c_oflag = (ushort) grab[1];
	cb.c_cflag = (ushort) grab[2];
	cb.c_lflag = (ushort) grab[3];

	for(i=0; i<8; i++)
		cb.c_cc[i] = (char) grab[i+4];
	return(1);
}

