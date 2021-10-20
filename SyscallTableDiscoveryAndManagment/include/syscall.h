#define EXPORT_SYMTAB
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define TAGS 256
#define LEVELS 32

extern int tag_descriptors_header_list[TAGS] ;
extern spinlock_t tag_descriptors_header_lock;


struct tag_descriptor_info{

	int key;          // 0 se IPC_PRIVATE oppure un intero strettamente positivo (key). -1 indica tag service disponibile
    int perm;         // 0 se utilizzabile da qualunque utente oppure è settato al EUID dell'utente creatore
	spinlock_t lock;

};


//struct tag_descriptor_info *tag_descriptors_info_array[TAGS];


int initialize_tag_service_structures(void);