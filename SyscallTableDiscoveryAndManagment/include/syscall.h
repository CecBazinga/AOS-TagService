#define EXPORT_SYMTAB
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cred.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/rwlock.h>
#include <linux/rwlock_api_smp.h>
#include <linux/uaccess.h>

#define MODNAME "TAG SERVICE"

#define TAGS 256
#define LEVELS 32

#define IPC_PRIVATE 0

#define CREATE 0
#define OPEN 1

#define PERM_NONE 0
#define PERM_ALL 1

#define AWAKE_ALL 0
#define REMOVE 1

#define MAXSIZE 4096


struct tag_descriptor_info{

	int key;          // 0 se IPC_PRIVATE oppure un intero strettamente positivo (key). -1 indica tag service disponibile
    int perm;         // 0 se utilizzabile da qualunque utente oppure è settato al EUID dell'utente creatore
	kuid_t euid;
};


struct tag_level{

	int threads_waiting;
	char *buffer;
	size_t size;
	int awake;   // condizione su cui svegliarsi: se 1 i thread si risvegliano dalla waitqueue
	wait_queue_head_t wq;

};

struct tag{

	struct tag_level *levels[LEVELS];
	spinlock_t levels_locks[LEVELS];
};



int init_tag_service(void);
void free_tag_service(void);

int tag_get(int key, int command, int permission);

int tag_send(int tag, int level, char* buffer, size_t size);

int tag_receive(int tag, int level, char* buffer, size_t size);

int tag_ctl(int tag, int command);

struct tag** get_tag_array_ptr(void);

struct tag_descriptor_info** get_tag_info_array_ptr(void);

rwlock_t* get_tag_lock_array_ptr(void);