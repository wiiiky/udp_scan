#include "udp_scan.h"
#include <signal.h>


/* 仅仅为了中断recvfrom系统调用 */
static void alarmHanlder(int signo)
{
	alarm(1);
}

/* 发送udp数据包 */
void sender(struct scaninfo *arg)
{
	if (arg == NULL)
		return;

	int sockfd;
	unsigned short i, start, end;
	int len, total;
	struct sockaddr_in servaddr;
	struct scaninfo *info = (struct scaninfo *) arg;

	start = info->start;
	end = info->end;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket(AF_INET,SOCK_DGRAM,0) error");
		exit(-1);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	memcpy(&servaddr.sin_addr, &info->addr, sizeof(servaddr.sin_addr));
	len = strlen(info->data);
	/* 发送数据包,每次发送间隔5s */
	setbuf(stdout, NULL);		/* 标准输出无缓冲 */
	for (total = 1; total <= info->attempts; total++) {
		printf("sending UDP packets(%d)", total);
		for (i = start; i <= end; i++) {
			servaddr.sin_port = htons(i);
			if (sendto(sockfd, info->data, len, 0,
					   (struct sockaddr *) &servaddr,
					   sizeof(servaddr)) < 0) {
				perror("sendto error");
				exit(-1);
			}
			usleep(info->interval);
			printf(".");
		}
		printf("\ncomplete sending(%d)\n", total);
		if (total == info->attempts)
			break;
		for (i = 1; i <= 20; i++) {
			printf(".");
			usleep(100000);
		}
		printf("\n");
	}

	printf("+++++++++++++++++++++++++++++\nall packets are sent!\n");
	printf("wait %d seconds to exit!\n", info->wait);

	/* recvfrom会阻塞，通过alarm产生的信号中断 */
	struct sigaction act;
	act.sa_handler = alarmHanlder;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_INTERRUPT;	/* 中断系统调用 */
	sigaction(SIGALRM, &act, NULL);
	alarm(info->wait);

	char buf[1024];
	struct sockaddr_in recvaddr;
	socklen_t recvlen;
	ssize_t n;
	while ((n = recvfrom(sockfd, buf, 1024, 0,
						 (struct sockaddr *) &recvaddr, &recvlen)) > 0) {
		if (recvaddr.sin_addr.s_addr == servaddr.sin_addr.s_addr) {
			unsigned short port = ntohs(recvaddr.sin_port);
			info->status[port - info->start] = PORT_OPEN;
		}
	}

	close(sockfd);

	return;
}

#define ICMP_LEN 8

void *receiver(void *arg)
{
	int sockfd;
	struct scaninfo *sip;
	struct sockaddr_in raddr;
	socklen_t len;
	char buf[BUFSIZE];
	struct iphdr *ip;
	unsigned int iplen;
	struct icmphdr *icmp;
	struct udphdr *udp;
	char straddr[256];
	unsigned short start, end, total;
	unsigned short port;

	sip = (struct scaninfo *) arg;
	if (sip == NULL)
		return NULL;

	start = sip->start;
	end = sip->end;
	total = end - start + 1;

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
		perror("socket(AF_INET,SOCK_RAW,IPPROTO_ICMP) error");
		exit(-1);
	}
	setuid(getuid());			/* 使用真实的用户id，不再需要超级用户特权 */

	memset(&raddr, 0, sizeof(raddr));
	len = sizeof(raddr);
	raddr.sin_family = AF_INET;
	memcpy(&raddr.sin_addr, &sip->addr, sizeof(raddr.sin_addr));
	/* 
	 * 对原始套接字调用connect不执行任何网络操作，只是在本地将套接字与地址绑定；
	 * 因为创建套接时指定了IPPROTO_ICMP，因此内核只将从raddr地址发送过来的ICMP
	 * 数据包传递到此套接字
	 */
	if (connect(sockfd, (struct sockaddr *) &raddr, len) < 0) {
		perror("connect error");
		exit(-1);
	}

	/*memset(&raddr, 0, len); */
	while (recv(sockfd, buf, BUFSIZE, 0) > 0) {
		ip = (struct iphdr *) buf;	/* IP首部 */

		if (inet_ntop(AF_INET, &ip->saddr, straddr, 256) != 1)
			strncpy(straddr, "unknown", 256);

		if (ip->protocol == IPPROTO_ICMP) {	/* 是ICMP数据包 */
			iplen = ip->ihl << 2;	/* IP首部长度 */
			icmp = (struct icmphdr *) (buf + iplen);	/* ICMP首部 */
			if (icmp->type != ICMP_DEST_UNREACH || icmp->code != ICMP_PORT_UNREACH) {	/* 不是ICMP端口不可达错误 */
				printf
					("an unexpected packet from %s, type = %d, code = %d\n",
					 straddr, icmp->type, icmp->code);
				continue;
			}

			ip = (struct iphdr *) (buf + iplen + ICMP_LEN);	/* 负载的IP数据包 */
			iplen = ip->ihl << 2;
			udp = (struct udphdr *) ((char *) ip + iplen);	/* 负载的UDP首部 */
			port = ntohs(udp->dest);
			if (port < start || port > end) {
				printf("an unexpected packet from %s, udp port = %d\n",
					   straddr, port);
				continue;
			}
			sip->status[port - start] = PORT_CLOSED;
		}
	}

	/* */
	close(sockfd);
	return NULL;
}


