#include "config.h"

struct thread_arguments{

    int tag;
    int level;
    char* buffer;
    size_t size;
};

void* receive(struct thread_arguments *the_struct){

    printf("Thread: %d. Before receive buffer is :%s",the_struct->buffer);
    syscall(174,the_struct->tag, the_struct->level, the_struct->buffer, the_struct->size); 
    printf("Thread: %d. After receive buffer is :%s",the_struct->buffer);

    return;
}


int main(int argc, char** argv){
	
    int i;
    char *buffer = "PIPPO PLUTO PAPERINO MINNIE ZIOPAPERONE QUI QUO QUA";
    size_t size = 60;
    pthread_t tid[TOTAL_THREADS];
    struct thread_arguments *the_struct[TOTAL_THREADS] = { NULL };

    int tag_descriptor = syscall(134,IPC_PRIVATE,CREATE,PERM_ALL);

    printf("tag descriptor is: %d", tag_descriptor);

    for(i=0; i<5; i++){

        the_struct[i] = malloc(sizeof(struct thread_arguments));
        the_struct[i]->tag = tag_descriptor;
        the_struct[i]->level = 1;
        the_struct[i]->buffer = malloc(sizeof(char)*size);
        the_struct[i]-> size = size;

        pthread_create(&tid[i], NULL, receive, the_struct[i]);

    }

    syscall(156,tag_descriptor, 1, buffer, size);


    for(i=5; i<10; i++){

        the_struct[i] = malloc(sizeof(struct thread_arguments));
        the_struct[i]->tag = tag_descriptor;
        the_struct[i]->level = 1;
        the_struct[i]->buffer = malloc(sizeof(char)*size);
        the_struct[i]-> size = size;

        pthread_create(&tid[i], NULL, receive, the_struct[i]);

    }

    for(i=0;i<10;i++){

        pthread_join(tid[i], 0);
    }

    return 0;
	

}