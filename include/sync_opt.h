#ifndef __SYNC_OPT__
#define __SYNC_OPT__

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
typedef struct {
	char* directory; /* the directory specified as arg */
	int is_auto; /* 1 if auto sync enabled, 0 otherwise */
} optArgs;

void usage(char* exec);

int getArgs(int argc, char** argv, optArgs **optargs);

void freeArgs(optArgs *optArgs);

#endif
