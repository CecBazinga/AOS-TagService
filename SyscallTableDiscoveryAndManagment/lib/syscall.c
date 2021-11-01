/*------------------------------------------------------------------------------- 
                           Syscalls file    
-------------------------------------------------------------------------------*/

#include "../include/syscall.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Amici <a.amici@outlook.it>");
MODULE_DESCRIPTION("TAG SERVICE");


/* strutture dati */


/* array che mantiene la mappa dei tag in uso e quelli liberi, indicando anche la natura del tag (privata o pubblica)  */
int tag_descriptors_header_list[TAGS] = {[0 ... (TAGS-1)] -1};

/* spinlock associato alla mappa dei tag liberi od in uso  */
spinlock_t tag_descriptors_header_lock;


/* array che mantiene i puntatori alle strutture dati contenti le informazioni di ogni tag  */
struct tag_descriptor_info *tag_descriptors_info_array[TAGS] = { NULL };

/* array che mantiene i puntatori alle strutture dati utilizzate per implementare i tag */
struct tag *tags[TAGS] = { NULL };

/* array di lock read_write per l'accesso alle strutture dati dei tag e quelle contenenti le loro informazioni */
rwlock_t lock_array[TAGS];





/* funzioni utils */


/* funzione che ritorna il puntatore all'array dei tag */
struct tag** get_tag_array_ptr(void){

    return tags;
}


/* funzione che ritorna il puntatore all'array delle informazioni dei tag */
struct tag_descriptor_info** get_tag_info_array_ptr(void){

    return tag_descriptors_info_array;
}


/* funzione che ritorna il puntatore all'array dei lock read_write relativi ai tag */
rwlock_t* get_tag_lock_array_ptr(void){

    return lock_array;
}


/* funzione che alloca ed inizializza le strutture dati necessarie al modulo (l'array per le info dei singoli tag service e l'array di rwlocks) */
int init_tag_service(void){

    int i;

    for(i=0; i<TAGS; i++){

        // allocazione strutture dati per il mantenimento delle informazioni dei tag
        tag_descriptors_info_array[i] = kzalloc(sizeof(struct tag_descriptor_info), GFP_KERNEL);
        if(tag_descriptors_info_array[i] == NULL){
            printk(KERN_ERR "%s: Error during tag service structure allocation! \n", MODNAME);
            return -1;
        }

        // inizializzazione valori 
        tag_descriptors_info_array[i]->key = -1;
        tag_descriptors_info_array[i]->perm = -1;

        // inizializzazione degli rwlock
        rwlock_init(&lock_array[i]);
    }

    printk("%s: Tag service structures have been initialized succesfully! \n", MODNAME);
    return 0;
}



/* funzione che dealloca le strutture dati relative al modulo */
void free_tag_service(void){

    int i,j;

    for(i=0; i<TAGS; i++){

        if(tag_descriptors_info_array[i] != NULL){

            // deallocazione delle strutture dati per le informazioni dei tag
            kfree(tag_descriptors_info_array[i]); 
            tag_descriptors_info_array[i] = NULL;

            if(tags[i] != NULL){
              
                for(j=0;j<LEVELS;j++){

                    // deallocazione dei livelli (generalmente superflua perchè operata dai receivers)
                    if(tags[i]->levels[j] != NULL){

                        if(tags[i]->levels[j]->buffer != NULL){
                            kfree(tags[i]->levels[j]->buffer);
                        }

                        kfree(tags[i]->levels[j]);
                    }
                }

                // deallocazione delle strutture dati relative ai tag veri e propri
                kfree(tags[i]);
                tags[i] = NULL;
            }
        }
    }

    printk(KERN_INFO "%s: Tag service structures have been removed succesfully! \n", MODNAME);
    return;
}



/* funzione che permette la creazione della struttura dati realtiva al tag */
struct tag* create_tag(void){

    int i;

    // allocazione della struttura dati relativa al tag
    struct tag *the_tag = kzalloc(sizeof(struct tag), GFP_KERNEL);

    // errore durante l'allocazione di memoria
    if(the_tag == NULL){
        return NULL;
    }

