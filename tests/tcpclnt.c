#define DEBUG
#include <stdio.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
extern int errno;
struct protoent *getprotobyname();
struct hostent *gethostbyname();
int client_id,client_port;
struct protoent *pp;
struct hostent *server;
struct sockaddr_in client_sock,server_sock;
struct in_addr server_addr;
main(argc,argv)
int argc;
char* argv[];
{
   char hostname[40];
   char command[BUFSIZ],data[BUFSIZ];
   int lenght,nbytes,port;
	
	if (argc < 3) {
		fprintf(stderr, "uso: %s hostname port \n",argv[0]);
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
	sscanf(argv[2],"%d",&port);
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
   server_sock.sin_port = htons(port);
#ifdef DEBUG
	printf("client: connessione su host definito da:\n");
	printf("		macchina: %s \n",hostname);
	printf("		numero di porta %d \n",ntohs(port));
	printf("		indirizzo %s \n",inet_ntoa(server_sock.sin_addr));
#endif

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
	strcpy(command,"richiesta di servizio da client");
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
	printf("client: ricevuto \"%s\" \n",data);
	close(client_id);
}

	


