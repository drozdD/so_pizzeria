#include "utils.h"

void create_shared_memory(int *shmid) {
	*shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
	if (*shmid < 0) {
		perror("shmget");
		exit(1);
	}
}

void create_semaphore(int *semid) {
	*semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
	if (*semid < 0) {
		perror("semget");
		exit(1);
	}
	semctl(*semid, 0, SETVAL, 1); // Initialize semaphore to 1
}
						  
void cleanup(int shmid, int semid) {
	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);
}
