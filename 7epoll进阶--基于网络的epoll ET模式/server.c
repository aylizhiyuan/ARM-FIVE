/* server.c */
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAXLINE 10
#define SERV_PORT 8080

int main(void)
{
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int listenfd, connfd;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];
	int i, efd;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	listen(listenfd, 20);

	struct epoll_event event;
	struct epoll_event resevent[10];
	int res, len;
	efd = epoll_create(10);
	event.events = EPOLLIN | EPOLLET;		/* ET 边沿触发 ，默认是水平触发 */

	printf("Accepting connections ...\n");
	cliaddr_len = sizeof(cliaddr);
	//这个例子只能跟一个客户端建立连接
	connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
	printf("received from %s at PORT %d\n",
			inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
			ntohs(cliaddr.sin_port));

	event.data.fd = connfd;
	//将客户端加入读事件监听中去
	epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);

	while (1) {
		//循环监听客户端的读事件，当有数据过来的时候，读取客户端的数据，并写到屏幕中去
		res = epoll_wait(efd, resevent, 10, -1);
		printf("res %d\n", res);
		if (resevent[0].data.fd == connfd) {
			len = read(connfd, buf, MAXLINE/2);
			write(STDOUT_FILENO, buf, len);
		}
	}
	return 0;
}
