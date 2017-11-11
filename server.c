#define MAX_CLIENTS 50

typedef struct{
	int uid;
	int sockfd;
	char name[30];
	int partner_id; // client dang ket noi
	int partner_sockfd;
} client;

int number_of_online_clients = 0; // 

// danh sach nguoi dung online
void en_queue(client *cli);

// khi nguoi dung offline
void de_queue(int uid);

// client gui message toi client
void send_message(int receiver, char *message);

// notify to all online client except me
void notify_except_me(char *message);

// notify to all online client
void notify_to_all(char *message);

// gui danh sach online client toi client moi ket noi
void send_online_clients(int sockfd){
	// khoi tao message
	send_message_to_me(sockfd, message);
}

//send message to me
void send_message_to_me(int sockfd, char *message);

// ghep doi chat
void pair(int partner_id);

// unpair
void unpair();

// xu ly request tu user
void handle_request(client *me){
	send_online_clients(me->sockfd);
	notify_except_me(message); // notify cho cac client khac biet ban da online
	do{
		// print menu
		// 
		
	}while();
}

void main(){
	//connect , fork
}