    // inizializzazione dei puntatori ai livelli del tag a null
    for(i=0; i<LEVELS; i++){
        the_tag->levels[i] = NULL;
    }

    return the_tag;

}




/* funzione che permette di creare un nuovo tag: popola la struttura relativa alle informazioni del tag con i valori di creazione di quest
   ultimo e chiama la funzione di creazione della stuttura dati relativa al tag vero e proprio*/
int insert_tag_descriptor(int key, int tag_descriptor, int permission){

    // accedo alla struct corrispondente per inserire i valori della create 
    write_lock(&lock_array[tag_descriptor]);

    // controllo che le strutture dati associate al tag non siano popolate
    if(tag_descriptors_info_array[tag_descriptor]->key != -1){

        write_unlock(&lock_array[tag_descriptor]);
        printk(KERN_ERR "%s: Error during tag service structure creation! \n", MODNAME);
        return -1;
        
    }else{

        // inizializzo i valori di info del tag service
        tag_descriptors_info_array[tag_descriptor]->key = key ;
        tag_descriptors_info_array[tag_descriptor]->perm = permission ;
        tag_descriptors_info_array[tag_descriptor]->euid = get_current_user()->uid;

        // inizializzo il tag service
        tags[tag_descriptor] = create_tag();
        if(tags[tag_descriptor]==NULL){
            printk(KERN_ERR "%s: Error during tag creation! \n", MODNAME);
            return -1;
        }
       
        write_unlock(&lock_array[tag_descriptor]);
        printk(KERN_INFO "%s: Tag service infos and tag struct correctly created with key: %d  and tag-descriptor : %d \n", MODNAME, key, tag_descriptor);

        return tag_descriptor;
    }
}




/* funzione che controlla i permessi associati ad un tag */
int check_tag_permission(int tag_descriptor){

    // caso in cui il tag è accessibile solo all'utente creatore dello stesso
    if(tag_descriptors_info_array[tag_descriptor]->perm == 0){

        if(tag_descriptors_info_array[tag_descriptor]->euid.val == get_current_user()->uid.val){
            return tag_descriptor;
        
        }else{
            return -1;
        }
            
    }
    // caso in cui il tag è accessibile a tutti gli utenti
    else if(tag_descriptors_info_array[tag_descriptor]->perm == 1){

        return tag_descriptor;

    }else{
        return -1;
    }

}



/* funzione utilizzata per deallocare un livello relativo ad un tag e liberarne le risorse (chiamata solo dai receivers) */
void remove_and_deallocate_level(int tag, int level){

    spin_lock(&(tags[tag]->levels_locks[level]));

    // decremento il numero di receivers in attesa sul livello    
    tags[tag]->levels[level]->threads_waiting --;

    // se sono l'ultimo receiver rimasto dealloco le strutture dati del livello e ne imposto il puntatore nell'array del tag a null
    if(tags[tag]->levels[level]->threads_waiting == 0){

        if(tags[tag]->levels[level]->buffer != NULL){
            kfree(tags[tag]->levels[level]->buffer);
        }

        kfree(tags[tag]->levels[level]);
        tags[tag]->levels[level] = NULL;

    }

    spin_unlock(&(tags[tag]->levels_locks[level]));

    return ;
}



/* funzione che permette di creare tag o di aprire tag già esistenti.
   Il comando CREATE è usato per la creazione dei tag, mentre OPEN per l'apertura di tag pubblici precedentemente creati.
   L'apertura di tag privati non è consentita. La chiave viene fornita dall'utente ed è un valore intero positivo: il valore 0
   indica una chiave privata che non può essere usata per aprire il tag ma solo per creare tag privati. Un valore della chiave
   strettamente positivo è associato ai tag pubblici e viene utilizzato nella open per aprire il tag corrispondente (se creato in 
   precedenza).I permessi indicano, nella fase di creazione del tag, se questo possa essere accessibile al solo utente creatore del
   tag o anche agli altri utenti. */
