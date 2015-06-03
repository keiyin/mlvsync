#ifndef __SYNC_LOG__
#define __SYNC_LOG__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define LEN_LOG_NAME 21
#define FOLDER_LOG "log/"
#define SIZE_BUF_CWD 256

typedef enum {ERROR, WARNING, INFO} LogStatus;

int create_log(void);

int add_log(LogStatus status, char* msg);

int add_logArg(LogStatus status, char* msg, char* arg);

void close_log(void);



#endif
