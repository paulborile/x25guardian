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
int server_id,s1_id;
struct protoent *pp;
struct hostent *server;
struct sockaddr_in server_sock;
struct in_addr server_addr;

main(argc,argv)
int argc;
char* argv[];
{
	char hostname[40];
	char command[BUFSIZ],data[BUFSIZ];
	int lenght,nbytes,pid;
/*
		preleva nome della macchina locale
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
#ifdef DEBUG
	printf("server: macchina locale del server\n");
	printf("		nome %s \n",server->h_name);
	printf("		tipo indirizzo %d \n",server->h_addrtype);
	printf("		lung. indirizzo %d \n",server->h_length);
	printf("		indirizzo %s \n",inet_ntoa(server_addr));
#endif
/*
		ritrova il tipo di protocollo
*/
	if ((pp = getprotobyname("tcp")) == 0) {
		fprintf(stderr,"server: errore getprotobyname()\n");
		exit(1);
	}
	printf("		protocollo %s \n",pp->p_name);
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
					port = 0 cioe` viene assegnata la prima libera
*/
	server_sock.sin_family = AF_INET;
	server_sock.sin_addr = server_addr;
	server_sock.sin_port = 0;
	lenght = sizeof(server_sock);
	if (bind(server_id,&server_sock,sizeof(server_sock)) == -1) {
		perror("server: bind()\n");
		exit(1);
	}
/*
		leggo il nome socket (dominio,indirizzo,porta)
*/
	if (getsockname(server_id, &server_sock, &lenght)  == -1) {
		perror("server: getsockname()\n");
		exit(1);
	}      
	printf("server: port %d \n", htons(server_sock.sin_port));

	if(listen(server_id,4) == -1)
	{
		perror("server: errore listen() ");
		exit(1);
	}
	while (1) {
		if((s1_id = accept(server_id,0,0)) == -1)
		{
			perror("server: accept \n");
			exit(1);
		}
		pid = fork();
		if (pid == 0) {
			close(server_id);
		/* ricevo richiesta dal client cui sono connesso */
			if ((nbytes = recv(s1_id, command, sizeof(command), 0)) < 0) {
				perror("server figlio: errore recv() \n");
				exit(1);
			}
			printf("server figlio: ricevuto -> %s\n", command);

		/* invia replica */

			strcpy(data,"ok dal server!");

			if (send(s1_id, data, strlen(data)+1,0) < 0) {
				perror("server: send() \n");
				exit(1);
			}
			close(s1_id);
			exit(0);
		}	
			pid = wait(0);
			printf("server padre: e` terminato figlio %d \n", pid);
	}
}

