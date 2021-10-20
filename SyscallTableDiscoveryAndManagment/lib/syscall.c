#include "../include/syscall.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Amici <a.amici@outlook.it>");
MODULE_DESCRIPTION("TAG SERVICE");

#define MODNAME "TAG SERVICE"

int tag_descriptors_header_list[TAGS] = {[0 ... (TAGS-1)] -1};
spinlock_t tag_descriptors_header_lock;



int initialize_tag_service_structures(void){


    int i; 
    //tag_descriptors_info_array = kmalloc(TAGS * sizeof(struct tag_descriptor_info), GFP_KERNEL);

    /*
    for(i=0; i< TAGS; i++){

        tag_descriptors_header_list[i] = -1;

        //tag_descriptors_info_array[i].key = -1;
        //tag_descriptors_info_array[i].perm = -1;
    }
    */


    

    printk( "%s : MI SONO ROTTO I COGLIONI \n", MODNAME);
    return 0;

}





