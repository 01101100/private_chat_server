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

#define MAX_CLIENTS 50
#define MAX_NAME_LEN 30
#define MSG_SIZE 1024
#define SERV_PORT 9877
#define HELP "\
    Server supported command:\n\
    --------------------------------------------------------------\n\
    \\help : how to use command\n\
    \\name <your name> : change your name\n\
    \\connect <ID> : request to connect with partner has 'ID'\n\
    \\accept <ID> : accept request of people has 'ID'\n\
    \\decline <ID> : decline request of people has <ID>\n\
    \\pp : left the current conversation.\n\
    \\quit : exit program, offline.\n\
    --------------------------------------------------------------\n"

typedef struct {
    int sockfd;
    int partner_sockfd;
    char name[MAX_NAME_LEN];
    int status; //-1 if none-partner 0 if pendding, 1 if connected.
}
Client;

Client clients[MAX_CLIENTS];
int maxi;
fd_set allset;

void send_message_all(char message[MSG_SIZE]);
void send_message(int sockfd, char message[MSG_SIZE]);
void process_client_activity(int sockfd, char message[MSG_SIZE]);
void exit_client(int sockfd);
void process_keyboard_activity(char * cmd, int serer_sockfd);
void init();
int add_client(int sockfd);
void send_active_clients(int sockfd);
int get_client_index(int sockfd);
void pp(int index);

void init() {
    int i;
    maxi = -1;
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sockfd = -1;
        clients[i].partner_sockfd = -1;
        clients[i].status = -1;
    }
    return;
}

/**
quit
*/
void process_keyboard_activity(char * cmd, int server_sockfd) {
    printf("%s\n", "process_keyboard_activity()\n");
    char msg[MSG_SIZE];
    if (strcmp(cmd, "quit") == 0) {
        sprintf(msg, "System: Server is shutting down.");
        for (int i = 0; i <= maxi; i++) { // send all client
            if (clients[i].sockfd > 0) {
                send_message(clients[i].sockfd, msg);
                close(clients[i].sockfd);
            }
        } // endfor
        close(server_sockfd);
        exit(0);
    } // endif
}

/**
close sockfd
FD_CLR(socfd, &allset)
*/
void exit_client(int sockfd) {
    int i = get_client_index(sockfd);
    if (clients[i].status > 0) pp(i);
    close(sockfd);
    FD_CLR(sockfd, & allset);
    clients[i].sockfd = -1;
    clients[i].partner_sockfd = -1;
    clients[i].name[0] = '\0';
    clients[i].status = -1;
    return;
}

/**
\help
\getonline
\name $name
\connect $partner_sockfd
\accept $partner_sockfd
\decline $partner_sockfd
\pp
\quit
*/
void process_client_activity(int sockfd, char message[MSG_SIZE]) {
    char * first_str, * last_str;
    char msg[MSG_SIZE];
    int i = get_client_index(sockfd);
    if(i == -1) {
        printf("not found sockfd: %d\n", sockfd);
        return;
    }
    if (message[0] == '\\') {
        // xu ly xau
        first_str = strtok(message, " ");
        if (strcmp(first_str, "\\help") == 0) {
            send_message(sockfd, HELP);
        } else if (strcmp(first_str, "\\getonline") == 0) {
            //debug:
            printf("request \\getonline from %s-%d\n", clients[i].name, clients[i].sockfd);
            // get online user list 
            send_active_clients(sockfd);
        } else if (strcmp(first_str, "\\name") == 0) {
            //debug
            printf("request \\name from %s-%d\n", 
                    clients[i].name, clients[i].sockfd);
            last_str = strtok(NULL, "");
            strcpy(clients[i].name, last_str);

        } else if (strcmp(first_str, "\\connect") == 0) { // connect
            //debug
            printf("request \\connect from %s-%d\n", 
                    clients[i].name, clients[i].sockfd);
            if (clients[i].status > 0) {
                pp(i);
            }
            last_str = strtok(NULL, "");
            clients[i].partner_sockfd = atoi(last_str);
            clients[i].status = 0; // not connected
            sprintf(msg, "System: Request chat from user: %s - %d", 
                    clients[i].name, clients[i].sockfd);
            send_message(sockfd, "System: Send request successfull!");
            send_message(clients[i].partner_sockfd, msg);
        } else if (strcmp(first_str, "\\accept") == 0) { // accept
            // TODO:
            //debug
            printf("request \\accept from %s-%d\n", 
                    clients[i].name, clients[i].sockfd);
            last_str = strtok(NULL, "");
            int partner_sockfd = atoi(last_str);
            int partner_index = get_client_index(partner_sockfd);
            if (clients[partner_index].partner_sockfd == sockfd) {
                clients[partner_index].status = 1;
                clients[i].partner_sockfd = partner_sockfd;
                clients[i].status = 1;
                sprintf(msg, "System: Ban da chap nhan loi moi cua %s-%d", 
                        clients[partner_index].name, partner_sockfd);
                send_message(sockfd, msg);
                sprintf(msg, "System: %s-%d da chap nhan loi moi cua ban.", 
                        clients[i].name, sockfd);
                send_message(partner_sockfd, msg);
            } else {
                send_message(sockfd, "System: Deo ai them yeu cau ghep doi voi ban dau. Cut ^^.");
            }
        } else if (strcmp(first_str, "\\decline") == 0) { // decline
            //debug
            printf("request \\decline from %s-%d\n", 
                    clients[i].name, clients[i].sockfd);
            // TODO:
            last_str = strtok(NULL, "");
            int partner = atoi(last_str);
            if (clients[get_client_index(partner)].partner_sockfd == sockfd) {
                send_message(partner, "System: Ban da bi tu choi, den vl:)))");
                clients[get_client_index(partner)].partner_sockfd = -1;
                clients[get_client_index(partner)].status = -1;
            } else {
                send_message(sockfd, "System: Deo ai them yeu cau ghep doi voi ban dau. Cut ^^.");
            }

        } else if (strcmp(first_str, "\\pp") == 0) { // pp
            //debug
            printf("request \\pp from %s-%d\n", clients[i].name, clients[i].sockfd);
            pp(i);
        } else if (strcmp(first_str, "\\quit") == 0) { // quit
            //debug
            printf("request \\quit from %s-%d\n", clients[i].name, clients[i].sockfd);
            exit_client(sockfd);
        } else {
            send_message(sockfd, "System: Incorrect command!");
        }
    } else {
        /**
        kiem tra da ghep doi hay chua,
        neu chua ghep doi tra ve msg loi,
        neu da ghep doi gui msg toi partner
        */
        if (clients[i].partner_sockfd <= 0) {
            send_message(sockfd, HELP);
            return;
        }

        sprintf(msg, "%s: %s", clients[i].name, message);
        send_message(clients[i].partner_sockfd, msg);
    }
    return;
}

