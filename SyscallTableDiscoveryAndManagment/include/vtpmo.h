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



int sys_vtpmo(unsigned long vaddr);


//stuff for actual service operations
#define ADDRESS_MASK 0xfffffffffffff000
//#define PAGE_TABLE_ADDRESS phys_to_virt(_read_cr3() & ADDRESS_MASK)
#define PAGE_TABLE_ADDRESS __va(_read_cr3() & ADDRESS_MASK)
#define PT_ADDRESS_MASK 0x7ffffffffffff000
#define VALID 0x1
#define MODE 0x100
#define LH_MAPPING 0x80

#define PML4(addr) (((long long)(addr) >> 39) & 0x1ff)
#define PDP(addr)  (((long long)(addr) >> 30) & 0x1ff)
#define PDE(addr)  (((long long)(addr) >> 21) & 0x1ff)
#define PTE(addr)  (((long long)(addr) >> 12) & 0x1ff)


#define NO_MAP (-1)

#define AUDIT if(0)

