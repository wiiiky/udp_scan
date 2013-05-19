#include "udp_scan.h"

#define _GNU_SOURCE
#include <getopt.h>

/* 解析命令行参数 */
static void parse_scanpara(int argc, char *argv[], struct scaninfo *info);

int main(int argc, char *argv[])
{
	pthread_t tid;
	struct scaninfo info;

	/* 解析命令行参数,初始化scaninfo结构 */
	parse_scanpara(argc, argv, &info);

	/* 创建线程发送udp数据包 */
	if (pthread_create(&tid, NULL, sender, &info) != 0) {
		perror("pthread_create error");
		exit(-1);
	}
	if (pthread_join(tid, NULL) != 0) {
		perror("pthread_detach error");
		exit(-1);
	}

	return 0;
}

/* 显示帮助，并退出程序 */
static void help(void)
{
	printf("Usage:udp_scan\tdest\t[-s start_port]\n");
	printf("\t\t\t[-e end_port]\n");
	printf("\t\t\t[-i interval]\n");
	printf("\t\t\t[-a attempts]\n");
	printf("\t\t\t[-p payload]\n");
	printf("\t\t\t[-h help]\n");
	exit(0);
}

static void parse_scanpara(int argc, char *argv[], struct scaninfo *info)
{
	if (info == NULL)
		return;
	memset(info, 0, sizeof(struct scaninfo));
	/* 初始化scaninfo结构 */
	info->attempts = 1;
	info->usec = 500000;
	strncpy(info->data, "hello world!", MAX_PAYLOAD);

	int opt;
	struct option longopts[] = {
		{"help", 0, NULL, 'h'},	/* 帮助 */
		{"start-port", 1, NULL, 's'},	/* 起始端口号 */
		{"end-port", 1, NULL, 'e'},	/* 结束端口号 */
		{"interval", 1, NULL, 'i'},	/* 发送udp数据包的间隔 */
		{"attempts", 1, NULL, 'a'},	/* 尝试发送次数 */
		{"payload", 1, NULL, 'p'},	/* 负载数据 */
		{0, 0, 0, 0}
	};

	while ((opt =
			getopt_long(argc, argv, "hs:e:i:a:p:", longopts,
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
			info->usec = atoi(optarg);
			break;
		case 'a':
			info->attempts = atoi(optarg);
			break;
		case 'p':
			strncpy(info->data, optarg, MAX_PAYLOAD);
			break;
		default:
			fprintf(stderr, "Unknown option - %c\n", opt);
			help();
			break;
		}
	}
	if (optind != argc - 1)
		help();

	if (info->start == 0 || info->end == 0 || info->end < info->start
		|| info->attempts <= 0) {
		fprintf(stderr, "Invalid argument\n");
		exit(-1);
	}
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
