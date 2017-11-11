/** 
* Server side 
* authors:
* created date:
* last modified date:
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "server.h"

void main(){
    int i, maxi, listenfd, connfd;
    Client clients[MAX_CLIENTS];
    fd_set rset, allset;
    int fd;
    char buf[MAX_LINE];
    socklen_t cli_len;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for( ; ; ) {

    }


}