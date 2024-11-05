#define DEBUG
#define OK '0'
#define NOK '1'
#define SERVER_PORT 2100
#include <stdio.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

extern int errno;
extern char* sys_errlist[];
struct protoent *getprotobyname();
struct hostent *gethostbyname();
int client_id;
struct protoent *pp;
struct hostent *server;
struct sockaddr_in client_sock,server_sock;
struct in_addr client_addr,server_addr;
main(argc,argv)
int argc;
char* argv[];
{
   char hostname[40];
   char command[BUFSIZ],data[BUFSIZ];
   int lenght,nbytes,port,n_err;
	
	if (argc < 3) {
		fprintf(stderr, "uso: getf hostname file \n");
		exit(1);
	}
/*
	if ((pp = getprotobyname("tcp")) == 0) {
		fprintf(stderr, "client: errore getprotobyname() \n");
		exit(1);
	}
*/

/*
		crea il socket
*/
	if ((client_id = socket(AF_INET,SOCK_STREAM,0)) == -1) {
   	perror("client: socket()\n");
      exit(1);
   } 
	strcpy(hostname,argv[1]);
/*
		ritrova l'indirizzo internet della macchina remota tramite il
		nome ricevuto in input
*/
	if ((server=gethostbyname(hostname)) == 0) {
		fprintf(stderr,"client: errore gethostbyname() \n");
		close(client_id);
		exit(1);
	}
	bcopy(server->h_addr,(char*)&server_addr,server->h_length);
	server_sock.sin_family = AF_INET;
   server_sock.sin_addr = server_addr;
	server_sock.sin_port = htons(SERVER_PORT);

/*
		richiesta di connessione
*/
	if (connect(client_id,&server_sock,sizeof(server_sock)) == -1) {
		perror("client:errore connect() ");
		exit(1);
	}
/*
		invia richiesta
*/
	strcpy(command,argv[2]);
	if (send(client_id,command,strlen(command)+1,0) < 0) {
		perror("client: errore send() ");
		exit(1);
	}
/*
		ricevo replica dal server
*/
	if ((nbytes = recv(client_id,data,sizeof(data),0)) < 0) {
		perror("client: errore recv()");
		exit(1);
	}
	if (data[0] == NOK)
	{
		sscanf(&data[1],"%d",&n_err);
		printf("client error: %s\n",sys_errlist[n_err]);
		exit(1);
	}

	while ((nbytes = recv(client_id,data,sizeof(data),0)) != 0) {
		if (nbytes == -1) {
			perror("client: recv() ");
			exit(1);
		}
		write(1,data,nbytes);
	}
	close(client_id);
}

	


