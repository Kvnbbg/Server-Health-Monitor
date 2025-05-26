#ifndef OS_MEM_TEST_H
#define OS_MEM_TEST_H

#include <stddef.h>

void* test_malloc(size_t size);
void test_free(void* ptr);
void enable_memory_tracking();
void disable_memory_tracking();
size_t get_allocated_bytes();
void check_for_leaks();

#endif // OS_MEM_TEST_H
