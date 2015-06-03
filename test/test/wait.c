#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv){

	int sock;
	socklen_t len = 0 ;
	struct sockaddr_in sin, *sinbis;
	char buff[6];
	int port = 4567;
	
	struct sockaddr saddr;
	char str[INET_ADDRSTRLEN];
	
	sin.sin_port = htons(port);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htons(INADDR_ANY);
	len = sizeof(struct sockaddr);
	
	if(-1 == (sock = socket(AF_INET, SOCK_DGRAM, 0))){
		perror("SOCKET");
		exit(EXIT_FAILURE);
	}
	
	if(-1 == (bind(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr))))
		perror("BIND");
		
	printf("Waiting for a connexion on port %d...\n", port);
		
	if(-1 == recvfrom(sock, buff, 6, 0, &saddr, &len)){
		perror("RECV");
	}
		
	sinbis = (struct sockaddr_in*)&saddr;

	inet_ntop(AF_INET, &(sinbis->sin_addr), str, INET_ADDRSTRLEN);
		
	printf("Data received from %s : %s\n",str, buff);
	
	return 0;
}
