#define _POSIX_C_SOURCE 2


#include <sys/wait.h>
#include <pthread.h>

#include "../include/walk_tree.h"
#include "../include/sync_comm.h"
#include "../include/sync_log.h"
#include "../include/sync_opt.h"
#include "../include/sync_network.h"

typedef struct {
	MemInfo* mi;
	optArgs* opt;
} scanInfo;

int serveur_side(MemInfo* PeerList, int inst_number);

int client_side(optArgs* , MemInfo* PeerList);

void* scan_directory(void* _si);

int get_file_changed(FileInfo** fi, MemInfo* mi);

int main(int argc, char** argv){
	
	int i;
	int forki;
	optArgs *opt;
	MemInfo* PeerList = NULL;
	
	/* CODE MULTI INST */
	pthread_mutex_t inst_mutex;
	int* mem_inst = NULL;
	/*******************/
	
	create_log();	
	
	if(getArgs(argc, argv, &opt)){
		add_log(ERROR, "Failure while reading options !");
		exit(EXIT_FAILURE);
	}
	
	/* Here we create the shared memory for the peer list */
	if(init_memory(&PeerList, NB_PEER, sizeof(PeerElem))){
		add_log(ERROR, "Can't initialize shared memory");
		exit(EXIT_FAILURE);
	}
	
	/* CODE MULTI INST */
	if(NULL == (mem_inst = create_shared_memory(INSTANCE_NB_NAME,( 1 + INSTANCE_MAX) * sizeof *mem_inst))){
		add_log(ERROR, "Initialization failed for instance compt");
		exit(EXIT_FAILURE);
	}
	for(i=1; i<=INSTANCE_MAX; i++)
		mem_inst[i] = -1;
	/*******************/
	
	forki = fork();
	switch(forki){
		case -1 :
			add_log(ERROR, "FAIL fork");
			free_memory(PeerList);
			freeArgs(opt);
			close_log();
			return EXIT_FAILURE;
		case 0:;
			int inst_number = -1;
			/* On incremente le nombre d'instance */
			pthread_mutex_lock(&inst_mutex);
			*mem_inst = (*mem_inst) + 1;
			if(-1 == (inst_number = getInstanceNumber(mem_inst))){
				add_log(ERROR, "Can't get instance number. Maybe there is too many ?");
				pthread_mutex_unlock(&inst_mutex);
				exit(EXIT_FAILURE);
			}
			pthread_mutex_unlock(&inst_mutex);
			
			/* Server side */
			while(serveur_side(PeerList, inst_number));
			break;
		default :
			/* Client side */
			client_side(opt, PeerList);
			break;		
	}		
	
	pthread_mutex_lock(&inst_mutex);
	*mem_inst = (*mem_inst) - 1;
	pthread_mutex_unlock(&inst_mutex);
	
	free_memory(PeerList);
	freeArgs(opt);
	close_log();
	
	return 0;
}

int serveur_side(MemInfo* PeerList, int inst_number){
	static int index = 0;
	int sock;
	struct sockaddr_in sin;
	waitBro wtbro;
	
	wtbro.port = PORT_BROAD + inst_number - 1;
	
	waitBroadcastMsg((void*)&wtbro);
	printf("Message %s recu de %s !\n", wtbro.msg, wtbro.addr);
	
	/* We create a socket that connect to the program that send us data */
	if(-1 == (sock = socket(AF_INET, SOCK_STREAM, 0))){
		perror("SOCKET SERVER");
		return 0;
	}
	
	sin.sin_port = atoi(wtbro.msg);
	sin.sin_family = AF_INET;
	
	inet_pton(AF_INET, wtbro.addr, &(sin.sin_addr));
	
	if(-1 == bind(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr))){
		perror("SOCKET SERVEUR 2");
		return 0;
	}
	
	/* We add the new program we discovered to the PeerList */	
	
	sem_wait(PeerList->sem_ptr[NON_PLEIN]);
	((PeerElem*)PeerList->sh_mem)[index].port = atoi(wtbro.msg);
	((PeerElem*)PeerList->sh_mem)[index].addr = malloc(sizeof(char) * strlen(wtbro.addr));
	memcpy(((PeerElem*)PeerList->sh_mem)[index].addr, wtbro.addr, strlen(wtbro.addr));
	((PeerElem*)PeerList->sh_mem)[index].fd = sock;
	sem_post(PeerList->sem_ptr[NON_VIDE]);
	index++;
	
	/* We reply the port on wich we want to talk ! */
	
	
	return 1;
}

int client_side(optArgs* optargs, MemInfo* PeerList){
	
	pthread_t scanThread;
	int port = PORT_BROAD;
	scanInfo si;
	MemInfo* filelist = NULL;
	FileInfo* changed;
			
	if(sendBroadcastMsg((void*)&port)){
		/* An error have occured */
		add_log(ERROR, "Can't retrieved network data. Usualy, it's caused because you're not connected on any interface.");
		return 1;
	}
	
	
	
	/* Here we create the shared memory for the files list */
	if(init_memory(&filelist, NB_FILE, sizeof(FileInfo))){
		add_log(ERROR, "Can't initialize shared memory");
		exit(EXIT_FAILURE);
	}
	
	si.opt = optargs;
	si.mi = filelist;

	pthread_create(&scanThread, NULL, scan_directory, (void*)&si);
	
	get_file_changed(&changed, filelist);
		
	free_memory(si.mi);
	
	return 0;
}


void* scan_directory(void* _si){
	
	WalkTree* wt = NULL;
	int first = 1;
	srand(time(NULL));	
	scanInfo* si = (scanInfo*)_si;
	
	/* Init the struct wt */
	init_WalkTree(&wt);
	
	while(first || si->opt->is_auto){
		FileInfo fi;
		int up;
		first = 0;
		if(-1 == (up = update_WalkTree(wt, si->opt->directory, ""))){
			add_log(ERROR, "FATAL F****** ERROR !");
			return NULL;
		}
		if(up){
			while(get_NextUpdated(wt, &fi)){
				put_file(&fi,si->mi);
			}
		}
		sleep(SLEEP_TIME);
	}
	free_WalkTree(&wt);
	return NULL;
}

int get_file_changed(FileInfo** fi, MemInfo* mi){
		
	*fi = get_file(mi);
	return 1;
}
