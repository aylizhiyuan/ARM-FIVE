#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>

#define SERVER_PORT 6666
#define CLIENT_PORT 9000
#define MAXLINE 1500
#define GROUP "239.0.0.2"

int main(void)
{
	int sockfd, i ;
	struct sockaddr_in serveraddr, clientaddr;
	char buf[MAXLINE] = "itcast\n";
	char ipstr[INET_ADDRSTRLEN]; /* 16 Bytes */
	socklen_t clientlen;
	ssize_t len;
	struct ip_mreqn group;

	/* 构造用于UDP通信的套接字 */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET; /* IPv4 */
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 本地任意IP INADDR_ANY = 0 */
	serveraddr.sin_port = htons(SERVER_PORT);

	bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	/*设置组地址*/
	inet_pton(AF_INET, GROUP, &group.imr_multiaddr);
	/*本地任意IP*/
	inet_pton(AF_INET, "0.0.0.0", &group.imr_address);
	/* eth0 --> 编号 命令：ip ad */
	group.imr_ifindex = if_nametoindex("eth0");
	setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &group, sizeof(group));

	/*构造 client 地址 IP+端口 */
	bzero(&clientaddr, sizeof(clientaddr));
	clientaddr.sin_family = AF_INET; /* IPv4 */
	inet_pton(AF_INET, GROUP, &clientaddr.sin_addr.s_addr);
	clientaddr.sin_port = htons(CLIENT_PORT);

	while (1) {
		//fgets(buf, sizeof(buf), stdin);
		sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
		sleep(1);
	}
	close(sockfd);
	return 0;
}
