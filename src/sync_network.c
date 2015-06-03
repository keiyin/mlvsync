#include "../include/sync_network.h"


static int createMessage(char** msg,char* addr_ip, int port){
	int len = strlen(addr_ip) + 6;
	
	*msg = malloc( len * sizeof(char));
	if(NULL == *msg)
		return 1;
		
	sprintf(*msg, "%d%c", port, '\0');
	return 0;	
}

static int getIfSockStruct(struct sockaddr_in **sin, char* addr_ip){
	struct ifaddrs *ifaddr, *ifa = NULL;
	
	if(-1 == getifaddrs(&ifa)){
		fprintf(stderr, "Erreur lors de l'appel à getifaddrs\n");
		return 1;
	}
	
	ifaddr = ifa;
	
	while(ifa != NULL){
		
		if( (strcmp(ifa->ifa_name, "lo") == 0) || !(ifa->ifa_flags & (IFF_BROADCAST))){
			ifa = ifa->ifa_next;
			continue;
		}
		
		if(ifa->ifa_broadaddr->sa_family == AF_INET){
			if(NULL == (*sin = malloc(sizeof(struct sockaddr_in))))
				return -1;
			
			/* on caste en sockaddr_in */
			*sin = (struct sockaddr_in*)ifa->ifa_broadaddr;
			inet_ntop(AF_INET, &(((struct sockaddr_in*)ifa)->sin_addr), addr_ip, INET_ADDRSTRLEN);
			break;
		}
			
		ifa = ifa->ifa_next;
		
	}
	
	freeifaddrs(ifaddr);
	return 1;
		
}

static int sendBroadcast(struct sockaddr_in *sin, char* msg){
	int sock; /* socket */
	int ok_for_broad = 1; /* permission to use broadcast */
	
	if(-1 == (sock = socket(AF_INET, SOCK_DGRAM, 0))){
		perror("SOCKET");
		exit(EXIT_FAILURE);
	}
		
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &ok_for_broad, sizeof ok_for_broad);
			
	if(-1 == sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)sin, sizeof(struct sockaddr)))
		perror("SENDTO");
	return 0;
	
}

void* waitBroadcastMsg(void* waitbro){
	
	int sock;
	socklen_t len = 0 ;
	struct sockaddr_in sin, *sinbis;
	char buff[6];
		
	struct sockaddr saddr;
	char str[INET_ADDRSTRLEN];
	waitBro* wb = (waitBro*)waitbro;
	
	sin.sin_port = htons(wb->port);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htons(INADDR_ANY);
	len = sizeof(struct sockaddr);
	
	if(-1 == (sock = socket(AF_INET, SOCK_DGRAM, 0))){
		perror("SOCKET");
		exit(EXIT_FAILURE);
	}
	
	if(-1 == (bind(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr))))
		perror("BIND");
		
	if(-1 == recvfrom(sock, buff, 6, 0, &saddr, &len)){
		perror("RECV");
	}
		
	sinbis = (struct sockaddr_in*)&saddr;

	inet_ntop(AF_INET, &(sinbis->sin_addr), str, INET_ADDRSTRLEN);
		
	memcpy(wb->msg, buff, 6);
	memcpy(wb->addr, str, strlen(str));
	
	return NULL;	
}

void* sendBroadcastMsg(void* port){
	struct sockaddr_in *sin = NULL;
	char buff[INET_ADDRSTRLEN];
	char* msg = NULL;
	char addr_ip[INET_ADDRSTRLEN];
	
	getIfSockStruct(&sin, addr_ip);
	sin->sin_port = htons(*(int*)port);
	
	printf("Sending data to %s on port %d\n", inet_ntop(AF_INET, &(sin->sin_addr), buff, INET_ADDRSTRLEN),*(int*)port);
	
	createMessage(&msg, addr_ip, PORT_LISTEN);
	
	sendBroadcast(sin, msg);
	free(msg);
	
	
	return NULL;
}


