#include "syscall.h"


int initialize_tag_service_structures(void){

    tag_descriptors_header_array = kzalloc(sizeof(tag_descriptors_header), GFP_KERNEL);
    tag_descriptors_header_array -> tag_descriptors_header = {[0 ... (TAGS-1)] -1};
    int lock_init = rwlock_init(&(tag_descriptors_header_array -> lock));
    if(lock_init != 0){
            pr_alert_ratelimited("tag_descriptors_header_array  struct creation failed\n");
            return -1;
    }


    tag_descriptor_info_array = kzalloc(TAGS * sizeof(tag_descriptor_info), GFP_KERNEL);
    for(int i=0; i< TAGS; i++){
        tag_descriptors_info_array[i] -> key = -1;
        tag_descriptors_info_array[i] -> perm = -1;
        lock_init = rwlock_init(&(tag_descriptors_info_array[i] -> lock));
        if(lock_init != 0){
             pr_alert_ratelimited("tag_descriptor_info_array  struct creation failed\n");
             return -1;
        }
    }

    return 0;

    


}





