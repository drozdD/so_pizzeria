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

#define SEM_KEY 2222 // semaphores
#define SHM_KEY_1 1111 // current tables 
#define SHM_KEY_2 2137 // total number of current tables

typedef struct {
    long mtype; // Message type (customer's ID)
    long pid;
    int group_size; // Number of people in the group
    int table_index;
} MessageAsk;
 
typedef struct {
    int max_capacity;        // Maksymalna liczba miejsc przy stoliku
    int occupied_capacity;   // Aktualna liczba zajętych miejsc
    int group_count;         // Liczba grup przy stoliku
} Table;
void ignore_sigint(int sig);
void reap_children();
int how_many_cashiers_running();
int customers_running();
int initialize_semaphore(int sem_id, int sem_num, int initial_value);
int semaphore_wait(int sem_id, int sem_num);
int semaphore_signal(int sem_id, int sem_num);
int get_semaphore_value(int sem_id, int sem_num);
void remove_semaphore(int sem_id);
int connect_to_mess_queue();
int allocate_totalTab_shared_memory();
void write_totalTab_to_shared_memory(int shmid, int value);
int get_totalTab_from_shared_memory(int shmid);
int allocate_tables_shared_memory(int total_tables);
int write_tables_to_shared_memory(Table *tables, int total_tables);
Table* get_tables_from_shared_memory(int *total_tables);
#endif
