#include "config.h"

void* receive(struct thread_arguments *the_struct){

    int ret;

    ret = syscall(174,the_struct->tag, the_struct->level, the_struct->buffer, the_struct->size); 
    if(ret != 0){
        printf("Thread: %d. Something went wrong during receive\n", the_struct->thread_id);
         pthread_exit(NULL);
    }

    printf("Thread: %d. After receive buffer is :%s\n", the_struct->thread_id, the_struct->buffer);
    pthread_exit(NULL);
}



void* read_device(struct thread_arguments *the_struct){

    int ret, fd;

    // apertura del device driver
    fd = open(device, O_RDONLY);
    if (fd == -1) {
        printf("Error: open error on device %s\n", device);
        pthread_exit(NULL);
    }

    char *content = malloc(sizeof(char) * USER_DEVICE_BUFF);
    memset(content, 0, sizeof(char) * USER_DEVICE_BUFF);
    if (content == NULL) {
        printf("Error: allocation error for thread %d\n", the_struct->thread_id);
        pthread_exit(NULL);

    }else{
        ret = read(fd, content, USER_DEVICE_BUFF);
        if (ret < 0){
            printf("Error during read from thread %d\n", the_struct->thread_id);
        }else{
            printf("Thread %d retrieved from device file: \n%s\n", the_struct->thread_id, content);
        }
    }

    free(content);
    content = malloc(sizeof(char) * USER_DEVICE_BUFF);
    memset(content, 0, sizeof(char) * USER_DEVICE_BUFF);

    while(!sync_barrier_1){}
    
    ret = pread(fd, content, USER_DEVICE_BUFF,0);
    if (ret < 0){
        printf("Error during read from thread %d\n", the_struct->thread_id);
    }else{
        printf("Thread %d retrieved from device file: \n%s\n", the_struct->thread_id, content);
    }

    free(content);
    content = malloc(sizeof(char) * USER_DEVICE_BUFF);
    memset(content, 0, sizeof(char) * USER_DEVICE_BUFF);
 

    while(!sync_barrier_2){}
    

    ret = pread(fd, content, USER_DEVICE_BUFF,0);
    if (ret < 0){
        printf("Error during read from thread %d\n", the_struct->thread_id);
    }else{
        printf("Thread %d retrieved from device file: \n%s\n", the_struct->thread_id, content);
    }


    close(fd);
    free(content);
    pthread_exit(NULL);

}



int main(int argc, char** argv){

    sync_barrier_1 = false;
    sync_barrier_2 = false;
    int i, ret, tag_id, tag_id2;
    size_t size = 60;
    int device_readers = 10;
    int receivers = RECEIVERS/100;
    int total_threads = receivers + device_readers;
    pthread_t tid[total_threads];
    struct thread_arguments *the_struct[(RECEIVERS/100)+10] = { NULL };




    // creazione tag services
    tag_id = syscall(134,135,CREATE,PERM_ALL);
    if(tag_id == -1){
        printf("Erro: tag create failed!\n");
        return -1;
    }

    tag_id2 = syscall(134,136,CREATE,PERM_ALL);
    if(tag_id2 == -1){
        printf("Error: tag create failed!\n");
        return -1;
    }

    // creazione receivers
    for(i=0; i<receivers; i++){

        the_struct[i] = malloc(sizeof(struct thread_arguments));
        if(i%2 == 0){
            the_struct[i]->tag = tag_id;
        }else{
             the_struct[i]->tag = tag_id2;
        }
        the_struct[i]->level = i%4;
        the_struct[i]->buffer = malloc(sizeof(char)*size);
        the_struct[i]->size = size;
        the_struct[i]->thread_id = i;

        pthread_create(&tid[i], NULL, receive, the_struct[i]);

    }


    // creazione device readers
    for(i=receivers; i<receivers + device_readers; i++){

        the_struct[i] = malloc(sizeof(struct thread_arguments));
        the_struct[i]->tag = -1;
        the_struct[i]->level = -1;
        the_struct[i]->buffer = NULL;
        the_struct[i]->size = -1;
        the_struct[i]->thread_id = i;
       

        pthread_create(&tid[i], NULL, read_device, the_struct[i]);

    }

    sleep(10);

    ret = syscall(156,tag_id, 2, buffer, 52);
    if(ret != 0){
        printf("Error during send!\n");
    }

    sleep(10);
    sync_barrier_1 = true;


    sleep(10);
    // awake all con receivers nel tag service: dovrebbe svegliare i thread su tutti i livelli
    ret = syscall(177,tag_id,AWAKE_ALL);
    if(ret == -1){
        printf("Error: awake all failed!\n");
    }

    ret = syscall(177,tag_id2,AWAKE_ALL);
    if(ret == -1){
        printf("Error: awake all failed!\n");

    }

    sleep(10);
    sync_barrier_2 = true;


    // attessa dei thread receivers e device readers
    for(i=0;i<total_threads;i++){
        pthread_join(tid[i], 0);
    }
	
    
    ret = syscall(177,tag_id,REMOVE);
    if(ret == -1){
        printf("Error: awake all failed!\n");

    }

    ret = syscall(177,tag_id2,REMOVE);
    if(ret == -1){
        printf("Error: awake all failed!\n");

    }

}
