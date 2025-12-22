#include "monitor_status.h"

const char* monitor_status_message(MonitorStatus status) {
    switch (status) {
        case MONITOR_STATUS_OK:
            return "OK";
        case MONITOR_STATUS_INVALID_ARGUMENT:
            return "invalid argument";
        case MONITOR_STATUS_PARSE_ERROR:
            return "parse error";
        case MONITOR_STATUS_RANGE_ERROR:
            return "value out of range";
        case MONITOR_STATUS_IO_ERROR:
            return "I/O error";
        case MONITOR_STATUS_UNSUPPORTED:
            return "unsupported";
        case MONITOR_STATUS_INTERNAL_ERROR:
            return "internal error";
        default:
            return "unknown error";
    }
}
