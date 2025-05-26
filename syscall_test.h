#ifndef OS_SYSCALL_TEST_H
#define OS_SYSCALL_TEST_H

#include <stdint.h>

typedef struct {
    uint32_t syscall_num;
    const char* name;
    int (*test_function)();
} SyscallTest;

void register_syscall_test(uint32_t syscall_num, const char* name, int (*test_function)());
int run_syscall_tests();

#endif // OS_SYSCALL_TEST_H
