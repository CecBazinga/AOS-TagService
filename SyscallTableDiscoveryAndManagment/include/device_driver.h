#define EXPORT_SYMTAB
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>


static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *buffer, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

#define DEVICE_NAME "tag_system_device"  /* Device file name in /dev/ - not mandatory  */

static int major;            /* Major number assigned to device driver */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
#define get_major(session)	MAJOR(session->f_inode->i_rdev)
#define get_minor(session)	MINOR(session->f_inode->i_rdev)
#else
#define get_major(session)	MAJOR(session->f_dentry->d_inode->i_rdev)
#define get_minor(session)	MINOR(session->f_dentry->d_inode->i_rdev)
#endif

#define SINGLE_DEVICE_LINE 30 // just one device line max number of bytes


static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open =  dev_open,
  .release = dev_release,
  .write = dev_write,
  .read = dev_read
};