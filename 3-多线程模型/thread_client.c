#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 9965

int main()
{
	struct sockaddr_in serverAddr;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET,SERVER_IP,&serverAddr.sin_addr.s_addr);
	
	int sockfd;
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if(connect(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr)) == -1)
	{
		perror("connect failed");
		exit(0);	
	}
	
	char buffer[4096];
	bzero(buffer,sizeof(buffer));
	int len = recv(sockfd,buffer,sizeof(buffer),0);
	printf("%s\n",buffer);
	while(fgets(buffer,sizeof(buffer),stdin) != NULL)
	{
		
		if(send(sockfd,(void*)buffer,strlen(buffer),0) == -1)
		{
			perror("send fail");
			exit(0);
		}
		if(recv(sockfd,buffer,sizeof(buffer),0) > 0)
		{
			printf("Server back:%s\n",buffer);
		}
		bzero(buffer,sizeof(buffer));
	}
	close(sockfd);
	return 0;
}
