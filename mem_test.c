#include "mem_test.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static size_t allocated_bytes = 0;
static int tracking_enabled = 0;

void* test_malloc(size_t size) {
    size_t total_size = size + sizeof(size_t);
    unsigned char* raw = malloc(total_size);
    if (!raw) {
        return NULL;
    }

    memcpy(raw, &size, sizeof(size_t));
    if (tracking_enabled) {
        allocated_bytes += size;
    }
    return raw + sizeof(size_t);
}

void test_free(void* ptr) {
    if (!ptr) {
        return;
    }

    unsigned char* raw = (unsigned char*)ptr - sizeof(size_t);
    size_t size = 0;
    memcpy(&size, raw, sizeof(size_t));
    if (tracking_enabled) {
        if (allocated_bytes >= size) {
            allocated_bytes -= size;
        } else {
            allocated_bytes = 0;
        }
    }
    free(raw);
}

void enable_memory_tracking() {
    tracking_enabled = 1;
}

void disable_memory_tracking() {
    tracking_enabled = 0;
}

size_t get_allocated_bytes() {
    return allocated_bytes;
}

void check_for_leaks() {
    if (allocated_bytes > 0) {
        fprintf(stderr, "Memory leak detected: %zu bytes not freed\n", allocated_bytes);
        exit(EXIT_FAILURE);
    }
}
