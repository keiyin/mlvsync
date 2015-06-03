#ifndef __SYNC_NETWORK__
#define __SYNC_NETWORK__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>

#define IFF_BROADCAST 0x2
#define PORT_BROAD 4567
#define PORT_LISTEN 7654
#define NB_PEER 4

typedef struct {
	int port;
	char* msg;
	char* addr;
} waitBro;

typedef struct {
	int port;
	char* addr;
	int fd;
} PeerElem;

/* We use a struct for thread param that is void* */
void* waitBroadcastMsg(void* waitbro);

void* sendBroadcastMsg(void* port);

#endif