int tag_get(int key, int command, int permission){

    int i;
    int tag_descriptor = -1;

    // controllo dei valori del comando
    if (command != OPEN && command != CREATE){
        printk(KERN_ERR "%s: Invalid command flag: chose one of OPEN or CREATE! \n", MODNAME);
        return -1;
    }

    // controllo dei valori del permesso
    if (permission != PERM_NONE && permission != PERM_ALL){
        printk(KERN_ERR "%s: Invalid permission flag: chose one of PERM_NONE or PERM_ALL! \n", MODNAME);
        return -1;
    }


    // caso della chiave privata
    if (key == IPC_PRIVATE){

        printk(KERN_INFO "%s: Macro IPC_PRIVATE value is : %d \n", MODNAME, key);
    
        // la open su chiave privata non è permessa
        if(command == OPEN){
            printk(KERN_ERR "%s: Cannot open a tag-service with IPC_PRIVATE key! \n", MODNAME);
            return -1;
        }

        // creazione del tag con chiave privata 
        if(command == CREATE){

            // recupero del primo slot libero per un nuovo tag-service
            spin_lock(&tag_descriptors_header_lock);

            for(i=0; i<TAGS ; i++){

                if (tag_descriptors_header_list[i] == -1){
                
                    tag_descriptors_header_list[i] = 0;
                    spin_unlock(&tag_descriptors_header_lock);
                    tag_descriptor = i;
                    break;
                }
            }

    
            // caso in cui non si hanno piu slot liberi a disposizione
            if (tag_descriptor == -1){
                spin_unlock(&tag_descriptors_header_lock);
                printk(KERN_ERR "%s: No more room left to instantiate new tag-services! \n", MODNAME);
                return -1;
            }

            // creazione del nuovo tag e strutture dati associate
            return insert_tag_descriptor(key,tag_descriptor,permission);
        }

        return -1;


    }
    // caso della chiave pubblica
    else if (key > 0){

        // creazione del tag con chiave pubblica 
        if(command == CREATE){

            spin_lock(&tag_descriptors_header_lock);

            for(i=0; i<TAGS ; i++){

                // salvo momentaneamente il valore del primo tag-service libero se esistente
                if (tag_descriptors_header_list[i] == -1 && tag_descriptor == -1 ){
                    tag_descriptor = i;
                }

                //verifico che la chiave non sia gia in uso
                if(tag_descriptors_header_list[i] == key){
                    spin_unlock(&tag_descriptors_header_lock);
                    printk(KERN_ERR "%s: Error during tag service creation: key already in use! \n", MODNAME);
                    return -1;
                }
            }

            // caso in cui non si hanno piu slot liberi a disposizione
            if (tag_descriptor == -1){
                spin_unlock(&tag_descriptors_header_lock);
                printk(KERN_ERR "%s: No more room left to instantiate new tag-services! \n", MODNAME);
                return -1;
            }

            // se ho trovato lo slot libero e la chiave non è gia in uso registro il nuovo tag service
            tag_descriptors_header_list[tag_descriptor] = key;
            spin_unlock(&tag_descriptors_header_lock);

            // creazione del nuovo tag e strutture dati associate
            return insert_tag_descriptor(key,tag_descriptor,permission);
        }

        // apertura del tag con chiave pubblica 
        if(command == OPEN){

            // controllo che la chiave esista
            spin_lock(&tag_descriptors_header_lock);

            for(i=0; i<TAGS ; i++){

                if(tag_descriptors_header_list[i] == key){
                    tag_descriptor = i;
                    spin_unlock(&tag_descriptors_header_lock);
                }
            }

            // se non esiste sollevo un errore
            if(tag_descriptor == -1){
                spin_unlock(&tag_descriptors_header_lock);
                printk(KERN_ERR "%s: No tag service found for this key! \n", MODNAME);
                return -1;
            }

            // controllo la chiave associata al tag service
            read_lock(&lock_array[tag_descriptor]);

            if(tag_descriptors_info_array[tag_descriptor]->key != key){

                read_unlock(&lock_array[tag_descriptor]);
                printk(KERN_ERR "%s: Error during tag service opening with key %d! \n", MODNAME, key);
                return -1;
            }

            // controllo i permessi associati al tag service
            int check = check_tag_permission(tag_descriptor);

            read_unlock(&lock_array[tag_descriptor]);

            if(check == -1){
                printk(KERN_ERR "%s: Cannot open tag-service with key %d : insufficient permissions or tag service corrupted! \n", MODNAME, key);
                return -1;
            
            }
            
            return check;

        }

        return -1;


    }else{
        printk(KERN_ERR "%s: Invalid key value: chose IPC_PRIVATE or an integer greater than 0! \n", MODNAME);
        return -1;
    }

}





