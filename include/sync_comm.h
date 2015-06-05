#ifndef __SYNC_COMM__
#define __SYNC_COMM__

#define _XOPEN_SOURCE 666

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h>          
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include "walk_tree.h"

#define NB_FILE 4
#define LEN_MIN 4
#define LEN_MAX 15
#define EXEC "MLVSync"
#define NON_VIDE 0
#define NON_PLEIN 1

typedef struct {
	FileInfo* sh_mem;
	sem_t** sem_ptr;
	int nb_elem;
	int size_elem;
} MemInfo;

void* create_shared_memory(char* name_mem, size_t size);

int init_memory(MemInfo** mi, size_t size_elem, int nb_elem);

void free_memory(MemInfo* mi);

void put_file(FileInfo* fi, MemInfo* mi);

FileInfo* get_file(MemInfo* mi);

#endif
