#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include "constant.h"
#include "struct.h"

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
int get_partner_index(int i, int partner_sockfd);
void print_struct(int sockfd);