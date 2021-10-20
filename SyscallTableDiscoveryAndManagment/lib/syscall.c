#include "../include/syscall.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Amici <a.amici@outlook.it>");
MODULE_DESCRIPTION("TAG SERVICE");

#define MODNAME "TAG SERVICE"

int tag_descriptors_header_list[TAGS] = {[0 ... (TAGS-1)] -1};
spinlock_t tag_descriptors_header_lock;

struct tag_descriptor_info *tag_descriptors_info_array[TAGS] = { NULL };


int initialize_tag_service_structures(void){


    int i; 

    printk( "%s : MI SONO ROTTO I COGLIONI \n", MODNAME);
    return 0;

}





