#define EXPORT_SYMTAB
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/kprobes.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/apic.h>
#include <linux/syscalls.h>
#include "vtpmo.h"
#include "device_driver.h"


#define ADDRESS_MASK 0xfffffffffffff000//to migrate

#define START 			0xffffffff00000000ULL		// use this as starting address --> this is a biased search since does not start from 0xffff000000000000
#define MAX_ADDR		0xfffffffffff00000ULL
#define FIRST_NI_SYSCALL	134
#define SECOND_NI_SYSCALL	174
#define THIRD_NI_SYSCALL	182 
#define FOURTH_NI_SYSCALL	183
#define FIFTH_NI_SYSCALL	214	
#define SIXTH_NI_SYSCALL	215	
#define SEVENTH_NI_SYSCALL	236	

#define ENTRIES_TO_EXPLORE 256

#define MAX_FREE 15
int free_entries[MAX_FREE];


unsigned long *hacked_ni_syscall=NULL;
unsigned long **hacked_syscall_tbl=NULL;

unsigned long sys_call_table_address = 0x0;
module_param(sys_call_table_address, ulong, 0660);

unsigned long sys_ni_syscall_address = 0x0;
module_param(sys_ni_syscall_address, ulong, 0660);

module_param_array(free_entries,int,NULL,0660);//default array size already known - here we expose what entries are free



extern int sys_vtpmo(unsigned long vaddr);

extern int init_tag_service(void);
extern void free_tag_service(void);

extern int tag_get(int key, int command, int permission);

extern int tag_send(int tag, int level, char* buffer, size_t size);

extern int tag_receive(int tag, int level, char* buffer, size_t size);

extern int tag_ctl(int tag, int command);

extern int init_device_driver(void);

extern void free_device_driver(void);