/******************************************
 *  EECS 489 PA 1
 *  
 *  client.c
 *  	usage: ./client <host_name> <port>
 *  
 *  Name:Jingyi Chen
 *  Uniqname:jingyich
 *           
 *  If you connect to local machine, use localhost as <host_name>
 *  You can use pipe to redirect the file content to the input, 
 *      e.g., ./client hostname port < data
 *
 *****************************************/

#include "header.h"


int main (int argc, char *argv[])
{
	int sock;
	struct sockaddr_in server;
	struct hostent *hp;
	char message[BUFFER_SIZE];
	char last=' ';
	
	
    if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}
	//create socket
	sock=socket(AF_INET,SOCK_STREAM,0);
	if (sock==-1)
		error("could not create socket");
	puts("socket created");
	
	//gethostbyname returns a structure including the network address
	//of the specified host.
	hp = gethostbyname(argv[1]);
	server.sin_family=AF_INET;
	server.sin_port=htons(atoi(argv[2]));
	
	if (hp == (struct hostent *) 0) {
		fprintf(stderr, "%s: unknown host\n", argv[1]);
		exit(2);
	}
	memcpy((char *) &server.sin_addr, (char *) hp->h_addr,
	    hp->h_length);
	//connect to server
	if (connect(sock,(struct sockaddr*)&server,sizeof(server))<0){
		error("connect failed.");
		
	}
	puts("connected!");
	

	//get string from input
	while(fgets(message,BUFFER_SIZE,stdin)!=NULL){
		
	
		//check double <enter>
		if (message[0]=='\n' && last=='\n'){
			break;
		}
		
		last=message[strlen(message)-1];
		
	
		if (send(sock,message,strlen(message),0)<0){
			error("send failed");
			
		}
		memset(message,0,sizeof(message));
			
		
	}

	close(sock);
	return 0;
}