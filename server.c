/**
 * Server side 
 * authors: longtt and hienpd hedspi b k59 dhbkhn
 * created date:
 * last modified date:
 */
#include "server.h"

Client clients[MAX_CLIENTS];
User users[MAX_USERS];
int maxi, num_users;
fd_set allset;

/**
* get user index
* @return -1 if not found
*/
int get_user_index(char *username) {
    int i;
    for (i = 0; i <= num_users; i++) {
        if (strcmp(username, users[i].username) == 0) 
            return i;
    }
    return -1;
}


void init() {
    int i, j;
    char username[MAX_NAME_LEN], password[MAX_NAME_LEN];
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL){
    	printf("Database is not connected.\nRun \'touch users.data\' berfore run the ./server\n");
    }
    maxi = -1;
    num_users = -1;
    printf("parsing init()\n");
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sockfd = -1;
        clients[i].partner_sockfd = -1;
        clients[i].status = -1;
        clients[i].max_index = -1;
        for(j = 0; j < MAX_PARTNERS; j++){
            clients[i].partners[j] = clients[i].pair_status[j] = -1;
        }
    }
    // Read database
    while (fscanf(fp, "%s %s%*c", username, password) == 2){
        printf("%s %s\n", username, password);
        strcpy(users[++num_users].username, username);
        strcpy(users[num_users].password, password);
    }
    fclose(fp);
    printf("exit init()\n");
    return;
}

/**
 * [process_keyboard_activity description]
 * @param cmd           [description]
 * @param server_sockfd [description]
 */
void process_keyboard_activity(char * cmd, int server_sockfd) {
	FILE *fp = fopen(DATA_FILE, "w");
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
        for (int i = 0; i <= num_users; ++i)
        {
        	fprintf(fp, "%s %s\n", users[i].username, users[i].password);
        }
        fclose(fp);
        exit(0);
    } // endif
}

/**
 * [exit_client description]
 * @param sockfd [description]
 */
void exit_client(int sockfd) {
    printf("parsing exit_client(%d)\n", sockfd);
    int i = get_client_index(sockfd), j;
    for (j = 0; j <= clients[i].max_index; j++){
        if(clients[i].pair_status[j] == -1) continue;
        clients[i].partner_sockfd = clients[i].partners[j];
        clients[i].status = clients[i].pair_status[j];
        pp(i);
    }
    close(sockfd);
    FD_CLR(sockfd, & allset);
    clients[i].sockfd = -1;
    clients[i].partner_sockfd = -1;
    clients[i].name[0] = '\0';
    clients[i].status = -1;
    clients[i].max_index = -1;
    printf("exit exit_client(%d)\n", sockfd);
    return;
}
/**
 * [add_partner description]
 * @param  index          [description]
 * @param  partner_sockfd [description]
 * @return                [description]
 */
int add_partner(int index, int partner_sockfd){
    int si, ri;
    int partner_index = get_client_index(partner_sockfd);
    char msg[MSG_SIZE];
    for(si = 0; si < MAX_PARTNERS; si++){
        if(clients[index].partners[si] < 0){
            break;
        }
    }
    for(ri = 0; ri < MAX_PARTNERS; ri++){
        if (clients[partner_index].partners[ri] < 0) break;
    }
  
    if(si == MAX_PARTNERS){
        send_message(clients[index].sockfd, "Too much partner. You cant connect anymore.");
        return -1;
    }
  
    if(ri == MAX_PARTNERS){
        sprintf(msg, "%s is limit connection, you cant connect with %s.", clients[partner_index].name, clients[partner_index].name);
        send_message(clients[index].sockfd, msg);
        return -1;
    }
    printf("add_partner(%d, %d)", index, partner_sockfd); // log
    clients[index].partners[si] = partner_sockfd;
    clients[index].pair_status[si] = SPEND;
    clients[partner_index].partners[ri] = clients[index].sockfd;
    clients[partner_index].pair_status[ri] = RPEND;
    if (clients[index].max_index < si) clients[index].max_index = si;
    if (clients[partner_index].max_index < ri) clients[partner_index].max_index = ri;
    return si;
}

/**
* return partner's index in clients[k]
* return -1 if not found
*/
int get_partner_index(int k, int partner_sockfd) {
    int i;
    for ( i=0; i <= clients[k].max_index; i++) {
        if(clients[k].partners[i] == partner_sockfd) {
            return i;
        }
    }
    return -1;
}

/**
* accept connect 
*/
int accept_connect(int sockfd, int partner_sockfd) {
    int i = get_client_index(sockfd);
    int partner_index = get_client_index(partner_sockfd);
    int index_in = get_partner_index(i, partner_sockfd);
    if (index_in == -1) return -1;
    else if(clients[i].pair_status[index_in] == CONNECTED) return 0; // already connected
    else if (clients[i].pair_status[index_in] == RPEND) {
        clients[i].pair_status[index_in] = CONNECTED;
        clients[i].status = CONNECTED;
        clients[i].partner_sockfd = partner_sockfd;
        int index_in_partner = get_partner_index(partner_index, sockfd);
        clients[partner_index].pair_status[index_in_partner] = CONNECTED;
        clients[partner_index].status = CONNECTED;
        return 1;
    }
}