/* funzione che permette di inviare un messaggio su un dato livello di un dato tag in modo non bloccante, trasferendo i dati
   dal livello user al livello kernel. */
int tag_send(int tag, int level, char* buffer, size_t size){

    // controllo il valore della size
    if(size<1 || size>MAXSIZE){
        printk(KERN_ERR "%s: Buffer's size must be in between 1 for empty buffers and %d which is maxsize allowed! \n", MODNAME, MAXSIZE);
        return -1;
    }

    // controllo i permessi associati al tag service
    read_lock(&lock_array[tag]);

    if(check_tag_permission(tag) == -1){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Cannot access tag-service with tag descriptor %d : insufficient permissions or tag service corrupted or doesn't exist! \n", MODNAME, tag);
        return -1;
    
    }


    // se i permessi sono corretti accedo al tag service per l' invio del messaggio
    if(tags[tag]==NULL){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Error in tag service with tag descriptor %d this tag doesn't exist! \n", MODNAME, tag);
        return -1;
    }

    // accedo al livello desiderato
    spin_lock(&(tags[tag]->levels_locks[level]));

    // se il livello non è inizializzato non ci sono receivers, quindi scarto il messaggio
    if(tags[tag]->levels[level]==NULL){
        spin_unlock(&(tags[tag]->levels_locks[level]));
        read_unlock(&lock_array[tag]);
        printk(KERN_INFO "%s: There are no receivers for this message on tag %d and level %d, ence has been discarded!\n", MODNAME, tag, level);
        return 0;
    }


    // se il buffer è già in uso vuol dire che è in atto un'altra send e questa viene scartata
    if(tags[tag]->levels[level]->buffer != NULL){
        spin_unlock(&(tags[tag]->levels_locks[level]));
        read_unlock(&lock_array[tag]);
        printk(KERN_INFO "%s: Buffer already in use on tag %d and level %d, ence this send has been discarded!\n", MODNAME, tag, level);
        return 0;
    }

    
    // copio il messaggio nel buffer del livello del tag service
    tags[tag]->levels[level]->buffer = kzalloc(size*sizeof(char), GFP_KERNEL);

    if(tags[tag]->levels[level]->buffer == NULL){
        spin_unlock(&(tags[tag]->levels_locks[level]));
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Error during buffer allocation for tag service %d on level %d! \n", MODNAME, tag, level);
        return -1;
    }

    int copied_byte = copy_from_user(tags[tag]->levels[level]->buffer,buffer,size);

    // verifico che la copia abbia esito positivo, altrimenti libero il buffer e lo reimposto a NULL
    if(copied_byte != 0){
        kfree(tags[tag]->levels[level]->buffer);
        tags[tag]->levels[level]->buffer = NULL;
        spin_unlock(&(tags[tag]->levels_locks[level]));
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Error copying message on tag %d and level %d\n",MODNAME, tag, level);
        return -1;
    }

    // imposto la condizione di risveglio
    tags[tag]->levels[level]->awake = 1;
    tags[tag]->levels[level]->size = size;

    // sveglio tutti i receivers sulla wait queue
    wake_up_all(&(tags[tag]->levels[level]->wq));

    // rilascio il lock esclusivo sul livello 
    spin_unlock(&(tags[tag]->levels_locks[level]));

    // rilascio il read lock sul tag
    read_unlock(&lock_array[tag]);
    
    return 0;

}






/* funzione che permette di mettersi in ascolto per un messaggio su un dato livello di un dato tag in modo bloccante, trasferendo i dati
   dal livello kernel al livello user. */
