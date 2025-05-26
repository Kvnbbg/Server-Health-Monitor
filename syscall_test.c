#include "syscall_test.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_SYSCALL_TESTS 256
static SyscallTest syscall_tests[MAX_SYSCALL_TESTS];
static int num_tests = 0;

void register_syscall_test(uint32_t syscall_num, const char* name, int (*test_function)()) {
    if (num_tests >= MAX_SYSCALL_TESTS) {
        fprintf(stderr, "Too many syscall tests registered\n");
        exit(EXIT_FAILURE);
    }
    
    syscall_tests[num_tests] = (SyscallTest){
        .syscall_num = syscall_num,
        .name = name,
        .test_function = test_function
    };
    num_tests++;
}

int run_syscall_tests() {
    int passed = 0;
    int failed = 0;
    
    printf("Running syscall tests...\n");
    
    for (int i = 0; i < num_tests; i++) {
        SyscallTest test = syscall_tests[i];
        printf("  %s (syscall %u)... ", test.name, test.syscall_num);
        
        int result = test.test_function();
        if (result == 0) {
            printf("PASSED\n");
            passed++;
        } else {
            printf("FAILED\n");
            failed++;
        }
    }
    
    printf("\nSyscall test results:\n");
    printf("  Passed: %d\n", passed);
    printf("  Failed: %d\n", failed);
    
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
