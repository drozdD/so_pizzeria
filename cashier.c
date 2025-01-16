#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include "utils.h"

// Function to check if other cashier processes are running
int other_cashiers_running() {
    char buffer[256];
    FILE *fp = popen("ps -ef | grep './cashier' | grep -v grep | wc -l", "r");
    if (fp == NULL) {
        perror("Error checking other cashier processes");
        return 1;
    }

    fgets(buffer, sizeof(buffer), fp);
    int count = atoi(buffer);
    pclose(fp);

    // Return true if more than one cashier process is running
    return count > 1;
}

int main(int argc, char *argv[]) {
    int duration;
    int shm_id = shmget(SHM_KEY, 0, 0640);
    if (shm_id != -1) {
        // Shared memory exists
        if (argc > 3) {
            // Arguments were provided, which is not allowed when shared memory exists
            printf("Error: Tables are already initialized. More than one arguments are not allowed. Usage: %s <duration_in_seconds> \n", argv[0]);
            return 1;
        } else {
            // Shared memory exists, and no arguments were provided
	    duration = (argc == 2) ? atoi(argv[1]) : 20;		
            printf("Tables are already set up. No changes made.\n");
        }
    } else {
        // Shared memory does not exist
        if (argc != 6) {
            printf("Usage: %s <num_1person_tables> <num_2person_tables> <num_3person_tables> <num_4person_tables> <duration_in_seconds>\n", argv[0]);
            return 1;
        }

        // Parse the number of tables from the arguments
        int num_1person = atoi(argv[1]);
        int num_2person = atoi(argv[2]);
        int num_3person = atoi(argv[3]);
        int num_4person = atoi(argv[4]);
        duration = atoi(argv[5]);

        // Validate the input values
        if (num_1person < 0 || num_2person < 0 || num_3person < 0 || num_4person < 0 || duration <= 0) {
            printf("Error: Invalid arguments. Table numbers must be non-negative and duration must be positive.\n");
            return 1;
        }

        int total_tables = num_1person + num_2person + num_3person + num_4person;
        if (total_tables == 0) {
            printf("Error: You must define at least one table.\n");
            return 1;
        }

        // Dynamically allocate memory for the tables array
        Table *tables = (Table *)malloc(total_tables * sizeof(Table));
        if (tables == NULL) {
            perror("Error allocating memory for tables array");
            return 1;
        }

        // Initialize the tables array
        int index = 0;
        for (int i = 0; i < num_1person; i++, index++) {
            tables[index].max_capacity = 1;
            tables[index].occupied_capacity = 0;
            tables[index].group_count = 0;
        }
        for (int i = 0; i < num_2person; i++, index++) {
            tables[index].max_capacity = 2;
            tables[index].occupied_capacity = 0;
            tables[index].group_count = 0;
        }
        for (int i = 0; i < num_3person; i++, index++) {
            tables[index].max_capacity = 3;
            tables[index].occupied_capacity = 0;
            tables[index].group_count = 0;
        }
        for (int i = 0; i < num_4person; i++, index++) {
            tables[index].max_capacity = 4;
            tables[index].occupied_capacity = 0;
            tables[index].group_count = 0;
        }

        // Allocate shared memory
        shm_id = allocate_shared_memory(total_tables);
        if (shm_id == -1) {
            free(tables);
            return 1;
        }

        // Write the tables array to shared memory
        if (write_to_shared_memory(shm_id, tables, total_tables) == -1) {
            free(tables);
            return 1;
        }
        printf("Tables successfully initialized in shared memory.\n");

        // Free the dynamically allocated memory
        free(tables);
    }

    // Run the cashier process for the specified duration
    printf("Cashier running for %d seconds...\n", duration);
    sleep(duration);

    // Check if other cashier processes are running
    if (!other_cashiers_running()) {
        // No other cashier processes; clean up shared memory
        printf("No other cashier processes detected. Cleaning up shared memory...\n");
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("Error removing shared memory segment");
            return 1;
        }
        printf("Shared memory successfully cleaned up.\n");
    } else {
        printf("Other cashier processes are still running. Shared memory not removed.\n");
    }

    return 0;
}
