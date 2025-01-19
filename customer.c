#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    // Check arguments count
    if (argc != 3) {
        printf("Usage: %s <number_of_people> <eating_time_in_seconds>\n", argv[0]);
        return 1;
    }

    // Get and validate number of people
    int group_size = atoi(argv[1]);
    if (group_size < 1 || group_size > 3) {
        printf("Error: Number of people have to be in [1;3]. Given: %d\n", group_size);
        return 1;
    }

    // Get and validate time of eating
    int eating_time = atoi(argv[2]);
    if (eating_time <= 0) {
        printf("Error: Time of eating is incorecct. Given: %d\n", eating_time);
        return 1;
    }

    MessageAsk msg;
    // Generate a unique customer ID based on the process ID (PID)
    int customer_pid = (long)getpid();
    // Create or connect to the message queue
    int msg_id = connect_to_mess_queue();
    msg.mtype = 1;
    msg.pid = customer_pid;
    msg.group_size = group_size;
    msg.table_index = -1;
    // Send request to the cashier
    if (msgsnd(msg_id, &msg, sizeof(msg) - sizeof(msg.mtype), 0) == -1) {
        perror("Error sending message");
        return EXIT_FAILURE;
    }
    printf("\033[1;35m[Customer %d]\033[0m: Sent request for group size %d. Msg_id: %d\n", customer_pid, group_size, msg_id);
    sleep(1);

    MessageAsk response;

    // Wait for a response from the cashier
    if (msgrcv(msg_id, &response, sizeof(msg) - sizeof(msg.mtype), customer_pid, 0) == -1) {
        perror("Error receiving response");
        return EXIT_FAILURE;
    }

    if (response.group_size) { // Cashier admitted the group
        printf("\033[1;35m[Customer %d]\033[0m: GROUP ADMITTED! Eating for %d seconds.\n", customer_pid, eating_time);
        sleep(eating_time); // Simulate eating time
        printf("\033[1;35m[Customer %d]\033[0m: Finished eating. Quiting...\n", customer_pid);
        msg.mtype = 2;
        msg.table_index = response.table_index;
        if (msgsnd(msg_id, &msg, sizeof(msg) - sizeof(msg.mtype), 0) == -1) {
            perror("Error sending message");
            return EXIT_FAILURE;
        }
    } else { // Cashier denied the group
        printf("\033[1;35m[Customer %d]\033[0m: Group denied. Quiting...\n", customer_pid);
    }

    return 0;
}

