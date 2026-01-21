#ifndef MONITOR_CONFIG_H
#define MONITOR_CONFIG_H

#include <stdbool.h>
#include <stddef.h>

#include "monitor_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MONITOR_DEFAULT_INTERVAL_MS 1000
#define MONITOR_DEFAULT_DURATION_MS 60000
#define MONITOR_MIN_INTERVAL_MS 100
#define MONITOR_MAX_INTERVAL_MS 3600000
#define MONITOR_MIN_DURATION_MS 1000
#define MONITOR_MAX_DURATION_MS 86400000
#define MONITOR_MAX_SERVER_NAME 64
#define MONITOR_MAX_ITERATIONS (MONITOR_MAX_DURATION_MS / MONITOR_MIN_INTERVAL_MS)

typedef struct {
    char server_name[MONITOR_MAX_SERVER_NAME];
    int interval_ms;
    int duration_ms;
    bool non_interactive;
    int iterations;
} MonitorConfig;

void monitor_config_init(MonitorConfig* config);
MonitorStatus parse_int_range(const char* value, int min, int max, int* out);
MonitorStatus parse_bool(const char* value, bool* out);
MonitorStatus monitor_config_apply_env(MonitorConfig* config, char* error, size_t error_size);
MonitorStatus monitor_config_apply_args(MonitorConfig* config, int argc, char** argv,
                                       bool* show_help, char* error, size_t error_size);
MonitorStatus monitor_config_validate(const MonitorConfig* config, char* error, size_t error_size);
void monitor_config_print(const MonitorConfig* config);

#ifdef __cplusplus
}
#endif

#endif // MONITOR_CONFIG_H
