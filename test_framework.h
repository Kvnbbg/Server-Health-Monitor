#ifndef OS_TEST_FRAMEWORK_H
#define OS_TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#define TEST_PASSED 0
#define TEST_FAILED 1

typedef struct {
    const char* name;
    int (*test_func)();
} TestCase;

extern jmp_buf test_env;
extern int tests_passed;
extern int tests_failed;

#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printf("Assertion failed: %s, file %s, line %d\n", #cond, __FILE__, __LINE__); \
            longjmp(test_env, TEST_FAILED); \
        } \
    } while (0)

#define TEST_CASE(name) \
    int name##_test(); \
    TestCase name##_test_case = {#name, name##_test}; \
    int name##_test()

#define RUN_TEST(test) \
    do { \
        printf("Running test: %s\n", test.name); \
        if (setjmp(test_env) { \
            tests_failed++; \
        } else { \
            test.test_func(); \
            tests_passed++; \
            printf("  PASSED\n"); \
        } \
    } while (0)

void run_test_suite(TestCase* tests, int count);

#endif // OS_TEST_FRAMEWORK_H
