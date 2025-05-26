#include "test_framework.h"

jmp_buf test_env;
int tests_passed = 0;
int tests_failed = 0;

void run_test_suite(TestCase* tests, int count) {
    printf("Starting test suite...\n");
    
    for (int i = 0; i < count; i++) {
        RUN_TEST(tests[i]);
    }
    
    printf("\nTest results:\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("  Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed > 0) {
        exit(EXIT_FAILURE);
    }
}
