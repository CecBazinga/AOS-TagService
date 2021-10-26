#include "config.h"

int main(int argc, char** argv){
	
	// testing tag get

	// testing combinazioni per la creazione
	int i;
	for(i=0;i<TAGS; i ++){

		//printf("working with i: %d",i);
		if(i<64){

			syscall(134,IPC_PRIVATE,CREATE,PERM_NONE);
		
		}else if(i>63 && i<128 ){

			syscall(134,IPC_PRIVATE,CREATE,PERM_ALL);
		
		}else if(i>127 && i<192){

			syscall(134,i,CREATE,PERM_NONE);
		
		}else if(i>191){

			syscall(134,i,CREATE,PERM_ALL);
		}

	}

	// testing combinazioni per la creazione ed apertura su tag service gia istanziati
	syscall(134,IPC_PRIVATE,CREATE,PERM_NONE);

	sleep(2);
	syscall(134,IPC_PRIVATE,CREATE,PERM_ALL);

	sleep(2);
	syscall(134,IPC_PRIVATE,OPEN,PERM_NONE);

	sleep(2);
	syscall(134,IPC_PRIVATE,OPEN,PERM_ALL);

	sleep(2);
	syscall(134,60,CREATE,PERM_NONE);

	sleep(2);
	syscall(134,60,CREATE,PERM_ALL);

	sleep(2);
	syscall(134,60,OPEN,PERM_NONE);

	sleep(2);
	syscall(134,60,OPEN,PERM_ALL);

	sleep(2);
	syscall(134,70,CREATE,PERM_NONE);

	sleep(2);
	syscall(134,70,CREATE,PERM_ALL);

	sleep(2);
	syscall(134,70,OPEN,PERM_NONE);

	sleep(2);
	syscall(134,70,OPEN,PERM_ALL);
	
	sleep(2);
	syscall(134,140,CREATE,PERM_NONE);

	sleep(2);
	syscall(134,140,CREATE,PERM_ALL);

	sleep(2);
	syscall(134,240,CREATE,PERM_NONE);

	sleep(2);
	syscall(134,240,CREATE,PERM_ALL);

	sleep(2);
	syscall(134,240,OPEN,PERM_NONE);

	sleep(2);
	syscall(134,240,OPEN,PERM_ALL);
	
	
}
