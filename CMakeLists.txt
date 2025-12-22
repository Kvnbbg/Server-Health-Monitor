cmake_minimum_required(VERSION 3.16)
project(ServerHealthMonitor LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_WERROR "Treat warnings as errors" ON)
option(ENABLE_SANITIZERS "Enable Address/Undefined sanitizers" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck static analysis" OFF)

if (ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if (CPPCHECK)
        set(CMAKE_C_CPPCHECK "${CPPCHECK};--enable=warning,style,performance,portability;--inline-suppr;--error-exitcode=1")
        set(CMAKE_CXX_CPPCHECK "${CMAKE_C_CPPCHECK}")
    else ()
        message(WARNING "cppcheck requested but not found")
    endif ()
endif ()

if (MSVC)
    add_compile_options(/W4)
    if (ENABLE_WERROR)
        add_compile_options(/WX)
    endif ()
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion -Wsign-conversion)
    if (ENABLE_WERROR)
        add_compile_options(-Werror)
    endif ()
    add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=2 -fno-omit-frame-pointer)
    add_link_options(-Wl,-z,relro,-z,now)

    if (ENABLE_SANITIZERS)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif ()
endif ()

add_library(server_monitor_lib
    monitor.c
    monitor_config.c
    monitor_status.c)

target_include_directories(server_monitor_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(server_monitor server_monitor.c)

target_link_libraries(server_monitor PRIVATE server_monitor_lib)

add_executable(server_monitor_tests
    server_monitor_tests.c
    test_framework.c)

target_link_libraries(server_monitor_tests PRIVATE server_monitor_lib)

add_executable(example_unit_tests
    example-unit-test.c
    mem_test.c
    test_framework.c)

target_include_directories(example_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_test(NAME server_monitor_tests COMMAND server_monitor_tests)
add_test(NAME example_unit_tests COMMAND example_unit_tests)
