/*-------------------------------------------------------------- 
                 File di test della syscall tag_ctl    
--------------------------------------------------------------*/

#include "config.h"

/* routine dei threads per la ricezione dei messaggi  */
void* receive(struct thread_arguments *the_struct){

    int ret;

    // syscall di receive del modulo
    ret = syscall(174,the_struct->tag, the_struct->level, the_struct->buffer, the_struct->size); 
    if(ret != 0){
        printf("Thread: %d. Something went wrong during receive\n", the_struct->thread_id);
        pthread_exit(NULL);
    }

    printf("Thread: %d. After receive buffer is :%s\n", the_struct->thread_id, the_struct->buffer);
    pthread_exit(NULL);
}



/* funzione per testare la syscal tag_ctl. Verifica che sia la awake all che la remove falliscano in assenza del tag target; 
   quindi crea il tag e verifica che la awake all abbia successo in assenza di receiver in attesa sul tag. Genera quindi un
   insieme di thread "receivers" che si mettono in ascolto su diversi livelli del tag: verifica che la rimozione del tag fallsica
   in quanto è in uso da parte di altri threads.Verifica il corretto funzionamento della awake all nel risvegliare tutti i threads
   in attesa, quindi verifica il corretto funzionamento della remove per la distruzione del tag stesso, ora non più busy. */
int main(int argc, char** argv){

    int i, ret, tag_id;
    size_t size = 60;
    pthread_t tid[RECEIVERS/100];
    struct thread_arguments *the_struct[RECEIVERS/100] = { NULL };

    // fallimento awake all senza tag service istanziato
    ret = syscall(177,34,AWAKE_ALL);
    if(ret == -1){
        printf("Correct: awake all failed because tag doesn't exist!\n");
    
    }else{
        printf("Error on awake all: tag doesn't exist!\n");
    }


    // fallimento remove senza tag service istanziato
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


    // successo awake all senza threads in attesa sul tag service
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


    // fallimento remove quando ci sono receivers sul tag service
    ret = syscall(177,tag_id,REMOVE);
    if(ret == -1){
        printf("Correct: unable to remove tag %d because there are threads waiting on it!\n", tag_id);
    
    }else{
        printf("Error: remove should fail because there are threads waiting on this tag %d!\n", tag_id);
    }

    
    // verifica corretto funzionamento awake all con receivers nel tag service
    ret = syscall(177,tag_id,AWAKE_ALL);
    if(ret == -1){
        printf("Error: awake all failed!\n");
    
    }else if(ret == 0){
        printf("Correct: awake all success !\n");
    }


    sleep(10);

    // verifiac il corretto funzionamento della remove quando non ci sono piu thread in attesa
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


