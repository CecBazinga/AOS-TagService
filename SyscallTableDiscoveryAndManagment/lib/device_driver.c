#include "device_driver.h"


//TODO: chiamare register e unregister nell'init module del tag service
static int dev_open(struct inode *inode, struct file *file) {

   // device opened by a default nop
   printk(KERN_INFO"%s: Device file with major %d successfully opened!\n",MODNAME,major);
   return 0;
}


static int dev_release(struct inode *inode, struct file *file) {

   // device closed by default nop
   printk(KERN_INFO"%s: Device file with major %d succesfully closed!\n",MODNAME,major);
   return 0;

}


static ssize_t dev_write(struct file *filp, const char *buff, size_t len, loff_t *off) {

  // write not implemented cause this device is read only
  printk(KERN_INFO"%s: Device file with major %d is read only: no write enabled!\n",MODNAME,major);
  return -1;

}


static ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off) {

    printk("%s: Device file with major %d: somebody called a read operation!\n",MODNAME,major);

    int i,j;

    struct tag_descriptor_info *tags_info_array = get_tag_info_array_ptr();
    rwlock_t locks_array = get_tag_lock_array_ptr();
    struct tag *tags_array = get_tag_array_ptr();

    char *final_buffer;
    char *old_buffer = header;
    char *single_level_buffer = kmalloc(sizeof(char)*SINGLE_DEVICE_LINE, GFP_KERNEL);
    if(single_level_buffer == NULL){
        printk(KERN_ERR "%s: Error during device driver single level buffer allocation! \n", MODNAME);
        return -1;
    }
    

    for(i=0; i<TAGS; i++){

        // prendo il lock in lettura sul tag 
        read_lock(&locks_array[i]);

        // verifico il tag esista
        if(tags_array[i] != NULL){
            
            // verifico il livello esista
            for(j=0; j<LEVELS; j++){

                spin_lock(&(tags_array[i]->levels_locks[j]));

                if(tags_array[i]->levels[j] != NULL){

                    if(tags_info_array[i]->key == 0){
                        sprintf(single_level_buffer,"%-15s %-15d %-15d %-15d\n", "IPC_PRIVATE", tags_info_array[i]->euid.val, j, 
                                tags_array[i]->levels[j]->threads_waiting);
                    
                    }else if(tags_info_array[i]->key > 0){
                        sprintf(single_level_buffer,"%-15d %-15d %-15d %-15d\n", tags_info_array[i]->key, tags_info_array[i]->euid.val, j, 
                                tags_array[i]->levels[j]->threads_waiting);

                    }else{
                        printk(KERN_ERR "%s: Error accessing info for tag %d: tag's metadata are corrupted! \n", MODNAME, tag);
                        spin_unlock(&(tags_array[i]->levels_locks[j]));
                        read_unlock(&locks_array[i]);
                        kfree(single_level_buffer);
                        return -1;
                    }

                    // concateno la nuova riga contenente le informazioni del livello con le informazioni degli altri tag e livelli e libero il vecchio buffer
                    final_buffer = concat(old_buffer, single_level_buffer);
                    if(old_buffer != header){
                        kfree(old_buffer);
                    }
                    old_buffer = final_buffer;
                    spin_unlock(&(tags_array[i]->levels_locks[j]));
                   
                }else{
                    spin_unlock(&(tags_array[i]->levels_locks[j]));
                }

            }

            read_unlock(&locks_array[i]);
        
        }else{
            read_unlock(&locks_array[i]);
        }
    }

    kfree(single_level_buffer);

    //TODO: copiare il buffer finale nel buffer utente e fare controlli sulle size varie

    if(*off > the_object->valid_bytes) {

        return 0;
    } 
    if((the_object->valid_bytes - *off) < len) len = the_object->valid_bytes - *off;
    ret = copy_to_user(buff,&(the_object->stream_content[*off]),len);
    
    *off += (len - ret);
    

    return len - ret;
}





char *concat(char *buff1, char *buff2) {
   
    // Determine new size
    int new_size = strlen(buff1) + strlen(buff2) + 1; 

    // Allocate new buffer
    char * newBuffer = kmalloc(sizeof(char)*new_size, GFP_KERNEL);
    if(newBuffer == NULL){
        printk(KERN_ERR "%s: Error during device driver buffer allocation! \n", MODNAME);
        return NULL;
    }

    // do the copy and concat
    strcpy(newBuffer,buff1);
    strcpy(newBuffer,buff2); 

    return new_buffer;
}















