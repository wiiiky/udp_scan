#include "udp_scan.h"
#include <signal.h>

static struct scaninfo info;

/* 用户按下CTRL_C产生SIGINT信号的回调函数 */
static void sig_int(int);
/* 打印扫描结果 */
static void show_result(void);

int main(int argc, char *argv[])
{
	pthread_t rid;

	/* 解析命令行参数,初始化scaninfo结构 */
	parse_scanpara(argc, argv, &info);

	if (signal(SIGINT, sig_int) < 0) {
		perror("signal error");
		exit(-1);
	}
	/* 创建接受ICMP数据包的线程 */
	if (pthread_create(&rid, NULL, receiver, &info) != 0) {
		perror("pthread_create error");
		exit(-1);
	}

	/* 发送UDP数据包 */
	sender(&info);

	printf("+++++++++++++++++++++++++++++\nall packets are sent!\n");
	printf("wait %d seconds to exit!\n", info.wait);
	sleep(info.wait);
	show_result();

	return 0;
}


static void show_result(void)
{
	unsigned short total;
	unsigned short i, j;

	total = info.end - info.start + 1;
	printf("\nclosed port:\t");
	for (i = 0, j = 0; i < total; i++) {
		if (info.status[i] == PORT_CLOSED) {
			j++;
			printf("%d ", i + info.start);
			if (j == 10) {
				printf("\n\t\t");
				j = 0;
			}
		}
	}
	printf("\nunknown port:\t");
	for (i = 0, j = 0; i < total; i++) {
		if (info.status[i] != PORT_CLOSED) {
			j++;
			printf("%d ", i + info.start);
			if (j == 10) {
				printf("\n\t\t");
				j = 0;
			}
		}
	}
	printf("\n");
}

static void sig_int(int signo)
{
	show_result();
	exit(0);
}
