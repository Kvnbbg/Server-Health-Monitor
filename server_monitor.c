#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

// Function to simulate getting CPU usage
float getServerCpuLoad() {
    return (rand() % 100); // Random value between 0 and 99
}

// Function to simulate getting used RAM
float getServerUsedRam() {
    return (rand() % 100); // Random value between 0 and 99
}

// Function to simulate getting max RAM
float getServerMaxRam() {
    return 100.0; // Fixed value for simplicity
}

// Function to log health status
void logHealthStatus(const char* server) {
    float cpuUsage = getServerCpuLoad();
    float ramUsed = getServerUsedRam();
    float ramMax = getServerMaxRam();
    float ramUsagePercentage = (ramUsed / ramMax) * 100;

    printf("Server Health Report for: %s\n", server);
    printf("CPU Usage: %.2f%%\n", cpuUsage);
    printf("RAM Usage: %.2f%% (%.2f GB / %.2f GB)\n", ramUsagePercentage, ramUsed, ramMax);

    const float criticalThreshold = 90.0;
    const float warningThreshold = 75.0;

    if (cpuUsage > criticalThreshold) {
        printf("Critical: High CPU usage detected!\n");
        printf("Solution: Killing unnecessary processes and notifying administrator.\n");
    } else if (cpuUsage > warningThreshold) {
        printf("Warning: CPU usage is high.\n");
    }

    if (ramUsagePercentage > criticalThreshold) {
        printf("Critical: High RAM usage detected!\n");
        printf("Solution: Freeing up RAM and notifying administrator.\n");
    } else if (ramUsagePercentage > warningThreshold) {
        printf("Warning: RAM usage is high.\n");
    }

    printf("----------------------------------\n");
}

// Function to display the menu
void displayMenu() {
    printf("\nServer Health Monitor\n");
    printf("1. Monitor Server Health\n");
    printf("2. Set Monitoring Interval\n");
    printf("3. Set Monitoring Duration\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
}

// Function to get integer input with validation
int getIntegerInput() {
    int value;
    char buffer[100];
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Error reading input.\n");
            exit(1);
        }
        if (sscanf(buffer, "%d", &value) == 1) {
            return value;
        } else {
            printf("Invalid input. Please enter a valid integer: ");
        }
    }
}

// Function to monitor server health
void monitorServerHealth(const char* server, int interval, int duration) {
    int endTime = time(NULL) + duration / 1000; // Convert duration to seconds

    while (time(NULL) < endTime) {
        logHealthStatus(server);
        usleep(interval * 1000); // Convert interval to microseconds
    }

    printf("Health monitoring completed for server: %s\n", server);
}

int main() {
    srand(time(NULL)); // Seed the random number generator

    const char* server = "home";
    int interval = 1000; // Default interval in milliseconds
    int duration = 60000; // Default duration in milliseconds
    int choice;
    bool running = true;

    printf("Server Health Monitor by Kevin Marville\n");
    printf("GitHub: https://github.com/kvnbbg\n");

    while (running) {
        displayMenu();
        choice = getIntegerInput();

        switch (choice) {
            case 1:
                printf("Monitoring server health...\n");
                monitorServerHealth(server, interval, duration);
                break;
            case 2:
                printf("Enter the new monitoring interval (in milliseconds): ");
                interval = getIntegerInput();
                if (interval <= 0) {
                    printf("Interval must be a positive number. Using default interval.\n");
                    interval = 1000;
                } else {
                    printf("Monitoring interval set to %d milliseconds.\n", interval);
                }
                break;
            case 3:
                printf("Enter the new monitoring duration (in milliseconds): ");
                duration = getIntegerInput();
                if (duration <= 0) {
                    printf("Duration must be a positive number. Using default duration.\n");
                    duration = 60000;
                } else {
                    printf("Monitoring duration set to %d milliseconds.\n", duration);
                }
                break;
            case 4:
                running = false;
                printf("Exiting the program.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
