#ifndef OS_KMOD_TEST_HPP
#define OS_KMOD_TEST_HPP

#include <vector>
#include <functional>
#include <string>

class KernelModuleTester {
public:
    void register_test(const std::string& name, std::function<bool()> test);
    bool run_all_tests();
    
private:
    struct TestCase {
        std::string name;
        std::function<bool()> test_func;
    };
    
    std::vector<TestCase> tests;
};

#define KMOD_TEST(name) \
bool name##_test(); \
static bool name##_registered = []() { \
    KernelModuleTester::instance().register_test(#name, name##_test); \
    return true; \
}(); \
bool name##_test()

#endif // OS_KMOD_TEST_HPP
