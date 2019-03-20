# liunx网络编程笔记

1. 数据的封包 

数据在网络传输过程中，不能直接扔到网络中去，必须由操作系统加上应用层的首部、TCP首部、IP首部、以太网帧首部、尾部（注意，只有这里是有尾部的）
后，再扔到网络中进行传输，传输到目标计算机后，再由目标计算机的操作系统进行解封，去掉以太网帧首部、尾部、IP首部、TCP首部、应用层首部最终得到数据部分。这所有的操作都是由操作系统中的内核来完成的，应用层例外，应用层通常由浏览器、或其他应用程序来操作。

2. 路由选路的一般思路

其实这部分就是数据从网卡出来的时候，应该如何达到目的地。原则上是路由器根据IP地址寻路的。    
每一个路由器中都有一个路由表，记录着目标IP地址所对应的下一跳地址，路由器就根据自己的路由表选择下一跳的路由节点。
TCP协议的路径一旦确定就不会再改变了，UDP每次发送数据的时候都会重新进行选路，每次的路径可能都不一样

3. 以太网帧格式

包括6字节的目标MAC地址、6字节的源MAC地址、2字节的类型（首部）和4个字节的校验（尾部），剩余的就是数据部分，46-1500个字节的数据，保证最小的帧大小不会低于64个字节，如果数据部分不够46个字节的话，会做一部分的填充，保证整个帧不低于64个字节，接收端在接收的时候会把添加的字节去掉。所以理论上以太网帧的大小是64-1518（加上首部和尾部字段）

帧还需要加上帧开始符和帧结束符，但是对于以太网帧格式来讲的话，它是没有帧结束符的，它有7个字节的前同步码和1个字节的帧开始符

4. arp协议

ARP请求总共的长度是46字节（加上帧首部和帧尾部正好是最小的帧大小）。包括28字节的请求报文/响应报文 以及18字节的填充

包括2字节的硬件类型、2字节的协议类型、1字节的硬件地址长度、1字节的协议地址长度、2字节的op（请求或者应答）、6字节的发送端以太网地址、4字节的发送端IP地址、6字节的目标以太网地址、4字节的目标IP地址

源MAC地址和目标MAC地址出现了两次，分别是在以太网首部和ARP请求中，对于链路层为以太网的情况是多余的，但是如果链路层是其他的类型的网络则有可能是必要的。这点代为考察？为什么？

    请求帧如下（为了清晰在每行的前面加了字节计数，每行16个字节）：
    以太网首部（14字节）
    0000: ff ff ff ff ff ff 00 05 5d 61 58 a8 08 06
    ARP帧（28字节）
    0000: 00 01
    0010: 08 00 06 04 00 01 00 05 5d 61 58 a8 c0 a8 00 37
    0020: 00 00 00 00 00 00 c0 a8 00 02
    填充位（18字节）
    0020: 00 77 31 d2 50 10
    0030: fd 78 41 d3 00 00 00 00 00 00 00 00

> 以太网首部：目标MAC地址采用广播地址ff:ff:ff:ff:ff:ff，源MAC地址为 00:05:5d:61:58:a8，0806表示上层是arp协议    

> ARP帧：硬件类型0x0001表示以太网，协议类型0x0800表示IP协议，硬件地址长度为0x06,协议地址长度为0x04,op为0x0001表示是一个请求ARP，源MAC地址是00:05:5d:61:58:a8,源IP地址是c0 a8 00 37(192.168.0.55),目标MAC地址全0待填写，目标IP地址为c0 a8 00 02 (192.168.0.55)

    应答帧如下：
    以太网首部
    0000: 00 05 5d 61 58 a8 00 05 5d a1 b8 40 08 06
    ARP帧
    0000: 00 01
    0010: 08 00 06 04 00 02 00 05 5d a1 b8 40 c0 a8 00 02
    0020: 00 05 5d 61 58 a8 c0 a8 00 37
    填充位
    0020: 00 77 31 d2 50 10
    0030: fd 78 41 d3 00 00 00 00 00 00 00 00

> 以太网首部：目的主机的MAC地址是00:05:5d:61:58:a8，源主机的MAC地址是00:05:5d:a1:b8:40，上层协议类型0x0806表示ARP。

