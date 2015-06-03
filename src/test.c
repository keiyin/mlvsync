/* test program */

#include "../include/sync_comm.h"
#include "../include/walk_tree.h"
#include "../include/sync_log.h"

#define TEST_LEN 5

void test_log(){
	printf("\n *** TEST LOG ***\n");
	printf("Creating log file ...");
	int fd = create_log();
	if(fd == -1){
		printf(" FAILED !\n");
		return;
	}
	printf(" OK !\nFile descriptor = %d\nAdding lines to log ...", fd);
	if(!add_log(INFO, "test adding lines to log")){
		printf(" FAILED !\n");
		return;
	}
	if(!add_log(WARNING, "this is a warning")){
		printf(" FAILED !\n");
		return;
	}
	if(!add_log(ERROR, "THIS IS A BADASS ERROR !")){
		printf(" FAILED !\n");
		return;
	}
	if(!add_logArg(INFO, "Test message avec arg : ", "ARG !")){
		printf("FAILED !\n");
		return;
	}
	printf(" OK !\nSee log file to make sure it works !\nClosing log file ...");
	close(fd);
	printf(" OK !\n*** END TEST ***\n\n");
}	

void test_genName(){
	char* test;
	printf("\n *** TEST GEN NAME ***\n");
	printf("Size between %d and %d\nCreating a string randomly ...", LEN_MIN, LEN_MAX);
	generateNameForInstance(&test);
	printf(" OK ! str = %s\n", test);
	free(test);
	printf("*** END TEST ***\n\n");
}	

void test_semaphore(){
	sem_t* sem1 = NULL;
	int value = TEST_LEN;
	printf("\n*** TEST SEMAPHORES ***\n");
	printf("Creating a semaphore with value %d ...", value);
	init_sem(&sem1, "/sem1", value);
	value = 0;
	sem_getvalue(sem1, &value);
	printf(" OK !\nptr = %p - value = %d\n*** END TEST ***\n\n", sem1, value);
}

void test_mem_shared(){
	
	char* mem_sh = NULL;
	
	printf("\n*** TEST SHARED MEMORY ***\n");
	printf("Creating shared memory.\n\tType : char*\n\tSize : %d\n\n...", 2000 * sizeof(char));
	mem_sh = create_shared_memory("/mem_test", 2000 * sizeof(char));
	if(NULL == mem_sh){
		printf(" FATAL ERROR. ptr is NULL :(\n*** TEST FAILED ***\n\n");
		return;
	}
	printf(" OK !\nptr = %p\nWRITING ...", mem_sh);
	memcpy(mem_sh, "chaine de test", strlen("chaine de test"));
	memcpy(mem_sh+500, "chaine de test", strlen("chaine de test"));
	memcpy(mem_sh+1000, "chaine de test", strlen("chaine de test"));
	memcpy(mem_sh+1500, "chaine de test", strlen("chaine de test"));
	printf(" OK !\nREADING ...");
	printf("\n%s\n%s\n%s\n%s\n OK !\nFree ressources ...",mem_sh, mem_sh+500, mem_sh+1000, mem_sh+1500);
	free_shared_memory(mem_sh, 2000 * sizeof(char));
	printf(" OK !\n*** END TEST ***\n\n");
	
}

void test_walk_tree(){
	WalkTree* wt;
	printf("\n*** TEST WALK TREE ***\n");
	printf("Creating a walk tree ... ");
	init_WalkTree(&wt);
	printf("OK !\nptr = %p - Nb Files = %d\n", wt, wt->nb_files);
	printf("Free ressources ...");
	free_WalkTree(&wt);
	printf(" OK !\n*** END TEST ***\n\n");
}


int main(int argc, char** argv){
	
	printf("STARTING TEST PROGRAM FOR MLVSYNC\n");
	srand(time(NULL));
	test_semaphore();
	test_walk_tree();
	test_mem_shared();
	test_genName();
	test_log();
	printf("END OF TEST PROGRAM\n");	
	
return 0;	
}
