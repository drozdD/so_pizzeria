#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

int connect_to_mess_queue() {
    key_t key;
    key = ftok("main.c", 1);
    int msg_id = msgget(key, IPC_CREAT | 0640);
    if (msg_id == -1) {
        perror("Error creating/accessing message queue");
        return EXIT_FAILURE;
    }
    return msg_id;
}

// Function to allocate shared memory
int allocate_shared_memory(int total_tables) {
    // Create a shared memory segment with minimal permissions
    int shm_id = shmget(SHM_KEY, total_tables * sizeof(Table), IPC_CREAT | IPC_EXCL | 0640);
    if (shm_id == -1) {
        perror("Error creating shared memory segment");
    }
    return shm_id;
}

// Function to write table data into shared memory
int write_to_shared_memory(int shm_id, Table *tables, int total_tables) {
    // Map the shared memory segment to the process's address space
    Table *shared_tables = (Table *)shmat(shm_id, NULL, 0);
    if (shared_tables == (void *)-1) {
        perror("Error attaching shared memory segment");
        return -1;
    }

    // Copy the table data into shared memory
    for (int i = 0; i < total_tables; i++) {
        shared_tables[i] = tables[i];
    }

    // Detach the shared memory segment from the process
    if (shmdt(shared_tables) == -1) {
        perror("Error detaching shared memory segment");
        return -1;
    }

    return 0;
}

// Function to get the table array from shared memory
Table* get_tables_from_shared_memory(key_t key, int *total_tables, int *shm_id_out) {
    // Get the shared memory ID
    int shm_id = shmget(key, 0, 0640);
    if (shm_id == -1) {
        perror("Error accessing shared memory");
        return NULL;
    }

    // Attach the shared memory segment
    Table *shared_tables = (Table *)shmat(shm_id, NULL, 0);
    if (shared_tables == (void *)-1) {
        perror("Error attaching shared memory segment");
        return NULL;
    }

    // Determine the size of the shared memory segment
    struct shmid_ds shm_info;
    if (shmctl(shm_id, IPC_STAT, &shm_info) == -1) {
        perror("Error getting shared memory segment info");
        shmdt(shared_tables);
        return NULL;
    }

    // Calculate the number of tables
    *total_tables = shm_info.shm_segsz / sizeof(Table);

    // Return the shared memory ID for later use
    if (shm_id_out != NULL) {
        *shm_id_out = shm_id;
    }

    // Return the pointer to the shared memory
    return shared_tables;
}
