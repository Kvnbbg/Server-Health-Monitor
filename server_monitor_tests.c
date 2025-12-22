#include "monitor_config.h"
#include "test_framework.h"

TEST_CASE(parse_int_range_accepts_valid) {
    int value = 0;
    ASSERT(parse_int_range("1500", 1, 2000, &value) == MONITOR_STATUS_OK);
    ASSERT(value == 1500);
    return TEST_PASSED;
}

TEST_CASE(parse_int_range_rejects_partial) {
    int value = 0;
    ASSERT(parse_int_range("42ms", 1, 100, &value) == MONITOR_STATUS_PARSE_ERROR);
    return TEST_PASSED;
}

TEST_CASE(parse_int_range_rejects_out_of_range) {
    int value = 0;
    ASSERT(parse_int_range("0", 1, 100, &value) == MONITOR_STATUS_RANGE_ERROR);
    return TEST_PASSED;
}

TEST_CASE(config_validation_enforces_duration) {
    MonitorConfig config;
    char error[128] = {0};

    monitor_config_init(&config);
    config.interval_ms = 1000;
    config.duration_ms = 500;

    ASSERT(monitor_config_validate(&config, error, sizeof(error)) == MONITOR_STATUS_RANGE_ERROR);
    return TEST_PASSED;
}

int main(void) {
    TestCase tests[] = {
        parse_int_range_accepts_valid_test_case,
        parse_int_range_rejects_partial_test_case,
        parse_int_range_rejects_out_of_range_test_case,
        config_validation_enforces_duration_test_case,
    };

    run_test_suite(tests, sizeof(tests) / sizeof(TestCase));
    return 0;
}
