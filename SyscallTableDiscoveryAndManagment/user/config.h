#include <unistd.h>
#include <stdio.h>
#include <sys/fsuid.h>
#include <pthread.h>
#include <stdlib.h>

//#include <time.h>

#define TAGS 256
#define LEVELS 32

#define IPC_PRIVATE 0

#define CREATE 0
#define OPEN 1

#define PERM_NONE 0
#define PERM_ALL 1

#define AWAKE_ALL 0
#define REMOVE 1

#define TOTAL_THREADS 11000
#define RECEIVERS 10000
#define SENDERS 1000


struct thread_arguments{

    int tag;
    int level;
    char* buffer;
    size_t size;
    int thread_id;
};
