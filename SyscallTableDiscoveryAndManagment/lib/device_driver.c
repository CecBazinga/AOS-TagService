#include "../include/device_driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Amici <a.amici@outlook.it>");
MODULE_DESCRIPTION("TAG SERVICE");

char *header = "Tag-key        Tag-creator        Tag-level        Waiting-threads\n";

char *concat(char *buff1, char *buff2) {
   
    // determino la nuova dimensione del buffer
    int new_size = strlen(buff1) + strlen(buff2) + 1; 

    // alloco il nuovo buffer
    char* new_buffer = kmalloc(sizeof(char)*new_size, GFP_KERNEL);
    if(new_buffer == NULL){
        printk(KERN_ERR "%s: Error during device driver buffer allocation! \n", MODNAME);
        return NULL;
    }

    // copio e concateno i buffer in un unico buffer
    strcpy(new_buffer,buff1);
    strcat(new_buffer,buff2); 
    printk(KERN_INFO "%s: BUFFER IS %s \n", MODNAME, new_buffer);

    return new_buffer;
}


int init_device_driver(void){

    major = register_chrdev(0, DEVICE_NAME, &fops);

	if (major < 0) {
	  printk(KERN_ERR "%s: registering device failed\n",MODNAME);
	  return major;
	}

	printk(KERN_INFO "%s: new device registered, it is assigned major number %d\n",MODNAME, major);

    return 0;
}


void free_device_driver(void){

    unregister_chrdev(major, DEVICE_NAME);
	printk(KERN_INFO "%s: new device unregistered, it was assigned major number %d\n",MODNAME, major);

    return;

}


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

    printk("%s: Device file with major %d: a read operation has been called!\n",MODNAME,major);

    int i, j, ret, buffer_size;

    struct tag_descriptor_info **tags_info_array = get_tag_info_array_ptr();
    rwlock_t *locks_array = get_tag_lock_array_ptr();
    struct tag **tags_array = get_tag_array_ptr();

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
                        printk(KERN_ERR "%s: Error accessing info for tag %d: tag's metadata are corrupted! \n", MODNAME, i);
                        spin_unlock(&(tags_array[i]->levels_locks[j]));
                        read_unlock(&locks_array[i]);
                        kfree(single_level_buffer);
                        return -1;
                    }

                    // concateno la nuova riga contenente le informazioni del livello con le informazioni degli altri tag e livelli 
                    final_buffer = concat(old_buffer, single_level_buffer);

                    // libero il vecchio buffer allocato in precedenza
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

    // libero il buffer di appoggio per generare le info relative ai singoli livelli
    kfree(single_level_buffer);

    // trasferisco le info generate verso lo user space
    buffer_size = strlen(final_buffer);

    if(*off > buffer_size) {

        return 0;
    } 
    if((buffer_size - *off) < len){
        len = buffer_size - *off;
    } 

    ret = copy_to_user(buff,&(final_buffer[*off]),len);
    if(ret != 0){
         printk(KERN_INFO "%s: Attention: partial device driver read has been perfmormed!\n", MODNAME);
    }
    
    *off += (len - ret);
    

    return len - ret;
}




















