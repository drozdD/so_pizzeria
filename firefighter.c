#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "utils.h"

void fire_signal_handler(int signum) {
    // Handle fire signal
    printf("Fire signal received! Evacuating...\n");
    // Notify customers to leave
    kill(0, SIGTERM); // Send signal to all processes in the same group
}

int main() {
    signal(SIGINT, fire_signal_handler);
    while (1) {
        pause(); // Wait for signals
    }
    return 0;
}