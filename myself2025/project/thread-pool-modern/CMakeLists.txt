cmake_minimum_required(VERSION 3.14)
project(ThreadPoolModern LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Thread library
find_package(Threads REQUIRED)

add_executable(thread_pool_demo
    src/main.cpp
    src/Command.cpp
    src/Thread.cpp
    src/ThreadPoolMgr.cpp
)

target_include_directories(thread_pool_demo PRIVATE include)
target_link_libraries(thread_pool_demo PRIVATE Threads::Threads)
