#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int main() {
    int shmid, semid;
    create_shared_memory(&shmid);
    create_semaphore(&semid);

    // Manager logic to handle customers and tables
    // This will include logic to allocate tables and manage customers

    cleanup(shmid, semid);
    return 0;
}