#include "../include/sync_comm.h"

/* generate a name randomly, with a size beetween LEN_MIN and LEN_max. only MAJ an min */
static void generateNameForInstance(char** name){
	static int nb_appel = 0;
	static const char alpha[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int len, i, sizeAlpha;
	sizeAlpha = strlen(alpha);
	len = (rand() % (LEN_MAX - LEN_MIN)) + LEN_MIN;
	
	(*name) = (char*)malloc(len * sizeof(char) + 1);
	
	for(i=0; i<len; i++)
		(*name)[i] = alpha[ ((rand() + nb_appel) % sizeAlpha) -1];
		
	(*name)[len] = 0;	
	
	/* On incremente nb_appel pour plus de sureté sur l'unicitée des noms */
	nb_appel++;
}

/* return a created shared memory, with a size of size, named name_mem */
static void* create_shared_memory(char* name_mem, size_t size){
	int fd_shared, i;
	char vide = '\0';
	void* ptr = NULL;
	if(-1 == (fd_shared = shm_open(name_mem, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR))){
		perror("Mémoire shared");
		return NULL;
	}
	
	if(-1 == (ftruncate(fd_shared,size))){
		perror("patate");
		return NULL;
	}
	ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shared, 0);
	
	for(i=0; i< size/sizeof(char); i++)
		memcpy(ptr + i, &vide, 1); 
	
	
	return ptr;
}

/* Init the sem with the name 'sem_name' and the value 'sem_value' */
int init_sem(sem_t** sem_ptr, char* sem_name, unsigned int sem_value){
	if(NULL == sem_ptr || NULL == sem_name)
		return 1;
		
	(*sem_ptr) = sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR, sem_value);
	
	if((*sem_ptr) == SEM_FAILED){
		perror("Initialisation");
		return 1;	
	}
	return 0;
}
/* free the shared memory created with create_shared_memory */
static void free_shared_memory(void* ptr, size_t size){
	munmap(ptr, size);	
}

void put_file(FileInfo* fi, MemInfo* mi){
	static int prod = 0;
	
	sem_wait(mi->sem_ptr[NON_PLEIN]);
	
	memcpy(mi->sh_mem[prod].name, fi->name, strlen(fi->name));
	mi->sh_mem[prod].last_time = fi->last_time;
	
	prod = (prod + 1) % mi->nb_elem;
	sem_post(mi->sem_ptr[NON_VIDE]);
}

FileInfo* get_file(MemInfo* mi){
	static int prod = 0;
	FileInfo* fi = (FileInfo*)calloc(1, sizeof(FileInfo));
	
	sem_wait(mi->sem_ptr[NON_VIDE]);
	int len = strlen(mi->sh_mem[prod].name);
	memcpy(fi->name, mi->sh_mem[prod].name, len);
	fi->last_time = mi->sh_mem[prod].last_time;
	prod = (prod + 1) % mi->nb_elem;
	sem_post(mi->sem_ptr[NON_PLEIN]);	
	return fi;
}

int init_memory(MemInfo** mi, size_t size_elem, int nb_elem){
	char* sem_name[2];
	char* name = NULL;
	int a = 0;
	
	generateNameForInstance(&(sem_name[NON_VIDE]));
	generateNameForInstance(&(sem_name[NON_PLEIN]));
	
	(*mi) = malloc(sizeof *(*mi));
	if(NULL == (*mi))
		return 1;
	
	(*mi)->sem_ptr = malloc(2 * sizeof *((*mi)->sem_ptr));
	
	 /* Init shared memory */
	generateNameForInstance(&name);
	(*mi)->sh_mem = create_shared_memory(name, nb_elem * size_elem);
	free(name);
	
	if(NULL == (*mi)->sem_ptr ||  NULL == (*mi)->sh_mem)
		return 1;
		
	while(a != nb_elem) {
		
		init_sem(&((*mi)->sem_ptr[NON_VIDE]), sem_name[NON_VIDE], 0);
		init_sem(&((*mi)->sem_ptr[NON_PLEIN]), sem_name[NON_PLEIN], nb_elem);
		
		sem_getvalue((*mi)->sem_ptr[NON_PLEIN], &a);
		if(a != nb_elem){
			sem_unlink(sem_name[NON_VIDE]);
			sem_unlink(sem_name[NON_PLEIN]);
		}	
	}
	
	free(sem_name[NON_VIDE]);
	free(sem_name[NON_PLEIN]);
	
	(*mi)->nb_elem = nb_elem;
	(*mi)->size_elem = size_elem;
	
	return 0;
}	
	
		
void free_memory(MemInfo* mi){
	
	free_shared_memory(mi->sh_mem, (mi->nb_elem * mi->size_elem));
	sem_close(mi->sem_ptr[NON_VIDE]);
	sem_close(mi->sem_ptr[NON_PLEIN]);
	free(mi->sem_ptr);
	free(mi);
	
}
