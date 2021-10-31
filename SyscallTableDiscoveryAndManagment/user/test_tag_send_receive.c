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



void* send(struct thread_arguments *the_struct){

    int ret;

    ret = syscall(156,the_struct->tag, the_struct->level, the_struct->buffer, the_struct->size);
    if(ret != 0){
        printf("Thread: %d. Something went wrong during send\n", the_struct->thread_id);
    }
    
    pthread_exit(NULL);
}



int main(int argc, char** argv){
	
    int i, ret;
    size_t size = 60;
    pthread_t tid[TOTAL_THREADS];
    struct thread_arguments *the_struct[TOTAL_THREADS] = { NULL };
    int tag_descriptor_array[RECEIVERS/100];


    for (i=0;i<RECEIVERS/100;i++){

        tag_descriptor_array[i] = syscall(134,i+1,CREATE,PERM_ALL);
        if(tag_descriptor_array[i] == -1){
            printf("Error in tag service creation for tag service with key: %d\n",10*i);
        }

    }
    


    for(i=0; i<RECEIVERS; i++){

        if((i%2)==0){

            the_struct[i] = malloc(sizeof(struct thread_arguments));
            the_struct[i]->tag = tag_descriptor_array[(i/100)+1];
            the_struct[i]->level = i%2;
            the_struct[i]->buffer = malloc(sizeof(char)*(((i/1000)+1)*6));
            the_struct[i]->size = ((i/1000)+1)*6;
            the_struct[i]->thread_id = i;

            pthread_create(&tid[i], NULL, receive, the_struct[i]);

        }
        

    }

    
    for(i=0; i<SENDERS; i++){

        if((i%2)==0){

            the_struct[RECEIVERS + i] = malloc(sizeof(struct thread_arguments));
            the_struct[RECEIVERS + i]->tag = tag_descriptor_array[(i/10)+1];
            the_struct[RECEIVERS + i]->level = i%2;
            the_struct[RECEIVERS + i]->buffer = buffer;
            the_struct[RECEIVERS + i]->size = ((i/100)+1)*6;
            the_struct[RECEIVERS + i]->thread_id = RECEIVERS + i;

            pthread_create(&tid[RECEIVERS + i], NULL, send, the_struct[RECEIVERS + i]);

        }

        
    }
    


    for(i=0; i<RECEIVERS; i++){

        if((i%2)!=0){

            the_struct[i] = malloc(sizeof(struct thread_arguments));
            the_struct[i]->tag = tag_descriptor_array[(i/100)+1];
            the_struct[i]->level = i%2;
            the_struct[i]->buffer = malloc(sizeof(char)*(((i/1000)+1)*6));
            the_struct[i]->size = ((i/1000)+1)*6;
            the_struct[i]->thread_id = i;

            pthread_create(&tid[i], NULL, receive, the_struct[i]);

        }
        

    }


    
    for(i=0; i<SENDERS; i++){

        if((i%2)!=0){

            the_struct[RECEIVERS + i] = malloc(sizeof(struct thread_arguments));
            the_struct[RECEIVERS + i]->tag = tag_descriptor_array[(i/10)+1];
            the_struct[RECEIVERS + i]->level = i%2;
            the_struct[RECEIVERS + i]->buffer = buffer;
            the_struct[RECEIVERS + i]->size = ((i/100)+1)*6;
            the_struct[RECEIVERS + i]->thread_id = RECEIVERS + i;

            pthread_create(&tid[RECEIVERS + i], NULL, send, the_struct[RECEIVERS + i]);

        }

        
    }

    for(i=0;i<TOTAL_THREADS;i++){

        pthread_join(tid[i], 0);
    }


    for (i=0;i<RECEIVERS/100;i++){


        ret= syscall(177,tag_descriptor_array[i],REMOVE);
        if(ret == -1){
            printf("Error removing tag service with tag descriptor: %d\n",tag_descriptor_array[i]);
        }

    }
    

    return 0;
	

}