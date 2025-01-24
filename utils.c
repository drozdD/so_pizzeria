#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>

// Ignore ctrl+c
void ignore_sigint(int sig) {}

// Kill all zaombies processes
void reap_children() {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {}
}

// Function to check if other cashier processes are running
int how_many_cashiers_running() {
    char buffer[256];
    FILE *fp = popen("ps -ef | grep 'cashier' | grep -v grep | wc -l", "r");
    if (fp == NULL) {
        perror("Error checking other cashier processes");
        return 1;
    }

    fgets(buffer, sizeof(buffer), fp);
    int countAllCashiers = atoi(buffer);
    pclose(fp);

    char buffer2[256];
    FILE *fpr = popen("ps -ef | grep -F '[cashier] <defunct>' | grep -v grep | wc -l", "r");
    if (fpr == NULL) {
        perror("Error checking other cashier processes");
        return 1;
    }

    fgets(buffer2, sizeof(buffer2), fpr);
    int countFakeCashiers = atoi(buffer2);
    pclose(fpr);

    // Return true if more than one cashier process is running
    return countAllCashiers - countFakeCashiers;
}

int customers_running() {
    char buffer[256];
    FILE *fp = popen("ps -ef | grep 'customer' | grep -v grep |  wc -l", "r");
    if (fp == NULL) {
        perror("Error checking other cashier processes");
        return 1;
    }

    fgets(buffer, sizeof(buffer), fp);
    int count = atoi(buffer);
    pclose(fp);

    // Return true if more than one customer process is running
    return count > 0;
}

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

int allocate_totalTab_shared_memory() {
    // Create a shared memory segment with minimal permissions
    int shm_id = shmget(SHM_KEY_2, sizeof(int), IPC_CREAT | IPC_EXCL | 0640);
    if (shm_id == -1) {
        perror("Error creating shared memory segment");
    }
    return shm_id;
}

void write_totalTab_to_shared_memory(int shmid, int value) {
    // Function to write value into shared memory
    int *shared_mem = (int *)shmat(shmid, NULL, 0);
    if (shared_mem == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    *shared_mem = value;

    if (shmdt(shared_mem) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
}

int get_totalTab_from_shared_memory(int shmid) {
    // Attach the shared memory segment
    int *shared_mem = (int *)shmat(shmid, NULL, 0);
    if (shared_mem == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    int value = *shared_mem;

    if (shmdt(shared_mem) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
    return value;
}


// Function to allocate shared memory
int allocate_tables_shared_memory(int total_tables) {
    // Create a shared memory segment with minimal permissions
    int shm_id = shmget(SHM_KEY_1, total_tables * sizeof(Table), IPC_CREAT | IPC_EXCL | 0640);
    if (shm_id == -1) {
        perror("Error creating shared memory segment");
    }
    return shm_id;
}

// Function to write table data into shared memory
int write_tables_to_shared_memory(Table *tables, int total_tables) {
    int shm_id = shmget(SHM_KEY_1, 0, 0640);
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
Table* get_tables_from_shared_memory(int *total_tables) {
    // Get the shared memory ID
    int shm_id = shmget(SHM_KEY_1, 0, 0640);
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

    // Return the pointer to the shared memory
    return shared_tables;
}
