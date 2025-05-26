#include "test_framework.h"
#include "mem_test.h"

TEST_CASE(test_memory_allocation) {
    enable_memory_tracking();
    
    void* ptr = test_malloc(100);
    ASSERT(ptr != NULL);
    ASSERT(get_allocated_bytes() >= 100);
    
    test_free(ptr);
    check_for_leaks();
    
    disable_memory_tracking();
    return TEST_PASSED;
}

int main() {
    TestCase tests[] = {
        test_memory_allocation_test_case,
        // Add more tests here
    };
    
    run_test_suite(tests, sizeof(tests) / sizeof(TestCase));
    return 0;
}
