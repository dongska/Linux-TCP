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
	/* 网络初始化 */
	int sever_fd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sever_fd,(struct sockaddr*)&addr,sizeof(addr));
	listen(sever_fd,128);

	fd_set listen_set,ready_set;
	FD_ZERO(&listen_set); //初始化监听集合
	FD_SET(sever_fd,&listen_set); // 将sever_fd添加到监听列表


	int Readycode;//已就绪的数量
	int Maxfd = sever_fd ; // 最大文件描述符

	
	struct sockaddr_in clientAddr;
	int client_fd;

	int client_arry[1021]; // 用来存储client_fd的数组,最多同时1021个,-1表示为空
	for(int i =0;i<1021;i++)
		client_arry[i] = -1;

	char buffer[1024];
	bzero(buffer,sizeof(buffer));
 	int addrlen;
	char clientIp[20];

	while(1)
	{
		ready_set = listen_set; // 备份listen_set , 避免越来越少
		Readycode = select(Maxfd + 1,&ready_set,NULL,NULL,NULL); // 一轮select 
		
		while(Readycode) // 逐一处理所有就绪的
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
				if(Maxfd < client_fd) // 如果大于当前最大文件描述符
					Maxfd = client_fd;
				FD_SET(client_fd,&listen_set);
				FD_CLR(sever_fd,&ready_set);
				Readycode --;
			}
			else //客户端
			{
				for(int i=0;i<1021;i++) // 逐一遍历每一个sock_fd,
				{
					if(client_arry[i] != -1 && FD_ISSET(client_arry[i],&ready_set)) // 如果当前数组中当前位不为-1,并且此socket已经就绪v
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
