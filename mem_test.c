#include "mem_test.h"
#include <stdlib.h>
#include <stdio.h>

static size_t allocated_bytes = 0;
static int tracking_enabled = 0;

void* test_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr && tracking_enabled) {
        allocated_bytes += size;
    }
    return ptr;
}

void test_free(void* ptr) {
    if (ptr && tracking_enabled) {
        // Note: Actual size would need more sophisticated tracking
        allocated_bytes -= sizeof(ptr); // Simplified example
    }
    free(ptr);
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
