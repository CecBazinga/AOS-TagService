/*-------------------------------------------------------------- 
                 Syscalls header file     
--------------------------------------------------------------*/

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


/* nome del modulo e parametri */
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

/* strutture dati del modulo */

/* struttura dati contenente le informazioni sensibili di un singolo tag  */
struct tag_descriptor_info{

	int key;                                 // 0 se IPC_PRIVATE oppure un intero strettamente positivo (key). -1 indica tag service disponibile
    int perm;                                // 0 se utilizzabile da qualunque utente oppure è settato al EUID dell'utente creatore
	kuid_t euid;                             // uid dell'utente creatore del tag
};


/* struttura dati contenente le informazioni sensibili di un singolo livello  */
struct tag_level{

	int threads_waiting;                     // numero di thread in attesa su questo livello (receiver che non hanno ancora ricevuto alcun messaggio/segnale)
	char *buffer;                            // puntatore al buffer relativo al livello
	size_t size;                             // dimensione del buffer relativo al livello
	int awake;                               // condizione su cui svegliarsi: se "1" i thread si risvegliano dalla waitqueue
	wait_queue_head_t wq;                    // waitqueu associata al livello

};


/* struttura dati per operare effettivamente con un determinato tag  */
struct tag{

	struct tag_level *levels[LEVELS];        // array di puntatori alle strutture dati relative i livelli del tag
	spinlock_t levels_locks[LEVELS];         // array di lock ognuno relativo ad un livello del tag
};



/* segnature delle operazioni esportate */

/* funzione che inizializza le strutture dati comuni del tag service ed alloca le risorse necessarie */
int init_tag_service(void);

/* funzione che rilascia le strutture dati comuni del tag service e le eventuali risorse allocate */
void free_tag_service(void);

/* funzione che permette la creazione o apertura di un tag */
int tag_get(int key, int command, int permission);

/* funzione che permette l'invio di un messaggio su un livello specifico di un tag */
int tag_send(int tag, int level, char* buffer, size_t size);

/* funzione che permette di mettersi in attesa di un messaggio su un livello specifico di un tag */
int tag_receive(int tag, int level, char* buffer, size_t size);

/* funzione che permette di controllare un tag risvegliandone tutti i threads in attesa o rimuovendo il tag stesso */
int tag_ctl(int tag, int command);


/* funzione per ottenre i puntatori alle strutture dati comuni del tag service */

/* funzione per ottenre il puntatore all'array dei tag */
struct tag** get_tag_array_ptr(void);

/* funzione per ottenre il puntatore all'array delle informazioni dei tag */
struct tag_descriptor_info** get_tag_info_array_ptr(void);

/* funzione per ottenre il puntatore all'array dei lock dei tag */
rwlock_t* get_tag_lock_array_ptr(void);