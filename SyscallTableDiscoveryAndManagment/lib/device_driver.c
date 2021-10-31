#include "device_driver.h"

header = "Tag-key       Tag-creator       Tag-level       Waiting-threads\n";

//TODO: chiamare register e unregister nell'init module del tag service
static int dev_open(struct inode *inode, struct file *file) {

   // device opened by a default nop
   printk("%s: Device file with major %d successfully opened!\n",MODNAME,major);
   return 0;
}


static int dev_release(struct inode *inode, struct file *file) {

   // device closed by default nop
   printk("%s: Device file with major %d succesfully closed!\n",MODNAME,major);
   return 0;

}


static ssize_t dev_write(struct file *filp, const char *buff, size_t len, loff_t *off) {

  // write not implemented cause this device is read only
  printk("%s: Device file with major %d is read only: no write enabled!\n",MODNAME,major);
  return -1;

}


static ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off) {

    printk("%s: Device file with major %d: somebody called a read operation!\n",MODNAME,major);

    //TODO: nella read ciclare sui tag ed i livelli

    
    //TODO: ricordarsi le free dei buffer che alloco


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















