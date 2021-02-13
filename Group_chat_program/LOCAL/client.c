
 #include <stdio.h>
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

int sockfd = 0;
char id[2];
pthread_t thread1,thread2;

void *read_func(void *arg)
{
    char str[100];
	read(sockfd,str, 100);
    if(strcmp(str,"EXIT")==0)
    {
        printf("EXITING!\n");
        close(sockfd);
        exit(0);
    }
    printf("%c has texted: %s\n",str[99],str);
    pthread_create(&thread1,NULL,read_func,NULL);
	return (NULL);
}
void *write_func(void *arg)
{
    char str[100];
    fflush(stdout);
	scanf("%[^\n]%*c", str);
    str[99] = id[0];
	write(sockfd, str, 100);
    if(strcmp(str,"EXIT")==0)
    {
        close(sockfd);
        exit(0);
    }
    pthread_create(&thread2,NULL,write_func,NULL);
	return (NULL);
}
int main()
{
    int n = 0;
    char Buffer[1024];int temp;
    struct sockaddr_in serv_addr;
	  char s[100];

    memset(Buffer, '0',sizeof(Buffer));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Error in creating the socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "3.141.5.64", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(5555);

    printf("Initiating connection \n");
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
		printf("\n Error : Connect Failed \n");
		return 1;
    }

    printf("connection successful\n");
    read(sockfd, id, 5);  // The server will be allocating an id to the server.
    printf("The id is allocated is : %c\n", id[0]);
    pthread_create(&thread1,NULL,read_func,NULL); // Thread created to read messages.
    pthread_create(&thread2,NULL,write_func,NULL);  // Thread created to write messages.
    while(1);
    return 0;
}
