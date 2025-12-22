#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "monitor.h"
#include "monitor_config.h"
#include "monitor_status.h"

static void log_info(const char* message) {
    printf("[INFO] %s\n", message);
}

static void log_warning(const char* message) {
    fprintf(stderr, "[WARN] %s\n", message);
}

static void log_error(const char* message) {
    fprintf(stderr, "[ERROR] %s\n", message);
}

static void print_usage(const char* program) {
    printf("Server Health Monitor\n\n");
    printf("Usage: %s [options]\n\n", program);
    printf("Options:\n");
    printf("  --server NAME          Server name to display (default: local)\n");
    printf("  --interval-ms MS       Sampling interval in milliseconds\n");
    printf("  --duration-ms MS       Total monitoring duration in milliseconds\n");
    printf("  --iterations N         Run N samples (non-interactive only)\n");
    printf("  --non-interactive      Run without the menu (use flags/env)\n");
    printf("  -h, --help             Show this help message\n\n");
    printf("Environment variables:\n");
    printf("  SHM_SERVER_NAME, SHM_INTERVAL_MS, SHM_DURATION_MS,\n");
    printf("  SHM_NON_INTERACTIVE, SHM_ITERATIONS\n");
}

static void display_menu(void) {
    printf("\nServer Health Monitor\n");
    printf("1. Monitor Server Health\n");
    printf("2. Set Monitoring Interval\n");
    printf("3. Set Monitoring Duration\n");
    printf("4. Show Current Configuration\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
}

static MonitorStatus get_integer_input(const char* prompt, int min, int max, int* out) {
    char buffer[128] = {0};
    MonitorStatus status = MONITOR_STATUS_OK;

    if (!prompt || !out) {
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    while (true) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            return MONITOR_STATUS_IO_ERROR;
        }

        buffer[strcspn(buffer, "\n")] = '\0';
        status = parse_int_range(buffer, min, max, out);
        if (status == MONITOR_STATUS_OK) {
            return status;
        }
        printf("Invalid input. Enter a value between %d and %d.\n", min, max);
    }
}

static long long now_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return -1;
    }
    return (long long)ts.tv_sec * 1000LL + (long long)ts.tv_nsec / 1000000LL;
}

static void sleep_ms(int milliseconds) {
    if (milliseconds <= 0) {
        return;
    }

    struct timespec req;
    req.tv_sec = milliseconds / 1000;
    req.tv_nsec = (long)(milliseconds % 1000) * 1000000L;

    while (nanosleep(&req, &req) == -1 && errno == EINTR) {
        // Restart sleep with the remaining duration.
    }
}

static MonitorStatus log_health_status(const char* server, CpuTracker* tracker) {
    double cpu_usage = 0.0;
    MemoryUsage memory = {0};
    MonitorStatus status = monitor_read_cpu_usage(tracker, &cpu_usage);
    if (status != MONITOR_STATUS_OK) {
        log_error("Failed to read CPU usage.");
        return status;
    }

    status = monitor_read_memory_usage(&memory);
    if (status != MONITOR_STATUS_OK) {
        log_error("Failed to read memory usage.");
        return status;
    }

    printf("Server Health Report for: %s\n", server);
    printf("CPU Usage: %.2f%%\n", cpu_usage);
    printf("RAM Usage: %.2f%% (%.2f GB / %.2f GB)\n", memory.usage_percent, memory.used_gb, memory.total_gb);

    const double critical_threshold = 90.0;
    const double warning_threshold = 75.0;

    if (cpu_usage > critical_threshold) {
        printf("Critical: High CPU usage detected.\n");
    } else if (cpu_usage > warning_threshold) {
        printf("Warning: CPU usage is elevated.\n");
    }

    if (memory.usage_percent > critical_threshold) {
        printf("Critical: High RAM usage detected.\n");
    } else if (memory.usage_percent > warning_threshold) {
        printf("Warning: RAM usage is elevated.\n");
    }

    printf("----------------------------------\n");
    return MONITOR_STATUS_OK;
}

