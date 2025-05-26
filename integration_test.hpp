#ifndef OS_INTEGRATION_TEST_HPP
#define OS_INTEGRATION_TEST_HPP

#include <string>
#include <vector>
#include <functional>

class IntegrationTestRunner {
public:
    static IntegrationTestRunner& instance();
    
    void add_test_suite(const std::string& name, 
                      std::function<void()> setup,
                      std::function<void()> teardown,
                      const std::vector<std::pair<std::string, std::function<bool()>>>& tests);
    
    bool run_all_tests();

private:
    IntegrationTestRunner() = default;
    
    struct TestSuite {
        std::string name;
        std::function<void()> setup;
        std::function<void()> teardown;
        std::vector<std::pair<std::string, std::function<bool()>>> tests;
    };
    
    std::vector<TestSuite> test_suites;
};

#endif // OS_INTEGRATION_TEST_HPP
