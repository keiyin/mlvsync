#include "../include/walk_tree.h"
#include "../include/sync_log.h"

static int get_file_info(struct stat* info, char* path){			
	if(stat(path,  info) != 0){
		add_logArg(WARNING, "Problem while getting info for file : ", path);
		return 1;
	}
	return 0;
}

static char* create_fullname(char* str1, char* str2){
	int add = 0;
	int less = 0;
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);	
	char* str = NULL;	
		
	if(str1[len1-1] != '/')
		add = 1;
		
	if(str2[0] == '/')
		less = 1;
		
	if(NULL == (str = calloc((len1 + len2 + 1 - less + add),sizeof *str)))
		return NULL;
	
	memcpy(str, str1, len1);
	
	if(add)
		memcpy(str+len1, "/", 1);
	memcpy(str+(len1+add), str2+less, len2-less);	
	return str;	
}

static int exist_WalkTree(WalkTree* wt, char* name){
	int i;
	for(i=0; i<wt->nb_files; i++){
		if(strcmp(name, wt->name_tab[i]) == 0)
			return i;
	}
	return -1;
}

static int add_WalkTree(WalkTree* wt, char* name, time_t time){
	static int nb_allocated = WALKTREE_ALLOC_DEFAULT;
	static int cur_index = 0;
	
	/* We make sure we have enough space allocated */
	if(nb_allocated == cur_index){
		nb_allocated *= 2;
		if(NULL == ( wt->name_tab = (char**)realloc(wt->name_tab, nb_allocated * sizeof(char*))))
			return -1;
		if(NULL == ( wt->updated = (int*)realloc(wt->updated, nb_allocated * sizeof(int))))
			return -1;
		if(NULL == ( wt->last_time = (time_t*)realloc(wt->last_time, nb_allocated * sizeof(time_t))))
			return -1;
	}
	wt->name_tab[cur_index] = name;	
	if(time != 0)
		wt->last_time[cur_index] = time;
	wt->nb_files++;	
	wt->updated[cur_index] = 1;
	return cur_index ++;
}

int init_WalkTree(WalkTree** wt){
	if(NULL == wt)
		return 1;
	if(NULL == (*wt = (WalkTree*)malloc(sizeof(WalkTree))))
		return 1;		
	if(NULL == ((*wt)->name_tab = (char**)calloc(WALKTREE_ALLOC_DEFAULT, sizeof(char*))))
		return 1;		
	if(NULL == ((*wt)->updated = (int*)calloc(WALKTREE_ALLOC_DEFAULT, sizeof(int))))
		return 1;		
	if(NULL == ((*wt)->last_time = (time_t*)calloc(WALKTREE_ALLOC_DEFAULT, sizeof(time_t))))
		return 1;
	(*wt)->nb_files = 0;
	return 0;	
}

int update_WalkTree(WalkTree* wt,char* src, char* rep){
	/* recursive function */	
	int updated = 0;
	DIR* directory = NULL;
	struct dirent* dir = NULL;	
	char* fullPath = NULL;
	char* fullPathDirectory = NULL;
	struct stat info;
	
	if(wt == NULL || NULL == src || NULL == rep){
		add_log(ERROR, "NULL pointer when updating file list !");
		return -1;	
	}
	if(NULL == (fullPathDirectory = create_fullname(src, rep))){
		add_log(ERROR, "Failure while creating fullname ...");
		return -1;		
	}
	if( NULL == (directory = opendir(fullPathDirectory))){
		perror("opendir");
		return -1;
	}
	while( NULL != (dir = readdir(directory))){	
		char* fullname; /* fullname is free'd in free_walkTree */ 
		
		/* If the repertory if etheir "." or ".." we skip it */
		if((strcmp (dir->d_name, ".") == 0) || (strcmp(dir->d_name, "..") == 0 ))
			continue;
		
		if(NULL == (fullname = create_fullname(rep, dir->d_name)))
			return -1;
		
		if(NULL == (fullPath = create_fullname(src, fullname)))
			return -1;
		
		if(get_file_info(&info, fullPath))
			return -1;
			
		if(S_ISDIR(info.st_mode)){	
		/*	printf("DEBUG : scaning folder %s\n", fullname);	*/			
			updated += update_WalkTree(wt, src, fullname);			
		} else {	
			int ind;	
			
			if(-1 == (ind = exist_WalkTree(wt, fullname))){
				if(NULL == fullname)
					add_log(WARNING, "FULLNAME NULL !!");
			/*	printf("DEBUG : file added : %s\n", fullname);*/
				add_WalkTree(wt, fullname, info.st_mtime);
				updated = 1;
			} else {
				if(wt->last_time[ind] < info.st_mtime){
					/* The file have changed, we update the time */
					wt->last_time[ind] = info.st_mtime;
					wt->updated[ind] = 1;
					updated = 1;				
				}
			}
		}	
	} /* end of while */
	closedir(directory);
	
	free(fullPath);
	free(fullPathDirectory);
	return (updated > 0);	
}

void free_WalkTree(WalkTree** wt){
	int i;
	if(NULL == wt || NULL == *wt)
		return;
	
	for(i=0; i<(*wt)->nb_files; i++)
		if(NULL != (*wt)->name_tab[i])
			free((*wt)->name_tab[i]);
	free((*wt)->name_tab);
	free((*wt)->updated);
	free((*wt)->last_time);
	free((*wt));
	wt = NULL;	
}

int get_NextUpdated(WalkTree* wt, FileInfo* fi){
	static int index = 0;
	int i = 0;
	if(NULL == wt)
		return 0;
	i = wt->nb_files; /* We make sure we don't make endless loop */		
	while(!(wt->updated[index])){
		if(index == wt->nb_files)
			index = 0;
		index++;
		if(i == 0) /* If we look at all the tab, we exit */
			return 0; 
		i--;
	}
	wt->updated[index] = 0; /* We mark the selected file as 'no updated' because we work on it */
	for(i=0; i<LENGTH_WORD; i++)
		fi->name[i] = '\0';	
	memcpy(fi->name, wt->name_tab[index], LENGTH_WORD - 1);
	fi->last_time = wt->last_time[index];
		
	return 1;		
}
