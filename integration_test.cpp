#include "integration_test.hpp"
#include <iostream>

IntegrationTestRunner& IntegrationTestRunner::instance() {
    static IntegrationTestRunner instance;
    return instance;
}

void IntegrationTestRunner::add_test_suite(
    const std::string& name,
    std::function<void()> setup,
    std::function<void()> teardown,
    const std::vector<std::pair<std::string, std::function<bool()>>>& tests) {
    
    test_suites.push_back({name, setup, teardown, tests});
}

bool IntegrationTestRunner::run_all_tests() {
    int total_passed = 0;
    int total_failed = 0;
    
    std::cout << "Running integration tests...\n" << std::endl;
    
    for (const auto& suite : test_suites) {
        std::cout << "Test Suite: " << suite.name << std::endl;
        suite.setup();
        
        int suite_passed = 0;
        int suite_failed = 0;
        
        for (const auto& test : suite.tests) {
            std::cout << "  " << test.first << "... ";
            bool result = test.second();
            
            if (result) {
                std::cout << "PASSED" << std::endl;
                suite_passed++;
            } else {
                std::cout << "FAILED" << std::endl;
                suite_failed++;
            }
        }
        
        suite.teardown();
        
        std::cout << "\n  Suite results:" << std::endl;
        std::cout << "    Passed: " << suite_passed << std::endl;
        std::cout << "    Failed: " << suite_failed << std::endl << std::endl;
        
        total_passed += suite_passed;
        total_failed += suite_failed;
    }
    
    std::cout << "Integration test summary:" << std::endl;
    std::cout << "  Total passed: " << total_passed << std::endl;
    std::cout << "  Total failed: " << total_failed << std::endl;
    
    return total_failed == 0;
}
