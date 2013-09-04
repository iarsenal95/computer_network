/******************************************
 *  EECS 489 Homework 1
 *  
 *  header.h
 *  
 *  Contains required header files, constant definitions and auxiliary functions
 *
 *****************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

//Use this buffer size whenever you want to open a buffer to store data
const int BUFFER_SIZE = 1000000; 

//The server should allow up to 10 concurrent connections
const int MAX_CONCURRENT = 10; 

//print error, example: error("I am a weird error");
void error(char *msg)
{
    perror(msg);
    exit(0);
}
