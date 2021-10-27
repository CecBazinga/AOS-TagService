#include "../include/syscall.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Amici <a.amici@outlook.it>");
MODULE_DESCRIPTION("TAG SERVICE");

#define MODNAME "TAG SERVICE"

int tag_descriptors_header_list[TAGS] = {[0 ... (TAGS-1)] -1};
spinlock_t tag_descriptors_header_lock;


struct tag_descriptor_info *tag_descriptors_info_array[TAGS] = { NULL };
rwlock_t lock_array[TAGS];
struct tag *tags[TAGS] = { NULL };



// funzione che inizializza l'array per le info dei singoli tag service e l'array di rwlocks
int init_tag_service(void){

    int i;

    for(i=0; i<TAGS; i++){

        tag_descriptors_info_array[i] = kmalloc(sizeof(struct tag_descriptor_info), GFP_KERNEL);
        if(tag_descriptors_info_array[i] == NULL){
            printk(KERN_ERR "%s: Error during tag service structure allocation! \n", MODNAME);
            return -1;
        }
        tag_descriptors_info_array[i]->key = -1;
        tag_descriptors_info_array[i]->perm = -1;
        //tag_descriptors_info_array[i]->euid.val = -1;

        // inizializzazione degli rwlock
        rwlock_init(&lock_array[i]);
    }

    printk("%s: Tag service structures have been initialized succesfully! \n", MODNAME);
    return 0;
}




int free_tag_service(void){

    int i;

    for(i=0; i<TAGS; i++){

        if(tag_descriptors_info_array[i] != NULL){

            write_lock(&(lock_array[i]));
            kfree(tag_descriptors_info_array[i]); 
            tag_descriptors_info_array[i] = NULL;
            write_unlock(&(lock_array[i]));

        }
    }

    printk("%s: Tag service structures have been removed succesfully! \n", MODNAME);
    return 0;
}




struct tag* create_tag(void){

    int i;

