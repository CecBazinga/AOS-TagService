#define EXPORT_SYMTAB
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define TAGS 256
#define LEVELS 32

#define IPC_PRIVATE 0

#define CREATE 0
#define OPEN 1

#define PERM_NONE 0
#define PERM_ALL 1


struct tag_descriptor_info{

	int key;          // 0 se IPC_PRIVATE oppure un intero strettamente positivo (key). -1 indica tag service disponibile
    int perm;         // 0 se utilizzabile da qualunque utente oppure è settato al EUID dell'utente creatore

};


int test(void);
int init_tag_service(void);
int free_tag_service(void);