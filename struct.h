#include "constant.h"

typedef struct {
    int sockfd;
    int partner_sockfd;
    int partners[MAX_PARTNERS];
    int pair_status[MAX_PARTNERS];
    int max_index;
    int status; //-1 if none-partner 0 if pendding, 1 if connected.
    char name[MAX_NAME_LEN];
} Client;

typedef struct
{
	char username[MAX_NAME_LEN];
	char password[MAX_NAME_LEN];
} User;