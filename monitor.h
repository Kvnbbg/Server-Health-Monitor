#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>

#include "monitor_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned long long prev_idle;
    unsigned long long prev_total;
    bool has_prev;
} CpuTracker;

typedef struct {
    double used_gb;
    double total_gb;
    double usage_percent;
} MemoryUsage;

MonitorStatus monitor_read_cpu_usage(CpuTracker* tracker, double* out_percent);
MonitorStatus monitor_read_memory_usage(MemoryUsage* usage);

#ifdef __cplusplus
}
#endif

#endif // MONITOR_H
