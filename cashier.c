#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
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

int customers_running() {
    char buffer[256];
    FILE *fp = popen("ps -ef | grep './customer' | grep -v grep |  wc -l", "r");
    if (fp == NULL) {
        perror("Error checking other cashier processes");
        return 1;
    }

    fgets(buffer, sizeof(buffer), fp);
    int count = atoi(buffer);
    pclose(fp);

    printf("count: %d\n", count);

    // Return true if more than one cashier process is running
    return count > 0;
}

int assign_group(Table *tables, int table_count, int group_size, int *index) {
    Table *best_table = NULL;

    // Seraching for best table
    for (int i = 0; i < table_count; i++) {
        if (tables[i].occupied_capacity + group_size <= tables[i].max_capacity &&
            (tables[i].group_count == 0 || 
             tables[i].occupied_capacity / tables[i].group_count == group_size)) {
            // Choose the best table (with minimal capacity)
            if (!best_table || tables[i].max_capacity < best_table->max_capacity) {
                best_table = &tables[i];
                *index = i;
            }
        }
    }
  
    // if table found, assign group
    if (best_table) {
        best_table->occupied_capacity += group_size;
        best_table->group_count++;
        printf("\033[1;32m[Cashier]\033[0m: Group of %d has been assigned to table with capacity of %d.\n", group_size, best_table->max_capacity);
        return 1;
    } else {
        // If there is not proper table
        printf("\033[1;32m[Cashier]\033[0m: Sorry, cannot accept group of %d.\n", group_size);
        return 0;
    }
}

void handle_message_queue(int duration) {
    MessageAsk msg;

    // Create or connect to the messageAsk queue
    int msg_id = connect_to_mess_queue();
    printf("\033[1;32m[Cashier]\033[0m: Waiting for customers with msd_id = %d...\n\n", msg_id);
    time_t start_time = time(NULL);
    // Listen to the message queue during the specified duration
    int shm_id = shmget(SHM_KEY_2, 0, 0640);
    int total_tables = get_totalTab_from_shared_memory(shm_id);
    Table *tables = (Table *)malloc(total_tables * sizeof(Table));
    if (tables == NULL) {
        perror("Error allocating memory for tables array");
    }
    while (time(NULL) - start_time < duration) {
        if (msgrcv(msg_id, &msg, sizeof(msg) - sizeof(msg.mtype), 1, IPC_NOWAIT) != -1) {
            printf("\033[1;32m[Cashier]\033[0m: Received group of %d people (pid=%ld).\n", msg.group_size, msg.pid);
            fflush(stdout); 

            // Decide whether to allow entry
            // ---------------------------------------------------------------------------
            tables = get_tables_from_shared_memory(&total_tables);
            int index_of_table = -1;
            bool allow_entry = assign_group(tables, total_tables, msg.group_size, &index_of_table);
            write_tables_to_shared_memory(tables, total_tables);
            // ---------------------------------------------------------------------------

            // Send a response back to the customer
            msg.group_size = allow_entry ? 1 : 0; // 1 = allowed, 0 = denied
            msg.table_index = index_of_table;
            msg.mtype = msg.pid;
            if (msgsnd(msg_id, &msg, sizeof(msg) - sizeof(msg.mtype), 0) == -1) {
                perror("Error sending response to customer");
            } 
           
        } else if (errno != ENOMSG) {
            perror("Error receiving message");
        }

        if (msgrcv(msg_id, &msg, sizeof(msg) - sizeof(msg.mtype), 2, IPC_NOWAIT) != -1) {
            tables = get_tables_from_shared_memory(&total_tables);
            tables[msg.table_index].group_count -= 1;
            tables[msg.table_index].occupied_capacity -= msg.group_size;
            write_tables_to_shared_memory(tables, total_tables);
            printf("tables[msg.table_index].group_count = %d, tables[msg.table_index].occupied_capacity = %d\n\n", tables[msg.table_index].group_count, tables[msg.table_index].occupied_capacity);
        }

        usleep(500000); // Sleep for 0.5 seconds to reduce CPU usage
    }

    if(customers_running()){
        printf("\033[1;32m[Cashier]\033[0m: Customers still eating, have to stay overtime...\n");
        if (msgrcv(msg_id, &msg, sizeof(msg) - sizeof(msg.mtype), 2, 0) != -1) {
            tables = get_tables_from_shared_memory(&total_tables);
            tables[msg.table_index].group_count -= 1;
            tables[msg.table_index].occupied_capacity -= msg.group_size;
            write_tables_to_shared_memory(tables, total_tables);
            printf("tables[msg.table_index].group_count = %d, tables[msg.table_index].occupied_capacity = %d\n\n", tables[msg.table_index].group_count, tables[msg.table_index].occupied_capacity);
        }
    }
    
}

