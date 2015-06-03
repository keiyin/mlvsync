#include "../include/sync_opt.h"

void usage(char* exec){
	printf("Usage : %s -d directory -a\n\t-d : the directory to sync\n\t-a : auto sync", exec);
}

int getArgs(int argc, char** argv, optArgs **optargs){
	int c;
	
	if(NULL == (*optargs = malloc( sizeof(optArgs)))){
		return 1;
	}
	
	while ((c = getopt (argc, argv, "d:a")) != -1){
		 switch(c){
			case 'd' :;
				int len = strlen(optarg);
				(*optargs)->directory = calloc((len + 1), sizeof(char));
				memcpy((*optargs)->directory,optarg, len);
				break;
			case 'a':
				/* synchro auto */
				(*optargs)->is_auto = 1;
				break;				
			default :
				usage(argv[0]);
				exit(EXIT_FAILURE);
				break;		 
		 }
		 
	 }	 
	 
	if(NULL == (*optargs)->directory)
		return 1;
	return 0;
	
}

void freeArgs(optArgs *optargs){
	
	free(optargs->directory);
	free(optargs);
}

