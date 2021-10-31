#include "config.h"

int main(int argc, char** argv){
	
    // testing della verifica dei permessi
	sleep(2);
	syscall(134,140,OPEN,PERM_NONE); //dovrebbe dare errore sulla open

	sleep(2);
	syscall(134,140,OPEN,PERM_ALL); //dovrebbe dare errore sulla open

}
