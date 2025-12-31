#include <iostream>
#include <thread>
#include <mutex>

// Global mutex for thread-safe printing
std::mutex print_mutex;

void printNumber(int i) {
    // Thread guard using lock_guard for RAII
    std::lock_guard<std::mutex> guard(print_mutex);
    std::cout << " " << i << std::endl;
}

void countFrom1to10() {
    for(int i = 1; i <= 10; ++i) {
        printNumber(i);
    }
}

int main() {
    // Create thread to count from 1 to 10
    std::thread t(countFrom1to10);
    
    // Main thread counts from 11 to 20
    for(int i = 11; i <= 20; ++i) {
        printNumber(i);
    }
    
    // Wait for the thread to finish
    t.join();
    
    return 0;
}
