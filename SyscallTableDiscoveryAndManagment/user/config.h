#include <unistd.h>
#include <stdio.h>
#include <sys/fsuid.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>



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

#define PAGE 4096
#define USER_DEVICE_BUFF 500


char *buffer = "PIPPO PLUTO PAPERINO MINNIE ZIOPAPERONE QUI QUO QUA";

char *device = "/dev/tag_system_device";

bool sync_barrier_1, sync_barrier_2;


struct thread_arguments{

    int tag;
    int level;
    char* buffer;
    size_t size;
    int thread_id;
};
