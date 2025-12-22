#ifndef MONITOR_STATUS_H
#define MONITOR_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MONITOR_STATUS_OK = 0,
    MONITOR_STATUS_INVALID_ARGUMENT,
    MONITOR_STATUS_PARSE_ERROR,
    MONITOR_STATUS_RANGE_ERROR,
    MONITOR_STATUS_IO_ERROR,
    MONITOR_STATUS_UNSUPPORTED,
    MONITOR_STATUS_INTERNAL_ERROR
} MonitorStatus;

const char* monitor_status_message(MonitorStatus status);

#ifdef __cplusplus
}
#endif

#endif // MONITOR_STATUS_H
