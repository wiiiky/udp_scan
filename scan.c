#include "scan.h"

/* 根据指定的参数获取服务端的地址结构 */
struct addrinfo* serv_host(const char *host, const char *service,
        int family, int socktype)
{
    int n;
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_flags = AI_CANONNAME; /* 总是返回规范名 */
    hints.ai_socktype = socktype;
    hints.ai_family = family;

    if ((n = getaddrinfo(host, service, &hints, &result)) != 0)
        return NULL;
    return result; /* 必须调用freeaddrinfo函数释放动态分配的资源 */
}

int send_packet(struct proto *pr)
{
    unsigned int start, end, i;
    int sockfd;
    int family;
    const char *packet = "hello there";

    family = pr->family;
    if ((sockfd = socket(family, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket error");
        return -1;
    }

    start = pr->start;
    end = pr->end;
    for (i = start; i <= end; i++) {
        if (family == AF_INET)
            ((struct sockaddr_in*) &pr->servaddr)->sin_port = htons(i);
        else
            ((struct sockaddr_in6*) &pr->servaddr)->sin6_port = htons(i);
        if (sendto(sockfd, packet, 12, 0, &pr->servaddr, pr->servlen) < 0) {
            perror("sendto error");
            return -1;
        }
    }
    return 0;
}

static int timeout = 0;

int ipv4_recv(struct proto *pr)
{
    int sockfd;
    unsigned length, n;
    unsigned char *ports; /* 记录端口状态 */
    char buf[BUFSIZE];
    struct ip *iphdr;
    struct icmp *icmphdr;
    struct udphdr *udphdr;
    int iplen, icmplen;

    timeout = 0;
    n = 0;
    length = pr->end - pr->start;

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("create SOCK_RAW error");
        return -1;
    }
    if (connect(sockfd, &pr->servaddr, pr->servlen) < 0) {
        perror("connect error");
        return -1;
    }

    ports = (unsigned char*) malloc(length * sizeof (unsigned char));
    if (ports == NULL) {
        fprintf(stderr, "malloc error\n");
        return -1;
    }
    memset(ports, PORT_OPEN, length); /* 假设所有端口都是打开的 */
    for (; timeout == 0;) {
        n = recv(sockfd, buf, BUFSIZE, 0);
        if(n<0)
            continue;
        iphdr = (struct ip*) buf;
        iplen = iphdr->ip_hl << 2;
        if (iphdr->ip_p != IPPROTO_ICMP)
            continue;
        icmphdr = (struct icmp*) (buf + iplen); /* ICMP首部 */
        if ((icmplen = n - iplen) < 8) { /* ICMP数据包过小 */
            printf("receive a malformed packet!\n");
            continue;
        }
        if (icmphdr->icmp_type != ICMP_DEST_UNREACH ||
                icmphdr->icmp_code != ICMP_PORT_UNREACH) { /* 不是ICMP端口不可达消息 */
            printf("receive a unexpected packet(type=%d,code=%d)\n",
                    icmphdr->icmp_type, icmphdr->icmp_code);
            continue;
        }
        udphdr=(struct udphdr*)(((char*)icmphdr)+8+20);
        printf("%d\n",ntohs(udphdr->dest));
    }
    free(ports);
}

int ipv6_recv(struct proto *pr)
{
}

int recv_icmp(struct proto *pr)
{
    if (pr->family == AF_INET){
        return ipv4_recv(pr);
    }
    else
        return ipv6_recv(pr);

    return -1; /* never come here */
}