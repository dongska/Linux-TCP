#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<ctype.h>

#define PORT 9965
#define BACKLOG 128

//编写进程模型的tcp服务端 hi

int main()
{
	/*创建套接字*/
	int sockfd;		
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket call failed");
		exit(0);
	}
	/*绑定IP/端口*/
	struct sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sockfd,(struct sockaddr*)&addr,sizeof(addr)) == -1)
	{
		perror("bind call failed");
		exit(0);
	}

	if(listen(sockfd,BACKLOG) == -1)
	{
		perror("listen call failed");
		exit(0);
	}
	printf("TCP server running...\n");
	struct sockaddr_in client_addr;
	bzero(&client_addr,sizeof(client_addr));
	char respond[4096];
	bzero(respond,sizeof(respond));
	char clientIp[16];
	bzero(clientIp,16);
	while(1)
	{
		int clientfd;
		int len = sizeof(client_addr);
		if((clientfd = accept(sockfd,(struct sockaddr*)&client_addr,&len) ) > 0)
		{
			
			inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,clientIp,16);
			int clientPort = ntohs(client_addr.sin_port);
			printf("Client [%s] port:[%d] connected\n",clientIp,clientPort);         
			sprintf(respond,"Hi %s,this is TCP test Server",clientIp);
			if(send(clientfd,(void*)respond,strlen(respond),0) == -1)
			{
				perror("send call failed");

			}
			else
				printf("Send back to [%s] success\n",clientIp);

			
			pid_t pid = fork();
			if(pid > 0)
			{
				printf("Parent process accepting...\n");
			}
			else if(pid == 0)
			{
				int len ;
				bzero(respond,sizeof(respond));
				while((len = recv(clientfd,respond,sizeof(respond),0)) > 0)
				{
					int j = 0;
					while(j < len)
					{
						respond[j] = toupper(respond[j]);
						j++;
					}
					send(clientfd,(void*)respond,strlen(respond),0);
					printf("Send back to [%s] success\n",clientIp);
					bzero(respond,sizeof(respond));
				}
				if(len == 0)
				{
					close(clientfd);
					exit(0);
				}
			}
			else
			{
				perror("fork call failed");
				exit(0);
			}
		}
		else
		{
			perror("accept call fail");
			exit(0);
		}
		
	}
	return 0;
}


