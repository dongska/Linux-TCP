#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<pthread.h>
#include<signal.h>
#include<wait.h>

#define PORT 9965
#define BACKLOG 128

//编写多线程模型的tcp服务端 长连接 

/*子线程函数,创建时已将其设置为DETACH*/
void* thread(void* arg)
{
	printf("Child Thread [0x%x] running...\n",(unsigned int)pthread_self());
	int clientfd = *(int*)arg;
	int len ;
	char respond[4096];
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
					printf("Child thread [0x%x] Send back success\n",(unsigned int)pthread_self());
					bzero(respond,sizeof(respond));
	}

	pthread_exit(NULL);
}

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
	/*设置最大连接数量*/
	if(listen(sockfd,BACKLOG) == -1)
	{
		perror("listen call failed");
		exit(0);
	}
		
	printf("TCP server running...\n");
	struct sockaddr_in client_addr; //客户端 网络信息结构体
	bzero(&client_addr,sizeof(client_addr));
	char respond[4096]; //回复的buffer
	bzero(respond,sizeof(respond));
	char clientIp[16]; //使用字符串存储客户ip
	bzero(clientIp,16);
	int clientfd;//客户端socket描述符

	/*主任务.accept()*/
	while(1)
	{
		int len = sizeof(client_addr);
		/*阻塞等待客户端连接*/
		if((clientfd = accept(sockfd,(struct sockaddr*)&client_addr,&len) ) > 0)
		{	
			inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,clientIp,16); //大段序ip转字符串
			int clientPort = ntohs(client_addr.sin_port); //大端序端口号转int
			printf("Client [%s] port:[%d] connected\n",clientIp,clientPort);         
			sprintf(respond,"Hi %s,this is TCP test Server",clientIp);
			if(send(clientfd,(void*)respond,strlen(respond),0) == -1)
			{
				perror("send call failed");

			}
			else
				printf("Send back to [%s] success\n",clientIp);

			/*创建对应客户端的线程*/
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			int detachstate;
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);//设为分离
			pthread_t tid;
			int err;
			if((err = pthread_create(&tid,&attr,thread,(void*)&clientfd)) > 0)
			{
				printf("pthread_create call failed:%s\n",strerror(err));
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


