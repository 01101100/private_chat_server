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
    printf("%s%s%s\n",RED, USAGE, NORMAL);
    return;
}

/**
 * get username and password
 * @param uname [description]
 * @param pass  [description]
 */
void get_credential(char *username, char *password){
    // printf("\nEnter username and password (not include space):\n");
    printf("username: ");
    scanf("%s%*c", username);
    printf("password: ");
    scanf("%s%*c", password);
    return;
}

int login(char *username, char *password, int sockfd){
    printf("parsing login(%s, %s, %d)...\n", username, password, sockfd);
    char msg[MSG_SIZE];
    int len;
    sprintf(msg, "\\login %s %s", username, password);
    send_message(sockfd, msg);
    len = read(sockfd, msg, MSG_SIZE);
    msg[len] = '\0';
    return atoi(msg);
}

int sign_up(char *username, char *password, int sockfd) {
    printf("parsing sign_up(%s, %s, %d)...\n", username, password, sockfd);
    int len;
    char msg[MSG_SIZE];
    sprintf(msg, "\\sign_up %s %s", username, password);
    send_message(sockfd, msg);
    len = read(sockfd, msg, MSG_SIZE);
    msg[len] = '\0';
    return atoi(msg);
}

void main(int argc, char *argv[]) {
	int client_sockfd;
    char choice_str[MAX_NAME_LEN];
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
    // connect first
    client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if(connect(client_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("error connect socket!\n");
        exit(1);
    }
    while(1) {
        printf("\n\
===================================================\n\
------------------- CHAT SERVICE ------------------\n\
===================================================\n\n\
1. LOGIN\n\
2. SIGN UP\n\
3. EXIT\n");
        while(1) {
            printf("\nEnter your choice:...");
            scanf("%[^\n]%*c", choice_str);
            if(strlen(choice_str) != 1) continue;
            else choice = atoi(choice_str);
            
            if(choice !=1 && choice != 2 && choice != 3) {
                printf("\nSelect 1, 2, or 3.");
                continue;
            } else if(choice == 3) {
                exit(0);
            } else break;
        } // end while
        // if choice = 1 or 2
        get_credential(username, password);

        if (choice == 1) {
            // TODO: LOGIN
            if(login(username, password, client_sockfd)) {
                printf("\nLogin successfully!\n");
                break;
            } else {
                printf("\nLogin false!");
                continue;
            }
        } else if (choice == 2) {
            // TODO: sign_up
            if(sign_up(username, password, client_sockfd) == 1) {
                printf("\nSign_up successfully!\n");
                break;
            } else {   // sign_up false
                printf("\nSign_up false!");
                continue;
            }
        }
    }
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
            } else printf("%s%s\n", msg, NORMAL);
    	}
    }
}
