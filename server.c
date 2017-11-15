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


int get_sockfd_from_name(char *name) {
    int i;
    for(i=0; i <= maxi; i++) {
        if(strcmp(name, clients[i].name) == 0)
            return clients[i].sockfd;  // return sockfd from name
    }
    return NOT_ACTIVE; // if not active
}

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
                send_system_message(clients[i].sockfd, msg);
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
    printf("add_partner(%d, %d)\n", index, partner_sockfd); // log
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
    if (index_in == -1) return -1; // neu k co yeu cau
    else if(clients[i].pair_status[index_in] == CONNECTED) return 0; // already connected
    else if (clients[i].pair_status[index_in] == RPEND) { // neu chua connected 
        clients[i].pair_status[index_in] = CONNECTED;
        int index_in_partner = get_partner_index(partner_index, sockfd);
        clients[partner_index].pair_status[index_in_partner] = CONNECTED;
        if (clients[partner_index].partner_sockfd == sockfd) clients[partner_index].status = CONNECTED;
        return 1;
    }
}

/*
xoa sockfd trong partners[], pair_status, 
*/
void decline(int sockfd, int partner_sockfd){
  int mi, pi, i, j;
  char msg[MSG_SIZE];
  mi = get_client_index(sockfd);
  pi = get_client_index(partner_sockfd);
  if ((i = get_partner_index(mi, partner_sockfd)) < 0){
    send_system_message(sockfd, "System: This person is not request to chat with you.");
  } else {
    j = get_partner_index(pi, sockfd);
    clients[mi].partners[i] = -1;
    clients[mi].pair_status[i] = -1;
    clients[pi].partners[j] = -1;
    clients[pi].pair_status[j] = -1;
    sprintf(msg, "System: %s denied your request.", clients[mi].name);
    send_system_message(partner_sockfd, msg);
    sprintf(msg, "System: Denied %s's request.", clients[pi].name);
    send_message(sockfd, msg);
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

/**
 * [login description]
 * @param  username [description]
 * @param  password [description]
 * @return          [description]
 */
int login(char *username, char *password){
	int i = get_user_index(username);
	if (i == -1) return 0; // sai username
	else if (strcmp(users[i].password, password) == 0){ // dung username, password
		i = get_sockfd_from_name(username);
		if (i == NOT_ACTIVE) return 1; // login success
		return 2; // da dang nhap tren may khac
	} else return 0; // sai mat khau
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
 * [connect description]
 * @param  sockfd         [description]
 * @param  partner_sockfd [description]
 * @return                [description]
 */
int handle_connect_cmd(int sockfd, int partner_sockfd){
	int i = get_client_index(sockfd);
	char msg[MSG_SIZE];
	if(add_partner(i, partner_sockfd) < 0){
	    send_system_message(sockfd, "Can't add connect any more!");
	    return 0;
	};
	sprintf(msg,
	        "System: Request chat from user: %s - %d\n\
	        Type: %s\\accept %s%s to accept the request.\n\
	        Type: %s\\decline %s%s to deny the reqest.", 
	        clients[i].name, clients[i].sockfd, GREEN, clients[i].name, RED, GREEN, clients[i].name, RED);
	send_system_message(sockfd, "System: Request sent successfull!");
	send_system_message(partner_sockfd, msg);
	return 1;
}

/**
 * [accept description]
 * @param  sockfd         [description]
 * @param  partner_sockfd [description]
 * @return                [description]
 */
void handle_accept_cmd(int sockfd, int partner_sockfd){
	int j, i = get_client_index(sockfd);
	char msg[MSG_SIZE];
    if ((j = accept_connect(sockfd, partner_sockfd)) == -1){ // neu k co yeu cau
        send_system_message(sockfd, "System: this guy is not request to chat with you.");
    } else if (j == 0){  // neu da ket noi
        send_system_message(sockfd, "System: Already connected");
    } else {  // neu chua ket noi
        sprintf(msg, "System: Ban da chap nhan loi moi cua %s-%d", 
                clients[get_client_index(partner_sockfd)].name, partner_sockfd);
        send_system_message(sockfd, msg);
        sprintf(msg, "System: %s-%d da chap nhan loi moi cua ban.", 
                clients[i].name, sockfd);
        send_system_message(partner_sockfd, msg);
    }
    return;
}

/**
 * [process_client_activity description]
 * @param sockfd  [description]
 * @param message [description]
 */
void process_client_activity(int sockfd, char message[MSG_SIZE]) {
    char first_str[MAX_IDENT_LEN], middle_str[MAX_IDENT_LEN], last_str[MAX_IDENT_LEN];
    char msg[MSG_SIZE];
    int params;
    int i = get_client_index(sockfd);
    if(i == -1) {
        printf("not found sockfd: %d\n", sockfd);
        return;
    }
    if (message[0] == '\\') {
    	first_str[0] = middle_str[0] = last_str[0] = '\0';
    	params = sscanf(message, "%s%s%s", first_str, middle_str, last_str);
    	printf("cmd: %-10s%-15s%-15sparams: %-2dsockfd: %-4d\n", first_str, middle_str, last_str, params, sockfd); // log
        if (strcmp(first_str, "\\login") == 0) {  // login
        	int state;
            if ((state = login(middle_str, last_str)) == 1) { // login success
            	send_message(sockfd, "1");
                strcpy(clients[i].name, middle_str); // rename to username login
                sleep(1);
	            send_active_clients(sockfd);
			} else if(state == 2){ // dang nhap tren may khac
				send_message(sockfd, "2");
			} else send_message(sockfd, "0"); // login failed
        } else if (strcmp(first_str, "\\sign_up") == 0) { // sign_up
            int check = sign_up(middle_str, last_str);
            if(check == 1) {
                send_message(sockfd, "1");
                strcpy(clients[i].name, middle_str); // rename to username sign_up
                send_active_clients(sockfd);
            } else
                send_message(sockfd, "0");

        } else if (strcmp(first_str, "\\with") == 0) {        // with
        	if (params != 1){
        		send_system_message(sockfd, "Invalid \\with command.\nType \\help for more information.");
        		return;
        	}
            int partner_sockfd = clients[i].partner_sockfd;
            int partner_index = get_client_index(partner_sockfd);
            if (partner_sockfd != INIT) sprintf(msg, "System: Connected with %s - %d", clients[partner_index].name, partner_sockfd);
            else sprintf(msg, "System: You are not in any conversation right now.");
            send_system_message(sockfd, msg);
        } else if (strcmp(first_str, "\\debug") == 0){   // debug
            print_struct(sockfd);
        }else if (strcmp(first_str, "\\to") == 0) {        // to
        	int partner_index, partner_sockfd, index_in;
            partner_sockfd = get_sockfd_from_name(middle_str);
        	if (params != 2 || partner_sockfd == 0){
        		send_system_message(sockfd, "System: Invalid \\to command\nType \\help for more information.");
        		return;
        	}
            partner_index = get_client_index(partner_sockfd);
            index_in = get_partner_index(i, partner_sockfd);
            /* change state of sender */
            clients[i].partner_sockfd = partner_sockfd;
            if(index_in == -1) { // chua ton tai partner
                handle_connect_cmd(sockfd, partner_sockfd);
                clients[i].status = SPEND;
            } else { // da ton tai partner
                if (clients[i].pair_status[index_in] == SPEND){  // neu dang yeu cau den partner
                    sprintf(msg, "System: Waiting accept from %s", clients[partner_index].name);
                    send_system_message(sockfd, msg);
                    clients[i].status = SPEND;
                }
                else if(clients[i].pair_status[index_in] == RPEND) {  // duoc gui yeu cau nhung chua accept
                	handle_accept_cmd(sockfd, partner_sockfd);
                	clients[i].status = CONNECTED;
                } else { // CONNECTED
                    clients[i].status = CONNECTED;
                    sprintf(msg, "System: Now you can send message to %s-%d", clients[partner_index].name, partner_sockfd);
                    send_system_message(sockfd, msg);
                }
            }
        } else if (strcmp(first_str, "\\help") == 0) {     // help
        	if(params != 1){
        		send_system_message(sockfd, "System: Just \\help.");
        		return;
        	}
            send_message(sockfd, HELP);
        } else if (strcmp(first_str, "\\getonline") == 0) {    // getonline
        	if(params != 1){
        		send_system_message(sockfd, "System: Just \\getonline.");
        		return;
        	}
            send_active_clients(sockfd);
        } else if (strcmp(first_str, "\\connect") == 0) { // connect
        	int partner_sockfd = get_sockfd_from_name(middle_str);
        	if (params != 2 || partner_sockfd == 0){
        		send_system_message(sockfd, "Invalid \\connect <> command\nType \\help for details.");
        		return;
        	}
        	handle_connect_cmd(sockfd, partner_sockfd);
        } else if (strcmp(first_str, "\\accept") == 0) { // accept
            int j;
            int partner_sockfd = get_sockfd_from_name(middle_str);
        	if (params != 2 || partner_sockfd == 0){
        		send_system_message(sockfd, "Invalid \\connect <> command\nType \\help for details.");
        		return;
        	}
        	handle_accept_cmd(sockfd, partner_sockfd);
        } else if (strcmp(first_str, "\\decline") == 0) { // decline
            int partner_sockfd = get_sockfd_from_name(middle_str);
        	if (params != 2 || partner_sockfd == 0){
        		send_system_message(sockfd, "Invalid \\decline <> command\nType \\help for details.");
        		return;
        	}
            decline(sockfd,partner_sockfd);
        } else if (strcmp(first_str, "\\pp") == 0) { // pp
        	if (params != 1){
        		send_system_message(sockfd, "System: Just \\pp");
        		return;
        	}
            pp(i);
        } else if (strcmp(first_str, "\\quit") == 0) { // quit
            exit_client(sockfd);
        } else {
            send_system_message(sockfd, "System: Incorrect command!");
        }
    } else { // message[0] 1= '\'
        if (clients[i].status < 0) {
            send_system_message(sockfd, "System: You it not in any conversation.");
        } else if (clients[i].status == 0) {
            send_system_message(sockfd, "System: Please wait this partner accept the request.");
        } else {
            sprintf(msg, "%s\n%s%s: %s%s\n%s", LINE, YELLOW, clients[i].name, message, NORMAL, LINE);
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
    return;
}

void send_system_message(int sockfd, char message[MSG_SIZE]) {
	char msg[MSG_SIZE];
	sprintf(msg, "%s%s", RED, message);
    write(sockfd, msg, strlen(msg));
    return;
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
    for (i = 0; i <= maxi; i++) {
        if (clients[i].sockfd == sockfd){
            res = i;
            break;
        };
    }
    return res;
}

/**
 * [send_active_clients description]
 * @param sockfd [description]
 */
void send_active_clients(int sockfd) {
    int i;
    char msg[MSG_SIZE];
    sprintf(msg, "\n%sActive Users:\n%-5s%-30s\n",GREEN, "ID", "Name");
    send_message(sockfd, msg);
    for (i = 0; i <= maxi; i++) {
        if (clients[i].sockfd > 0) {
        	if (clients[i].sockfd == sockfd) sprintf(msg, "%s%-5d%s ( * )\n",GREEN, clients[i].sockfd, clients[i].name);
            else sprintf(msg, "%s%-5d%-30s\n",GREEN, clients[i].sockfd, clients[i].name);
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
        send_system_message(clients[index].sockfd,"System: You are not in any conversation.");
        return;
    }
    partner_index = get_client_index(clients[index].partner_sockfd);
    sprintf(msg, "System: You left the conversation with %s.\n", clients[partner_index].name);
    send_system_message(clients[index].sockfd, msg);
    sprintf(msg, "System: %s left the conversation with you.\n", clients[index].name);
    send_system_message(clients[index].partner_sockfd, msg);
    remove_partner(index, clients[index].partner_sockfd);
    remove_partner(partner_index, clients[index].sockfd);
    clients[index].partner_sockfd = -1;
    clients[index].status = -1;
    clients[partner_index].partner_sockfd = clients[partner_index].status = INIT;
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
            printf("System: new connection from client_sockfd: %d\n", client_sockfd);
            /* save descriptor */
            i = add_client(client_sockfd);  // comment: should edit add_client()?
            
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