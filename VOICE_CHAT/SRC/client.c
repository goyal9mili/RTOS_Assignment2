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
#define BUFSIZE 1024

int sockfd= 0;
char write_buffer[BUFSIZE] = {0};
char name[20];

pthread_t read_thread, write_thread;


void end_client(int sig) {  // To handle closing of a client
		
        printf("The client is being exited \n");
        pthread_kill(read_thread, SIGKILL);
        pthread_kill(write_thread, SIGKILL);
		close(sockfd);
		exit(0);
}

void *read_func() {
    
	pa_simple *s = NULL;
    int ret = 1;
    int error;

    if (!(s = pa_simple_new(NULL, name, PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__ ": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }
	for (;;) {
        uint8_t buf[BUFSIZE];
		struct Message message;
        ssize_t num;
 
        if ((num = read(sockfd, &message, sizeof(message))) <= 0) {
            if (num == 0) /* EOF */
                break;

            fprintf(stderr, __FILE__": read() failed: %s\n", strerror(errno));
            goto finish;
        }

		memcpy(buf, message.msg, sizeof(message.msg));
		// printf("%s says : \n", message.name);
        /* ... and play it */
        if (pa_simple_write(s, buf, sizeof(buf), &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            goto finish;
        }
    }

    if (pa_simple_drain(s, &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
        goto finish;
    }
    ret = 0;
finish:
    if (s)
        pa_simple_free(s);

    return NULL;
} 

void *write_func() {
	pa_simple *s = NULL;
    int ret = 1;
    int error;

    if (!(s = pa_simple_new(NULL, name, PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }
    for (;;) {
		struct Message message;
        strcpy(message.name, name);
        message.msgtype = 0;

        uint8_t buf[BUFSIZE];

        if (pa_simple_read(s, buf, sizeof(buf), &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
            goto finish;
        }
        // message.msg = buf;
		memcpy(message.msg, buf, sizeof(buf));
		// strcpy(message.name, name);

        // struct Init init;
        // uint8_t init[BUFSIZE];
        // strcpy(init, "Mili");
        // write(sockfd, init, sizeof(init));
        if (write(sockfd, &message, sizeof(message)) !=  sizeof(message)) {
            fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
            goto finish;
        }
    }
    ret = 0;
finish:
    if (s)
        pa_simple_free(s);
}

int main()
{
    printf("Enter clients name \n");
    scanf("%[^\n]%*c", name);
    // strcpy(name, "Mili");
    signal(SIGINT, end_client);
	struct sockaddr_in serv_addr;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Error in creating the socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.29.160", &serv_addr.sin_addr);
    //serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    printf("Initiating connection \n"); 

	   if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
		printf("\n Error : Connect Failed \n");
		return 1;
    }

    printf("connection successful\n");

    struct Init init;
    printf("What is the id that you want for the client \n");
    scanf("%[^\n]%*c", init.user_id);
    // strcpy(init.user_id, "1");
    if(write(sockfd, &init, sizeof(init)) != sizeof(init))
    {
        fprintf(stderr, "write during the init failed : %s \n", strerror(errno));
    }

	pthread_create(&read_thread, NULL, read_func, NULL);
	pthread_create(&write_thread, NULL, write_func, NULL);

	pthread_join(read_thread, NULL);
	pthread_join(write_thread, NULL);

	return 0;
}
