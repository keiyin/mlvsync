#ifndef __WALK_TREE__
#define __WALK_TREE__


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define LENGTH_WORD 500
#define WALKTREE_ALLOC_DEFAULT 16
#define SLEEP_TIME 1

typedef struct {
	char** name_tab;
	int * updated;
	time_t* last_time;
	int nb_files;
} WalkTree;

typedef struct {
	char name[LENGTH_WORD];
	time_t last_time;
} FileInfo;

/* init the WalkTree struct, return 0 in case of succes, 1 otherwise */
int init_WalkTree(WalkTree** wt);

/* update the struct to get the information about the files in 'rep'
 * return 1 in case of files updated or created, 0 if nothing happen */
int update_WalkTree(WalkTree* wt,char* src, char* rep);

/* Free the ressources of the struct and set *wt at NULL */
void free_WalkTree(WalkTree** wt);

/* return the next file updated in wt, or NULL */
int get_NextUpdated(WalkTree* wt, FileInfo* fi);

#endif