> ARP帧：硬件类型0x0001表示以太网，协议类型0x0800表示IP协议，硬件地址（MAC地址）长度为6，协议地址（IP地址）长度为4，op为0x0002表示应答，源主机MAC地址为00:05:5d:a1:b8:40，源主机IP地址为c0 a8 00 02（192.168.0.2），目的主机MAC地址为00:05:5d:61:58:a8，目的主机IP地址为c0 a8 00 37（192.168.0.55）。

###### 思考题：如果源IP地址和目标IP地址不在同一个网段的话，ARP请求广播是无法穿透路由器的？那么他们如何通信？

5. IP数据包

包括4位的版本号、4位首部长度（以4字节为单位，最小值为5，最大值是15，20-60字节）、8位TOS字段、16位的总长度（包括首部和数据部分，最大65535）、16位的标识（用于重新组装分片数据包）、3位标志、13位片偏移（用于分片）、8位TTL时间、8位协议（上层协议TCP/UDP）、16位首部校验和、32位源IP地址、32位目标IP地址、剩余option、最后是数据

目前认为如果上层协议是TCP协议的话，那么IP数据包的大小基本都是不超过1500个字节的，如果大于的话，可能会分片的，所以尽量不分片的情况下，IP数据包的大小不会超过1500个字节。*IP包数据最大也就1480*

6. UDP数据包

包括16位的源端口号、16位的目标端口号、16位的UDP长度、16位的UDP检验和、数据

    下面分析一帧基于UDP的TFTP协议帧。
    以太网首部
    0000: 00 05 5d 67 d0 b1 00 05 5d 61 58 a8 08 00
    IP首部
    0000: 45 00
    0010: 00 53 93 25 00 00 80 11 25 ec c0 a8 00 37 c0 a8
    0020: 00 01
    UDP首部
    0020： 05 d4 00 45 00 3f ac 40
    TFTP协议
    0020: 00 01 'c'':''\''q'
    0030: 'w''e''r''q''.''q''w''e'00 'n''e''t''a''s''c''i'
    0040: 'i'00 'b''l''k''s''i''z''e'00 '5''1''2'00 't''i'
    0050: 'm''e''o''u''t'00 '1''0'00 't''s''i''z''e'00 '0'

> 以太网首部：源MAC地址是00:05:5d:61:58:a8，目的MAC地址是00:05:5d:67:d0:b1，上层协议类型0x0800表示IP。

> IP首部：每一个字节0x45包含4位版本号和4位首部长度，版本号为4，即IPv4，首部长度为5，说明IP首部不带有选项字段。服务类型为0，没有使用服务。16位总长度字段（包括IP首部和IP层payload的长度）为0x0053，即83字节，加上以太网首部14字节可知整个帧长度是97字节。IP报标识是0x9325，标志字段和片偏移字段设置为0x0000，就是DF=0允许分片，MF=0此数据报没有更多分片，没有分片偏移。TTL是0x80，也就是128。上层协议0x11表示UDP协议。IP首部校验和为0x25ec，源主机IP是c0 a8 00 37（192.168.0.55），目的主机IP是c0 a8 00 01（192.168.0.1）。

> UDP首部：源端口号0x05d4（1492）是客户端的端口号，目的端口号0x0045（69）是TFTP服务的well-known端口号。UDP报长度为0x003f，即63字节，包括UDP首部和UDP层pay-load的长度。UDP首部和UDP层payload的校验和为0xac40。

TFTP是基于文本的协议，各字段之间用字节0分隔，开头的00 01表示请求读取一个文件，接下来的各字段是：    
        c:\qwerq.qwe    
        netascii    
        blksize 512 
        timeout 10  
        tsize 0     

