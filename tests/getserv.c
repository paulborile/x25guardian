#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#define DEBUG
#define OK '0'
#define NOK '1'
#define SERVER_PORT 2100
#define NMAX 2
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
extern int errno;
struct protoent *getprotobyname();
struct hostent *gethostbyname();
int server_id,new_sock;
struct protoent *pp;
struct hostent *server;
struct sockaddr_in server_sock;
struct in_addr server_addr;
int mask,maskold;
int nfigli = 0;
int flag;              /* flag=1 il segnale ricevuto durante pause e` SIGCLD */

main(argc,argv)
int argc;
char* argv[];
{
   char hostname[40];
   char command[BUFSIZ],data[BUFSIZ],c[2];
   int lenght,nbytes,pid,fd,n_err,ret;
	int handler();
	
	signal(SIGCLD,handler);
/*
		preleva nome della macchina su cui gira il server
*/
   if (gethostname(hostname,40) != 0) {
      fprintf(stderr,"server: errore gethostname()\n");
      exit(1);
   }
/*
      scopre indirizzo internet della macchina
*/
   if ((server = gethostbyname(hostname)) == 0) {
      fprintf(stderr,"server:gethostbyname() locale sconosciuto\n");
      exit(1);
   }
   bcopy(server->h_addr, (char*)&server_addr,server->h_length);
/*
      ritrova il tipo di protocollo
*/
   if ((pp = getprotobyname("tcp")) == 0) {
      fprintf(stderr,"server: errore getprotobyname()\n");
      exit(1);
   }
/*
      crea il socket
*/
   if ((server_id = socket(AF_INET,SOCK_STREAM,0)) == -1) {
      perror("server: socket()\n");
      exit(1);
   }
/*
      faccio la bind con:
               indirizzo = server_addr
               port = SERVER_PORT
*/
   server_sock.sin_family = AF_INET;
   server_sock.sin_addr = server_addr;
   server_sock.sin_port = htons(SERVER_PORT) ;
   lenght = sizeof(server_sock);
   if (bind(server_id,&server_sock,sizeof(server_sock)) == -1) {
      perror("server: bind()\n");
		close(server_id);
      exit(1);
   }
   if(listen(server_id,4) == -1)
   {
      perror("server: errore listen() ");
      exit(1);
   }
	while(1)
	{
		while (nfigli >= NMAX) {
#ifdef DEBUG
			printf("numero serventi saturo = %d \n", nfigli);
#endif
			flag = 0;
			while (!flag)            /* va avanti solo se il segnale e' SIGCLD */
				pause();
		}
		printf("server in attesa di richieste di connessione \n");
		while (((new_sock = accept(server_id,0,0) == -1) && (errno == EINTR)))
		{
		}
		mask = sigmask(SIGCLD);
		maskold = sigblock(mask);          /* disabilitazione SIGCLD */
		if (new_sock == -1) {
			perror("server: errore accept()");
			exit(1);
		}
		printf("dopo la disabilitazione maskold = %x \n", maskold);
		if ((pid = fork()) == -1) {
			perror("server: errore  fork()");
			exit(1);
		}
		nfigli++;
		if (pid == 0) {
			if ((nbytes = recv(new_sock,command,sizeof(command),0)) < 0) {
				perror("server figlio: errore recv()");
				exit(1);
			}
			if ((fd = open(command,O_RDONLY) == -1)) {
				/* invia messaggio di errore al client !!!! */
				data[0]=NOK;
				sprintf(&data[1],"%d",errno);
				send(new_sock,data,sizeof(int) +1,0);
				perror("server figlio: errore open()");
				exit(1);
			}
			data[0]  = OK;		/* file esistente ed accessibile */
			if (send(new_sock,data,1,0) < 0) {
				perror("server figlio: send()");
				exit(1);
			}
			while ((nbytes = read(fd,data,BUFSIZ)) != 0) {
				if (nbytes == -1) {
					perror("server figlio: errore read()");
					exit(1);
				}
				if (send(new_sock,data,nbytes,0) < 0) {
					perror("server figlio: errore send()");
					exit(1);
				}
			}
			close(server_id);
			close(new_sock);
			close(fd);
			printf("server figlio: terminazione \n");
			exit(0);
		}
			close(new_sock);
			sigsetmask(maskold);       /* riabilitazione SIGCLD */
	}
}

handler(sig)	
int sig;
{
	int pid,status;
	printf("ricevuto segnale %d \n",sig);
	while ((pid = wait3(&status, WNOHANG, 0)) > 0) {
		printf("terminato processo %d \n",pid);
		nfigli--;
	}
	flag = 1;           /* indica che il segnale ricevuto e` SIGCLD */
	return;
}
