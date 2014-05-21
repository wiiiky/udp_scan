#ifndef _UDP_SCAN_H
#define _UDP_SCAN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>


#define _GNU_SOURCE
#include <getopt.h>

#define PORT_UNKNOWN 0			/* 什么都没有收到，这是默认值 */
#define PORT_CLOSED 1			/* 受到ICMP端口不可达差错 */
#define PORT_OPEN 2				/* 受到UDP数据包 */

struct scaninfo {
	unsigned short start;		/* 起始端口号 */
	unsigned short end;			/* 结束端口号 */
	char hostname[256];			/* 目标主机域名,可以没有 */
	struct in_addr addr;		/* 以网络字节序存放的目标地址,当前只支持IPv4 */
	char data[128];				/* 发送的数据 */
	useconds_t interval;		/* 每个udp数据包的发送间隔(以毫秒计)，不建议设为0，可能导致发送数据包的丢失,默认为500000us(500ms) */
	unsigned int attempts;		/* 数据包发送次数 */
	unsigned char *status;		/* 端口状态,动态分配空间 */
	unsigned int wait;			/* UDP数据包发送结束后,等待多少秒才退出,默认为interval/1000000.0*(end-start+1) */
};

#define MAX_PAYLOAD	128
#define MAX_HOSTNAME 256


#define BUFSIZE	4096
void sender(struct scaninfo *);	/* 发送UDP数据包 */
void *receiver(void *);			/* 接受ICMP数据包线程 */

/* 解析命令参数 */
void parse_scanpara(int argc, char *argv[], struct scaninfo *info);


#endif
