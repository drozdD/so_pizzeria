#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"

int main() {
	// Simulate customer arrival
	int group_size = rand() % 3 + 1; // Random group size between 1 and 3
     	printf("Customer group of %d entering...\n", group_size);
          
	// Logic to find a table and sit down
	// This will include semaphore operations to access shared memory
	
	return 0;
}
