/** 
* Client side
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

#define SERV_PORT 9877
#define MAX_NAME_LEN 30
#define MSG_SIZE 1024

void send_message(int sockfd, char message[MSG_SIZE]) {
    write(sockfd, message, strlen(message));
}

void main() {
	int client_sockfd;
	int fd, result_len;
	char buf[MSG_SIZE], msg[MSG_SIZE], name[MAX_NAME_LEN];
	struct sockaddr_in server_addr;
	fd_set testfds, clientfds;

	client_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    if(connect(client_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    	printf("error connect socket!\n");
    	exit(1);
    }
    printf("client_sockfd: %d\n", client_sockfd);

    FD_ZERO(&clientfds);
    FD_SET(client_sockfd, &clientfds);
    FD_SET(0, &clientfds);

    /* Now wait for messages from server */
    while (1) {
    	testfds = clientfds;
    	select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

    	if(FD_ISSET(0, &testfds)) { /* process keyboard activity */
    		//TODO:
    		scanf("%[^\n]%*c", msg);
    		send_message(client_sockfd, msg);
    		if(strcmp(msg, "\\quit")==0) {
    			close(client_sockfd);
    			exit(0);
    		}
    	} else if(FD_ISSET(client_sockfd, &testfds)) { /* accept data from open socket */
    		// TODO:
    		result_len = read(client_sockfd, msg, MSG_SIZE);
    		msg[result_len] = '\0';
    		printf("%s\n", msg);
    	}
    }
}
