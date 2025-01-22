#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include "utils.h"

// Global variables for simulation configuration
#define REAL_WORK_HOURS 8              // Real working hours in pizzeria
#define BASE_SIMULATION_SECONDS 28800   // Base simulation duration (8 hours)

// Time intervals in real seconds (before scaling)
#define BASE_MIN_CUSTOMER_ARRIVAL_TIME 60     // Minimum time between customer groups (1 minute)
#define BASE_MAX_CUSTOMER_ARRIVAL_TIME 600    // Maximum time between customer groups (10 minutes)
#define BASE_MIN_EATING_TIME 900             // Minimum eating time (15 minutes)
#define BASE_MAX_EATING_TIME 3600            // Maximum eating time (1 hour)
#define BASE_SECOND_CASHIER_START 7200      // Second cashier starts after 2 hours
#define BASE_FIRST_CASHIER_END 21600        // First cashier leaves after 6 hours

// Time multiplier (will be set from command line)
double TIME_MULTIPLIER = 1.0;

// Actual simulation durations (calculated based on multiplier)
int SIMULATION_DURATION;
int MIN_CUSTOMER_ARRIVAL_TIME;
int MAX_CUSTOMER_ARRIVAL_TIME;
int MIN_EATING_TIME;
int MAX_EATING_TIME;
int SECOND_CASHIER_START;
int FIRST_CASHIER_END;

// Structure to store table configuration
typedef struct {
    int tables_1person;
    int tables_2person;
    int tables_3person;
    int tables_4person;
} TableConfig;

// Function to calculate scaled time
int scale_time(int base_time) {
    return (int)(base_time / TIME_MULTIPLIER);
}

// Function to initialize time variables based on multiplier
void initialize_time_variables() {
    SIMULATION_DURATION = scale_time(BASE_SIMULATION_SECONDS);
    MIN_CUSTOMER_ARRIVAL_TIME = scale_time(BASE_MIN_CUSTOMER_ARRIVAL_TIME);
    MAX_CUSTOMER_ARRIVAL_TIME = scale_time(BASE_MAX_CUSTOMER_ARRIVAL_TIME);
    MIN_EATING_TIME = scale_time(BASE_MIN_EATING_TIME);
    MAX_EATING_TIME = scale_time(BASE_MAX_EATING_TIME);
    SECOND_CASHIER_START = scale_time(BASE_SECOND_CASHIER_START);
    FIRST_CASHIER_END = scale_time(BASE_FIRST_CASHIER_END);
}

// Function to generate random number in range
int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Function to create random table configuration
TableConfig generate_random_tables() {
    TableConfig config;
    config.tables_1person = random_range(0, 4);
    config.tables_2person = random_range(0, 4);
    config.tables_3person = random_range(0, 4);
    config.tables_4person = random_range(0, 4);
    return config;
}

// Function to start cashier process
pid_t start_cashier(TableConfig config, int duration) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process (cashier)
        char duration_str[20];
        sprintf(duration_str, "%d", duration);
        
        if (config.tables_1person >= 0) {
            // First cashier with table setup
            char t1[10], t2[10], t3[10], t4[10];
            sprintf(t1, "%d", config.tables_1person);
            sprintf(t2, "%d", config.tables_2person);
            sprintf(t3, "%d", config.tables_3person);
            sprintf(t4, "%d", config.tables_4person);
            
            execl("./cashier", "cashier", t1, t2, t3, t4, duration_str, NULL);
        } else {
            // Second cashier (tables already set up)
            execl("./cashier", "cashier", duration_str, NULL);
        }
        exit(1);
    }
    return pid;
}


// Function to start customer group
void spawn_customer_group() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process (customer group)
        int group_size = random_range(1, 3);
        int eating_time = scale_time(random_range(BASE_MIN_EATING_TIME, BASE_MAX_EATING_TIME));
        
        char size_str[10], time_str[20];
        sprintf(size_str, "%d", group_size);
        sprintf(time_str, "%d", eating_time);
        
        execl("./customer", "customer", size_str, time_str, NULL);
        exit(1);
    }
}

