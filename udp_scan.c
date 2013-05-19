#include "udp_scan.h"

/* 发送udp数据包的线程 */
void *sender(void *arg)
{
	if (arg == NULL)
		return NULL;

	int sockfd;
	unsigned short i, start, end;
	int len, count;
	struct sockaddr_in servaddr;
	struct scaninfo *ip = (struct scaninfo *) arg;

	start = ip->start;
	end = ip->end;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket(AF_INET,SOCK_DGRAM,0) error");
		return NULL;
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	memcpy(&servaddr.sin_addr, &ip->addr, sizeof(servaddr.sin_addr));
	len = strlen(ip->data);
	/* 发送数据包,每次发送间隔5s */
	for (count = 1; count <= ip->attempts; count++) {
		printf("开始发送第%d次数据包...\n", count);
		for (i = start; i <= end; i++) {
			servaddr.sin_port = htons(i);
			if (sendto
				(sockfd, ip->data, len, 0, (struct sockaddr *) &servaddr,
				 sizeof(servaddr)) < 0) {
				perror("sendto error");
				return NULL;
			}
			usleep(ip->usec);
		}
		printf("第%d次数据包发送完毕\n", count);
		if (count == ip->attempts)
			break;
		for (i = 1; i <= 5; i++) {
			printf("...");
			fflush(NULL);
			sleep(1);
		}
		printf("\n");
	}
	return NULL;
}