一般的网络通信都是像TFTP协议这样，通信的双方分别是客户端和服务器，客户端主动发起请求（上面的例子就是客户端发起的请求帧），而服务器被动地等待、接收和应答请求。客户端的IP地址和端口号唯一标识了该主机上的TFTP客户端进程，服务器的IP地址和端口号唯一标识了该主机上的TFTP服务进程，由于客户端是主动发起请求的一方，它必须知道服务器的IP地址和TFTP服务进程的端口号，所以，一些常见的网络协议有默认的服务器端口，例如HTTP服务默认TCP协议的80端口，FTP服务默认TCP协议的21端口，TFTP服务默认UDP协议的69端口（如上例所示）。在使用客户端程序时，必须指定服务器的主机名或IP地址，如果不明确指定端口号则采用默认端口，请读者查阅ftp、tftp等程序的man page了解如何指定端口号。/etc/services中列出了所有well-known的服务端口和对应的传输层协议，这是由IANA（Internet Assigned Numbers Authority）规定的，其中有些服务既可以用TCP也可以用UDP，为了清晰，IANA规定这样的服务采用相同的TCP或UDP默认端口号，而另外一些TCP和UDP的相同端口号却对应不同的服务。

很多服务有well-known的端口号，然而客户端程序的端口号却不必是well-known的，往往是每次运行客户端程序时由系统自动分配一个空闲的端口号，用完就释放掉，称为ephemeral的端口号，想想这是为什么？
前面提过，UDP协议不面向连接，也不保证传输的可靠性，例如：
发送端的UDP协议层只管把应用层传来的数据封装成段交给IP协议层就算完成任务了，如果因为网络故障该段无法发到对方，UDP协议层也不会给应用层返回任何错误信息。

接收端的UDP协议层只管把收到的数据根据端口号交给相应的应用程序就算完成任务了，如果发送端发来多个数据包并且在网络上经过不同的路由，到达接收端时顺序已经错乱了，UDP协议层也不保证按发送时的顺序交给应用层。

通常接收端的UDP协议层将收到的数据放在一个固定大小的缓冲区中等待应用程序来提取和处理，如果应用程序提取和处理的速度很慢，而发送端发送的速度很快，就会丢失数据包，UDP协议层并不报告这种错误。

因此，使用UDP协议的应用程序必须考虑到这些可能的问题并实现适当的解决方案，例如等待应答、超时重发、为数据包编号、流量控制等。一般使用UDP协议的应用程序实现都比较简单，只是发送一些对可靠性要求不高的消息，而不发送大量的数据。例如，基于UDP的TFTP协议一般只用于传送小文件（所以才叫trivial的ftp），而基于TCP的FTP协议适用于	各种文件的传输。TCP协议又是如何用面向连接的服务来代替应用程序解决传输的可靠性问题呢。

7. TCP包

包括16位源端口号、16位目标端口号、32位序号、32位确认序号、4位首部长度、6位保留位、6位标记位、16位窗口大小、16位校验和、16位紧急指针、option以及最后的数据部分（数据部分最大应该就是1460个字节）

8. 套接字

在liunx环境下，用于表示进程间网络通信的特殊文件类型。本质为内核借助缓存区形成的伪文件。     
在TCP/IP协议中，IP地址+端口号就对应着一个socket，建立连接的两个进程各自有一个socket来标识
因为是全双工的，所以，每一个socket都有两个缓冲区--读和写缓冲区

**网络字节序**

大端：低地址存在高位，小端：低地址存在低位，计算机的默认的存储方式是小端存储

TCP协议规定，网络数据流采用的是大端字节序。例如上一节的UDP段格式，地址0-1是16位的源端口号，如果这个端口号是1000，则地址0是0x03,地址1是oxe8,也就是先发0x03,再发0xe8,这16位在发送主机的缓冲区中也应该是低地址存0x03,高地址存0xe8.但是，如果发送的主机是小端字节序，这16位被解释成0xe803,而不是1000.因此，发送主机吧1000填到发送缓冲区之前需要做字节序的转换。

为了使网络程序具有可移植性，使同样的C代码在大端和小端计算机中编译后都可以正常运行，可以调用一下函数库做网络字节序和主机字节序的转换

        #include <arpa/inet.h>
        uint32_t htonl(uint32_t hostlong);
        uint16_t htons(uint16_t hostshort);
        uint32_t ntohl(uint32_t netlong);
        uint16_t ntohs(uint16_t netshort);

h 表示host,n表示network，l表示32位长整数，s表示16位短整数

