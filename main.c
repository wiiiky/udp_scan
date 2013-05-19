/* 
 * File:   main.c
 * Author: wiky L
 *
 * Created on May 18, 2013, 12:49 PM
 */

#include "scan.h"

/*
 * 
 */
int main(int argc, char** argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage:%s hostname start_port end_port\n", argv[0]);
        exit(0);
    }

    struct addrinfo *h;
    struct proto pr;
    char buf[256];
    h = serv_host(argv[1], NULL, AF_INET, SOCK_DGRAM);
    if (h == NULL) {
        fprintf(stderr, "Unknown hostname:%s\n", argv[1]);
        exit(0);
    }

    memset(&pr, 0, sizeof (struct proto));
    if (h->ai_family == AF_INET) {
        pr.family = AF_INET;
        memcpy(&pr.servaddr, h->ai_addr, h->ai_addrlen);
        pr.servlen = h->ai_addrlen;
    } else if (h->ai_family == AF_INET6) {
        pr.family = AF_INET6;
        memcpy(&pr.servaddr, h->ai_addr, h->ai_addrlen);
        pr.servlen = h->ai_addrlen;
    } else {
        fprintf(stderr, "Unknown protocol family\n");
        exit(-1);
    }
    freeaddrinfo(h); /* 释放addrinfo结构 */

    pr.start = atoi(argv[2]);
    pr.end = atoi(argv[3]);
    if (pr.start == 0 || pr.end == 0 || pr.end < pr.start) {
        fprintf(stderr, "Invalid port number\n");
        exit(-1);
    }

    if (send_packet(&pr) != 0) {
        fprintf(stderr, "send packet error!\n");
        exit(-1);
    }

    recv_icmp(&pr);

    return 0;
}

