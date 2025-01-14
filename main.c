#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int main() {
    srand(time(NULL)); // Seed for random number generation

    pid_t firefighter_pid, manager_pid;

    // Create the firefighter process
    firefighter_pid = fork();
    if (firefighter_pid < 0) {
        perror("Failed to fork firefighter");
        exit(1);
    } else if (firefighter_pid == 0) {
        // Child process for firefighter
        execlp("./firefighter", "firefighter", NULL);
        perror("Failed to exec firefighter");
        exit(1);
    }

    // Create the manager process
    manager_pid = fork();
    if (manager_pid < 0) {
        perror("Failed to fork manager");
        exit(1);
    } else if (manager_pid == 0) {
        // Child process for manager
        execlp("./manager", "manager", NULL);
        perror("Failed to exec manager");
        exit(1);
    }

    // Simulate customer processes
    for (int i = 0; i < 10; i++) {
        if (fork() == 0) {
            // Child process for customer
            execlp("./customer", "customer", NULL);
            perror("Failed to exec customer");
            exit(1);
        }
        sleep(rand() % 10 + 1); // Random delay between customer arrivals
    }

    // Wait for the manager and firefighter processes to finish
    waitpid(firefighter_pid, NULL, 0);
    waitpid(manager_pid, NULL, 0);

    return 0;
}