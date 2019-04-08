#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>

#define MAXLINE 10

int main(int argc, char *argv[])
{
	int efd, i;
	int pfd[2];
	pid_t pid;
	char buf[MAXLINE], ch = 'a';
	//创建一个无名的管道
	pipe(pfd);
	pid = fork();
	//目前父子进程共享无名管道的读端和写端
	//我让子进程负责写端
	if (pid == 0) {
		//子进程关闭读端
		close(pfd[0]);
		while (1) {
			//循环是buf里面写5个a
			for (i = 0; i < MAXLINE/2; i++)
				buf[i] = ch;
			//循环后向里面写入了4个a和一个换行	
			buf[i-1] = '\n';
			ch++;
			//bbbb\n
			for (; i < MAXLINE; i++)
				buf[i] = ch;
			buf[i-1] = '\n';
			ch++;
			// aaaa\nbbbb\n
			//向管道里面写入上面的数据
			write(pfd[1], buf, sizeof(buf));
			//睡眠5秒
			sleep(5);
		}
		close(pfd[1]);
	} else if (pid > 0) {
		//父进程负责读端
		struct epoll_event event;
		struct epoll_event resevent[10];
		int res, len;
		close(pfd[1]);
		//创建要监听的文件描述符的个数
		efd = epoll_create(10);
		/* event.events = EPOLLIN; */
		event.events = EPOLLIN | EPOLLET;		/* ET 边沿触发 ，默认是水平触发 */
		event.data.fd = pfd[0];//监听读端的文件描述符
		epoll_ctl(efd, EPOLL_CTL_ADD, pfd[0], &event);
		while (1) {
			//当子进程写完数据后，满足管道读端的事件触发
			res = epoll_wait(efd, resevent, 10, -1);
			printf("res %d\n", res);
			if (resevent[0].data.fd == pfd[0]) {
				//将管道里面的数据读出来，一次只读5个
				len = read(pfd[0], buf, MAXLINE/2);
				//将它写到屏幕上
				write(STDOUT_FILENO, buf, len);
			}
		}
		close(pfd[0]);
		close(efd);
	} else {
		perror("fork");
		exit(-1);
	}
	return 0;
}
