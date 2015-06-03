#define _POSIX_C_SOURCE 2


#include <sys/wait.h>
#include <pthread.h>

#include "../include/walk_tree.h"
#include "../include/sync_comm.h"
#include "../include/sync_log.h"
#include "../include/sync_opt.h"
#include "../include/sync_network.h"


int syncAuto = 0;

void scan_directory(char* directory, MemInfo* mi){
	WalkTree* wt = NULL;
	int first = 1;
	int status;
	srand(time(NULL));	
	
	/* Init the struct wt */
	init_WalkTree(&wt);
	
	while(first || syncAuto){
		FileInfo fi;
		int up;
		first = 0;
		if(-1 == (up = update_WalkTree(wt, directory, ""))){
			add_log(ERROR, "FATAL F****** ERROR !");
			return;
		}
		if(up){
			while(get_NextUpdated(wt, &fi)){
				put_file(&fi, mi->sh_mem, mi->sem_ptr);
			}
		}
		sleep(SLEEP_TIME);
	}
	
	wait(&status);
	free_WalkTree(&wt);
}

void* get_file_changed(void* meminfo){
	FileInfo* fi = NULL;
	while(1){		
		fi = get_file(((MemInfo*)meminfo)->sh_mem, ((MemInfo*)meminfo)->sem_ptr);
		printf("CONSOMATEUR : %s\n", fi->name);
	}	
	return NULL;
}

int main(int argc, char** argv){	
	
	char** sem_name = NULL;
	char* sh_mem_name = NULL; /* the name of the shared memory */
	int a = 0;
	optArgs *opt;
	pthread_t thread;
	MemInfo meminfo;
	waitBro* waitbro;
	pthread_t net_thread;
	
	create_log();
	
	if(getArgs(argc, argv, &opt)){
		add_log(ERROR, "FAILURE !");
		exit(EXIT_FAILURE);
	}	
	
	sem_name = malloc( 2 * sizeof *sem_name);
	meminfo.sem_ptr = malloc(2 * sizeof *(meminfo.sem_ptr));
	
	if(NULL == sem_name || NULL == meminfo.sem_ptr){
		add_log(ERROR, "Can't allocate memory for name_tab");
		exit(EXIT_FAILURE);
	}
		
	 generateNameForInstance(&sh_mem_name);
	 generateNameForInstance(&(sem_name[NON_VIDE]));
	 generateNameForInstance(&(sem_name[NON_PLEIN]));
	 
	 /* Init shared memory */
	 if(NULL == (meminfo.sh_mem = create_shared_memory(sh_mem_name, NB_CASES_SHARE_FILENAME * sizeof(FileInfo)))){
		 add_log(ERROR, "Failure while creating shared memory");
		 exit(EXIT_FAILURE);
	 }
	 
	while(a != NB_CASES_SHARE_FILENAME) {
		
		init_sem(&(meminfo.sem_ptr[NON_VIDE]), sem_name[NON_VIDE], 0);
		init_sem(&(meminfo.sem_ptr[NON_PLEIN]), sem_name[NON_PLEIN], NB_CASES_SHARE_FILENAME);
	
		sem_getvalue(meminfo.sem_ptr[NON_PLEIN], &a);
		if(a != NB_CASES_SHARE_FILENAME){
			add_log(WARNING, "Problem while creating semaphore, rettying");
			sem_unlink(sem_name[NON_VIDE]);
			sem_unlink(sem_name[NON_PLEIN]);
		}	
	}
	
/* *************** EN OF INIT ******************* */
	
	sendBroadcastMsg(PORT_BROAD);
	
	/* we launch the network listening crap */
	pthread_create(&net_thread, NULL, waitBroadcastMsg, (void*)&waitbro);

	pthread_create(&thread, NULL, get_file_changed, (void*)&meminfo);

	add_logArg(INFO, "Scaning ", opt->directory);
	scan_directory(opt->directory, &meminfo);
	
	pthread_cancel(thread);
	pthread_cancel(net_thread);
	
	freeArgs(opt);
	
	free_shared_memory(meminfo.sh_mem, NB_CASES_SHARE_FILENAME * sizeof(FileInfo));
	
	sem_close(meminfo.sem_ptr[NON_VIDE]);
	sem_close(meminfo.sem_ptr[NON_PLEIN]);
		
	/* free names ! */
	free(sh_mem_name);
	free(sem_name[NON_PLEIN]);
	free(sem_name[NON_VIDE]);
	free(meminfo.sem_ptr);
	free(sem_name);
	
	close_log();
	
	 return 0;
		
}