/*
xoa sockfd trong partners[], pair_status, 
*/
void decline(int sockfd, int partner_sockfd){
  int mi, pi, i, j;
  mi = get_client_index(sockfd);
  pi = get_client_index(partner_sockfd);
  if ((i = get_partner_index(mi, partner_sockfd)) < 0){
    send_message(sockfd, "System: Deo ai them yeu cau ghep doi voi ban dau. Cut ^^.");
  } else {
    j = get_partner_index(pi, sockfd);
    clients[mi].partners[i] = -1;
    clients[mi].pair_status[i] = -1;
    clients[pi].partners[j] = -1;
    clients[pi].pair_status[j] = -1;
    send_message(partner_sockfd, "System: Ban da bi tu choi, den vl:)))");
    send_message(sockfd, "Ban da tu choi loi moi thanh cong.");
  }
  return;
}

void print_struct(int sockfd){
    int j;
    int i = get_client_index(sockfd);
    printf("struct{\n");
    printf("\t%-30s%-10d\n", ".sockfd", clients[i].sockfd);
    printf("\t%-30s%-10d\n", ".partner_sockfd", clients[i].partner_sockfd);
    printf("\t%-30s%-10d\n", ".max_index", clients[i].max_index);
    printf("\t%-30s%-10d\n", ".status", clients[i].status);
    printf("\t%-30s%-30s\n", ".name", clients[i].name);
    printf("\t%s", ".partners");
    for(j = 0; j <= clients[i].max_index; j++){
        printf("[%d]", clients[i].partners[j]);
    }
    printf("\n");
    printf("\t%s", ".pair_status");
    for(j = 0; j <= clients[i].max_index; j++){
        printf("[%d]", clients[i].pair_status[j]);
    }
    printf("\n}\n");
    return;
}

int login(char *username, char *password){
	int i = get_user_index(username);
	if (i == -1) return 0;
	else if (strcmp(users[i].password, password) == 0) return 1;
	else return 0;
}

/**
* @return -1 if IS MAX_USERS
* @return 0 if can't add, da ton tai username
* @return 1 if added
*/
int sign_up(char *username, char *password) {
    int i = get_user_index(username);
    if(i != -1) return 0; // da ton tai username
    else {
        if(num_users == MAX_USERS - 1) return -1;
        else {
            ++num_users;
            strcpy(users[num_users].username, username);
            strcpy(users[num_users].password, password);
        }    
    }
    return 1;
}

/**
 * [process_client_activity description]
 * @param sockfd  [description]
 * @param message [description]
 */
