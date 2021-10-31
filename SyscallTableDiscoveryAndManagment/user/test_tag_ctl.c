#include "config.h"


int main(int argc, char** argv){

    int i, ret, tag_id;
    size_t size = 60;
    pthread_t tid[RECEIVERS/100];
    struct thread_arguments *the_struct[RECEIVERS/100] = { NULL };

    // awake all senza tag service istanziato: dovrebbe fallire
    ret = syscall(177,34,AWAKE_ALL);
    if(ret == -1){
        printf("Correct: awake all failed because tag doesn't exist!\n");
    
    }else{
        printf("Error on awake all: tag doesn't exist!\n");
    }


    // remove senza tag service istanziato: dovrebbe fallire
    ret = syscall(177,34,REMOVE);
    if(ret == -1){
        printf("Correct: remove failed because tag doesn't exist!\n");
    
    }else{
        printf("Error on remove: tag doesn't exist!\n");
    }

    // creazione tag service
    tag_id = syscall(134,35,CREATE,PERM_ALL);
    if(tag_id == -1){
        printf("Tag create failed!\n");
        return -1;
    }


    // awake all senza threads in attesa sul tag service: dovrebbe andare a buon fine
    ret = syscall(177,tag_id,AWAKE_ALL);
    if(ret == -1){
        printf("Error: awake all failed!\n");
    
    }else if(ret == 0){
        printf("Correct: awake all success with no threads waiting!\n");
    }


    // creazione receivers
    for(i=0; i<RECEIVERS/100; i++){

        the_struct[i] = malloc(sizeof(struct thread_arguments));
        the_struct[i]->tag = tag_id;
        the_struct[i]->level = i%10;
        the_struct[i]->buffer = malloc(sizeof(char)*size);
        the_struct[i]->size = size;
        the_struct[i]->thread_id = i;

        pthread_create(&tid[i], NULL, receive, the_struct[i]);

    }


    // remove quando ci sono receivers sul tag service: dovrebbe fallire
    ret = syscall(177,tag_id,REMOVE);
    if(ret == -1){
        printf("Correct: unable to remove tag %d because there are threads waiting on it!\n", tag_id);
    
    }else{
        printf("Error: remove should fail because there are threads waiting on this tag %d!\n", tag_id);
    }

    
    // awake all con receivers nel tag service: dovrebbe svegliare i thread su tutti i livelli
    ret = syscall(177,tag_id,AWAKE_ALL);
    if(ret == -1){
        printf("Error: awake all failed!\n");
    
    }else if(ret == 0){
        printf("Correct: awake all success !\n");
    }


    sleep(10);
    // remove quando non ci sono piu thread in attesa: dovrebbe avere successo
    ret = syscall(177,tag_id,REMOVE);
    if(ret == -1){
        printf("Error: remove failed!\n");
    
    }else if(ret == 0){
        printf("Correct: removed successfully tag %d !\n", tag_id);
    }
    

    // attessa dei thread receivers
    for(i=0;i<RECEIVERS/100;i++){
        pthread_join(tid[i], 0);
    }
	
}


