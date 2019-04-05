#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "wrap.h"
#define SERV_PORT 6666
int main(int argc,char *argv[]){
    int i,j,n,maxi;
    //将我们的客户端的文件描述符存在这个数组里面，防止遍历1024个文件描述符
    int nready,client[FD_SETSIZE];
    //
    int maxfd,listenfd,connfd,sockfd;
    //读数据的时候用到的缓冲区
    char buf[BUFSIZ],str[INET_ADDRSTRLEN];
    struct sockaddr_in clie_addr,serv_addr;
    socklen_t clie_addr_len;
    //这是个读事件文件描述符的集合
    fd_set rset,allset;
    //首先建立连接
    listenfd = Socket(AF_INET,SOCK_STREAM,0);
    //现将我们的配置参数里面的数据清零
    bzero(&serv_addr,sizeof(serv_addr));
    //配置一下我们建立连接的IP地址和端口号
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //建立连接的Bind阶段
    Bind(listenfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    //监听的最大数量
    Listen(listenfd,20);
    //listenfd是我们的建立连接的文件描述符，先保存起来
    maxfd = listenfd;
    //将来用作client[]的下标
    maxi = -1;
    //将数组里面所有的元素都赋值为-1
    for(i=0;i<FD_SETSIZE;i++){
        client[i] = -1;
    }
    //将allset清空
    FD_ZERO(&allset);
    //把listenfd加入到allset中
    FD_SET(listenfd,&allset);
    while(1){
        //第一次循环allset里面只有服务端的文件描述符 
        //第二次循环的时候allset里面就有服务端和客户端的文件描述符了
        rset = allset;
        //将我们的服务器的文件描述符添加到可读事件集合中，监听它的变化
        //以后每次我们都会调用select函数监听服务器的可读事件，判断是否有新的连接过来

        //第二次循环的时候监听客户端和服务端的读事件
        nready = select(maxfd+1,&rset,NULL,NULL,NULL);
        if(nready < 0){
            perr_exit("select error");
        }
        //判断一下服务器是否发生了可读的操作
        //是否有客户端向服务器发出连接请求
        //以后一旦有新的客户端发来了连接请求，我们就直接运行下面的代码

        //第二次循环的时候，此段代码不再执行
        if(FD_ISSET(listenfd,&rset)){
            clie_addr_len = sizeof(clie_addr);
            //掉accpet跟客户端建立连接，并且不会阻塞
            //经历三次握手后，返回一个新的文件描述符
            connfd = Accept(listenfd,(struct sockaddr *)&clie_addr,&clie_addr_len);
            printf("received form %s at port %d\n",
            inet_ntop(AF_INET,&clie_addr.sin_addr,str,sizeof(str)),
            ntohs(clie_addr.sin_port));
            //将所有的已经建立连接的客户端的文件描述符都添加到client数组里面
            //假设数组中某个下标中已经有文件描述符了，就不再存入了
            for(i=0;i<FD_SETSIZE;i++){
                if(client[i]<0){
                    client[i] = connfd;
                    break;
                }
            }
            //判断是否会超过1024个连接
            if(i == FD_SETSIZE){
                fputs("to many clients\n",stderr);
                exit(1);
            }
            //将客户端的文件描述符放入allset集合中
            //里面有服务器的文件描述符和客户端的文件描述符
            FD_SET(connfd,&allset);
            //将客户顿的文件描述符保存在maxfd中
            //之前maxfd的值是listenfd的值
            //后面如果有客户端进来的话，那么它的最大值也应该相应的发生变化
            if(connfd > maxfd){
                maxfd = connfd;
            }
            //保存一下存储了文件描述符的数组的下标maxi
            //保证maxi存的总是client[]最后一个元素的下标
            if(i > maxi){
                maxi = i;
            }
            //先放着....
            if(--nready == 0){
                continue;
            }
        }
        //上边这段的代码是为了监听服务器socket的可读事件，是否有客户端连接
        //并且把新生成的文件描述符加入到client[]中去

        //循环所有的客户端连接
        for(i=0;i<=maxi;i++){
            //将每一个客户端连接的文件描述符赋值给sockfd
            //如果小于0的话就继续下次循环，也就是循环下一个客户端连接
            if((sockfd = client[i]) < 0){
                continue;
            }
            //如果是建立连接的阶段的话，&rset里面只有listenfd,所以下面的代码不会执行
            //只有当连接建立后，再次去执行while循环的时候rset = allset
            //此时，allset里面，有一个服务端文件描述符和客户端文件描述符

            //当第二次循环来的时候，那么，已经连接上的客户端发来的了数据
            //监听到了他的读事件
            if(FD_ISSET(sockfd,&rset)){
                //读取客户端发来的数据
                if((n=Read(sockfd,buf,sizeof(buf))) == 0){
                    Close(sockfd);
                    //把我要监听的客户端从集合中清除
                    FD_CLR(sockfd,&allset);
                    //客户端集合中也清空
                    client[i] = -1;
                }else if(n > 0){
                    //实际读到的字节数
                    for(j=0;j<n;j++){
                        buf[j] = toupper(buf[j]);
                    }
                    sleep(10);
                    //写给客户端
                    Write(sockfd,buf,n);
                }
                if(--nready == 0){
                    break;
                }
            }
        } 
    }
    close(listenfd);
}