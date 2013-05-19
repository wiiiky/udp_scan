/* 
 * File:   scan.h
 * Author: wiky
 *
 * Created on May 18, 2013, 12:52 PM
 */

#ifndef _SCAN_H
#define	_SCAN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>


#define PORT_OPEN       0
#define PORT_CLOSE      1
#define PORT_UNKNOWN    2

#define BUFSIZE         4096

/* 协议无关 */
struct proto {
    int family;
    struct sockaddr servaddr;
    socklen_t servlen;
    unsigned int start;
    unsigned int end;
};

int ipv4_recv(struct proto *pr);
int ipv6_recv(struct proto *pr);
int recv_icmp(struct proto*);

/* 根据指定的参数获取服务端的地址结构 */
struct addrinfo* serv_host(const char *, const char *, int, int);
int send_packet(struct proto *pr);

#endif	/* _SCAN_H */

