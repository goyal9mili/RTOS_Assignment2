#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <pthread.h>
#include "sound.h"

#define PORT 6666
#define COUNT_MAX 1000		// defines the max connections that can be made
#define BUFSIZE 1024

pthread_mutex_t mutex_client;
int sockfd;						// This is the socket for the server that will be created.
int sd_array_client[COUNT_MAX];	// Array to store the client socket descriptors.
int count_cli;
pthread_t array_clients[COUNT_MAX];



void end_server(int sig) {
		
		printf("server is being closed \n");

		for (int i = 0; i < count_cli; i++) {
            pthread_kill(array_clients[i], SIGKILL);
        }
		for(int i = 0; i < count_cli; i++) {
			close(sd_array_client[i]);
		}
		close(sockfd);
		exit(0);

}

void *connection_handler(void* clientfd) {
	int client_fd = *((int *) clientfd);
	int num;
	struct Init init;
	read(client_fd, &init, sizeof(init));
	while(1) {
		struct Message message;
		uint8_t buf[BUFSIZE];
		num = read(client_fd, &message, sizeof(message));
		if(num == 0)		// case when a client has left the chat
		{
			printf("The client number %s has exited \n", init.user_id);
			close(client_fd);
			return NULL;
		}
		// num = loop_read(client_fd, buf, sizeof(buf)); 
		for(int i = 0; i < count_cli; i++) {
			if(client_fd != sd_array_client[i]) {
				// loop_write(sd_array_client[i], &message, sizeof(message));
				write(sd_array_client[i], &message, sizeof(message));
			}
		}
	}
}

int main(int argc, char const *argv[]) 
{ 
	int num, nsd; 
	struct sockaddr_in serv_addr; 
	int opt = 1; 
	int addrlen = sizeof(serv_addr); 

	signal(SIGINT, end_server);		// if ctrl+c is pressed to exit through the server.

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(PORT); 
	
    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(sockfd, 10);


	while(1) {
		nsd = accept(sockfd, (struct sockaddr *)&serv_addr, 
						(socklen_t*)&addrlen);

		pthread_mutex_lock(&mutex_client);	// locking the creation of a new client
		
		sd_array_client[count_cli] = nsd;
		pthread_create(&array_clients[count_cli], NULL, connection_handler, (void *) &nsd);
		count_cli++; 
		pthread_mutex_unlock(&mutex_client);
	}
	return 0; 
}