/*
group member:Anqi Dai, Jingyi Chen
*/


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>

using namespace std;
#define MAX_CONCURRENT 15
#define BUFFER_SIZE 100000

enum {OK, BAD_REQUEST, NOT_IMPLEMENTED};

/*********************function declaration***************************/
static void socketSetup(int portno);//setup socket between client and proxy
void *receivePacket(void* sock);//receive Packets from client
int sendChar(char* buf,int sock);//send char by char
int parseRequest(char* recvBuffer,unsigned short &serverPort,char* sendBuffer);//parse the request from client

/*******************************main*********************************/
int main(int argc,char* argv[]){
    if(argc < 2){
        fprintf(stderr, "No Port Provided!\n");
        exit(1);
    }
    socketSetup(atoi(argv[1]));
    return 0;
}

/***************************socketSetup*******************************/
/*
 this part set up socket between local proxy and client:
 1.create socket
 2.bind socket
 3.listen
 4.accept
 */
static void socketSetup(int portno){
    //create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("ERROR opening socket");
        exit(1);
    }
    int on = 1;
    if( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on,sizeof(on)) < 0){
        perror("reuse address");
        exit(1);
    }
    //bind socket
    struct sockaddr_in  proxyAddr, clientAddr;
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_addr.s_addr = INADDR_ANY;
    proxyAddr.sin_port = htons(portno);
    
    if (bind(sock, (struct sockaddr *) &proxyAddr, sizeof(proxyAddr)) < 0){
        perror("ERROR on binding");
        exit(1);
    }
    socklen_t proxyAddrlen = sizeof(proxyAddr);
    // Find assigned port number and print it out
    if(getsockname(sock, (struct sockaddr *) &proxyAddr, &proxyAddrlen) < 0){
        perror("Get socket name");
        exit(1);
    }
    portno = ntohs(proxyAddr.sin_port);
    //accept connection
    listen(sock,MAX_CONCURRENT);
    signal(SIGPIPE,SIG_IGN);
    //server keeps running ...
    pthread_t tid;
    socklen_t clientAddrlen = sizeof(clientAddr);
    
    while(true){
        
        int* sockPtr = (int*) malloc(sizeof(int));
        *sockPtr = -1;
        *sockPtr = accept(sock,(sockaddr *)&clientAddr,&clientAddrlen);
        if(*sockPtr == -1) continue;

        //multi-thread ...
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr,1024*1024);
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        pthread_create(&tid,&attr,receivePacket,(void*)sockPtr);
    }
}


/*********************receivePacket***************************/
/*
 this part we receive request like GET http://www.umich.edu/ HTTP/1.0 from client to proxy
 if there is an error with the first line user input, we print an error message
 1.call parseRequest to convert the request to standard format
 2.send the formatted the request to the remote server
 3.get response from the server
 4.resend all respond to the client
 */
void *receivePacket(void* sock){
    char recvBuffer[BUFFER_SIZE];
    char sendBuffer[BUFFER_SIZE];//save formatted request
	bool flag = false;
    memset(recvBuffer,'\0',BUFFER_SIZE);
    memset(sendBuffer,'\0',BUFFER_SIZE);
    
    int temp;
    int * socktmp = (int*) sock;
    for(int i = 0; i<BUFFER_SIZE;i++){
        int status = recv(*socktmp,recvBuffer+i,1,0);
        if(recvBuffer[i] == ':') 
			flag=true;
        if(status < 0){
            perror("receive error");
            return NULL;
        }
        
        else if(status == 0){
            break;
        }
        //nonstandard new line character
        else if(status==1 && flag==true && recvBuffer[i] =='\n' && recvBuffer[i-1]=='\r'){
            break;
        }//GET http://www.umich.edu/ HTTP/1.0
        //OR GET / HTTP/1.0
        //Host: www.umich.edu

    }
    
    unsigned short serverPort = 80;//if the user request is like GET http://www.umich.edu:80/ HTTP/1.0
    int status = parseRequest(recvBuffer,serverPort,sendBuffer);//parse the first line request

    if(status == NOT_IMPLEMENTED){
        char sendBuffer[] = "NOT IMPLEMENTED ERROR CODE:501\n";
        unsigned int i = 0;
        while(i<strlen(sendBuffer)){
            while(sendChar(sendBuffer+i,*socktmp) == 0){
                sendChar(sendBuffer+i,*socktmp);
            }
            i++;
        }
    }
    else if(status == BAD_REQUEST){

        char sendBuffer[]="BAD REQUEST ERROR CODE:400\n";
        unsigned int i = 0;
        while(i<strlen(sendBuffer)){
            while(sendChar(sendBuffer+i,*socktmp) == 0){
                sendChar(sendBuffer+i,*socktmp);
            }
            i++;
        }

    }
    else{
    //send request to server
    //initialize proxy

    	int serversock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    	if(serversock<0){
        	perror("socket");
        	printf("Fail to create socket\n");
        	exit(1);
    	}
    //establish proxy
    	struct sockaddr_in sin;

    //get hostpath from sendBuffer
    	char hostpath[3000];
        char *temp = sendBuffer;
        while(*temp != '\n')
            temp++;
    	char* ptr1 = strchr(temp,':');
    	int hostpathlen = strlen(sendBuffer) - (ptr1-sendBuffer+1) - 3;
    	strncpy(hostpath,ptr1+2,hostpathlen);
    	hostpath[hostpathlen] = '\0';

    	struct hostent *host = gethostbyname(hostpath);
    	if(!host){
        	char sendBuffer[]="BAD REQUEST ERROR CODE:400\n";
        	unsigned int i = 0;
        	while(i<strlen(sendBuffer)){
            	while(sendChar(sendBuffer+i,*socktmp) == 0){
                	sendChar(sendBuffer+i,*socktmp);
            	}
            	i++;
        	}
            return NULL;
    	}
    	unsigned int serverAddr = *(unsigned long *)host->h_addr_list[0];

    //clear buffer...
    	memset(&sin,0,sizeof(sin));
    
    //set up connection between the proxy and the server
    	sin.sin_family = AF_INET;
    	sin.sin_addr.s_addr = serverAddr;
    	sin.sin_port = htons(serverPort);
    	if(connect(serversock,(struct sockaddr *)&sin,sizeof(sin))<0){
        	perror("connection");
        	printf("cannot connect to server\n");
        	exit(1);
    	}

    //send request to server
    	unsigned int i = 0;
    	while(i<strlen(sendBuffer)){
        	while(sendChar(sendBuffer+i,serversock) == 0)
            	sendChar(sendBuffer+i,serversock);
        	i++;
    
    	}
    
    //get result back from server
	char recvfromserverBuffer[1];
    while(1){
		memset(recvfromserverBuffer,'\0',1);
		int status = recv(serversock,recvfromserverBuffer,1,0);
		if(status == 1){
			while(sendChar(recvfromserverBuffer,*socktmp) == 0)
            	sendChar(recvfromserverBuffer,*socktmp);
		}
		else if(status == 0){
			break;
		}	
		else{
			perror("receiving error");
            printf("socket receiving error");
		}
	}
	close(serversock);
	}
    close(*socktmp);
    free(sock);
    return NULL;
}



