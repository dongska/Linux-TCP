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

//编写进程模型的tcp服务端 长连接 多进程
//0.0.2实现僵尸进程的回收

/*子线程的信号捕捉函数,受到一个信号,回收所有线程*/
void sign_wait(int a)
{
	pid_t pid;
	while((pid = waitpid(-1,NULL,WNOHANG)) > 0)//使用非阻塞回收,没有僵尸返回0
	{
		printf("Wait thread 0x%x wait Z+[%d] success\n",(unsigned int)pthread_self(),pid);
	}
}

/*回收线程函数,创建时已将其设置为DETACH*/
void* thread(void* arg)
{
	struct sigaction act,oact;
	act.sa_handler = sign_wait;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	
	sigaction(SIGCHLD,&act,&oact);//设置信号捕捉函数
	sigprocmask(SIG_SETMASK,&act.sa_mask,NULL); //解除对SIGCHLD的屏蔽

	printf("Wait thread 0x%x waiting...\n",(unsigned int)pthread_self());
	while(1)
	{
		sleep(1);
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
	/*设置信号屏蔽,SIGCHLD,防止父进程产生信号中断,同时子线程继承后绑定捕捉函数*/
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask,SIGCHLD);
	sigprocmask(SIG_SETMASK,&act.sa_mask,NULL);
	/*创建回收子进程的线程*/
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int detachstate;
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);//设为分离态,住进程结束,系统回收子线程
	pthread_t tid;
	int err;
	if((err = pthread_create(&tid,&attr,thread,NULL) ) > 0)
	{
		printf("pthread_create call failed:%s\n",strerror(err));
		exit(0);
	}
		
	printf("TCP server running...\n");
	struct sockaddr_in client_addr; //客户端 网络信息结构体
	bzero(&client_addr,sizeof(client_addr));
	char respond[4096]; //回复的buffer
	bzero(respond,sizeof(respond));
	char clientIp[16]; //使用字符串存储客户ip
	bzero(clientIp,16);

	/*主任务.accept()*/
	while(1)
	{
		int clientfd;//客户端socket描述符
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

			/*创建对应客户端的子进程*/
			pid_t pid = fork();
			if(pid > 0)
			{
				printf("Parent process accepting...\n");
			}
			else if(pid == 0)
			{
				/*子进程工作*/
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
				/*如果子进程推出,recv返回0,关闭套接字,退出子进程*/
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