/* 显示帮助，并退出程序 */
static void help(void)
{
	printf("Usage:udp_scan\tdest\t[-s start_port]\n");
	printf("\t\t\t[-e end_port]\n");
	printf("\t\t\t[-i interval]\n");
	printf("\t\t\t[-a attempts]\n");
	printf("\t\t\t[-p payload]\n");
	printf("\t\t\t[-w time_to_exit]\n");
	printf("\t\t\t[-h help]\n");
	exit(0);
}

void parse_scanpara(int argc, char *argv[], struct scaninfo *info)
{
	if (info == NULL)
		return;
	memset(info, 0, sizeof(struct scaninfo));
	/* 初始化scaninfo结构 */
	info->attempts = 1;
	info->interval = 500000;
	/* 默认的负载数据是hello world */
	strncpy(info->data, "hello world!", MAX_PAYLOAD);

	int opt;
	struct option longopts[] = {
		{"help", 0, NULL, 'h'},	/* 帮助 */
		{"start-port", 1, NULL, 's'},	/* 起始端口号 */
		{"end-port", 1, NULL, 'e'},	/* 结束端口号 */
		{"interval", 1, NULL, 'i'},	/* 发送udp数据包的间隔 */
		{"attempts", 1, NULL, 'a'},	/* 尝试发送次数 */
		{"payload", 1, NULL, 'p'},	/* 负载数据 */
		{"wait", 1, NULL, 'w'},
		{0, 0, 0, 0}
	};

	while ((opt =
			getopt_long(argc, argv, "hs:e:i:a:p:w:", longopts,
						NULL)) != -1) {
		switch (opt) {
		case 'h':
			help();
			break;
		case 's':
			info->start = atoi(optarg);
			break;
		case 'e':
			info->end = atoi(optarg);
			break;
		case 'i':
			info->interval = atoi(optarg) * 1000;
			break;
		case 'a':
			info->attempts = atoi(optarg);
			break;
		case 'p':
			strncpy(info->data, optarg, MAX_PAYLOAD);
			break;
		case 'w':
			info->wait = atoi(optarg);
			break;
		default:
			help();
			break;
		}
	}
	if (optind != argc - 1)
		help();

	if (info->start == 0 || info->end == 0 || info->end < info->start
		|| info->attempts <= 0) {
		fprintf(stderr, "Invalid port number or attempts\n");
		exit(-1);
	}

	if (info->wait == 0) {
		info->wait =
			info->interval / 1000000.0 * (info->end - info->start + 1);
		if (info->wait == 0) {	/* 端口跨度过小 */
			info->wait = 1;
		}
	}
	info->status =
		(unsigned char *) malloc((info->end - info->start + 1) *
								 sizeof(unsigned char));
	if (info->status == NULL) {
		perror("malloc error");
		exit(-1);
	}
	memset(info->status, 0, info->end - info->start + 1);	/* 端口状态初始化为未知 */

	if (inet_pton(AF_INET, argv[optind], &info->addr) != 1) {	/* 如果不是IP地址，尝试解析域名 */
		struct hostent *hp;
		if ((hp = gethostbyname(argv[optind])) == NULL) {
			fprintf(stderr, "Invalid destination - %s\n", argv[optind]);
			exit(-1);
		}
		strncpy(info->hostname, argv[optind], MAX_HOSTNAME);
		memcpy(&info->addr, hp->h_addr, sizeof(info->addr));
	}
}
