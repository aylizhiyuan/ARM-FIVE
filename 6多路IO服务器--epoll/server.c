#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include "wrap.h"

#define MAXLINE 80
#define SERV_PORT 6666
#define OPEN_MAX 1024

int main(int argc, char *argv[])
{
	int i, j, maxi, listenfd, connfd, sockfd;
	int nready, efd, res;
	ssize_t n;
	char buf[MAXLINE], str[INET_ADDRSTRLEN];
	socklen_t clilen;
	int client[OPEN_MAX];
	struct sockaddr_in cliaddr, servaddr;
	struct epoll_event tep, ep[OPEN_MAX];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	Listen(listenfd, 20);
	//初始化我们存储客户端的数组
	for (i = 0; i < OPEN_MAX; i++)
		client[i] = -1;
	maxi = -1;
	//创建我们的epoll模型，传递建议最大的监听数量
	efd = epoll_create(OPEN_MAX);
	if (efd == -1)
		perr_exit("epoll_create");
	//监听的结构体赋值，监听结构体的监听类型和监听的文件描述符
	tep.events = EPOLLIN; tep.data.fd = listenfd;
	//调用ctl函数进行监听的参数设置,将listenfd添加到监听节点中去
	res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
	if (res == -1)
		perr_exit("epoll_ctl");

	while (1) {
		//刚开始的时候只监听服务器的读事件
		//eq是我们返回的发生事件的所有fd的集合，刚开始的时候是0
		//下次我们监听的就是服务器和客户端所有的读事件了
		nready = epoll_wait(efd, ep, OPEN_MAX, -1); /* 阻塞监听 */
		if (nready == -1)
			perr_exit("epoll_wait");
		//第一次的时候只要服务器发生了可读事件---有客户端发出请求连接，我们就循环eq数组
		//第二次的时候有可能是服务端读事件也有可能是客户端读事件
		for (i = 0; i < nready; i++) {
			//判断一下如果不是读事件的话下面的代码将不再执行
			//继续下一个数组的成员
			if (!(ep[i].events & EPOLLIN))
				continue;
			//如果是服务器的读事件发生的话	
			if (ep[i].data.fd == listenfd) {
				clilen = sizeof(cliaddr);
				//接收连接，并且将我们的客户端文件描述符放到client数组里面
				connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
				printf("received from %s at PORT %d\n", 
						inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), 
						ntohs(cliaddr.sin_port));
				for (j = 0; j < OPEN_MAX; j++) {
					if (client[j] < 0) {
						client[j] = connfd; /* save descriptor */
						break;
					}
				}

				if (j == OPEN_MAX)
					perr_exit("too many clients");
				if (j > maxi)
					maxi = j; 		/* max index in client[] array */
				//设置客户端的事件和文件描述符
				tep.events = EPOLLIN; 
				tep.data.fd = connfd;
				//将客户端的连接加入到我们的监听集合中去
				res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
				if (res == -1)
					perr_exit("epoll_ctl");
			} else {
				//如果是客户端的读事件发生的话
				sockfd = ep[i].data.fd;
				n = Read(sockfd, buf, MAXLINE);
				//客户端关闭了连接，读到了0字节
				if (n == 0) {
					for (j = 0; j <= maxi; j++) {
						if (client[j] == sockfd) {
							client[j] = -1;
							break;
						}
					}
					//将客户端从监听节点中去掉
					res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
					if (res == -1)
						perr_exit("epoll_ctl");

					Close(sockfd);
					printf("client[%d] closed connection\n", j);
				} else {
					//否则读取客户端的数据，写回给客户端
					for (j = 0; j < n; j++)
						buf[j] = toupper(buf[j]);
					Writen(sockfd, buf, n);
				}
			}
		}
	}
	close(listenfd);
	close(efd);
	return 0;
}