**IP地址转换函数**

    #include <arpa/inet.h>
    int inet_pton(int af, const char *src, void *dst);
    const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);

**sockaddr数据结构**

用来描述IPv4地址协议的

    struct sockaddr_in {
        __kernel_sa_family_t sin_family; 			/* Address family */  	地址结构类型
        __be16 sin_port;					 		/* Port number */		端口号
        struct in_addr sin_addr;					/* Internet address */	IP地址
        /* Pad to size of `struct sockaddr'. */
        unsigned char __pad[__SOCK_SIZE__ - sizeof(short int) -
        sizeof(unsigned short int) - sizeof(struct in_addr)];
    };

    struct in_addr {						/* Internet address. */
        __be32 s_addr;
    };

9. 网络套接字函数

**socket函数**

    #include <sys/types.h> /* See NOTES */
    #include <sys/socket.h>
    int socket(int domain, int type, int protocol);
    domain:
        AF_INET 这是大多数用来产生socket的协议，使用TCP或UDP来传输，用IPv4的地址
        AF_INET6 与上面类似，不过是来用IPv6的地址
        AF_UNIX 本地协议，使用在Unix和Linux系统上，一般都是当客户端和服务器在同一台及其上的时候使用
    type:
        SOCK_STREAM 这个协议是按照顺序的、可靠的、数据完整的基于字节流的连接。这是一个使用最多的socket类型，这个socket是使用TCP来进行传输。
        SOCK_DGRAM 这个协议是无连接的、固定长度的传输调用。该协议是不可靠的，使用UDP来进行它的连接。
        SOCK_SEQPACKET该协议是双线路的、可靠的连接，发送固定长度的数据包进行传输。必须把这个包完整的接受才能进行读取。
        SOCK_RAW socket类型提供单一的网络访问，这个socket类型使用ICMP公共协议。（ping、traceroute使用该协议）
        SOCK_RDM 这个类型是很少使用的，在大部分的操作系统上没有实现，它是提供给数据链路层使用，不保证数据包的顺序
    protocol:
        传0 表示使用默认协议。
    返回值：
        成功：返回指向新创建的socket的文件描述符，失败：返回-1，设置errno

> socket()打开一个网络通讯端口，如果成功的话，就像open()一样返回一个文件描述符，应用程序可以像读写文件一样用read/write在网络上收发数据，如果socket()调用出错则返回-1。对于IPv4，domain参数指定为AF_INET。对于TCP协议，type参数指定为SOCK_STREAM，表示面向流的传输协议。如果是UDP协议，则type参数指定为SOCK_DGRAM，表示面向数据报的传输协议。protocol参数的介绍从略，指定为0即可。


**bind函数**

    #include <sys/types.h> /* See NOTES */
    #include <sys/socket.h>
    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    sockfd：
        socket文件描述符
    addr:
        构造出IP地址加端口号
    addrlen:
        sizeof(addr)长度
    返回值：
        成功返回0，失败返回-1, 设置errno

> 服务器程序所监听的网络地址和端口号通常是固定不变的，客户端程序得知服务器程序的地址和端口号后就可以向服务器发起连接，因此服务器需要调用bind绑定一个固定的网络地址和端口号。

bind()的作用是将参数sockfd和addr绑定在一起，使sockfd这个用于网络通讯的文件描述符监听addr所描述的地址和端口号。前面讲过，struct sockaddr *是一个通用指针类型，addr参数实际上可以接受多种协议的sockaddr结构体，而它们的长度各不相同，所以需要第三个参数addrlen指定结构体的长度。如：

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6666);

首先将整个结构体清零，然后设置地址类型为AF_INET，网络地址为INADDR_ANY，这个宏表示本地的任意IP地址，因为服务器可能有多个网卡，每个网卡也可能绑定多个IP地址，这样设置可以在所有的IP地址上监听，直到与某个客户端建立了连接时才确定下来到底用哪个IP地址，端口号为6666。

**listen函数**

    #include <sys/types.h> /* See NOTES */
    #include <sys/socket.h>
    int listen(int sockfd, int backlog);
    sockfd:
        socket文件描述符
    backlog:
        排队建立3次握手队列和刚刚建立3次握手队列的链接数和

