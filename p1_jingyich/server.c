/******************************************
 *  EECS 489 PA 1
 *  
 *  Name:Jingyi Chen
 *  Uniqname:jingyich
 *  server.c
 *  	usage: ./server <port>
 *
 *****************************************/

#include "header.h"

int main (int argc, char *argv[])
{
	int socket_desc,client_sock,c,read_size;
	struct sockaddr_in server,client;
	char client_message[BUFFER_SIZE];
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	//create socket
	socket_desc=socket(AF_INET,SOCK_STREAM,0);
	if (socket_desc==-1){
		error("could not create socket");	
	}
	memset(&server, 0, sizeof(server));
	memset(client_message, 0, sizeof(client_message));
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(atoi(argv[1]));
	if (bind(socket_desc,(struct sockaddr *)&server,sizeof(server))<0){
		error("bind failed");
	
	}
	listen(socket_desc,MAX_CONCURRENT);
	//specifies maximum number of client connections 
	//that server will queue for this listening socket
	c=sizeof(struct sockaddr_in);
	while(1){
		//accept connetction from an incoming client
		client_sock=accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&c);
		if (client_sock<0){
			error("accept failed");
			
		}
		//receive message from client
		while ((read_size=recv(client_sock,client_message,BUFFER_SIZE,0))>0){
			//send the message back to client
			
			fputs(client_message,stdout);
			
			memset(client_message,0,sizeof(client_message));
		}
		if (read_size==0){
			
			fflush(stdout);
		}
		else if(read_size==-1){
			error("receive failed");
		}
		close(client_sock);
		sleep(1);
	}
	return 0;
}