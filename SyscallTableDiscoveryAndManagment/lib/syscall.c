#include "../include/syscall.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Amici <a.amici@outlook.it>");
MODULE_DESCRIPTION("TAG SERVICE");

#define MODNAME "TAG SERVICE"

int tag_descriptors_header_list[TAGS] = {[0 ... (TAGS-1)] -1};
spinlock_t tag_descriptors_header_lock;


struct tag_descriptor_info *tag_descriptors_info_array[TAGS] = { NULL };
spinlock_t lock_array[TAGS];



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
        tag_descriptors_info_array[i]->euid = -1;
    }

    printk("%s: Tag service structures have been initialized succesfully! \n", MODNAME);
    return 0;
}




int free_tag_service(void){

    int i;

    for(i=0; i<TAGS; i++){

        if(tag_descriptors_info_array[i] != NULL){

            spin_lock(&(lock_array[i]));
            kfree(tag_descriptors_info_array[i]); 
            tag_descriptors_info_array[i] = NULL;
            spin_unlock(&(lock_array[i]));

        }
    }

    printk("%s: Tag service structures have been removed succesfully! \n", MODNAME);
    return 0;
}



int insert_tag_descriptor_info(int key, int tag_descriptor, int premission){

    // accedo alla struct corrispondente per inserire i valori della create 
    spin_lock(&lock_array[tag_descriptor]);

    if(tag_descriptors_info_array[tag_descriptor]->key != -1){

        spin_unlock(&lock_array[tag_descriptor]);
        printk(KERN_ERR "%s: Error during tag service structure creation! \n", MODNAME);
        return -1;
        
    }else{

        tag_descriptors_info_array[tag_descriptor]->key = key ;
        tag_descriptors_info_array[tag_descriptor]->perm = permission ;
        tag_descriptors_info_array[tag_descriptor]->euid = get_current_user()->uid;


        spin_unlock(&lock_array[tag_descriptor]);
        printk(KERN_INFO "%s: Tag service infos correctly created with key IPC_PRIVATE and tag-descriptor : %d \n", MODNAME, tag_descriptor);

        return tag_descriptor;
    }
}




int tag_get(int key, int command, int permission){

    int i;
    int tag_descriptor = -1;

    if (command != OPEN || command != CREATE){
        printk(KERN_ERR "%s: Invalid command flag: chose one of OPEN or CREATE! \n", MODNAME);
        return -1;
    }

    if (permission != PERM_NONE || permission != PERM_ALL){
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

            return insert_tag_descriptor_info(key,tag_descriptor,permission);
        }


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
            return insert_tag_descriptor_info(key,tag_descriptor,permission);
        }

        if(command == OPEN){

            // controllo che la chiave esista
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

        }


    }else{
         printk(KERN_ERR "%s: Invalid key value: chose IPC_PRIVATE or an integer greater than 0! \n", MODNAME);
        return -1;
    }

}






int test(void){


    int i; 

    printk( "%s : MI SONO ROTTO I COGLIONI \n", MODNAME);
    return 0;

}





