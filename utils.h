#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define SEM_KEY 1234
#define SHM_KEY 5678

typedef struct {
    long mtype; // Message type (customer's ID)
    int group_size; // Number of people in the group
} MessageAsk;
 
typedef struct {
    int max_capacity;        // Maksymalna liczba miejsc przy stoliku
    int occupied_capacity;   // Aktualna liczba zajętych miejsc
    int group_count;         // Liczba grup przy stoliku
} Table;
int connect_to_mess_queue();
int allocate_shared_memory(int total_tables);
int write_to_shared_memory(int shm_id, Table *tables, int total_tables);
Table* get_tables_from_shared_memory(key_t key, int *total_tables, int *shm_id_out);
#endif