void send_message(int sockfd, char message[MSG_SIZE]) {
    write(sockfd, message, strlen(message));
}

void send_message_all(char message[MSG_SIZE]) {
    int i;
    for (i = 0; i <= maxi; i++) {
        if (clients[i].sockfd > 0) {
            send_message(clients[i].sockfd, message);
        }
    } // end for
}

/**
    add sockfd to allset
    add sockfd, name to clients
    */
int add_client(int sockfd) {
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sockfd < 0) {
            clients[i].sockfd = sockfd;
            sprintf(clients[i].name, "user_%d", sockfd);
            break;
        }
    }
    if (i == MAX_CLIENTS) perror("too many clients");
    FD_SET(sockfd, & allset); /* add new descriptor to set */
    //debug
    printf("Them vao vi tri i = %d, sockfd = %d\n", i, clients[i].sockfd);
    return i;
}

/**
get index of sockfd in clients list
return -1 if sockfd is not exist on clients list
*/
int get_client_index(int sockfd) {
    int i;
    for (i = 0; i <= maxi; i++) {
        if (clients[i].sockfd == sockfd) return i;
    }
    return -1;
}

/**
send online user list to user
*/
void send_active_clients(int sockfd) {
    int i;
    char msg[MSG_SIZE];
    sprintf(msg, "%-5s%-30s\n", "ID", "Name");
    send_message(sockfd, msg);
    for (i = 0; i <= maxi; i++) {
        if (clients[i].sockfd > 0) {
            sprintf(msg, "%-5d%-30s\n", clients[i].sockfd, clients[i].name);
            send_message(sockfd, msg);
        }
    }
}

/*
 * gui tin nhan ket thuc toi partner
 * xoa partner_sockfd o ca 2 Client struct
 */
void pp(int index) {
    int i = get_client_index(clients[index].partner_sockfd);
    char msg[MSG_SIZE];
    sprintf(msg, "You left the conversation.\n");
    send_message(clients[index].sockfd, msg);
    sprintf(msg, "%s left the conversation.\n", clients[index].name);
    send_message(clients[index].partner_sockfd, msg);
    clients[i].partner_sockfd = -1;
    clients[i].status = -1;
    clients[index].partner_sockfd = -1;
    clients[index].status = -1;
    return;
}

/**
 * check user online
 * @return 0 if not-online, 1 if online
 */
int is_online(int sockfd) {
    int i;
    for (i = 0; i <= maxi; i++) {
        if (sockfd == clients[i].sockfd) {}
    }
}

/**

*/
void main(int argc, char * argv[]) {
    int client_sockfd, server_sockfd, sockfd;
    int i, n, nready, maxfd;
    char name[MAX_NAME_LEN], msg[MSG_SIZE];
    fd_set rset;
    struct sockaddr_in client_addr, server_addr;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero( & server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);
    bind(server_sockfd, (struct sockaddr * ) & server_addr, sizeof(server_addr));
    listen(server_sockfd, 1);

    /*initialize a file descriptor set */
    maxfd = server_sockfd;
    init();
    FD_ZERO( & allset);
    FD_SET(server_sockfd, & allset);
    FD_SET(0, & allset);
    while (1) {
        rset = allset;
        nready = select(maxfd + 1, & rset, NULL, NULL, NULL);
        /* new client connection */
        if (FD_ISSET(server_sockfd, & rset)) {
            client_sockfd = accept(server_sockfd, NULL, NULL);
            printf("System: new connection from client_sockfd: %d\n\n", client_sockfd);
            /* save descriptor */
            i = add_client(client_sockfd);
            
            if (client_sockfd > maxfd) maxfd = client_sockfd; /* */
            if (i > maxi) maxi = i; /* */
            //debug
            printf("maxi = %d\n", maxi);
            send_active_clients(client_sockfd);
            if (--nready <= 0) continue; /* no more readable descriptors */
        }

        if (FD_ISSET(0, & rset)) {
            /* process keyboard activity */
            char cmd[100];
            scanf("%[^\n]%*c", cmd);
            process_keyboard_activity(cmd, server_sockfd);
            if (--nready <= 0) continue;
        }

        for (i = 0; i <= maxi; i++) {
            sockfd = clients[i].sockfd;
            if (sockfd <= 0) continue; // not set
            if (FD_ISSET(sockfd, & rset)) {
                /*Process Client specific activity*/
                n = read(sockfd, msg, MSG_SIZE);
                msg[n] = '\0';
                if (n == -1) perror("read()");
                else if (n == 0) exit_client(sockfd);
                else {
                    process_client_activity(sockfd, msg);
                }
            }
        }
    }
}