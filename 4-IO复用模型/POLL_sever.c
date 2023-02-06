#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<sys/poll.h>

#define PORT 8080

/* 实现IO复用模型 POLL模型 自己定义结构体作为监听集合 */

int main()
{
	/* 监听结构体数组 */
	struct pollfd node[1024];
	for(int i=0;i<1024;i++)
	{
		node[i].fd = -1;
	}
	/* 网络初始化 */
	int sever_fd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sever_fd,(struct sockaddr*)&addr,sizeof(addr));
	listen(sever_fd,128);

	/* 添加sever_fd到监听结构体数组中*/
	node[0].fd = sever_fd;
	node[0].events = POLLIN;

	int Readycode;
	
	struct sockaddr_in clientAddr;
	int client_fd;
	bzero(&clientAddr,sizeof(clientAddr));

	char buffer[1024];
	bzero(buffer,sizeof(buffer));
 	socklen_t addrlen;
	char clientIp[16];
	printf("POLL sever running\n");
	while(1)
	{
		Readycode = poll(node,1024,-1); // poll 函数,

		while(Readycode) // 逐一处理所有就绪的
		{
			if(node[0].revents == POLLIN) // node[0] 表示sever_fd
			{
				client_fd = accept(sever_fd,(struct sockaddr*)&clientAddr,&addrlen);
				printf("client [%s] port [%d] connected.\n",inet_ntop(AF_INET,&clientAddr.sin_addr.s_addr,clientIp,16),ntohs(clientAddr.sin_port));
				for(int i = 1;i < 1024;i++) // 在监听数组中查找空位并添加客户端
				{
					if(node[i].fd == -1)
					{
						node[i].fd = client_fd;
						node[i].events = POLLIN;
						break;
					}
				}
				node[0].revents = 0;
				Readycode --;
			}
			else //客户端
			{
				for(int i=1;i<1024;i++) // 逐一遍历每一个sock_fd,
				{
					if(node[i].revents == POLLIN) // 此socket已经就绪v
					{
						int len = recv(node[i].fd,buffer,sizeof(buffer),0);
						if(len > 0)
						{
							int j = 0;
							while(j < len)
						 	{
								buffer[j] = toupper(buffer[j]);
								j++;
							}
							send(node[i].fd,buffer,sizeof(buffer),0);
							bzero(buffer,sizeof(buffer));
							node[i].revents = 0;
							Readycode --;
						}
						else if (len == 0)
						{
							printf("client fd [%d] exit.\n",node[i].fd);
							close(node[i].fd);
							node[i].fd = -1;
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
