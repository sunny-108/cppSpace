// ThreadSanitizer Example 2: Shared Counter Race
// Compile: clang++ -fsanitize=thread -g -O1 tsan_02_shared_counter.cpp -o tsan_02
// Run: ./tsan_02

#include <iostream>
#include <thread>
#include <vector>

class Counter {
public:
    void increment() {
        // BUG: count is not protected
        count++;
    }
    
    int get() const {
        return count;  // BUG: Reading without synchronization
    }
    
private:
    int count = 0;  // Shared state
};

int main() {
    std::cout << "=== TSan Example 2: Shared Counter Race ===" << std::endl;
    
    Counter counter;
    std::vector<std::thread> threads;
    
    // Create 10 threads, each incrementing 10000 times
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 10000; j++) {
                counter.increment();
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final count: " << counter.get() << std::endl;
    std::cout << "Expected: 100000" << std::endl;
    
    return 0;
}

/* EXERCISE:
 * 1. Compile with TSan and run
 * 2. Observe multiple data race warnings
 * 3. Notice incorrect final count
 * 4. Fix by adding mutex to Counter class
 * 5. Verify correct result and no races
 * 
 * FIX APPROACH:
 * class Counter {
 * public:
 *     void increment() {
 *         std::lock_guard<std::mutex> lock(mtx);
 *         count++;
 *     }
 *     
 *     int get() const {
 *         std::lock_guard<std::mutex> lock(mtx);
 *         return count;
 *     }
 *     
 * private:
 *     int count = 0;
 *     mutable std::mutex mtx;
 * };
 * 
 * OR use std::atomic<int> count{0};
 */
