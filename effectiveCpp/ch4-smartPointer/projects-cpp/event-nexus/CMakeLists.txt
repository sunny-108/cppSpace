cmake_minimum_required(VERSION 3.20)
project(event-nexus
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Thread-safe async event bus with smart pointer ownership semantics"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ─── Compiler warnings ───────────────────────────────────────────────
add_compile_options(
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused
    -Woverloaded-virtual -Wformat=2 -Wnull-dereference
)

# ─── Sanitizer options (opt-in) ──────────────────────────────────────
option(ENABLE_ASAN   "Enable AddressSanitizer"  OFF)
option(ENABLE_TSAN   "Enable ThreadSanitizer"   OFF)
option(ENABLE_UBSAN  "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

if(ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

# ─── Core library ────────────────────────────────────────────────────
add_library(event_nexus_core
    src/core/thread_pool.cpp
    src/core/event_bus.cpp
    src/core/event_dispatcher.cpp
)

target_include_directories(event_nexus_core
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(event_nexus_core
    PRIVATE pthread
)

# ─── Main demo executable ────────────────────────────────────────────
add_executable(event_nexus_demo src/main.cpp)
target_link_libraries(event_nexus_demo PRIVATE event_nexus_core)

# ─── Tests ────────────────────────────────────────────────────────────
option(BUILD_TESTS "Build unit and stress tests" ON)

if(BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    FetchContent_MakeAvailable(googletest)

    # Unit tests
    add_executable(unit_tests
        tests/unit/thread_pool_test.cpp
        tests/unit/event_bus_test.cpp
        tests/unit/event_dispatcher_test.cpp
    )
    target_link_libraries(unit_tests PRIVATE event_nexus_core GTest::gtest_main)
    target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    include(GoogleTest)
    gtest_discover_tests(unit_tests)

    # Stress tests
    add_executable(stress_tests
        tests/stress/concurrent_pub_sub_test.cpp
    )
    target_link_libraries(stress_tests PRIVATE event_nexus_core GTest::gtest_main)
    target_include_directories(stress_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    gtest_discover_tests(stress_tests)
endif()

# ─── Benchmarks ──────────────────────────────────────────────────────
option(BUILD_BENCHMARKS "Build Google Benchmark targets" OFF)

if(BUILD_BENCHMARKS)
    find_package(benchmark REQUIRED)

    add_executable(bench_event_bus benchmarks/bench_main.cpp)
    target_link_libraries(bench_event_bus PRIVATE event_nexus_core benchmark::benchmark)
endif()
