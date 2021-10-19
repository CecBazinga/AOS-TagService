#define EXPORT_SYMTAB
#include <linux/rwlock.h>
#include <linux/slab.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Amici");
MODULE_DESCRIPTION("Syscalls");



#define MODNAME "SYSCALLS"

#define TAGS 256
#define LEVELS 32

typedef struct tag_descriptors_header{

	int tag_descriptors_header[TAGS];
	rwlock_t lock;

} tag_descriptors_header;


typedef struct tag_descriptor_info{

	int key;          // 0 se IPC_PRIVATE oppure un intero strettamente positivo (key). -1 indica tag service disponibile
    int perm;         // 0 se utilizzabile da qualunque utente oppure è settato al EUID dell'utente creatore
	rwlock_t lock;

} tag_descriptor_info;


struct tag_descriptors_header tag_descriptors_header_array;
struct tag_descriptor_info tag_descriptors_info_array[TAGS];


int initialize_tag_service_structures(void);