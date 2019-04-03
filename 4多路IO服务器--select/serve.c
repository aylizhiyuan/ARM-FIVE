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
    int nready,client[FD_SETSIZE];
    int maxfd,listenfd,connfd,sockfd;
    char buf[BUFSIZ],str[INET_ADDRSTRLEN];
    struct sockaddr_in clie_addr,serv_addr;
    socklen_t clie_addr_len;
    fd_set rset,allset;
    listenfd = Socket(AF_INET,SOCK_STREAM,0);
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listenfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    Listen(listenfd,20);
    maxfd = listenfd;

    maxi = -1;
    for(i=0;i<FD_SETSIZE;i++){
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd,&allset);
    while(1){
        rset = allset;
        nready = select(maxfd+1,&rset,NULL,NULL,NULL);
        if(nready < 0){
            perr_exit("select error");
        }
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
            FD_SET(connfd,&allset);
            if(connfd > maxfd){
                maxfd = connfd;
            }
            if(i > maxi){
                maxi = i;
            }
            if(--nready == 0){
                continue;
            }
        }
        for(i=0;i<=maxi;i++){
            if((sockfd = client[i]) < 0){
                continue;
            }
            if(FD_ISSET(sockfd,&rset)){
                if((n=Read(sockfd,buf,sizeof(buf))) == 0){
                    Close(sockfd);
                    FD_CLR(sockfd,&allset);
                    client[i] = -1;
                }else if(n > 0){
                    for(j=0;j<n;j++){
                        buf[j] = toupper(buf[j]);
                    }
                    sleep(10);
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