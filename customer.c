#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "utils.h"

volatile sig_atomic_t fireFlag = 0;

// Handler for SIGUSR1 from Firefighter
void handle_sigusr1(int sig) {
    printf("\033[1;43m[Customer %d]\033[0m: FIRE!!! Received signal from Firefighter. Exiting...\n", getpid());
    fireFlag = 1;
    exit(0);
}

int main(int argc, char *argv[]) {
    // Ignore Ctrl+C
    signal(SIGINT, ignore_sigint);

    // Handle signal from Firefighter
    signal(SIGUSR1, handle_sigusr1);

    // Check argument count
    if (argc != 3) {
        printf("Usage: %s <number_of_people> <eating_time_in_seconds>\n", argv[0]);
        return 1;
    }

    // Get and validate the group size
    int group_size = atoi(argv[1]);
    if (group_size < 1 || group_size > 3) {
        printf("Error: Number of people must be in [1;3]. Given: %d\n", group_size);
        return 1;
    }

    // Get and validate the eating time
    int eating_time = atoi(argv[2]);
    if (eating_time <= 0) {
        printf("Error: Eating time is incorrect. Given: %d\n", eating_time);
        return 1;
    }

    // Check if the pizzeria is open
    if (how_many_cashiers_running() == 0) {
        printf("Error: Pizzeria is closed, no cashier available.\n");
        return 1;
    }

    // Initialize the message request
    MessageAsk msg;
    int customer_pid = (long)getpid(); // Generate a unique customer ID using PID
    int msg_id = connect_to_mess_queue();
    msg.mtype = 1;
    msg.pid = customer_pid;
    msg.group_size = group_size;
    msg.table_index = -1;


    // Send the request to the cashier
    printf("\033[1;35m[Customer %d]\033[0m: Sent request for group size %d. Msg_id: %d\n", customer_pid, group_size, msg_id);
    if (msgsnd(msg_id, &msg, sizeof(msg) - sizeof(msg.mtype), 0) == -1) {
        perror("Error sending message");
        return EXIT_FAILURE;
    }

    MessageAsk response;

    // Wait for a response from the cashier
    if (msgrcv(msg_id, &response, sizeof(msg) - sizeof(msg.mtype), customer_pid, 0) == -1) {
        return EXIT_FAILURE;
    }

    if (response.group_size) { // Group admitted
        printf("\033[1;35m[Customer %d]\033[0m: GROUP ADMITTED! Eating for %d seconds.\n", customer_pid, eating_time);
        for (int i = 0; i < eating_time; i++) {
            sleep(1);
            if (fireFlag) {exit(0);}
        }

        printf("\033[1;35m[Customer %d]\033[0m: Group of %d finished eating. Quitting...\n", customer_pid, group_size);
        msg.mtype = 2;
        msg.table_index = response.table_index;
        if (msgsnd(msg_id, &msg, sizeof(msg) - sizeof(msg.mtype), 0) == -1) {
            perror("Error sending message");
            return EXIT_FAILURE;
        }
    } else { // Group denied
        printf("\033[1;35m[Customer %d]\033[0m: Group denied. Quitting...\n", customer_pid);
    }

    return 0;
}
