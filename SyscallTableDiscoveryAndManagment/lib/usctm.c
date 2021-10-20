/*
* 
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 3 of the License, or (at your option) any later
* version.
* 
* This module is distributed in the hope that it will be useful, but WITHOUT ANY
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
* A PARTICULAR PURPOSE. See the GNU General Public License for more details.
* 
* @file usctm.c 
* @brief This is the main source for the Linux Kernel Module which implements
* 	 the runtime discovery of the syscall table position and of free entries (those 
* 	 pointing to sys_ni_syscall) 
*
* @author Francesco Quaglia
*
* @date November 22, 2020
*/

#include "../include/usctm.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Francesco Quaglia <framcesco.quaglia@uniroma2.it>");
MODULE_DESCRIPTION("USCTM");

#define MODNAME "USCTM"



#define SYS_CALL_INSTALL
#ifdef SYS_CALL_INSTALL

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(2, _trial, unsigned long, A, unsigned long, B){
#else
asmlinkage long sys_trial(unsigned long A, unsigned long B){
#endif

        printk("%s: thread %d requests a trial PLUTO sys_call with %lu and %lu as parameters\n",MODNAME,current->pid,A,B);

        return 0;

}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
static unsigned long sys_trial = (unsigned long) __x64_sys_trial;	
#else
#endif



#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(2, _pippo, unsigned long, A, unsigned long, B){
#else
asmlinkage long sys_pippo(unsigned long A, unsigned long B){
#endif

        printk("%s: thread %d requests a trial PIPPO sys_call with %lu and %lu as parameters\n",MODNAME,current->pid,A,B);

        return 0;

}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
static unsigned long sys_pippo = (unsigned long) __x64_sys_pippo;	
#else
#endif









int good_area(unsigned long * addr){

	int i;
	
	for(i=1;i<FIRST_NI_SYSCALL;i++){
		if(addr[i] == addr[FIRST_NI_SYSCALL]) goto bad_area;
	}	

	return 1;

bad_area:

	return 0;

}



/* This routine checks if the page contains the begin of the syscall_table.  */
int validate_page(unsigned long *addr){
	int i = 0;
	unsigned long page 	= (unsigned long) addr;
	unsigned long new_page 	= (unsigned long) addr;
	for(; i < PAGE_SIZE; i+=sizeof(void*)){		
		new_page = page+i+SEVENTH_NI_SYSCALL*sizeof(void*);
			
		// If the table occupies 2 pages check if the second one is materialized in a frame
		if( 
			( (page+PAGE_SIZE) == (new_page & ADDRESS_MASK) )
			&& sys_vtpmo(new_page) == NO_MAP
		) 
			break;
		// go for patter matching
		addr = (unsigned long*) (page+i);
		if(
			   ( (addr[FIRST_NI_SYSCALL] & 0x3  ) == 0 )		
			   && (addr[FIRST_NI_SYSCALL] != 0x0 )			// not points to 0x0	
			   && (addr[FIRST_NI_SYSCALL] > 0xffffffff00000000 )	// not points to a locatio lower than 0xffffffff00000000	
	//&& ( (addr[FIRST_NI_SYSCALL] & START) == START ) 	
			&&   ( addr[FIRST_NI_SYSCALL] == addr[SECOND_NI_SYSCALL] )
			&&   ( addr[FIRST_NI_SYSCALL] == addr[THIRD_NI_SYSCALL]	 )	
			&&   ( addr[FIRST_NI_SYSCALL] == addr[FOURTH_NI_SYSCALL] )
			&&   ( addr[FIRST_NI_SYSCALL] == addr[FIFTH_NI_SYSCALL] )	
			&&   ( addr[FIRST_NI_SYSCALL] == addr[SIXTH_NI_SYSCALL] )
			&&   ( addr[FIRST_NI_SYSCALL] == addr[SEVENTH_NI_SYSCALL] )	
			&&   (good_area(addr))
		){
			hacked_ni_syscall = (void*)(addr[FIRST_NI_SYSCALL]);				// save ni_syscall
			sys_ni_syscall_address = (unsigned long)hacked_ni_syscall;
			hacked_syscall_tbl = (void*)(addr);				// save syscall_table address
			sys_call_table_address = (unsigned long) hacked_syscall_tbl;
			return 1;
		}
	}
	return 0;
}

/* This routines looks for the syscall table.  */
void syscall_table_finder(void){
	unsigned long k; // current page
	unsigned long candidate; // current page

	for(k=START; k < MAX_ADDR; k+=4096){	
		candidate = k;
		if(
			(sys_vtpmo(candidate) != NO_MAP) 	
		){
			// check if candidate maintains the syscall_table
			if(validate_page( (unsigned long *)(candidate)) ){
				printk("%s: syscall table found at %px\n",MODNAME,(void*)(hacked_syscall_tbl));
				printk("%s: sys_ni_syscall found at %px\n",MODNAME,(void*)(hacked_ni_syscall));
				break;
			}
		}
	}
	
}




unsigned long cr0;

static inline void
write_cr0_forced(unsigned long val)
{
    unsigned long __force_order;

    /* __asm__ __volatile__( */
    asm volatile(
        "mov %0, %%cr0"
        : "+r"(val), "+m"(__force_order));
}

static inline void
protect_memory(void)
{
    write_cr0_forced(cr0);
}

static inline void
unprotect_memory(void)
{
    write_cr0_forced(cr0 & ~X86_CR0_WP);
}

#else
#endif



int init_module(void) {
	
	int res = init_tag_service();
	if(res!=0){
		printk("%s: failed to initialize_tag_service_structures\n",MODNAME);
		return -1;
	}

	res = free_tag_service();
	

	//printk("%d", tag_descriptors_header_array -> tag_descriptors_header[128])
	//printk("key is %d and perm is %d", tag_descriptors_info_array[128] -> key,tag_descriptors_info_array[128] -> perm);

	int i,j;
		
    printk("%s: initializing\n",MODNAME);
	
	syscall_table_finder();

	if(!hacked_syscall_tbl){
		printk("%s: failed to find the sys_call_table\n",MODNAME);
		return -1;
	}

	j=0;
	for(i=0;i<ENTRIES_TO_EXPLORE;i++)
		if(hacked_syscall_tbl[i] == hacked_ni_syscall){
			printk("%s: found sys_ni_syscall entry at syscall_table[%d]\n",MODNAME,i);	
			free_entries[j++] = i;
			if(j>=MAX_FREE) break;
		}

#ifdef SYS_CALL_INSTALL
	cr0 = read_cr0();
        unprotect_memory();
        // inserimento di piu syscall nelle entry sys_ni trovate da fare qui
        hacked_syscall_tbl[free_entries[0]] = (unsigned long*)sys_trial;
		hacked_syscall_tbl[free_entries[1]] = (unsigned long*)sys_pippo;
        protect_memory();
	printk("%s: a sys_call with 2 parameters has been installed as a trial on the sys_call_table at displacement %d and %d\n",MODNAME,free_entries[0],free_entries[1]);	
#else
#endif

        printk("%s: module correctly mounted\n",MODNAME);

        return 0;

}

void cleanup_module(void) {
                
#ifdef SYS_CALL_INSTALL
	cr0 = read_cr0();
        unprotect_memory();
        // rimozione delle syscall inserite da fare qui
        hacked_syscall_tbl[free_entries[0]] = (unsigned long*)hacked_ni_syscall;
		hacked_syscall_tbl[free_entries[1]] = (unsigned long*)hacked_ni_syscall;
        protect_memory();
#else
#endif
        printk("%s: shutting down\n",MODNAME);
        
}