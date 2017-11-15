#define MAX_CLIENTS 50
#define MAX_IDENT_LEN 32
#define MAX_USERS 50
#define MAX_NAME_LEN 32
#define MSG_SIZE 1024
#define SERV_PORT 9877
#define MAX_PARTNERS 30
#define INIT -1
#define NOT_ACTIVE -99999
#define SPEND 0
#define CONNECTED 1
#define RPEND 2
#define HELP "\
    Server supported command:\n\
    --------------------------------------------------------------\n\
    \\help : how to use command\n\
    \\with : show current partner\n\
    \\name <your name> : change your name\n\
    \\connect <name> : request to connect with partner has 'name'\n\
    \\accept <name> : accept request of people has 'name'\n\
    \\to <name> : send_message to <name>\n\
    \\decline <name> : decline request of people has <name>\n\
    \\pp : left the current conversation.\n\
    \\quit : exit program, offline.\n\
    --------------------------------------------------------------\n"
#define USAGE "Run:\n ./client <server_address>"
#define DATA_FILE "users.data"
#define LINE "----------------------------------------------------------------"
// Color
#define NORMAL  "\x1B[0m"
#define PINK  "\x1B[31m"
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define RED  "\x1B[35m"
#define CYAN  "\x1B[36m"
