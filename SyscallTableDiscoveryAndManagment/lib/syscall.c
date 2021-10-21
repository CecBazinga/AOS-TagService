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

        printk(KERN_ERR "%s: Macro IPC_PRIVATE value is : %d \n", MODNAME, key);

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


            // inserimento dei valori della creazione all'interno della struct corrispondente al tag-descriptor ottenuto
            spin_lock(&lock_array[tag_descriptor]);

            if(tag_descriptors_info_array[tag_descriptor] != NULL){

                spin_unlock(&lock_array[tag_descriptor]);
                printk(KERN_ERR "%s: Error during tag service structure creation! \n", MODNAME);
                return -1;
                
            }else if(tag_descriptors_info_array[tag_descriptor] == NULL){

                tag_descriptors_info_array[tag_descriptor]->key = 0 ;
                tag_descriptors_info_array[tag_descriptor]->perm = permission ;
                tag_descriptors_info_array[tag_descriptor]->euid = get_current_user()->uid;


                spin_unlock(&lock_array[tag_descriptor]);
                printk(KERN_INFO "%s: Tag service correctly created with key IPC_PRIVATE and tag-descriptor : %d \n", MODNAME, tag_descriptor);

                return tag_descriptor;


            }
            

        }

    }
    /*
    if (key > 0){
            //TODO: open e create nel caso di chiave non privata
            
    }
    */

    else{
         printk(KERN_ERR "%s: Invalid key value: chose IPC_PRIVATE or an integer greater than 0! \n", MODNAME);
        return -1;
    }
}






int test(void){


    int i; 

    printk( "%s : MI SONO ROTTO I COGLIONI \n", MODNAME);
    return 0;

}





