/* Gerenciador de memoria compartilhada entre processos */
/* Autor: Bruno Cesar, @bcesarg6, bcesar.g6@gmail.com   */
/* 		  Cristofer Alexandre Oswald 				    */
/* Maio de 2018   */

#include "SharedMemoryController.h"

int smhId; //Shared memory identifier
int mem_ptr = 0;

void createShMem(int size){

	if (shmId = shmget(IPC_PRIVATE, size, (SHM_R | SHM_W)) < 0){
		printf("Deu muito ruim\n");
		exit(1);
	}

	shmctl(shmId, IPC_RMID, (struct shmid_ds *) NULL);
}

void* shAlloc(int size){

	void* mem;

	if ((mem = shmat(shmId,0,0)) == (void*) -1){
		printf("Deu muito ruim 2\n");
		exit(1);
	}

	mem = mem + mem_ptr;
	mem_ptr += size;

	return mem;
}

void shFree(void* ptr){

	void* mem

}


char* ShareMalloc(int size)
{
	int shmId;
	char* returnPtr;
	
	if ((shmId=shmget(IPC_PRIVATE, size, (SHM_R | SHM_W)) < 0)
		Abort(“Failure on shmget\n”);
	
	if (returnPtr=(char*)shmat(shmId,0,0)) == (void*) -1)
		Abort(“Failure on shmat\n”);
	shmctl(shmId, IPC_RMID, (struct shmid_ds *) NULL);
	
	return (returnPtr);
}
