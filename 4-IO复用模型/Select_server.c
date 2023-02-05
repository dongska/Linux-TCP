#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<ctype.h>

#define PORT 8080

/* 实现IO复用模型 select */

int main()
{
	int sever_fd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sever_fd,(struct sockaddr*)&addr,sizeof(addr));
	listen(sever_fd,128);

	fd_set listen_set;
	FD_ZERO(&listen_set); //初始化监听集合
	FD_SET(sever_fd,&listen_set); // 将sever_fd添加到监听列表


	int Readycode;
	int Maxfd = sever_fd ;
	fd_set ready_set;
	struct sockaddr_in clientAddr;
	int client_fd;
	int client_arry[1021];
	for(int i =0;i<1021;i++)
		client_arry[i] = -1;
	char buffer[1024];
	bzero(buffer,sizeof(buffer));
 	int addrlen;
	char clientIp[20];
	while(1)
	{
		ready_set = listen_set;
		Readycode = select(Maxfd + 1,&ready_set,NULL,NULL,NULL); // 一轮select 
		
		while(Readycode)
		{
			if(FD_ISSET(sever_fd,&ready_set)) // severfd的
			{
				client_fd = accept(sever_fd,(struct sockaddr*)&clientAddr,&addrlen);
				printf("client [%s] port [%d] connected.\n",inet_ntop(AF_INET,&clientAddr.sin_addr.s_addr,clientIp,16),ntohs(clientAddr.sin_port));
				for(int i=0;i<1021;i++)
				{
					if(client_arry[i] == -1)
					{
						client_arry[i] = client_fd;
						break;
					}
				}
				if(Maxfd < client_fd)
					Maxfd = client_fd;
				FD_SET(client_fd,&listen_set);
				FD_CLR(sever_fd,&ready_set);
				Readycode --;
			}
			else //客户端
			{
				for(int i=0;i<1021;i++)
				{
					if(client_arry[i] != -1 && FD_ISSET(client_arry[i],&ready_set))
					{
						int len = recv(client_arry[i],buffer,sizeof(buffer),0);
						if(len > 0)
						{
							int j = 0;
							while(j < len)
							{
								buffer[j] = toupper(buffer[j]);
								j++;
							}
							send(client_arry[i],buffer,sizeof(buffer),0);
							bzero(buffer,sizeof(buffer));
							FD_CLR(client_arry[i],&ready_set);
							Readycode --;
						}
						else if (len == 0)
						{
							printf("client fd [%d] exit.\n",client_arry[i]);
							FD_CLR(client_arry[i],&ready_set);
							FD_CLR(client_arry[i],&listen_set);
							close(client_arry[i]);
							client_arry[i] = -1;
							Readycode --;
						}
						else
						{
							perror("recv call failed");
							exit(0);
						}
					}
				}
			}
		}
	}

}
