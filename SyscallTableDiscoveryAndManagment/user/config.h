#include <unistd.h>
#include <stdio.h>
#include <sys/fsuid.h>
#include <pthread.h>

//#include <time.h>

#define TAGS 256
#define LEVELS 32

#define IPC_PRIVATE 0

#define CREATE 0
#define OPEN 1

#define PERM_NONE 0
#define PERM_ALL 1

#define TOTAL_THREADS 10