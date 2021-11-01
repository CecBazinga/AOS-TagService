/*-------------------------------------------------------------- 
                 File di test dei permessi    
--------------------------------------------------------------*/

#include "config.h"

/* funzione che permette di verificare il corretto funzionamento dei permessi.Questo file è stato eseguito 
   con permessi di root così che l'apertura del tag precedentemente creato avvenisse ad opera di un utente 
   diverso dal creatore del tag stesso: se il tag è stato creato con permessi ristretti al solo utente creatore
   entrambe le syscall tag_get per l'apertura del tag fallsicono  */
int main(int argc, char** argv){
	
	int ret;

    // testing della verifica dei permessi
	ret = syscall(134,140,OPEN,PERM_NONE); 
	if(ret != -1){
		printf("Error: open on tag with wrong permission should have failed!\n");
	}

	ret = syscall(134,140,OPEN,PERM_ALL);
	if(ret != -1){
		printf("Error: open on tag with wrong permission should have failed!\n");
	}

}
