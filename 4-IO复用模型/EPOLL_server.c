#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<sys/poll.h>
#include<sys/epoll.h>

#define LISTEN_MAX 4096
#define PORT 8080

/* 实现IO复用模型 EPOLL模型  */

int main()
{
	/* 就绪队列结构体数组 */
	struct epoll_event readyArr[LISTEN_MAX];
	/* 创建epoll红黑树 */
	int epollfd = epoll_create(LISTEN_MAX);

	/* 网络初始化 */
	int sever_fd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sever_fd,(struct sockaddr*)&addr,sizeof(addr));
	listen(sever_fd,128);

	/* 将server_fd 添加到树上 */
	struct epoll_event node; // 定义叶子属性结构体
	node.data.fd = sever_fd;
	node.events = EPOLLIN;
	epoll_ctl(epollfd,EPOLL_CTL_ADD,sever_fd,&node);

	int Readycode;
	
	struct sockaddr_in clientAddr;
	int client_fd;
	bzero(&clientAddr,sizeof(clientAddr));

	char buffer[1024];
	bzero(buffer,sizeof(buffer));
 	socklen_t addrlen;
	char clientIp[16];
	printf("EPOLL sever running\n");
	while(1)
	{
		/* 循环 epoll将就绪的节点放在readyArr中  */
		Readycode = epoll_wait(epollfd,readyArr,LISTEN_MAX,-1);
		for(int k =0;k<Readycode;k++) // 逐一处理所有就绪的
		{
			if(readyArr[k].data.fd == sever_fd) // sever_fd
			{
				client_fd = accept(sever_fd,(struct sockaddr*)&clientAddr,&addrlen);
				printf("client [%s] port [%d] connected.\n",inet_ntop(AF_INET,&clientAddr.sin_addr.s_addr,clientIp,16),ntohs(clientAddr.sin_port));
				/* 将clientfd作为节点加到树上*/
				node.data.fd = client_fd;
				epoll_ctl(epollfd,EPOLL_CTL_ADD,client_fd,&node);
			}
			else //客户端
			{
					
				
						int len = recv(readyArr[k].data.fd,buffer,sizeof(buffer),0);
						if(len > 0)
						{
							int j = 0;
							while(j < len)
						 	{
								buffer[j] = toupper(buffer[j]);
								j++;
							}
							send(readyArr[k].data.fd,buffer,sizeof(buffer),0);
							bzero(buffer,sizeof(buffer));
						}
						else if (len == 0)
						{
							printf("client fd [%d] exit.\n",readyArr[k].data.fd);
							close(readyArr[k].data.fd);
							epoll_ctl(epollfd,EPOLL_CTL_DEL,readyArr[k].data.fd,NULL);
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