void process_client_activity(int sockfd, char message[MSG_SIZE]) {
    char * first_str,  * middle_str, * last_str;
    char msg[MSG_SIZE];
    int i = get_client_index(sockfd);
    if(i == -1) {
        printf("not found sockfd: %d\n", sockfd);
        return;
    }
    if (message[0] == '\\') {
        // xu ly xau
        first_str = strtok(message, " ");
        if (strcmp(first_str, "\\login") == 0) {  // login
            // TODO : LOGIN
            middle_str = strtok(NULL, " ");
            last_str = strtok(NULL, "");
            if (login(middle_str, last_str)) {
            	send_message(sockfd, "1");
	            send_active_clients(sockfd);
			} else send_message(sockfd, "0");
        } else if (strcmp(first_str, "\\sign_up") == 0) { // sign_up
            // TODO : sign_up
            middle_str = strtok(NULL, " ");
            last_str = strtok(NULL, "");
            int check = sign_up(middle_str, last_str);
            if(check == 1) {
                send_message(sockfd, "1");
                send_active_clients(sockfd);
            } else
                send_message(sockfd, "0");

        } else if (strcmp(first_str, "\\with") == 0) {        // with
            int partner_sockfd = clients[i].partner_sockfd;
            int partner_index = get_client_index(partner_sockfd);
            sprintf(msg, "Connected with %s - %d", clients[partner_index].name, partner_sockfd);
            send_message(sockfd, msg);
        } else if (strcmp(first_str, "\\debug") == 0){   // debug
            print_struct(sockfd);
        }else if (strcmp(first_str, "\\to") == 0) {        // to
            // TODO : change conversation to paired partner, cmd: \to <ID>
            last_str = strtok(NULL, "");
            int partner_sockfd = atoi(last_str);
            int partner_index = get_partner_index(i, partner_sockfd);
            if(partner_index == -1) {
                sprintf(msg, "Partner_sockfd: %d was not connected!", partner_sockfd);
                send_message(sockfd, msg);
            } else {
                clients[i].partner_sockfd = partner_sockfd;
                sprintf(msg, "Now, send message to ID: %d", partner_sockfd);
                send_message(sockfd, msg);
            }
        } else if (strcmp(first_str, "\\help") == 0) {     // help
            send_message(sockfd, HELP);
        } else if (strcmp(first_str, "\\getonline") == 0) {    // getonline
            printf("request \\getonline from %s-%d\n", clients[i].name, clients[i].sockfd);
            // get online user list 
            send_active_clients(sockfd);
        } else if (strcmp(first_str, "\\name") == 0) {     // name
            //debug
            printf("request \\name from %s-%d\n", 
                    clients[i].name, clients[i].sockfd);
            last_str = strtok(NULL, "");
            strcpy(clients[i].name, last_str);

        } else if (strcmp(first_str, "\\connect") == 0) { // connect
            //debug
            printf("Request \\connect from %s-%d\n", 
                    clients[i].name, clients[i].sockfd);
            last_str = strtok(NULL, "");
            if(add_partner(i, atoi(last_str)) < 0){
                return;
            };
            sprintf(msg,
                    "System: Request chat from user: %s - %d\n\
                    Type: \"\\accept %d\" to accept the request.\n\
                    Type: \"\\decline %d\" to decline the reqest.", 
                    clients[i].name, clients[i].sockfd, clients[i].sockfd, clients[i].sockfd);
            send_message(sockfd, "System: Send request successfull!");
            send_message(atoi(last_str), msg);
        } else if (strcmp(first_str, "\\accept") == 0) { // accept
            int j;
            last_str = strtok(NULL, "");
            int partner_sockfd = atoi(last_str);
            printf("%d\n", partner_sockfd);
            //debug
            printf("request \\accept from %s-%d\n", 
                    clients[i].name, clients[i].sockfd);
            if ((j = accept_connect(sockfd, partner_sockfd)) == -1){
                send_message(sockfd, "System: Deo ai them yeu cau ghep doi voi ban dau. Cut ^^.");
            } else if (j == 0){
                send_message(sockfd, "Already connected");
            } else {
                sprintf(msg, "System: Ban da chap nhan loi moi cua %s-%d", 
                        clients[get_client_index(partner_sockfd)].name, partner_sockfd);
                send_message(sockfd, msg);
                sprintf(msg, "System: %s-%d da chap nhan loi moi cua ban.", 
                        clients[i].name, sockfd);
                send_message(partner_sockfd, msg);
            };
        } else if (strcmp(first_str, "\\decline") == 0) { // decline
            printf("request \\decline from %s-%d\n", clients[i].name, clients[i].sockfd);
            last_str = strtok(NULL, "");
            decline(sockfd, atoi(last_str));
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
    } else { // message[0] 1= '\'
        if (clients[i].status < 0) {
            send_message(sockfd, HELP);
        } else if (clients[i].status == 0) {
            send_message(sockfd, "Please wait this partner accept the request.");
        } else {
            sprintf(msg, "%s: %s", clients[i].name, message);
            send_message(clients[i].partner_sockfd, msg);
        }
    } // end if
    return;
}

/**
 * [send_message description]
 * @param sockfd  [description]
 * @param message [description]
 */
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
 * [add_client description]
 * @param  sockfd [description]
 * @return        [description]
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
 * [get_client_index description]
 * @param  sockfd [description]
 * @return        [description]
 */
int get_client_index(int sockfd) {
    int res = -1, i;
    printf("get_client_index(%d)\n", sockfd);
    for (i = 0; i <= maxi; i++) {
        if (clients[i].sockfd == sockfd){
            res = i;
            break;
        };
    }
    printf("get_client_index(%d) return %d\n", sockfd, res);
    return res;
}

/**
 * [send_active_clients description]
 * @param sockfd [description]
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

void remove_partner(int index, int partner_sockfd){
    int i = get_partner_index(index, partner_sockfd);
    printf("parsing remove_partner(%d, %d)\n", index, partner_sockfd);
    clients[index].partners[i] = -1;
    clients[index].pair_status[i] = -1;
    printf("exit remove_partner(%d, %d)\n", index, partner_sockfd);
    return;
}

/**
 * [pp description]
 * @param index [description]
 */
void pp(int index) {
    char msg[MSG_SIZE];
    int partner_index;
    printf("Parsing pp(%d)\n", index);
    print_struct(clients[index].sockfd);
    if(clients[index].status == INIT){
        send_message(clients[index].sockfd,"You are not in any conversation.");
        return;
    }
    partner_index = get_client_index(clients[index].partner_sockfd);
    sprintf(msg, "System: You left the conversation with %s.\n", clients[partner_index].name);
    send_message(clients[index].sockfd, msg);
    sprintf(msg, "System: %s left the conversation.\n", clients[index].name);
    send_message(clients[index].partner_sockfd, msg);
    remove_partner(index, clients[index].partner_sockfd);
    remove_partner(partner_index, clients[index].sockfd);
    clients[index].partner_sockfd = -1;
    clients[index].status = -1;
    printf("exit pp(%d)\n", index);
    return;
}

/**
 * [is_online description]
 * @param  sockfd [description]
 * @return        [description]
 */
int is_online(int sockfd) {
    int i;
    for (i = 0; i <= maxi; i++) {
        if (sockfd == clients[i].sockfd) {}
    }
}

/**
 * [main description]
 * @param argc [description]
 * @param argv [description]
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
    printf("Server is starting at port %d\n", SERV_PORT);
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