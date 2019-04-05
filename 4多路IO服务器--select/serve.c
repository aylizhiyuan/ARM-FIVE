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
        //每次循环的时候都会将新的服务器listenfd和客户端n个fd放入监听队列中去
        rset = allset;
        //第一次监听服务器listnefd的读事件，以后监听所有已经建立好连接的客户端fd和服务器listenfd的读事件
        nready = select(maxfd+1,&rset,NULL,NULL,NULL);
        if(nready < 0){
            perr_exit("select error");
        }
        //只要有新的连接请求过来，我们就持续不断的将新的客户端加入到我们的监听集合中来
        //如果是已经建立连接的客户端的读事件发生了，我们压根也不用执行了，直接去读取客户端的数据就可以了。
        if(FD_ISSET(listenfd,&rset)){
            clie_addr_len = sizeof(clie_addr);
            connfd = Accept(listenfd,(struct sockaddr *)&clie_addr,&clie_addr_len);
            printf("received form %s at port %d\n",
            inet_ntop(AF_INET,&clie_addr.sin_addr,str,sizeof(str)),
            ntohs(clie_addr.sin_port));
            for(i=0;i<FD_SETSIZE;i++){
                if(client[i]<0){
                    client[i] = connfd;
                    break;
                }
            }
            if(i == FD_SETSIZE){
                fputs("to many clients\n",stderr);
                exit(1);
            }
            //将新的客户端的文件描述符也添加到读事件的集合中来
            FD_SET(connfd,&allset);
            if(connfd > maxfd){
                maxfd = connfd;
            }
            if(i > maxi){
                maxi = i;
            }
            //客户端a建立好连接后继续循环
            //客户端b建立好连接后继续循环
            //......
            if(--nready == 0){
                continue;
            }
        }
        //下面的代码非常的号理解，就是依次循环发生读事件的客户端进行数据处理
        for(i=0;i<=maxi;i++){
            if((sockfd = client[i]) < 0){
                continue;
            }
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