    struct tag *the_tag = kmalloc(sizeof(struct tag), GFP_KERNEL);

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




int insert_tag_descriptor(int key, int tag_descriptor, int permission){

    // accedo alla struct corrispondente per inserire i valori della create 
    write_lock(&lock_array[tag_descriptor]);

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



int check_tag_permission(int tag_descriptor){

    if(tag_descriptors_info_array[tag_descriptor]->perm == 0){

        if(tag_descriptors_info_array[tag_descriptor]->euid.val == get_current_user()->uid.val){
            return tag_descriptor;
        
        }else{
            return -1;
        }
            
    }else if(tag_descriptors_info_array[tag_descriptor]->perm == 1){

        return tag_descriptor;

    }else{
        return -1;
    }

}




int tag_get(int key, int command, int permission){

    int i;
    int tag_descriptor = -1;

    if (command != OPEN && command != CREATE){
        printk(KERN_ERR "%s: Invalid command flag: chose one of OPEN or CREATE! \n", MODNAME);
        return -1;
    }

    if (permission != PERM_NONE && permission != PERM_ALL){
        printk(KERN_ERR "%s: Invalid permission flag: chose one of PERM_NONE or PERM_ALL! \n", MODNAME);
        return -1;
    }


    if (key == IPC_PRIVATE){

        printk(KERN_INFO "%s: Macro IPC_PRIVATE value is : %d \n", MODNAME, key);
    

        if(command == OPEN){
            printk(KERN_ERR "%s: Cannot open a tag-service with IPC_PRIVATE key! \n", MODNAME);
            return -1;
        }

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

            return insert_tag_descriptor(key,tag_descriptor,permission);
        }

        return -1;


    }else if (key > 0){

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

            // accedo alla struct corrispondente per inserire i valori della create
            return insert_tag_descriptor(key,tag_descriptor,permission);
        }

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





int tag_send(int tag, int level, char* buffer, size_t size){

    
    printk(KERN_INFO "%s: Buffer's size is %d! \n", MODNAME, sizeof(buffer));

    // controllo il valore della size
    if(size<1 || size>MAXSIZE){
        printk(KERN_ERR "%s: Buffer's size must be in between 1 for empty buffers and %d which is maxsize allowed! \n", MODNAME, MAXSIZE);
        return -1;
    }

    // controllo i permessi associati al tag service
    read_lock(&lock_array[tag]);

    if(check_permission(tag) == -1){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Cannot access tag-service with tag descriptor %d : insufficient permissions or tag service corrupted! \n", MODNAME, tag);
        return -1;
    
    }


    // se i permessi sono corretti accedo al tag service per l' invio del messaggio
    if(tags[tag]==NULL){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Error in tag service with tag descriptor %d! \n", MODNAME, tag);
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
    tags[tag]->levels[level]->buffer = kmalloc(size*sizeof(char), GFP_KERNEL);

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

    // sveglio tutti i receivers sulla wait queue
    wake_up_all(tags[tag]->levels[level]->wq);

    // rilascio il lock esclusivo sul livello 
    spin_unlock(&(tags[tag]->levels_locks[level]));

    // rilascio il read lock sul tag
    read_unlock(&lock_array[tag]);
    
    return 0;

}




int tag_receive(int tag, int level, char* buffer, size_t size){

    // controllo il valore della size
    if(size<1 || size>MAXSIZE){
        printk(KERN_ERR "%s: Buffer's size must be in between 1 for empty buffers and %d which is maxsize allowed! \n", MODNAME, MAXSIZE);
        return -1;
    }

    // controllo i permessi associati al tag service
    read_lock(&lock_array[tag]);

    if(check_permission(tag) == -1){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Cannot access tag-service with tag descriptor %d : insufficient permissions or tag service corrupted! \n", MODNAME, tag);
        return -1;
    
    }

    // se i permessi sono corretti accedo al tag service per la ricezione del messaggio
    if(tags[tag]==NULL){
        read_unlock(&lock_array[tag]);
        printk(KERN_ERR "%s: Error in tag service with tag descriptor %d! \n", MODNAME, tag);
        return -1;
    }


    // accedo al livello desiderato
    spin_lock(&(tags[tag]->levels_locks[level]));

    // se il livello non è inizializzato non ci sono receivers, quindi lo inizializzo con un nuovo buffer
    if(tags[tag]->levels[level]==NULL){

        // inizializzo la struct level, il suo buffer, la sua waitqueue ed il numero di thread in waiting
        tags[tag]->levels[level] = kmalloc(sizeof(struct tag_level), GFP_KERNEL);

        if(tags[tag]->levels[level] == NULL){
            spin_unlock(&(tags[tag]->levels_locks[level]));
            read_unlock(&lock_array[tag]);
            printk(KERN_ERR "%s: Error during level structure allocation for tag service %d on level %d! \n", MODNAME, tag, level);
            return -1;
        }

        tags[tag]->levels[level]->buffer = NULL;
        tags[tag]->levels[level]->threads_waiting = 1;
        tags[tag]->levels[level]->awake = 0;
        init_waitqueue_head (&(tags[tag]->levels[level]->wq));
        
    }else{

        // se il livello è gia stato instanziato incremento il numero di thread in attesa
        tags[tag]->levels[level]->threads_waiting ++; 
    }

    // libero il lock
    spin_unlock(&(tags[tag]->levels_locks[level]));

    //rilascio il read lock sulla cella del tag service info
    read_unlock(&lock_array[tag]);

    printk(KERN_INFO "%s: Threads waiting value is: %d! \n", MODNAME,tags[tag]->levels[level]->threads_waiting);

    // mando il receiver in sleep sulla waitqueue
    wait_event_interruptible(tags[tag]->levels[level]->wq, tags[tag]->levels[level]->awake == 1);

    //TODO: inserire routine di deallocazione del livello e free delle risorse da parte del receiver

    // routine di copia del buffer da eseguire al risveglio dalla waitqueue
    if(tags[tag]->levels[level]->awake == 0){

        spin_lock(&(tags[tag]->levels_locks[level]));
        tags[tag]->levels[level]->threads_waiting --;

        // se sono l'ultimo receiver rimasto dealloco le strutture dati del livello
        if(tags[tag]->levels[level]->threads_waiting == 0){
            //TODO: funzione per deallocare il livello 

        }
        spin_unlock(&(tags[tag]->levels_locks[level]));

        printk("%s: thread %d exiting sleep on tag %d and level %d due to a signal\n",MODNAME, current->pid, tag, level);
        return 0;

    }else if(tags[tag]->levels[level]->awake == 1){

        // risveglio dovuto ad un awake all
        if(tags[tag]->levels[level]->buffer == NULL){

            spin_lock(&(tags[tag]->levels_locks[level]));
            tags[tag]->levels[level]->threads_waiting --;
            spin_unlock(&(tags[tag]->levels_locks[level]));

            printk("%s: thread %d exiting sleep on tag %d and level %d due to a signal\n",MODNAME, current->pid, tag, level);
            return 0;

        }

        // risveglio dovuto ad una send


    }

    



}













