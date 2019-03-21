#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666
int main(void){
    int lfd,cfd;
    struct sockaddr_in serv_addr,clie_addr;
    socklen_t clie_addr_len;
    char buf[BUFSIZ];
    int n,i;
    //创建一个套接字，传递给bind和listen
    lfd = socket(AF_INET,SOCK_STREAM,0);
    //参数初始化
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);//将端口转化为网络字节序
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);//将IP转化为网络字节序
    bind(lfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    //设定连接上限值
    listen(lfd,128);
    clie_addr_len = sizeof(clie_addr);
    //阻塞等待客户端建立连接，返回一个新的文件描述符，指向客户端那边的socket
    cfd = accept(lfd,(struct sockaddr *)&clie_addr,&clie_addr_len);
    while(1){
        n = read(cfd,buf,sizeof(buf));
        for(i=0;i<n;i++){
            buf[i] = toupper(buf[i]);
        }
        write(cfd,buf,n);
    }
    close(lfd);
    close(cfd);
    return 0;
}