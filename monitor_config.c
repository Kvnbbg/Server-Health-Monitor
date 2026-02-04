#include "monitor_config.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static void set_error(char* error, size_t error_size, const char* message) {
    if (error != NULL && error_size > 0) {
        snprintf(error, error_size, "%s", message);
    }
}

static void set_errorf(char* error, size_t error_size, const char* format, ...) {
    va_list args;

    if (error == NULL || error_size == 0 || format == NULL) {
        return;
    }

    va_start(args, format);
    vsnprintf(error, error_size, format, args);
    va_end(args);
}

void monitor_config_init(MonitorConfig* config) {
    if (!config) {
        return;
    }

    memset(config, 0, sizeof(*config));
    snprintf(config->server_name, sizeof(config->server_name), "%s", "local");
    config->interval_ms = MONITOR_DEFAULT_INTERVAL_MS;
    config->duration_ms = MONITOR_DEFAULT_DURATION_MS;
    config->non_interactive = false;
    config->iterations = 0;
}

MonitorStatus parse_int_range(const char* value, int min, int max, int* out) {
    char* end = NULL;
    long parsed = 0;

    if (!value || !out) {
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    errno = 0;
    parsed = strtol(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0') {
        return MONITOR_STATUS_PARSE_ERROR;
    }

    if (parsed < min || parsed > max) {
        return MONITOR_STATUS_RANGE_ERROR;
    }

    *out = (int)parsed;
    return MONITOR_STATUS_OK;
}

MonitorStatus parse_bool(const char* value, bool* out) {
    if (!value || !out) {
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    if (strcmp(value, "1") == 0 || strcasecmp(value, "true") == 0 ||
        strcasecmp(value, "yes") == 0 || strcasecmp(value, "on") == 0) {
        *out = true;
        return MONITOR_STATUS_OK;
    }

    if (strcmp(value, "0") == 0 || strcasecmp(value, "false") == 0 ||
        strcasecmp(value, "no") == 0 || strcasecmp(value, "off") == 0) {
        *out = false;
        return MONITOR_STATUS_OK;
    }

    return MONITOR_STATUS_PARSE_ERROR;
}

MonitorStatus monitor_config_apply_env(MonitorConfig* config, char* error, size_t error_size) {
    const char* value = NULL;
    int parsed = 0;
    bool flag = false;
    MonitorStatus status = MONITOR_STATUS_OK;
    const int max_iterations = MONITOR_MAX_ITERATIONS;

    if (!config) {
        set_error(error, error_size, "config is null");
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    value = getenv("SHM_SERVER_NAME");
    if (value && *value != '\0') {
        snprintf(config->server_name, sizeof(config->server_name), "%s", value);
    }

    value = getenv("SHM_INTERVAL_MS");
    if (value) {
        status = parse_int_range(value, MONITOR_MIN_INTERVAL_MS, MONITOR_MAX_INTERVAL_MS, &parsed);
        if (status != MONITOR_STATUS_OK) {
            set_error(error, error_size, "invalid SHM_INTERVAL_MS");
            return status;
        }
        config->interval_ms = parsed;
    }

    value = getenv("SHM_DURATION_MS");
    if (value) {
        status = parse_int_range(value, MONITOR_MIN_DURATION_MS, MONITOR_MAX_DURATION_MS, &parsed);
        if (status != MONITOR_STATUS_OK) {
            set_error(error, error_size, "invalid SHM_DURATION_MS");
            return status;
        }
        config->duration_ms = parsed;
    }

    value = getenv("SHM_NON_INTERACTIVE");
    if (value) {
        status = parse_bool(value, &flag);
        if (status != MONITOR_STATUS_OK) {
            set_error(error, error_size, "invalid SHM_NON_INTERACTIVE");
            return status;
        }
        config->non_interactive = flag;
    }

    value = getenv("SHM_ITERATIONS");
    if (value) {
        status = parse_int_range(value, 1, max_iterations, &parsed);
        if (status != MONITOR_STATUS_OK) {
            set_error(error, error_size, "invalid SHM_ITERATIONS");
            return status;
        }
        config->iterations = parsed;
        config->non_interactive = true;
    }

    return MONITOR_STATUS_OK;
}

MonitorStatus monitor_config_apply_args(MonitorConfig* config, int argc, char** argv,
                                       bool* show_help, char* error, size_t error_size) {
    int i = 1;
    MonitorStatus status = MONITOR_STATUS_OK;
    int parsed = 0;
    const int max_iterations = MONITOR_MAX_ITERATIONS;

    if (!config || !argv || !show_help) {
        set_error(error, error_size, "invalid arguments");
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    *show_help = false;

    while (i < argc) {
        const char* arg = argv[i];
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            *show_help = true;
            return MONITOR_STATUS_OK;
        }
        if (strcmp(arg, "--non-interactive") == 0) {
            config->non_interactive = true;
            i++;
            continue;
        }
        if (strcmp(arg, "--server") == 0) {
            if (i + 1 >= argc) {
                set_error(error, error_size, "--server requires a value");
                return MONITOR_STATUS_INVALID_ARGUMENT;
            }
            if (argv[i + 1][0] == '\0') {
                set_error(error, error_size, "--server requires a non-empty value");
                return MONITOR_STATUS_INVALID_ARGUMENT;
            }
            snprintf(config->server_name, sizeof(config->server_name), "%s", argv[i + 1]);
            i += 2;
            continue;
        }
        if (strcmp(arg, "--interval-ms") == 0) {
            if (i + 1 >= argc) {
                set_error(error, error_size, "--interval-ms requires a value");
                return MONITOR_STATUS_INVALID_ARGUMENT;
            }
            status = parse_int_range(argv[i + 1], MONITOR_MIN_INTERVAL_MS, MONITOR_MAX_INTERVAL_MS, &parsed);
            if (status != MONITOR_STATUS_OK) {
                set_error(error, error_size, "invalid --interval-ms");
                return status;
            }
            config->interval_ms = parsed;
            i += 2;
            continue;
        }
        if (strcmp(arg, "--duration-ms") == 0) {
            if (i + 1 >= argc) {
                set_error(error, error_size, "--duration-ms requires a value");
                return MONITOR_STATUS_INVALID_ARGUMENT;
            }
            status = parse_int_range(argv[i + 1], MONITOR_MIN_DURATION_MS, MONITOR_MAX_DURATION_MS, &parsed);
            if (status != MONITOR_STATUS_OK) {
                set_error(error, error_size, "invalid --duration-ms");
                return status;
            }
            config->duration_ms = parsed;
            i += 2;
            continue;
        }
        if (strcmp(arg, "--iterations") == 0) {
            if (i + 1 >= argc) {
                set_error(error, error_size, "--iterations requires a value");
                return MONITOR_STATUS_INVALID_ARGUMENT;
            }
            status = parse_int_range(argv[i + 1], 1, max_iterations, &parsed);
            if (status != MONITOR_STATUS_OK) {
                set_error(error, error_size, "invalid --iterations");
                return status;
            }
            config->iterations = parsed;
            config->non_interactive = true;
            i += 2;
            continue;
        }

        set_errorf(error, error_size, "unknown argument: %s", arg);
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    return MONITOR_STATUS_OK;
}

MonitorStatus monitor_config_validate(const MonitorConfig* config, char* error, size_t error_size) {
    if (!config) {
        set_error(error, error_size, "config is null");
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    if (config->interval_ms < MONITOR_MIN_INTERVAL_MS || config->interval_ms > MONITOR_MAX_INTERVAL_MS) {
        set_error(error, error_size, "interval_ms out of range");
        return MONITOR_STATUS_RANGE_ERROR;
    }

    if (config->duration_ms < MONITOR_MIN_DURATION_MS || config->duration_ms > MONITOR_MAX_DURATION_MS) {
        set_error(error, error_size, "duration_ms out of range");
        return MONITOR_STATUS_RANGE_ERROR;
    }

    if (config->iterations < 0) {
        set_error(error, error_size, "iterations must be non-negative");
        return MONITOR_STATUS_RANGE_ERROR;
    }

    if (config->iterations > 0 && config->non_interactive == false) {
        set_error(error, error_size, "iterations requires non-interactive mode");
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    if (config->iterations == 0 && config->duration_ms < config->interval_ms) {
        set_error(error, error_size, "duration must be >= interval");
        return MONITOR_STATUS_RANGE_ERROR;
    }

    return MONITOR_STATUS_OK;
}

void monitor_config_print(const MonitorConfig* config) {
    if (!config) {
        return;
    }

    printf("Current configuration:\n");
    printf("  Server name:   %s\n", config->server_name);
    printf("  Interval (ms): %d\n", config->interval_ms);
    printf("  Duration (ms): %d\n", config->duration_ms);
    if (config->non_interactive) {
        printf("  Mode:          non-interactive\n");
    } else {
        printf("  Mode:          interactive\n");
    }
    if (config->iterations > 0) {
        printf("  Iterations:    %d\n", config->iterations);
    }
}
