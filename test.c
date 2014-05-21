#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void *listen_in_port(void *arg);

int main(int argc, char *argv[])
{
	unsigned int port = 0;
	int i;
	pthread_t tid;

	if (argc <= 1)
		return;

	for (i = 1; i < argc; i++) {
		port = atoi(argv[i]);
		if (port > 0) {
			if (pthread_create
				(&tid, NULL, listen_in_port, (void *) (long) port) < 0) {
				perror("pthread_create error");
				exit(0);
			}
			if (pthread_detach(tid) < 0) {
				perror("pthread_detach error");
				exit(0);
			}
		}
	}
	while (1);

	return 0;
}


static void *listen_in_port(void *arg)
{
	unsigned int port = (unsigned int) (long) arg;
	int sockfd;
	struct sockaddr_in servaddr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket error");
		exit(-1);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror("bind error");
		exit(-1);
	}
	char buf[4096];
	struct sockaddr_storage recvaddr;
	socklen_t addrlen;
	ssize_t n;
	while ((n =
			recvfrom(sockfd, buf, 4096, 0, (struct sockaddr *) &recvaddr,
					 &addrlen)) > 0) {
		buf[n] = '\0';
		sendto(sockfd, buf, n, 0, (struct sockaddr *) &recvaddr, addrlen);
		printf("Receive from %s:%u\t%s\n",
			   inet_ntoa(((struct sockaddr_in *) &recvaddr)->sin_addr),
			   ntohs(((struct sockaddr_in *) &recvaddr)->sin_port), buf);
	}

	return NULL;
}
