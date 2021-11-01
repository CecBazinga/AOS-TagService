/*-------------------------------------------------------------- 
                 File di test del device driver    
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




/* routine dei threads per l'utilizzo concorrente del device driver  */
void* read_device(struct thread_arguments *the_struct){

    int ret, fd;

    // apertura del device driver
    fd = open(device, O_RDONLY);
    if (fd == -1) {
        printf("Error: open error on device %s\n", device);
        pthread_exit(NULL);
    }

    // allocazione del buffer su cui trasferire i dati letti grazie al device driver
    char *content = malloc(sizeof(char) * USER_DEVICE_BUFF);
    memset(content, 0, sizeof(char) * USER_DEVICE_BUFF);
    if (content == NULL) {
        printf("Error: allocation error for thread %d\n", the_struct->thread_id);
        pthread_exit(NULL);

    }else{

        // lettura dei dati
        ret = read(fd, content, USER_DEVICE_BUFF);
        if (ret < 0){
            printf("Error during read from thread %d\n", the_struct->thread_id);
        }else{
            printf("Thread %d retrieved from device file: \n%s\n", the_struct->thread_id, content);
        }
    }

    // deallocazione del buffer e successiva riallocazione per testare l'aggiornamento della read del device all'interno di un'unica sessione di lavoro
    free(content);
    content = malloc(sizeof(char) * USER_DEVICE_BUFF);
    memset(content, 0, sizeof(char) * USER_DEVICE_BUFF);

    while(!sync_barrier_1){}
    
    // nuova lettura dei dati dopo che lo stato del modulo è cambiato
    ret = pread(fd, content, USER_DEVICE_BUFF,0);
    if (ret < 0){
        printf("Error during read from thread %d\n", the_struct->thread_id);
    }else{
        printf("Thread %d retrieved from device file: \n%s\n", the_struct->thread_id, content);
    }

    // deallocazione del buffer e successiva riallocazione per testare l'aggiornamento della read del device all'interno di un'unica sessione di lavoro
    free(content);
    content = malloc(sizeof(char) * USER_DEVICE_BUFF);
    memset(content, 0, sizeof(char) * USER_DEVICE_BUFF);
 

    while(!sync_barrier_2){}
    
    // nuova lettura dei dati dopo che lo stato del modulo è cambiato
    ret = pread(fd, content, USER_DEVICE_BUFF,0);
    if (ret < 0){
        printf("Error during read from thread %d\n", the_struct->thread_id);
    }else{
        printf("Thread %d retrieved from device file: \n%s\n", the_struct->thread_id, content);
    }

    // chiusura del device e fine della sessione per questo thread; deallocazione delle risorse 
    close(fd);
    free(content);
    pthread_exit(NULL);

}


/* funzione per testare l'utilizzo del device driver in modo concorrente e per testarne il corretto funzionamento (aggiornamento) 
   in caso di read mulitple all'interno di un'unica sessione. La funzione crea 2 tags e genera un numero di receivers che mette in
   attesa di messaggi su vari livelli di questi tags.Genera quindi degli utilizzatori del device per leggere le informazioni del modulo.
   Effettua una send di un messaggio su alcuni livelli dei tag al fine di modificare le informazioni del tag e di verifiacre che tali 
   modifiche vengano osservate dagli utilizzatori del device driver. Utilizza quindi due awake all per risvegliare i threads rimasti in 
   attesa sui due tag e verifica nuovamente che questa modifica dei dati relativi al modulo venga catturata dagli utilizzatori del device.
   Quindi distrugge i due tag.
*/
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


    // creazione tag 
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

    // syscall per la send del messaggio su un livello di un tag
    ret = syscall(156,tag_id, 2, buffer, 52);
    if(ret != 0){
        printf("Error during send!\n");
    }

    sleep(10);
    sync_barrier_1 = true;
    sleep(10);

    // awake all di tutti i thread su tutti i livelli per ognuno dei due tag
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
	
    
    // rimozione dei tag creati
    ret = syscall(177,tag_id,REMOVE);
    if(ret == -1){
        printf("Error: awake all failed!\n");

    }

    ret = syscall(177,tag_id2,REMOVE);
    if(ret == -1){
        printf("Error: awake all failed!\n");

    }

}