// Table* getTables(){
//     int shm_id = shmget(SHM_KEY_2, 0, 0640);
//     int total_tables = get_totalTab_from_shared_memory(shm_id);
//     Table *tables = (Table *)malloc(total_tables * sizeof(Table));
//     if (tables == NULL) {
//         perror("Error allocating memory for tables array");
//         return 1;
//     }
//     return get_tables_from_shared_memory(total_tables);
// }

int main(int argc, char *argv[]) {
    int shm_id = shmget(SHM_KEY_1, 0, 0640);
    int duration;
    if (shm_id != -1) {
        // Shared memory exists
        if (argc > 3) {
            // Arguments were provided, which is not allowed when shared memory exists
            printf("Error: Tables are already initialized. More than one arguments are not allowed. Usage: %s <duration_in_seconds> \n", argv[0]);
            return 1;
        } else {
            // Shared memory exists, and no arguments were provided
	    duration = (argc == 2) ? atoi(argv[1]) : 20;		
            printf("\033[1;32m[Cashier]\033[0m: Tables are already set up. No changes made.\n");
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

        shm_id = allocate_totalTab_shared_memory();
        write_totalTab_to_shared_memory(shm_id, total_tables);

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

        // Allocate shared memory for tables
        shm_id = allocate_tables_shared_memory(total_tables);
        if (shm_id == -1) {
            free(tables);
            return 1;
        }

        // Write the tables array to shared memory
        if (write_tables_to_shared_memory(tables, total_tables) == -1) {
            free(tables);
            return 1;
        }
        printf("\033[1;32m[Cashier]\033[0m: Tables successfully initialized in shared memory.\n");

        // Free the dynamically allocated memory
        free(tables);
    }
    printf("\033[1;32m[Cashier]\033[0m: Cashier running for %d seconds...\n", duration);

    // Run the cashier process for the specified duration
    handle_message_queue(duration);

    // Check if other cashier processes are running
    if (!other_cashiers_running()) {
        //clean up
        int msg_id = connect_to_mess_queue();
        struct msqid_ds msq_status;
        if (msgctl(msg_id, IPC_STAT, &msq_status) == -1) {
            perror("Error checking message queue status");
        } else if (msq_status.msg_qnum == 0) {
            if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
                perror("Error removing message queue");
            } else {
                printf("\n\033[1;32m[Cashier]\033[0m: Message queue cleaned up.\n");
            }
        } else{
            printf("jest jeszcze else.......   %ld\n\n", msq_status.msg_qnum);
            if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
                perror("Error removing message queue");
            } else {
                printf("\n\033[1;32m[Cashier]\033[0m: Message queue cleaned up.\n");
            }
        }

        // No other cashier processes; clean up shared memory
        printf("\033[1;32m[Cashier]\033[0m: No other cashier processes detected. Cleaning up shared memory...\n");
        shm_id = shmget(SHM_KEY_1, 0, 0640);
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("Error removing shared memory segment");
            return 1;
        }
        shm_id = shmget(SHM_KEY_2, 0, 0640);
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("Error removing shared memory segment");
            return 1;
        }
        printf("\033[1;32m[Cashier]\033[0m: Shared memory successfully cleaned up.\n");
        
    } else {
        printf("\033[1;32m[Cashier]\033[0m: Other cashier processes are still running. Shared memory not removed.\n");
    }

    return 0;
}
