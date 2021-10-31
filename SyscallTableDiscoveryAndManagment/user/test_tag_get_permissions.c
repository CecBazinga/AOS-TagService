#include "config.h"

int main(int argc, char** argv){
	
	int ret;
    // testing della verifica dei permessi
	ret = syscall(134,140,OPEN,PERM_NONE); //dovrebbe dare errore sulla open
	if(ret != -1){
		printf("Error: open on tag with wrong permission should have failed!\n");
	}

	ret = syscall(134,140,OPEN,PERM_ALL); //dovrebbe dare errore sulla open
	if(ret != -1){
		printf("Error: open on tag with wrong permission should have failed!\n");
	}

}
