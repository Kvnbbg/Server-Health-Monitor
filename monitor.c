#include "monitor.h"

#include <stdio.h>
#include <string.h>

enum {
    CPU_FIELD_COUNT = 10
};

static const double KILOBYTES_PER_GIGABYTE = 1024.0 * 1024.0;
static const double MAX_USAGE_PERCENT = 100.0;

static MonitorStatus read_cpu_fields(unsigned long long* fields, size_t count) {
    FILE* file = fopen("/proc/stat", "r");
    if (!file) {
        return MONITOR_STATUS_IO_ERROR;
    }

    int scanned = fscanf(file, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                         &fields[0], &fields[1], &fields[2], &fields[3], &fields[4],
                         &fields[5], &fields[6], &fields[7], &fields[8], &fields[9]);
    fclose(file);

    if (scanned < 4) {
        return MONITOR_STATUS_PARSE_ERROR;
    }

    for (int i = scanned; i < (int)count; i++) {
        fields[i] = 0ULL;
    }

    return MONITOR_STATUS_OK;
}

/**
 * Reads CPU usage as a percentage based on deltas from the previous sample.
 *
 * @param tracker CPU tracker storing the previous totals.
 * @param out_percent Receives the calculated CPU usage percentage.
 * @return MonitorStatus indicating success or error state.
 */
MonitorStatus monitor_read_cpu_usage(CpuTracker* tracker, double* out_percent) {
    unsigned long long fields[CPU_FIELD_COUNT] = {0};
    unsigned long long total = 0ULL;
    unsigned long long idle = 0ULL;
    MonitorStatus status = MONITOR_STATUS_OK;

    if (!tracker || !out_percent) {
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    status = read_cpu_fields(fields, CPU_FIELD_COUNT);
    if (status != MONITOR_STATUS_OK) {
        return status;
    }

    for (size_t i = 0; i < CPU_FIELD_COUNT; i++) {
        total += fields[i];
    }
    idle = fields[3] + fields[4];

    if (!tracker->has_prev) {
        tracker->prev_total = total;
        tracker->prev_idle = idle;
        tracker->has_prev = true;
        *out_percent = 0.0;
        return MONITOR_STATUS_OK;
    }

    unsigned long long total_delta = total - tracker->prev_total;
    unsigned long long idle_delta = idle - tracker->prev_idle;

    tracker->prev_total = total;
    tracker->prev_idle = idle;

    if (total_delta == 0) {
        *out_percent = 0.0;
        return MONITOR_STATUS_OK;
    }

    *out_percent = (double)(total_delta - idle_delta) * MAX_USAGE_PERCENT / (double)total_delta;
    if (*out_percent < 0.0) {
        *out_percent = 0.0;
    }
    if (*out_percent > MAX_USAGE_PERCENT) {
        *out_percent = MAX_USAGE_PERCENT;
    }

    return MONITOR_STATUS_OK;
}

/**
 * Reads system memory usage from /proc/meminfo.
 *
 * @param usage Receives total, used, and percentage values in gigabytes.
 * @return MonitorStatus indicating success or error state.
 */
MonitorStatus monitor_read_memory_usage(MemoryUsage* usage) {
    FILE* file = NULL;
    char label[64] = {0};
    unsigned long long value_kb = 0ULL;
    unsigned long long total_kb = 0ULL;
    unsigned long long available_kb = 0ULL;

    if (!usage) {
        return MONITOR_STATUS_INVALID_ARGUMENT;
    }

    file = fopen("/proc/meminfo", "r");
    if (!file) {
        return MONITOR_STATUS_IO_ERROR;
    }

    while (fscanf(file, "%63s %llu kB", label, &value_kb) == 2) {
        if (strcmp(label, "MemTotal:") == 0) {
            total_kb = value_kb;
        } else if (strcmp(label, "MemAvailable:") == 0) {
            available_kb = value_kb;
        }

        if (total_kb > 0 && available_kb > 0) {
            break;
        }
    }

    fclose(file);

    if (total_kb == 0 || available_kb == 0 || available_kb > total_kb) {
        return MONITOR_STATUS_PARSE_ERROR;
    }

    double total_gb = (double)total_kb / KILOBYTES_PER_GIGABYTE;
    double available_gb = (double)available_kb / KILOBYTES_PER_GIGABYTE;
    if (total_gb <= 0.0) {
        return MONITOR_STATUS_PARSE_ERROR;
    }
    double used_gb = total_gb - available_gb;
    double usage_percent = (used_gb / total_gb) * MAX_USAGE_PERCENT;

    usage->total_gb = total_gb;
    usage->used_gb = used_gb;
    usage->usage_percent = usage_percent;

    return MONITOR_STATUS_OK;
}
