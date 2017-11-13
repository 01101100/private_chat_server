#include "help.h"

void print_struct(int sockfd){
	int i = get_client_index(sockfd);
	printf("%-10s%-10d\n", ".sockfd", clients[i].sockfd);
	printf("%-10s%-10d\n", ".partner_sockfd", clients[i].partner_sockfd);
	printf("%-10s%-10d\n", ".max_index", clients[i].max_index);
	printf("%-10s%-10d\n", ".status", clients[i].status);
	printf("%-10s%-30s\n", ".name", clients[i].name);
	printf("%-10s", ".partners");
	for(int j = 0; j <= clients[i].max_index; j++){
		printf("[%d]", clients[i].partners[j]);
	}
	printf("%-10s", ".pair_status");
	for(int j = 0; j <= clients[i].max_index; j++){
		printf("[%d]", clients[i].pair_status[j]);
	}
}