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

static bool supports_ansi_output(void) {
    const char* term = getenv("TERM");
    if (!term || strcmp(term, "dumb") == 0) {
        return false;
    }

    return isatty(STDOUT_FILENO);
}

static const char* ansi_color(bool enabled, const char* code) {
    return enabled ? code : "";
}

static const char* ansi_reset(bool enabled) {
    return enabled ? "\x1b[0m" : "";
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

static const char* usage_label(double usage_percent) {
    const double critical_threshold = 90.0;
    const double warning_threshold = 75.0;

    if (usage_percent > critical_threshold) {
        return "CRITICAL";
    }
    if (usage_percent > warning_threshold) {
        return "WARNING";
    }
    return "OK";
}

static void build_usage_bar(char* buffer, size_t buffer_size, double usage_percent, int width) {
    if (!buffer || buffer_size == 0 || width <= 0) {
        return;
    }

    if (usage_percent < 0.0) {
        usage_percent = 0.0;
    } else if (usage_percent > 100.0) {
        usage_percent = 100.0;
    }

    int filled = (int)((usage_percent / 100.0) * width + 0.5);
    if (filled < 0) {
        filled = 0;
    }
    if (filled > width) {
        filled = width;
    }

    size_t index = 0;
    for (int i = 0; i < width && index + 1 < buffer_size; i++) {
        buffer[index++] = (i < filled) ? '#' : '.';
    }
    buffer[index] = '\0';
}

static const char* status_color(bool ansi, const char* label) {
    if (!ansi) {
        return "";
    }
    if (strcmp(label, "CRITICAL") == 0) {
        return "\x1b[31m";
    }
    if (strcmp(label, "WARNING") == 0) {
        return "\x1b[33m";
    }
    return "\x1b[32m";
}

static void clear_screen(bool ansi) {
    if (!ansi) {
        return;
    }
    printf("\x1b[2J\x1b[H");
}

static MonitorStatus collect_health_snapshot(CpuTracker* tracker, double* cpu_usage, MemoryUsage* memory) {
    MonitorStatus status = monitor_read_cpu_usage(tracker, cpu_usage);
    if (status != MONITOR_STATUS_OK) {
        log_error("Failed to read CPU usage.");
        return status;
    }

    status = monitor_read_memory_usage(memory);
    if (status != MONITOR_STATUS_OK) {
        log_error("Failed to read memory usage.");
        return status;
    }

    return MONITOR_STATUS_OK;
}

static void log_threshold_messages(double cpu_usage, double memory_usage) {
    if (cpu_usage > 90.0) {
        printf("Critical: High CPU usage detected.\n");
    } else if (cpu_usage > 75.0) {
        printf("Warning: CPU usage is elevated.\n");
    }

    if (memory_usage > 90.0) {
        printf("Critical: High RAM usage detected.\n");
    } else if (memory_usage > 75.0) {
        printf("Warning: RAM usage is elevated.\n");
    }
}

static MonitorStatus log_health_status(const char* server, CpuTracker* tracker) {
    double cpu_usage = 0.0;
    MemoryUsage memory = {0};
    MonitorStatus status = collect_health_snapshot(tracker, &cpu_usage, &memory);
    if (status != MONITOR_STATUS_OK) {
        return status;
    }

    printf("Server Health Report for: %s\n", server);
    printf("CPU Usage: %.2f%%\n", cpu_usage);
    printf("RAM Usage: %.2f%% (%.2f GB / %.2f GB)\n", memory.usage_percent, memory.used_gb, memory.total_gb);

    log_threshold_messages(cpu_usage, memory.usage_percent);

    printf("----------------------------------\n");
    return MONITOR_STATUS_OK;
}

static void render_live_dashboard(const MonitorConfig* config,
                                  const char* server,
                                  double cpu_usage,
                                  const MemoryUsage* memory,
                                  long long elapsed_ms,
                                  long long remaining_ms,
                                  int sample_index,
                                  int total_samples,
                                  bool ansi) {
    const char spinner_chars[] = {'|', '/', '-', '\\'};
    char cpu_bar[64] = {0};
    char mem_bar[64] = {0};
    const char* cpu_label = usage_label(cpu_usage);
    const char* mem_label = usage_label(memory->usage_percent);
    const char* cpu_color = status_color(ansi, cpu_label);
    const char* mem_color = status_color(ansi, mem_label);
    const char* header_color = ansi_color(ansi, "\x1b[36m");
    const char* bold = ansi_color(ansi, "\x1b[1m");

    build_usage_bar(cpu_bar, sizeof(cpu_bar), cpu_usage, 28);
    build_usage_bar(mem_bar, sizeof(mem_bar), memory->usage_percent, 28);

    clear_screen(ansi);

    printf("%s%sServer Health Monitor%s\n", bold, header_color, ansi_reset(ansi));
    printf("Server: %s\n", server);
    if (total_samples > 0) {
        printf("Sample: %d / %d\n", sample_index, total_samples);
    } else {
        printf("Elapsed: %.2fs\n", elapsed_ms / 1000.0);
    }
    printf("\n");

    printf("CPU Usage: %s%6.2f%%%s [%s] %s%s%s\n",
           cpu_color,
           cpu_usage,
           ansi_reset(ansi),
           cpu_bar,
           cpu_color,
           cpu_label,
           ansi_reset(ansi));
    printf("RAM Usage: %s%6.2f%%%s (%.2f GB / %.2f GB) [%s] %s%s%s\n",
           mem_color,
           memory->usage_percent,
           ansi_reset(ansi),
           memory->used_gb,
           memory->total_gb,
           mem_bar,
           mem_color,
           mem_label,
           ansi_reset(ansi));

    printf("\n");
    log_threshold_messages(cpu_usage, memory->usage_percent);

    if (remaining_ms >= 0) {
        printf("\nNext sample in: %.2fs  %c\n",
               remaining_ms / 1000.0,
               spinner_chars[sample_index % 4]);
    }
    printf("%sSampling every %d ms. Press Ctrl+C to stop early.%s\n",
           ansi_color(ansi, "\x1b[2m"),
           config->interval_ms,
           ansi_reset(ansi));
    fflush(stdout);
}

static MonitorStatus monitor_server_health(const MonitorConfig* config, bool live_output) {
    CpuTracker tracker = {0};
    MonitorStatus status = MONITOR_STATUS_OK;
    const bool ansi = live_output && supports_ansi_output();

    if (!config) {
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    if (config->iterations > 0) {
        for (int i = 0; i < config->iterations; i++) {
            if (live_output) {
                double cpu_usage = 0.0;
                MemoryUsage memory = {0};
                long long remaining_ms = (i + 1 < config->iterations) ? config->interval_ms : -1;
                status = collect_health_snapshot(&tracker, &cpu_usage, &memory);
                if (status != MONITOR_STATUS_OK) {
                    return status;
                }
                render_live_dashboard(config,
                                      config->server_name,
                                      cpu_usage,
                                      &memory,
                                      0,
                                      remaining_ms,
                                      i + 1,
                                      config->iterations,
                                      ansi);
            } else {
                status = log_health_status(config->server_name, &tracker);
            }
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

        if (live_output) {
            double cpu_usage = 0.0;
            MemoryUsage memory = {0};
            long long remaining_ms = config->duration_ms - elapsed;
            status = collect_health_snapshot(&tracker, &cpu_usage, &memory);
            if (status != MONITOR_STATUS_OK) {
                return status;
            }
            render_live_dashboard(config,
                                  config->server_name,
                                  cpu_usage,
                                  &memory,
                                  elapsed,
                                  remaining_ms,
                                  (int)(elapsed / config->interval_ms) + 1,
                                  0,
                                  ansi);
        } else {
            status = log_health_status(config->server_name, &tracker);
        }
        if (status != MONITOR_STATUS_OK) {
            return status;
        }

        sleep_ms(config->interval_ms);
    }

    if (live_output) {
        clear_screen(ansi);
        printf("Health monitoring completed for server: %s\n", config->server_name);
    } else {
        printf("Health monitoring completed for server: %s\n", config->server_name);
    }
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
        status = monitor_server_health(&config, false);
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
                status = monitor_server_health(&config, true);
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
