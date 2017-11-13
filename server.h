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
#define MAX_PARTNERS 30
#define INIT -1
#define SPEND 0
#define CONNECTED 1
#define RPEND 2
#define HELP "\
    Server supported command:\n\
    --------------------------------------------------------------\n\
    \\help : how to use command\n\
    \\with : show current partner\n\
    \\name <your name> : change your name\n\
    \\connect <ID> : request to connect with partner has 'ID'\n\
    \\accept <ID> : accept request of people has 'ID'\n\
    \\to <ID> : send_message to <ID>\n\
    \\decline <ID> : decline request of people has <ID>\n\
    \\pp : left the current conversation.\n\
    \\quit : exit program, offline.\n\
    --------------------------------------------------------------\n"

typedef struct {
    int sockfd;
    int partner_sockfd;
    int partners[MAX_PARTNERS];
    int pair_status[MAX_PARTNERS];
    int max_index;
    int status; //-1 if none-partner 0 if pendding, 1 if connected.
    char name[MAX_NAME_LEN];
} Client;

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