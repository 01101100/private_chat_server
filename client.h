#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include <arpa/inet.h>

#define SERV_PORT 9877
#define MAX_NAME_LEN 30
#define MSG_SIZE 1024
#define USAGE "Run:\n ./client <server_address>"