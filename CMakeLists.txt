cmake_minimum_required(VERSION 3.10)
project(OS_Tests)

# Unit tests
add_executable(unit_tests 
    test_framework.c 
    mem_test.c 
    unit_tests.c)
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Kernel module tests
add_executable(kmod_tests 
    kmod_test.cpp 
    kmod_tests.cpp)
target_include_directories(kmod_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Integration tests
add_executable(integration_tests 
    integration_test.cpp 
    syscall_test.c 
    integration_tests.cpp)
target_include_directories(integration_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add all to test target
enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)
add_test(NAME kmod_tests COMMAND kmod_tests)
add_test(NAME integration_tests COMMAND integration_tests)
