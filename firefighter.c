#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

#define MAX_PIDS 128 // Maximum number of processes to handle

// Helper function to find PIDs of processes by name
int find_process_pids(const char *process_name, pid_t *pids, size_t max_pids) {
    char command[128];
    snprintf(command, sizeof(command), "pgrep -f %s", process_name); // Search for process by name

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Error running pgrep");
        return -1;
    }

    char pid_str[16];
    size_t count = 0;

    // Read each PID from pgrep output
    while (fgets(pid_str, sizeof(pid_str), fp) != NULL) {
        if (count >= max_pids) {
            fprintf(stderr, "Error: Too many processes found. Increase MAX_PIDS.\n");
            break;
        }
        pids[count++] = (pid_t)atoi(pid_str);
    }

    pclose(fp);
    return count - 1; // Return the number of found PIDs
}

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    pid_t customer_pids[MAX_PIDS];
    pid_t cashier_pids[MAX_PIDS];
    pid_t main_pids[MAX_PIDS];

    // Find PIDs of all "customer" and "cashier" processes
    int customer_count = find_process_pids("customer", customer_pids, MAX_PIDS);
    int cashier_count = find_process_pids("cashier", cashier_pids, MAX_PIDS);
    cashier_count = how_many_cashiers_running();
    int main_count = find_process_pids("main", main_pids, MAX_PIDS);

    // access semaphore set 
    int sem_id = semget(SEM_KEY, 3, 0640);
    if (sem_id == -1) {
        perror("Failed to access semaphore set");
        exit(EXIT_FAILURE);
    }

    semaphore_wait(sem_id, 1);

    printf("\033[1;43m[Firefighter]\033[0m: FIRE!!! Sending signals to Customer(s) and Cashier(s)...\n");
    // Send signals to all "customer" processes
    for (int i = 0; i < customer_count; i++) {
        kill(customer_pids[i], SIGUSR1);
        printf("\033[1;43m[Firefighter]\033[0m: Signal sent to Customer (PID: %d).\n", customer_pids[i]);
    }

    // Send signals to all "cashier" processes
    for (int i = 0; i < cashier_count; i++) {
        kill(cashier_pids[i], SIGUSR1);
        printf("\033[1;43m[Firefighter]\033[0m: Signal sent to Cashier (PID: %d).\n", cashier_pids[i]);
    }

    semaphore_wait(sem_id, 2);

    // Send signals to all "main" processes
    for (int i = 0; i < main_count; i++) {
        kill(main_pids[0], SIGUSR1);
        printf("\033[1;43m[Firefighter]\033[0m: Signal sent to main process (PID: %d).\n", main_pids[0]);
    }

    remove_semaphore(sem_id);

    printf("\033[1;43m[Firefighter]\033[0m: Exiting...\n");
    exit(0);
}

int main() {
    // Register SIGINT handler
    signal(SIGINT, handle_sigint);

    printf("\033[1;43m[Firefighter]\033[0m: Running. Press Ctrl+C to send signals to processes.\n");

    while (1) {
        pause(); // Wait for signals
    }

    return 0;
}
