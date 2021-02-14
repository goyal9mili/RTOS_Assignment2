
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

#define MAX 5 // defines the max allowed clients in the group chat
int connfd[MAX] = {0};
int i, e[MAX] = {0};
pthread_t thread2;
pthread_t thread1[MAX];

void *read_func(void *arg)
{
    int k = *(int *)arg;
    int l;
    char str[100];
	read(connfd[k],str, 100);
    if(strcmp(str,"EXIT")==0)
    {
        printf("Client %c has EXITED\n",(char)(65+k));
        close(connfd[k]);
        return (NULL);
    }
    if(str[97] == 'G')
    {
      // printf("Entered G \n");
      for(l=0;l<i;l++)
          if(l!=k)
              write(connfd[l], str, 100);
    }
    else
    {
      int p = str[98]-'0';
      // printf("%c ..\n", str[98]);
      // printf("....%d \n", p);
      write(connfd[p], str, 100);
    }

	printf("%c says: %s\n",str[99],str);
    pthread_create(&thread1[k],NULL,read_func,(void *)&k);;
	return (NULL);
}
void *write_msg(void *arg)
{
    int l;
    char str[100];
    fflush(stdin);
	scanf("%[^\n]%*c", str);
    str[99] = 'S';
    for(l=0;l<i;l++)
        write(connfd[l], str, 100);
    if(strcmp(str,"EXIT")==0)
        exit(0);
    pthread_create(&thread2,NULL,write_msg,NULL);
	return (NULL);
}
int main()
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[1025];
    char client_id[MAX][2]; // to assign ids to the clients
    int j[MAX];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5555);

    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(sockfd, 10);

    for (i = 0; i < MAX; ++i)
    {
        client_id[i][0] = (char)(65+i);
        client_id[i][1] = '\n';
        j[i] = i;
    }
    printf("My id is : S\n");
    pthread_create(&thread2,NULL,write_msg,NULL); // thread to write to the clients
    for(i = 0; i < MAX; i++)
    {
      // printf("entered loop\n");
        connfd[i] = accept(sockfd, (struct sockaddr*)NULL, NULL);
        printf("connection accepted\n");
        write(connfd[i], client_id[i], 5);  // assign client id to the client
        printf("Client %c is Online\n",client_id[i][0]);
        pthread_create(&thread1[i],NULL,read_func,(void *)(j+i));  // read messages from the client
    }

}
