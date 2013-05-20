CC=gcc
CFLAGS=-g -pthread -c
LDFLAGS=-pthread -g

all:udp_scan test

udp_scan:main.o udp_scan.o
	$(CC) $(LDFLAGS) main.o udp_scan.o -o udp_scan

main.o:main.c udp_scan.h
	$(CC) $(CFLAGS) main.c

udp_scan.o:udp_scan.c udp_scan.h
	$(CC) $(CFLAGS) udp_scan.c

test:test.c
	$(CC) test.c -g -pthread -o test

clean:
	indent -kr -ts4 *.c *.h
	rm -rf *.o *~ udp_scan test
