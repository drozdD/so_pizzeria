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
    int num_of_people = atoi(argv[1]);
    if (num_of_people < 1 || num_of_people > 3) {
        printf("Error: Number of people have to be in [1;3]. Given: %d\n", num_of_people);
        return 1;
    }

    // Get and validate time of eating
    int eating_time = atoi(argv[2]);
    if (eating_time <= 0) {
        printf("Error: Time of eating is incorecct. Given: %d\n", eating_time);
        return 1;
    }

    // Start simulation
    printf("Customer process started. Number of people: %d, eating time: %d seconds.\n", num_of_people, eating_time);

    // Eating simulation
    sleep(eating_time);

    printf("Customer process ended. Number of people: %d.\n", num_of_people);

    return 0;
}

