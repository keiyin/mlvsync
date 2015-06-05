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

#define INSTANCE_NB_NAME "/nb_inst"
#define INSTANCE_MAX 5

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

int sendBroadcastMsg(void* port);

/* Return the number of the actual instance, or -1 in case of error
 * Error can be :	mem_int is not good
 * 					There is too many instance on the computer
 * WARNING : Can only be used when mutex is locked !
 */
int getInstanceNumber(int* mem_inst);

#endif
