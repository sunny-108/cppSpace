// ThreadSanitizer Example 1: Basic Data Race
// Compile: clang++ -fsanitize=thread -g -O1 tsan_01_basic_race.cpp -o tsan_01
// Run: ./tsan_01

#include <iostream>
#include <thread>

int shared_counter = 0;  // BUG: Shared without synchronization

void increment() {
    for (int i = 0; i < 100000; i++) {
        // BUG: Data race - multiple threads accessing without protection
        shared_counter++;
    }
}

int main() {
    std::cout << "=== TSan Example 1: Basic Data Race ===" << std::endl;
    std::cout << "Initial counter: " << shared_counter << std::endl;
    
    std::thread t1(increment);
    std::thread t2(increment);
    
    t1.join();
    t2.join();
    
    std::cout << "Final counter: " << shared_counter << std::endl;
    std::cout << "Expected: 200000" << std::endl;
    
    return 0;
}

/* EXERCISE:
 * 1. Compile with TSan: clang++ -fsanitize=thread -g -O1 tsan_01_basic_race.cpp -o tsan_01
 * 2. Run: ./tsan_01
 * 3. Observe the data race warning
 * 4. Notice: Final counter is probably NOT 200000 (lost updates)
 * 5. Fix the bug using:
 *    - std::mutex and std::lock_guard
 *    - std::atomic<int>
 * 6. Verify no race and correct result
 * 
 * WHY IT'S A PROBLEM:
 * - Counter++ is NOT atomic (read-modify-write)
 * - Two threads can read same value
 * - Both increment and write back
 * - One update is lost
 * 
 * FIXES:
 * Option 1: std::atomic<int> shared_counter{0};
 * Option 2: Protect with mutex in increment()
 */
