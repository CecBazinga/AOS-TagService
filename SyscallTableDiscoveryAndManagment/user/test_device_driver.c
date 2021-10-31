#include "config.h"

int main(int argc, char** argv){

    int i, ret, tag_id, tag_id2;
    size_t size = 60;
    pthread_t tid[RECEIVERS/100];
    struct thread_arguments *the_struct[RECEIVERS/100] = { NULL };

    // creazione tag services
    tag_id = syscall(134,135,CREATE,PERM_ALL);
    if(tag_id == -1){
        printf("Tag create failed!\n");
        return -1;
    }

    tag_id2 = syscall(134,136,CREATE,PERM_ALL);
    if(tag_id2 == -1){
        printf("Tag create failed!\n");
        return -1;
    }

    // creazione receivers
    for(i=0; i<RECEIVERS/100; i++){

        the_struct[i] = malloc(sizeof(struct thread_arguments));
        if(i%2 == 0){
            the_struct[i]->tag = tag_id;
        }else{
             the_struct[i]->tag = tag_id2;
        }
        the_struct[i]->level = i%10;
        the_struct[i]->buffer = malloc(sizeof(char)*size);
        the_struct[i]->size = size;
        the_struct[i]->thread_id = i;

        pthread_create(&tid[i], NULL, receive, the_struct[i]);

    }


    //TODO: generare una pool di thread che utilizzino il device driver 



    //TODO: send che svegli solo alcuni thread su livelli specifici

    //TODO: seconda pool di thread che utilizzino il device

    // awake all con receivers nel tag service: dovrebbe svegliare i thread su tutti i livelli
    ret = syscall(177,tag_id,AWAKE_ALL);
    if(ret == -1){
        printf("Error: awake all failed!\n");
    
    }else if(ret == 0){
        printf("Correct: awake all success !\n");
    }


    sleep(3);

    //TODO: terza pool di thread che utilizzino il device
    

    // attessa dei thread receivers
    for(i=0;i<RECEIVERS/100;i++){
        pthread_join(tid[i], 0);
    }
	
}
