#include <unistd.h>
#include <stdio.h>
#include <sys/fsuid.h>

//#include <time.h>

#define TAGS 256
#define LEVELS 32

#define IPC_PRIVATE 0

#define CREATE 0
#define OPEN 1

#define PERM_NONE 0
#define PERM_ALL 1

int main(int argc, char** argv){
	
    // testing della verifica dei permessi
	sleep(2);
	syscall(134,140,OPEN,PERM_NONE); //dovrebbe dare errore sulla open

	sleep(2);
	syscall(134,140,OPEN,PERM_ALL); //dovrebbe dare errore sulla open

	
	
}