> 典型的服务器程序可以同时服务于多个客户端，当有客户端发起连接时，服务器调用的accept()返回并接受这个连接，如果有大量的客户端发起连接而服务器来不及处理，尚未accept的客户端就处于连接等待状态，listen()声明sockfd处于监听状态，并且最多允许有backlog个客户端处于连接待状态，如果接收到更多的连接请求就忽略。listen()成功返回0，失败返回-1。

**accept函数**

    #include <sys/types.h> 		/* See NOTES */
    #include <sys/socket.h>
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    sockdf:
        socket文件描述符
    addr:
        传出参数，返回链接客户端地址信息，含IP地址和端口号
    addrlen:
        传入传出参数（值-结果）,传入sizeof(addr)大小，函数返回时返回真正接收到地址结构体的大小
    返回值：
        成功返回一个新的socket文件描述符，用于和客户端通信，失败返回-1，设置errno

三方握手完成后，服务器调用accept()接受连接，如果服务器调用accept()时还没有客户端的连接请求，就阻塞等待直到有客户端连接上来。addr是一个传出参数，accept()返回时传出客户端的地址和端口号。addrlen参数是一个传入传出参数（value-result argument），传入的是调用者提供的缓冲区addr的长度以避免缓冲区溢出问题，传出的是客户端地址结构体的实际长度（有可能没有占满调用者提供的缓冲区）。如果给addr参数传NULL，表示不关心客户端的地址。
我们的服务器程序结构是这样的：

    while (1) {
        cliaddr_len = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        n = read(connfd, buf, MAXLINE);
        ......
        close(connfd);
    }

> 整个是一个while死循环，每次循环处理一个客户端连接。由于cliaddr_len是传入传出参数，每次调用accept()之前应该重新赋初值。accept()的参数listenfd是先前的监听文件描述符，而accept()的返回值是另外一个文件描述符connfd，之后与客户端之间就通过这个connfd通讯，最后关闭connfd断开连接，而不关闭listenfd，再次回到循环开头listenfd仍然用作accept的参数。accept()成功返回一个文件描述符，出错返回-1。


**connect函数**

    #include <sys/types.h> 					/* See NOTES */
    #include <sys/socket.h>
    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    sockdf:
        socket文件描述符
    addr:
        传入参数，指定服务器端地址信息，含IP地址和端口号
    addrlen:
        传入参数,传入sizeof(addr)大小
    返回值：
        成功返回0，失败返回-1，设置errno

> 客户端需要调用connect()连接服务器，connect和bind的参数形式一致，区别在于bind的参数是自己的地址，而connect的参数是对方的地址。connect()成功返回0，出错返回-1。

10. TCP程序的一般流程思路总结

服务器调用socket()、bind()、listen()完成初始化后，调用accept()阻塞等待，处于监听端口的状态，客户端调用socket()初始化后，调用connect()发出SYN段并阻塞等待服务器应答，服务器应答一个SYN-ACK段，客户端收到后从connect()返回，同时应答一个ACK段，服务器收到后从accept()返回。

数据传输的过程：

建立连接后，TCP协议提供全双工的通信服务，但是一般的客户端/服务器程序的流程是由客户端主动发起请求，服务器被动处理请求，一问一答的方式。因此，服务器从accpet()返回后立刻调用read(),读socket就像读管道一样，如果没有数据达到就阻塞等待，这时候，客户端调用write()发送请求给服务器，服务器接收到后从read()返回，对客户端的请求进行处理，在此期间客户端调用read()阻塞等待服务器的应答，服务器调用write()将处理结果发回给客户端，再次调用read()函数阻塞等待下一条请求，客户端收到后从read()返回，发送下一条请求，如此循环下去。

如果客户端没有更多的请求了，就调用close()关闭连接，就像写端关闭的管道一样，服务器的read()返回0，这样服务器就知道客户端关闭了连接，也调用close()关闭连接。注意，任何一方调用close()后，连接的两个传输方向都关闭了，不能再发送数据了。如果一方调用shutdown()则连接处于半关闭状态，仍可接受对方发来的数据


























































