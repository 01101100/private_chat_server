/** 
* Server side 
* authors:
* created date:
* last modified date:
*/
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_CLIENTS 50
#define MAX_NAME_LEN 30
#define MAX_LINE 100      // length buffer
#define PORT 100
#define LISTENQ 5  // use at line 39

typedef struct{
    int sockfd;
    int partner_sockfd;
    char name[MAX_NAME_LEN];
} Client;

void main(){
    int i, maxi, listenfd, connfd;
    Client clients[MAX_CLIENTS];
    fd_set rset, allset;
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