int tag_receive(int tag, int level, char* buffer, size_t size){

    // controllo il valore della size
    if(size<1 || size>MAXSIZE){
        printk(KERN_ERR "%s: Buffer's size must be in between 1 for empty buffers and %d which is maxsize allowed! \n", MODNAME, MAXSIZE);
        return -1;
    }

    // controllo i permessi associati al tag service
    read_lock(&lock_array[tag]);

    if(check_tag_permission(tag) == -1){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Cannot access tag-service with tag descriptor %d : insufficient permissions or tag service corrupted or doesn't exist! \n", MODNAME, tag);
        return -1;
    
    }

    // se i permessi sono corretti accedo al tag service per la ricezione del messaggio
    if(tags[tag]==NULL){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Error in tag service with tag descriptor %d this tag doesn't exist! \n", MODNAME, tag);
        return -1;
    }


    // accedo al livello desiderato
    spin_lock(&(tags[tag]->levels_locks[level]));

    // se il livello non è inizializzato non ci sono receivers, quindi lo inizializzo ed alloco un nuovo buffer
    if(tags[tag]->levels[level]==NULL){

        // inizializzo la struct level, il suo buffer, la sua waitqueue ed il numero di thread in waiting su quel livello
        tags[tag]->levels[level] = kzalloc(sizeof(struct tag_level), GFP_KERNEL);

        if(tags[tag]->levels[level] == NULL){
            spin_unlock(&(tags[tag]->levels_locks[level]));
            read_unlock(&lock_array[tag]);
            printk(KERN_ERR "%s: Error during level structure allocation for tag service %d on level %d! \n", MODNAME, tag, level);
            return -1;
        }

        tags[tag]->levels[level]->buffer = NULL;
        tags[tag]->levels[level]->threads_waiting = 1;
        tags[tag]->levels[level]->awake = 0;
        tags[tag]->levels[level]->size = 0;
        init_waitqueue_head (&(tags[tag]->levels[level]->wq));
        
    }else{

        // se il livello è gia stato instanziato incremento il numero di thread in attesa
        tags[tag]->levels[level]->threads_waiting ++; 
    }

    // libero il lock esclusivo sul livello
    spin_unlock(&(tags[tag]->levels_locks[level]));

    //rilascio il read lock sulla cella del tag service info
    read_unlock(&lock_array[tag]);

    printk(KERN_INFO "%s: Threads waiting value is: %d! \n", MODNAME,tags[tag]->levels[level]->threads_waiting);

    // mando il receiver in sleep sulla waitqueue
    wait_event_interruptible(tags[tag]->levels[level]->wq, tags[tag]->levels[level]->awake == 1);

    // routine di copia del buffer da eseguire al risveglio dalla waitqueue

    // risveglio dovuto ad un segnale
    if(tags[tag]->levels[level]->awake == 0){
        
        // controllo se sia necessario deallocare il livello
        remove_and_deallocate_level(tag, level);

        printk(KERN_INFO "%s: thread %d exiting sleep on tag %d and level %d due to a signal\n",MODNAME, current->pid, tag, level);
        return 0;

    }
    // risveglio dovuto all'invio di un messaggio oppure ad una awake all
    else if(tags[tag]->levels[level]->awake == 1){

        // risveglio dovuto ad un awake all
        if(tags[tag]->levels[level]->buffer == NULL){

            // controllo se sia necessario deallocare il livello
            remove_and_deallocate_level(tag, level);

            printk(KERN_INFO "%s: thread %d exiting sleep on tag %d and level %d due to an awake all syscall!\n",MODNAME, current->pid, tag, level);
            return 0;

        }

        // risveglio dovuto ad una send
        int copied = copy_to_user(buffer, tags[tag]->levels[level]->buffer, min(size, tags[tag]->levels[level]->size));
        
        if(copied != 0){

            // controllo se sia necessario deallocare il livello
            remove_and_deallocate_level(tag, level);

            printk(KERN_ERR "%s: Error copying message to user space for thread %d on tag %d and level %d\n",MODNAME, current->pid, tag, level);
            
            return -1;
        }

        // controllo se sia necessario deallocare il livello
        remove_and_deallocate_level(tag, level);

        printk(KERN_INFO "%s: Message red from tag %d on level %d from thread %d\n",MODNAME, tag, level, current->pid);
            
        return 0;

    }else{

        printk(KERN_ERR "%s: Somenthing wrong happened during awake of threads on tag %d on level %d \n",MODNAME, tag, level);
        return -1;
    }

}







