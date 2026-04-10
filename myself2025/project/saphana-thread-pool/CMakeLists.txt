cmake_minimum_required(VERSION 3.14)
project(SlidingWindowThreadPool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

add_executable(sliding_pool_demo
    src/main.cpp
    src/OperationThread.cpp
    src/Operation.cpp
    src/BackupOperation.cpp
    src/RestoreOperation.cpp
)

target_include_directories(sliding_pool_demo PRIVATE include)
target_link_libraries(sliding_pool_demo PRIVATE Threads::Threads)
