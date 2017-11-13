/** 
* Client side
* authors: hienpd and longtt - hedspi b k59 dhbkhn
* created date:
* last modified date:
*/
#include "client.h"

/**
 * [send_message description]
 * @param sockfd  [description]
 * @param message [description]
 */
void send_message(int sockfd, char message[MSG_SIZE]) {
    write(sockfd, message, strlen(message));
}

/**
 * [print_usage description]
 */
void print_usage(){
    printf("%s\n", USAGE);
    return;
}

void main(int argc, char *argv[]) {
	int client_sockfd;
    int choice;
	int fd, result_len;
	char buf[MSG_SIZE], msg[MSG_SIZE], 
            username[MAX_NAME_LEN], password[MAX_NAME_LEN];
	struct sockaddr_in server_addr;
	fd_set testfds, clientfds;
    if (argc != 2) {
        print_usage();
        exit(1);
    }

    printf("\n\n\
===================================================\n\
------------------- CHAT SERVICE ------------------\n\
===================================================\n\n\
1. LOGIN\n\
2. REGISTER\n\
3. EXIT\n");
    while(1) {
        scanf("%d%*c", &choice);
        if(choice !=1 && choice != 2 && choice != 3) {
            printf("Select 1, 2, or 3.\n");
            continue;
        } else if (choice == 3) exit(0);
        else break; // if login or register
    }
    printf("Enter username and password (not include space):\n");
    printf("\nusername: ");
    scanf("%s%*c", username);
    printf("\npassword: ");
    scanf("%s%*c", password);

	client_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if(connect(client_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    	printf("error connect socket!\n");
    	exit(1);
    }

    FD_ZERO(&clientfds);
    FD_SET(client_sockfd, &clientfds);
    FD_SET(0, &clientfds);

    if (choice == 1) {
        // TODO: LOGIN
        sprintf(msg, "\\login %s %s", username, password);
        send_message(client_sockfd, msg);
    } else if (choice == 2) {
        // TODO: REGISTER
        sprintf(msg, "\\register %s %s", username, password);
        send_message(client_sockfd, msg);
    }

    /* Now wait for messages from server */
    while (1) {
    	testfds = clientfds;
    	select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

    	if(FD_ISSET(0, &testfds)) { /* process keyboard activity */
    		//TODO:
    		scanf("%[^\n]%*c", msg);
    		send_message(client_sockfd, msg);
    		// if(strcmp(msg, "\\quit")==0) {
    		// 	close(client_sockfd);
    		// 	exit(1);
    		// }
    	} else if(FD_ISSET(client_sockfd, &testfds)) { /* accept data from open socket */
    		// TODO:
    		result_len = read(client_sockfd, msg, MSG_SIZE);
    		msg[result_len] = '\0';
            if (result_len == -1) perror("read()");
            else if (result_len == 0) {
                close(client_sockfd);
                exit(1);
            } else printf("%s\n", msg);
    	}
    }
}
