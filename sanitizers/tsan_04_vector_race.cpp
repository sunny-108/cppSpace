// ThreadSanitizer Example 4: Container Data Race
// Compile: clang++ -fsanitize=thread -g -O1 tsan_04_vector_race.cpp -o tsan_04
// Run: ./tsan_04

#include <iostream>
#include <thread>
#include <vector>

std::vector<int> shared_vec;  // BUG: Shared without protection

void writer() {
    for (int i = 0; i < 1000; i++) {
        // BUG: push_back modifies internal state (not thread-safe)
        shared_vec.push_back(i);
    }
}

void reader() {
    for (int i = 0; i < 1000; i++) {
        // BUG: Reading size/elements while writer modifies
        if (!shared_vec.empty()) {
            int val = shared_vec.back();
            (void)val;  // Use the value
        }
    }
}

int main() {
    std::cout << "=== TSan Example 4: Container Data Race ===" << std::endl;
    
    std::thread t1(writer);
    std::thread t2(writer);
    std::thread t3(reader);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "Final vector size: " << shared_vec.size() << std::endl;
    std::cout << "Expected: 2000 (but may crash or have wrong size)" << std::endl;
    
    return 0;
}

/* EXERCISE:
 * 1. Compile with TSan and run
 * 2. Observe multiple data race warnings
 * 3. Program may crash or have incorrect size
 * 4. Fix by protecting all vector access with mutex
 * 5. Verify no races and correct size (2000)
 * 
 * WHY DANGEROUS:
 * - std::vector is NOT thread-safe for concurrent modifications
 * - push_back can reallocate internal buffer
 * - Readers can access invalid memory
 * - Multiple writers cause corruption
 * 
 * FIX:
 * std::vector<int> shared_vec;
 * std::mutex vec_mutex;
 * 
 * void writer() {
 *     for (int i = 0; i < 1000; i++) {
 *         std::lock_guard<std::mutex> lock(vec_mutex);
 *         shared_vec.push_back(i);
 *     }
 * }
 * 
 * void reader() {
 *     for (int i = 0; i < 1000; i++) {
 *         std::lock_guard<std::mutex> lock(vec_mutex);
 *         if (!shared_vec.empty()) {
 *             int val = shared_vec.back();
 *         }
 *     }
 * }
 * 
 * KEY LESSON: Standard containers are NOT thread-safe!
 */
