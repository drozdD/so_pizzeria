#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define SHM_SIZE 1024
#define SEM_KEY 1234
#define SHM_KEY 5678

void create_shared_memory(int *shmid);
void create_semaphore(int *semid);
void cleanup(int shmid, int semid);

#endif
