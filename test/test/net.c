#include <stdio.h>
#include <stdlib.h>


#include <string.h>
#include <ifaddrs.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>

#define IFF_BROADCAST 0x2
#define PORT_BROAD 4567
#define PORT_LISTEN 7654

int createMessage(char** msg,char* addr_ip, int port){
	int len = strlen(addr_ip) + 6;
	
	*msg = malloc( len * sizeof(char));
	if(NULL == *msg)
		return 1;
		
	sprintf(*msg, "%d%c", port, '\0');
	return 0;	
}

int getIfSockStruct(struct sockaddr_in **sin, char* addr_ip){
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
			*sin = malloc(sizeof(struct sockaddr_in));
			
			if(*sin == NULL)
				return -1;
							
			/* on caste en sockaddr_in */
			*sin = (struct sockaddr_in*)ifa->ifa_broadaddr;
			inet_ntop(AF_INET, ((struct sockaddr_in*)ifa)->sin_addr, addr_ip, INET_ADDRSTRLEN);
			break;
		}
			
		ifa = ifa->ifa_next;
		
	}
	
	freeifaddrs(ifaddr);
	return 1;
		
}

int sendBroadcast(struct sockaddr_in *sin, char* msg){
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

int main(int argc, char** argv){
	struct sockaddr_in *sin = NULL;
	char buff[INET_ADDRSTRLEN];
	char* msg = NULL;
	char addr_ip[INET_ADDRSTRLEN];
	
	getIfSockStruct(&sin, addr_ip);
	sin->sin_port = htons(PORT_BROAD);
	
	printf("Sending data to %s on port %d\n", inet_ntop(AF_INET, &(sin->sin_addr), buff, INET_ADDRSTRLEN), 4567);
	
	createMessage(&msg, addr_ip, PORT_LISTEN);
	
	sendBroadcast(sin, msg);
	
	return 0;
}