// Function to start firefighter process
pid_t start_firefighter() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process (firefighter)
        pid_t pid = execl("./firefighter", "firefighter", NULL);
        exit(1);
    }
    return pid;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, ignore_sigint);
    srand(time(NULL));
    TableConfig config;
    
    // Check for time multiplier in first argument
    if (argc != 2 && argc != 6) {
        printf("Usage: %s <num_1person_tables> <num_2person_tables> <num_3person_tables> <num_4person_tables> <time_multiplier>\n", argv[0]);
        printf("or\n");
        printf("Usage: %s <time_multiplier>\n", argv[0]);
        return 1;
    }

    // Set time multiplier and initialize time variables
    if(argc == 2){TIME_MULTIPLIER = atof(argv[1]);}
    if(argc == 6){TIME_MULTIPLIER = atof(argv[5]);}
    if (TIME_MULTIPLIER <= 0) {
        printf("Time multiplier must be positive\n");
        return 1;
    }
    initialize_time_variables();
    
    printf("Running %d-hour simulation in %.1f hours\n", 
           REAL_WORK_HOURS, 
           REAL_WORK_HOURS / TIME_MULTIPLIER);
    
    // Parse table configuration or generate random
    if (argc == 6) {
        config.tables_1person = atoi(argv[1]);
        config.tables_2person = atoi(argv[2]);
        config.tables_3person = atoi(argv[3]);
        config.tables_4person = atoi(argv[4]);

        if (config.tables_1person < 0 || config.tables_2person < 0 || config.tables_3person < 0 || config.tables_4person < 0) {
            printf("Error: Invalid arguments. Table numbers must be non-negative and duration must be positive.\n");
            return 1;
        }

        int total_tables = config.tables_1person + config.tables_2person + config.tables_3person + config.tables_4person;
        if (total_tables == 0) {
            printf("Error: You must define at least one table.\n");
            return 1;
        }
    } else {
        config = generate_random_tables();
        printf("Generated random table configuration:\n");
        printf("1-person tables: %d\n", config.tables_1person);
        printf("2-person tables: %d\n", config.tables_2person);
        printf("3-person tables: %d\n", config.tables_3person);
        printf("4-person tables: %d\n", config.tables_4person);
    }
    
    // Start firefighter process
    pid_t firefighter_pid = start_firefighter();
    
    // Start first cashier
    pid_t cashier1_pid = start_cashier(config, SIMULATION_DURATION);
    
    // Simulation main loop
    time_t start_time = time(NULL);
    pid_t cashier2_pid = -1;
    int second_cashier_started = 0;
    int first_cashier_ended = 0;

    while (time(NULL) - start_time < SIMULATION_DURATION) {
        // Start second cashier after SECOND_CASHIER_START seconds
        if (!second_cashier_started && (time(NULL) - start_time >= SECOND_CASHIER_START)) {
            TableConfig empty_config = {-1, -1, -1, -1}; // Tables already set up
            cashier2_pid = start_cashier(empty_config, SIMULATION_DURATION - SECOND_CASHIER_START);
            printf("Second cashier started\n");
            second_cashier_started = 1;
        }
        
        // End first cashier after FIRST_CASHIER_END seconds
        if (!first_cashier_ended && (time(NULL) - start_time >= FIRST_CASHIER_END)) {
            kill(cashier1_pid, SIGTERM);
            printf("First cashier ended shift\n");
            first_cashier_ended = 1;
        }
        // Spawn new customer groups
        spawn_customer_group();
        // Wait random time before next customer group
        sleep(random_range(MIN_CUSTOMER_ARRIVAL_TIME, MAX_CUSTOMER_ARRIVAL_TIME));
        reap_children();
    }
    

    while (customers_running())
    {
        sleep(1);
    }
    
    
    // Cleanup
    if (cashier1_pid > 0 && !first_cashier_ended) {
        kill(cashier1_pid, SIGTERM);
    }
    if (cashier2_pid > 0) {
        kill(cashier2_pid, SIGTERM);
    }
    

    printf("Simulation completed\n");
    return 0;
}