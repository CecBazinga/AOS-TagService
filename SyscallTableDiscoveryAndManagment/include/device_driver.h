/*-------------------------------------------------------------- 
                 Device driver header file     
--------------------------------------------------------------*/

#define EXPORT_SYMTAB
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/version.h>
#include "syscall.h"


/* dichiarazione delle operazioni del device driver   */
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *buffer, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/* parametri del device driver  */

/* device file name in /dev/  */
#define DEVICE_NAME "tag_system_device"  

/* major number che sarà assegnato al device driver */
static int major;           

/* massimo numero di byte per una singola riga del device */
#define SINGLE_DEVICE_LINE 64 


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
#define get_major(session)	MAJOR(session->f_inode->i_rdev)
#define get_minor(session)	MINOR(session->f_inode->i_rdev)
#else
#define get_major(session)	MAJOR(session->f_dentry->d_inode->i_rdev)
#define get_minor(session)	MINOR(session->f_dentry->d_inode->i_rdev)
#endif


/* struttura contenete il riferimento al modulo proprietario del device ed alle operazioni del device stesso */
static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open =  dev_open,
  .release = dev_release,
  .write = dev_write,
  .read = dev_read
};


/* segnature delle operazioni esportate */

/* funzione per l'inizializzazione del device */
int init_device_driver(void);

/* funzione per la deregistrazione del device */
void free_device_driver(void);

