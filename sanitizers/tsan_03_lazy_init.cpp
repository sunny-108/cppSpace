// ThreadSanitizer Example 3: Broken Lazy Initialization
// Compile: clang++ -fsanitize=thread -g -O1 tsan_03_lazy_init.cpp -o tsan_03
// Run: ./tsan_03

#include <iostream>
#include <thread>
#include <vector>

class Resource {
public:
    Resource() {
        std::cout << "Resource created" << std::endl;
    }
    void use() {
        std::cout << "Using resource" << std::endl;
    }
};

Resource* resource = nullptr;  // Shared pointer

void get_resource() {
    // BUG: Classic double-checked locking without proper synchronization
    if (resource == nullptr) {  // First check (RACE!)
        // Multiple threads can pass this check
        resource = new Resource();  // RACE: Multiple allocations possible!
    }
    resource->use();  // RACE: Use while another thread is creating
}

int main() {
    std::cout << "=== TSan Example 3: Broken Lazy Initialization ===" << std::endl;
    
    std::vector<std::thread> threads;
    
    // Multiple threads try to initialize
    for (int i = 0; i < 5; i++) {
        threads.emplace_back(get_resource);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    delete resource;
    return 0;
}

/* EXERCISE:
 * 1. Compile with TSan and run
 * 2. Observe data races on resource pointer
 * 3. May see multiple "Resource created" messages (memory leak!)
 * 4. Fix using std::call_once or mutex
 * 5. Verify single initialization and no races
 * 
 * CORRECT SOLUTIONS:
 * 
 * Option 1: std::call_once (BEST)
 * std::once_flag init_flag;
 * void get_resource() {
 *     std::call_once(init_flag, []() {
 *         resource = new Resource();
 *     });
 *     resource->use();
 * }
 * 
 * Option 2: Mutex
 * std::mutex mtx;
 * void get_resource() {
 *     std::lock_guard<std::mutex> lock(mtx);
 *     if (resource == nullptr) {
 *         resource = new Resource();
 *     }
 *     resource->use();
 * }
 * 
 * Option 3: Static local (thread-safe in C++11+)
 * Resource& get_resource() {
 *     static Resource instance;  // Thread-safe initialization
 *     return instance;
 * }
 */
