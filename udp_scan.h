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

struct scaninfo {
	unsigned short start;		/* 起始端口号 */
	unsigned short end;			/* 结束端口号 */
	char hostname[256];			/* 目标主机域名,可以没有 */
	struct in_addr addr;		/* 以网络字节序存放的目标地址,当前只支持IPv4 */
	char data[128];				/* 发送的数据 */
	useconds_t usec;			/* 每个udp数据包的发送间隔，微妙 */
	unsigned int attempts;		/* 数据包发送次数 */
};

#define MAX_PAYLOAD	128
#define MAX_HOSTNAME 256


#define BUFSIZE	4096
void *sender(void *arg);


#endif