/*********************parseRequest***************************/
/*
 this part converts the input line of the user to the standard format
 */
int parseRequest(char* recvBuffer,unsigned short &serverPort,char* sendBuffer){

    char method[3000];//GET
    char path[3000];//http:// or /
    char http_version[3000];//HTTP/1.0
    
    char host[3000];//Host
    char hostpath[3000];//www...
    char port[200];//80
    int fields = sscanf(recvBuffer, "%s %s %s %s %s", method,path,http_version,host,hostpath);

    if(fields<3){
        return BAD_REQUEST;
    }
    //convert method to uppercase
    for(unsigned int i = 0;i<strlen(method);i++){
        if(method[i]>='a'&&method[i]<='z')
            method[i] -= 32;
    }
    //convert HTTP to uppercase
    for(unsigned int i = 0;i<strlen(http_version);i++){
        if(http_version[i]>='a' && http_version[i]<='z'){
            http_version[i] -= 32;
        }
    }
    //return 501 if the method is not "GET"
    if(strcmp(method, "GET")!=0){
        return NOT_IMPLEMENTED;
    }
    //return 400 if the http_version used is not http 1.0 or 1.1(inconsistent HTTP version)
    if(strcmp(http_version,"HTTP/1.1")!=0 && strcmp(http_version,"HTTP/1.0")!=0){                 return BAD_REQUEST;
    }

    //convert HTTP to uppercase
    if(fields == 5){
        for(unsigned int i = 0;i<strlen(host);i++){
            if(host[i]>='a'&&host[i]<='z'){
                host[i] -= 32;
            }
        }
    }
    //GET / HTTP/1.0\n
    //Host: www.umich.edu:80\n\n

    if(fields == 5 && (strcmp(host,"HOST:")) == 0){

        char* ptr = strchr(hostpath,':');//get 80 if there is one
		

        if(ptr){
            int portlen = strlen(hostpath) - (ptr-hostpath) - 1;
            strncpy(port,ptr+1,portlen);
            port[portlen] = '\0';
            int temp;
            sscanf(port,"%d",&temp);
            serverPort = (unsigned short)temp;
            hostpath[ptr-hostpath] = '\0';//get rid of 80
        }
    }
    //GET http://www.umich.edu:80/ HTTP/1.0\n
    else{

        if(strlen(path)<=7){
            return BAD_REQUEST;
        }
        char http[20];
        strncpy(http,path,7);
        if(strcmp(http,"http://")){
            return BAD_REQUEST;
        }
        char *ptr2 = strchr(path+7,':');
        char *ptr1 = strchr(path+7,'/');

        if(!ptr1){

            strcat(path,"/");//GET http://www.google.com HTTP/1.0 is tolerable
            ptr1 = path + strlen(path)-1;
        }

        if(ptr2 && (ptr2-path)<(ptr1-path)){//":" has to appear before "/"(i.e. :80/)
            strncpy(port,ptr2+1,ptr1-ptr2);
            port[ptr1-ptr2-1] = '\0';

            int temp;
            sscanf(port,"%d",&temp);
            serverPort = (unsigned short)temp;
            strncpy(hostpath,path+7,ptr2-path-7);//get rid of http://
            hostpath[ptr2-path-6] = '\0';

        }
        else{

            strncpy(hostpath,path+7,ptr1-path-7);//get rid of http://
            hostpath[ptr1-path-7] = '\0';
        }

        strncpy(path,ptr1,path+strlen(path)-ptr1+1);//still sth after "80/"
        
    }
    strcpy(sendBuffer,method);
    strcat(sendBuffer," ");
    strcat(sendBuffer,path);
    strcat(sendBuffer," HTTP/1.0\n");
    strcat(sendBuffer,"HOST: ");
    strcat(sendBuffer,hostpath);
    strcat(sendBuffer,"\n\n");

	printf("%s\n",sendBuffer);
    return OK;
}

/*********************sendChar*************************/
/*
 this part use send(sock,buf,1,0) to send character by character
 */
int sendChar(char *buffer,int sock){
    int status = send(sock,buffer,1,0);
    if(status<0){
//        perror("send error");
        return 0;
    }
    return 1;
}