/* funzione che permette di gestire un tag: se il comando è REMOVE permette di rimuovere il tag (se questo non è in uso ad opera di altri thread);
   se il comando è AWAKE_ALL permette di risvegliare tutti i thread bloccati in attesa di un messaggio su questo tag, indipendentemente dal livello. */
int tag_ctl(int tag, int command){

    int i;

    // controllo sui valori del comando
    if (command != AWAKE_ALL && command != REMOVE){
        printk(KERN_ERR "%s: Invalid command flag: chose one of AWAKE_ALL or REMOVE! \n", MODNAME);
        return -1;
    }

    // controllo i permessi associati al tag service
    read_lock(&lock_array[tag]);

    if(check_tag_permission(tag) == -1){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Cannot access tag-service with tag descriptor %d : insufficient permissions or tag service corrupted or doesn't exist! \n", MODNAME, tag);
        return -1;
    
    }

    // se i permessi sono corretti accedo al tag service per la ricezione del messaggio
    if(tags[tag]==NULL){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Error in tag service with tag descriptor %d this tag doesn't exist! \n", MODNAME, tag);
        return -1;
    }

    // caso del risveglio dei thread in attesa su ogni livello del tag service
    if(command == AWAKE_ALL){

        for(i=0;i<LEVELS;i++){
            
            spin_lock(&(tags[tag]->levels_locks[i]));

            // se il livello esiste, risveglio la sua waitqueue
            if(tags[tag]->levels[i] != NULL){

                // impostazione della condizione di risveglio
                tags[tag]->levels[i]->awake = 1;

                // risveglio dei threads
                wake_up_all(&(tags[tag]->levels[i]->wq));
                spin_unlock(&(tags[tag]->levels_locks[i]));

            }else{
                spin_unlock(&(tags[tag]->levels_locks[i]));
            }
        }

        // rilascio del lock condiviso sul tag
        read_unlock(&lock_array[tag]);

        printk(KERN_INFO "%s: Succesfully awoken all receivers threads on tag %d! \n", MODNAME, tag);

        return 0;

    }
    // caso  della rimozione del tag
    else if( command == REMOVE){

        read_unlock(&lock_array[tag]);

        // prendo un lock esclusivo sul tag per evitare il sopraggiungere di ulteriori threads
        write_lock(&lock_array[tag]);

        for(i=0;i<LEVELS;i++){

            spin_lock(&(tags[tag]->levels_locks[i]));

            // controllo che non ci siano thread in attesa su questo tag
            if(tags[tag]->levels[i] != NULL){

                // se il tag è in uso la rimozione fallisce
                spin_unlock(&(tags[tag]->levels_locks[i]));
                write_unlock(&lock_array[tag]);
                printk(KERN_ERR "%s: Error removing tag %d: unable to remove: this tag is currently being used! \n", MODNAME, tag);
                return -1;

            }else{
                spin_unlock(&(tags[tag]->levels_locks[i]));
            }
        }

        // se non ho receivers su questo tag posso rimuoverlo e deallocare le strutture corrispondenti

        // dealloco la struttura relativa al tag vero e proprio
        kfree(tags[tag]);
        tags[tag] = NULL;

        // resetto i valori della struttura dati relativa alle info del tag
        tag_descriptors_info_array[tag]->key = -1;
        tag_descriptors_info_array[tag]->perm = -1;

        // indico come libero lo slot relativo al tag nell' array che costituisce la mappa dei tag in uso
        spin_lock(&tag_descriptors_header_lock);
        tag_descriptors_header_list[tag] = -1;
        spin_unlock(&tag_descriptors_header_lock);

        // rilascio il lock esclusivo sul tag
        write_unlock(&lock_array[tag]);

        printk(KERN_INFO "%s: Tag %d was succesfully removed! \n", MODNAME, tag);

        return 0;

    }else{

        printk(KERN_ERR "%s: Error in tag_ctl on tag %d! \n", MODNAME, tag);
        return -1;
    }

}













