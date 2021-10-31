#include "config.h"

int main(int argc, char** argv){
	
	int i, ret;
	int tag_descriptors[TAGS];
	// testing tag get

	// testing combinazioni per la creazione
	for(i=0;i<TAGS; i ++){

		//printf("working with i: %d",i);
		if(i<64){

			tag_descriptors[i] = syscall(134,IPC_PRIVATE,CREATE,PERM_NONE);
		
		}else if(i>63 && i<128 ){

			tag_descriptors[i] = syscall(134,IPC_PRIVATE,CREATE,PERM_ALL);
		
		}else if(i>127 && i<192){

			tag_descriptors[i] = syscall(134,i,CREATE,PERM_NONE);
		
		}else if(i>191){

			tag_descriptors[i] = syscall(134,i,CREATE,PERM_ALL);
		}

		if(tag_descriptors[i] == -1){
			printf("Error: tag create failed for tag %d!\n", i);
			return -1;
		}

	}

	// testing combinazioni per la creazione ed apertura su tag service gia istanziati
	ret = syscall(134,IPC_PRIVATE,CREATE,PERM_NONE);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,IPC_PRIVATE,CREATE,PERM_ALL);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,IPC_PRIVATE,OPEN,PERM_NONE);
	if(ret != -1){
		printf("Error: open on private tag should have failed!\n");
	}

	ret = syscall(134,IPC_PRIVATE,OPEN,PERM_ALL);
	if(ret != -1){
		printf("Error: open on private tag should have failed!\n");
	}

	ret = syscall(134,60,CREATE,PERM_NONE);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,60,CREATE,PERM_ALL);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,60,OPEN,PERM_NONE);
	if(ret != -1){
		printf("Error: open on private tag should have failed!\n");
	}

	ret = syscall(134,60,OPEN,PERM_ALL);
	if(ret != -1){
		printf("Error: open on private tag should have failed!\n");
	}

	ret = syscall(134,70,CREATE,PERM_NONE);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,70,CREATE,PERM_ALL);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,70,OPEN,PERM_NONE);
	if(ret != -1){
		printf("Error: open on private tag should have failed!\n");
	}

	ret = syscall(134,70,OPEN,PERM_ALL);
	if(ret != -1){
		printf("Error: open on private tag should have failed!\n");
	}
	
	ret = syscall(134,140,CREATE,PERM_NONE);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,140,CREATE,PERM_ALL);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,140,OPEN,PERM_ALL);
	if(ret != tag_descriptors[140]){
		printf("Error: open with right permission should have succed!\n");
	}

	ret = syscall(134,140,OPEN,PERM_NONE);
	if(ret != tag_descriptors[140]){
		printf("Error: open with right permission should have succed!\n");
	}

	ret = syscall(134,240,CREATE,PERM_NONE);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,240,CREATE,PERM_ALL);
	if(ret != -1){
		printf("Error: creation on already existing tag should have failed!\n");
	}

	ret = syscall(134,240,OPEN,PERM_NONE);
	if(ret != tag_descriptors[240]){
		printf("Error: open with right permission should have succed!\n");
	}

	ret = syscall(134,240,OPEN,PERM_ALL);
	if(ret != tag_descriptors[240]){
		printf("Error: open with right permission should have succed!\n");
	}
	
	
	// rimozione dei tag service
	for(i=0;i<TAGS; i++){

		ret = syscall(177,tag_descriptors[i],REMOVE);
		if(ret == -1){
			printf("Error: remotion of non used tag %d should have succed!\n", i);
		}

	}
}