static MonitorStatus monitor_server_health(const MonitorConfig* config) {
    CpuTracker tracker = {0};
    MonitorStatus status = MONITOR_STATUS_OK;

    if (!config) {
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    if (config->iterations > 0) {
        for (int i = 0; i < config->iterations; i++) {
            status = log_health_status(config->server_name, &tracker);
            if (status != MONITOR_STATUS_OK) {
                return status;
            }
            if (i + 1 < config->iterations) {
                sleep_ms(config->interval_ms);
            }
        }
        return MONITOR_STATUS_OK;
    }

    long long start_ms = now_ms();
    if (start_ms < 0) {
        return MONITOR_STATUS_INTERNAL_ERROR;
    }

    while (true) {
        long long elapsed = now_ms() - start_ms;
        if (elapsed >= config->duration_ms) {
            break;
        }

        status = log_health_status(config->server_name, &tracker);
        if (status != MONITOR_STATUS_OK) {
            return status;
        }

        sleep_ms(config->interval_ms);
    }

    printf("Health monitoring completed for server: %s\n", config->server_name);
    return MONITOR_STATUS_OK;
}

int main(int argc, char** argv) {
    MonitorConfig config;
    MonitorStatus status = MONITOR_STATUS_OK;
    char error[128] = {0};
    bool show_help = false;
    bool running = true;

    monitor_config_init(&config);

    status = monitor_config_apply_env(&config, error, sizeof(error));
    if (status != MONITOR_STATUS_OK) {
        log_warning(error);
    }

    status = monitor_config_apply_args(&config, argc, argv, &show_help, error, sizeof(error));
    if (status != MONITOR_STATUS_OK) {
        log_error(error);
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (show_help) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    status = monitor_config_validate(&config, error, sizeof(error));
    if (status != MONITOR_STATUS_OK) {
        log_error(error);
        return EXIT_FAILURE;
    }

    printf("Server Health Monitor\n");
    printf("GitHub: https://github.com/kvnbbg\n");

    if (config.non_interactive) {
        log_info("Running in non-interactive mode.");
        status = monitor_server_health(&config);
        if (status != MONITOR_STATUS_OK) {
            log_error(monitor_status_message(status));
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    while (running) {
        display_menu();
        int choice = 0;
        status = get_integer_input("", 1, 5, &choice);
        if (status != MONITOR_STATUS_OK) {
            log_error("Failed to read menu input.");
            return EXIT_FAILURE;
        }

        switch (choice) {
            case 1:
                log_info("Monitoring server health...");
                status = monitor_server_health(&config);
                if (status != MONITOR_STATUS_OK) {
                    log_error(monitor_status_message(status));
                }
                break;
            case 2: {
                int new_interval = 0;
                status = get_integer_input("Enter new interval (ms): ",
                                           MONITOR_MIN_INTERVAL_MS,
                                           MONITOR_MAX_INTERVAL_MS,
                                           &new_interval);
                if (status == MONITOR_STATUS_OK) {
                    config.interval_ms = new_interval;
                    log_info("Monitoring interval updated.");
                }
                break;
            }
            case 3: {
                int new_duration = 0;
                status = get_integer_input("Enter new duration (ms): ",
                                           MONITOR_MIN_DURATION_MS,
                                           MONITOR_MAX_DURATION_MS,
                                           &new_duration);
                if (status == MONITOR_STATUS_OK) {
                    if (new_duration < config.interval_ms) {
                        log_warning("Duration must be >= interval; keeping previous value.");
                    } else {
                        config.duration_ms = new_duration;
                        log_info("Monitoring duration updated.");
                    }
                }
                break;
            }
            case 4:
                monitor_config_print(&config);
                break;
            case 5:
                running = false;
                log_info("Exiting.");
                break;
            default:
                log_warning("Invalid choice.");
                break;
        }
    }

    return EXIT_SUCCESS;
}
