#include "../include/sync_log.h"

static int log_fd;

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
		
	if(NULL == (str = (char*)calloc((len1 + len2 + 1 - less + add),sizeof(char))))
		return NULL;
	
	memcpy(str, str1, len1);
	
	if(add)
		memcpy(str+len1, "/", 1);
	memcpy(str+(len1+add), str2+less, len2-less);	
	return str;	
}

static void generate_logfilename(char** name){
	/* name like : AAAA_MM_JJ-HH_MM.log */
	time_t test;
	struct tm * tm_info;
	
	*name = (char*)calloc(LEN_LOG_NAME, sizeof(char));
	if(NULL == *name)
		return;
		
	time(&test);
	tm_info = localtime(&test);
	
	sprintf(*name, "%d_%d_%d-%d_%d.log", tm_info->tm_year + 1900,
										tm_info->tm_mon + 1,
										tm_info->tm_mday,
										tm_info->tm_hour,
										tm_info->tm_min);
}

int create_log(void){
	char* name = NULL;
	char* str = NULL;
	char* logPath = NULL;
	char* src;
	int fd;
	
	src = malloc(SIZE_BUF_CWD * sizeof(char));
	if(NULL == src)
		return 0;
		
	if(NULL == getcwd(src, SIZE_BUF_CWD))
		return 0;
	
	generate_logfilename(&name);
	
	if(NULL == name)
		return -1;
			
	str = create_fullname(FOLDER_LOG, name);
	logPath = create_fullname(src, str);
	
	
		
	if(-1 == (fd = open(logPath, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR))){
		perror("MLVSync");
		return -1;
	}
	
	free(str);
	free(src);
	free(name);
	free(logPath);
	log_fd = fd;
	return fd;
}

int add_log(LogStatus status, char* msg){
	int len;
	/* log entry like '[status]msg\n' */
	/* status can be ERROR, WARNING, INFO */
	
	if(NULL == msg)
		return 0;
	
	switch (status){
		case INFO :
			if(6 != write(log_fd, "[INFO]", 6))
				return 0;
			break;
		case WARNING :
			if(9 != write(log_fd, "[WARNING]", 9))
				return 0;
			break;
		case ERROR :
			if(7 != write(log_fd, "[ERROR]", 7))
				return 0;
			break;
		default :
			break;
		}
		write(log_fd, " ", 1);
		len = strlen(msg);
		if(len != write(log_fd, msg, len))
			return 0;;
		return (1 == write(log_fd, "\n", 1)); 
}

int add_logArg(LogStatus status, char* msg, char* arg){
	
	char *str = NULL;
	int len1, len2;
	len1 = strlen(msg);
	len2 = strlen(arg);
	
	str = (char*)calloc(len1 + len2 + 1, sizeof(char));
	if(NULL == str){
		add_log(ERROR, "calloc failed in add_logArg");
		return 0;
	}
	memcpy(str, msg, len1);
	memcpy(str + len1, arg, len2);
	add_log(status, str);
	free(str);
	return 1;
}

void close_log(void){
	close(log_fd);
}


