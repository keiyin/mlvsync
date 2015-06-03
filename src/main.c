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

int serveur_side();

int client_side(optArgs* optargs);

void* scan_directory(void* _si);

int get_file_changed(FileInfo** fi, MemInfo* mi);

int main(int argc, char** argv){
	
	int forki;
	optArgs *opt;
	MemInfo* PeerList = NULL;
	
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
	
	forki = fork();
	switch(forki){
		case -1 :
			add_log(ERROR, "FAIL fork");
			free_memory(PeerList);
			freeArgs(opt);
			close_log();
			return EXIT_FAILURE;
		case 0:
			/* Server side */
			serveur_side();
			break;
		default :
			/* Client side */
			client_side(opt);
			break;		
	}		
	
	free_memory(PeerList);
	freeArgs(opt);
	close_log();
	
	return 0;
}

int serveur_side(){
	waitBro wtbro;
	
	waitBroadcastMsg((void*)&wtbro);
	printf("Message %s recu de %s !\n", wtbro.msg, wtbro.addr);
	
	
	return 0;
}

int client_side(optArgs* optargs){
	
	pthread_t scanThread;
	int port = PORT_BROAD;
	scanInfo si;
	MemInfo* filelist = NULL;
	FileInfo* changed;
			
	sendBroadcastMsg((void*)&port);
	
	
	
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