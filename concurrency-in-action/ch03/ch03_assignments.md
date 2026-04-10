# Chapter 3: Sharing Data Between Threads - Programming Assignments

## Overview
These assignments progress from medium to advanced difficulty, focusing on real-world scenarios involving shared data protection, deadlock prevention, and thread-safe data structures.

---

## Assignment 1: Thread-Safe Bank Account System (Medium)

### Objective
Implement a thread-safe bank account system that handles concurrent deposits, withdrawals, and transfers.

### Requirements

1. **BankAccount Class:**
   - Private members: `account_id`, `balance`, `mutex`
   - Methods: `deposit()`, `withdraw()`, `get_balance()`, `transfer_to()`
   - Must handle insufficient funds gracefully
   - All operations must be thread-safe

2. **Features to Implement:**
   - Concurrent deposits from multiple threads
   - Concurrent withdrawals from multiple threads
   - Money transfers between accounts (avoid deadlock!)
   - Transaction history logging (thread-safe)

3. **Constraints:**
   - No negative balances allowed
   - Transfers must be atomic (both accounts updated or neither)
   - Must use appropriate synchronization primitives
   - Handle at least 1000 concurrent transactions

### Test Scenarios
```cpp
// Scenario 1: Multiple deposits
// 10 threads, each deposits $100, 100 times
// Expected final balance: $100,000

// Scenario 2: Concurrent deposits and withdrawals
// 5 threads deposit $50, 5 threads withdraw $50
// Expected: Balance remains stable

// Scenario 3: Circular transfers
// Account A -> B, B -> C, C -> A
// Must not deadlock, all transfers succeed
```

### Starter Code
```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <random>

class BankAccount {
private:
    int account_id;
    double balance;
    mutable std::mutex mtx;
    
public:
    BankAccount(int id, double initial_balance)
        : account_id(id), balance(initial_balance) {}
    
    // TODO: Implement deposit
    void deposit(double amount) {
        // Your code here
    }
    
    // TODO: Implement withdraw (return false if insufficient funds)
    bool withdraw(double amount) {
        // Your code here
    }
    
    // TODO: Implement get_balance
    double get_balance() const {
        // Your code here
    }
    
    // TODO: Implement transfer_to (must be deadlock-free!)
    bool transfer_to(BankAccount& other, double amount) {
        // Your code here
        // Hint: Use std::lock or std::scoped_lock
    }
    
    int get_id() const { return account_id; }
};

// TODO: Implement test scenarios
int main() {
    // Your test code here
    return 0;
}
```

### Expected Output
```
Initial balances:
Account 1: $1000.00
Account 2: $1000.00
Account 3: $1000.00

Running 10000 concurrent operations...

Final balances:
Account 1: $1000.00
Account 2: $1000.00
Account 3: $1000.00

All transactions completed successfully!
No deadlocks detected.
```

---

## Assignment 2: Thread-Safe Cache with Reader-Writer Lock (Medium-Advanced)

### Objective
Implement a high-performance thread-safe cache that allows concurrent reads but exclusive writes, using `std::shared_mutex`.

### Requirements

1. **LRU Cache Implementation:**
   - Fixed capacity (e.g., 100 items)
   - Key-value storage (string -> string)
   - Least Recently Used eviction policy
   - Thread-safe for concurrent access

2. **Operations:**
   - `get(key)`: Retrieve value (multiple readers allowed)
   - `put(key, value)`: Insert/update value (exclusive access)
   - `remove(key)`: Delete entry (exclusive access)
   - `size()`: Get current cache size (read operation)
   - `clear()`: Empty the cache (exclusive access)

3. **Performance Requirements:**
   - Multiple threads can read simultaneously
   - Write operations block all other operations
   - Implement proper LRU tracking (access updates order)
   - Measure and report cache hit/miss ratio

### Real-World Scenario
Simulate a web server cache storing HTML pages:
- 10 reader threads (simulate page requests)
- 2 writer threads (simulate content updates)
- Run for 10 seconds
- Report performance metrics

### Starter Code
```cpp
#include <iostream>
#include <shared_mutex>
#include <unordered_map>
#include <list>
#include <thread>
#include <chrono>
#include <optional>

template<typename Key, typename Value>
class ThreadSafeCache {
private:
    size_t capacity;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> cache_map;
    std::list<std::pair<Key, Value>> lru_list;
    mutable std::shared_mutex mtx;
    
    // Statistics
    std::atomic<size_t> hits{0};
    std::atomic<size_t> misses{0};
    
public:
    explicit ThreadSafeCache(size_t cap) : capacity(cap) {}
    
    // TODO: Implement get with shared lock
    std::optional<Value> get(const Key& key) {
        // Your code here
        // Use std::shared_lock for read access
    }
    
    // TODO: Implement put with unique lock
    void put(const Key& key, const Value& value) {
        // Your code here
        // Use std::unique_lock for write access
        // Implement LRU eviction
    }
    
    // TODO: Implement remove
    bool remove(const Key& key) {
        // Your code here
    }
    
    // TODO: Implement size
    size_t size() const {
        // Your code here
    }
    
    // TODO: Implement clear
    void clear() {
        // Your code here
    }
    
    void print_statistics() const {
        size_t total = hits + misses;
        double hit_rate = total > 0 ? (100.0 * hits / total) : 0.0;
        std::cout << "Cache Statistics:\n";
        std::cout << "  Hits: " << hits << "\n";
        std::cout << "  Misses: " << misses << "\n";
        std::cout << "  Hit Rate: " << hit_rate << "%\n";
    }
};

// TODO: Implement test with concurrent readers and writers
int main() {
    // Your test code here
    return 0;
}
```

### Expected Behavior
- Cache hit rate > 70% with proper LRU implementation
- No data corruption from concurrent access
- Readers don't block each other
- Writers block all other operations

---

## Assignment 3: Deadlock Detection and Recovery System (Advanced)

### Objective
Implement a sophisticated resource allocation system with deadlock detection and recovery mechanisms.

### Requirements

1. **Resource Manager:**
   - Manages multiple shared resources (e.g., printers, files, network connections)
   - Multiple threads request multiple resources
   - Implements timeout-based deadlock detection
   - Provides deadlock recovery strategies

2. **Features:**
   - Resource request with timeout
   - Deadlock detection algorithm (e.g., wait-for graph or banker's algorithm)
   - Automatic deadlock recovery (resource preemption or thread rollback)
   - Logging of deadlock events

3. **Implementation Strategies:**
   - Use `std::timed_mutex` for timeout-based detection
   - Implement resource ordering to prevent deadlocks
   - Provide both prevention and detection mechanisms

### Real-World Scenario
Simulate a database transaction system:
- Multiple transactions accessing multiple database tables
- Each transaction needs locks on 2-3 tables
- Some transactions intentionally create potential deadlock situations
- System must detect and resolve deadlocks automatically

### Starter Code
```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <map>
#include <set>

enum class ResourceType {
    DATABASE_TABLE_1,
    DATABASE_TABLE_2,
    DATABASE_TABLE_3,
    DATABASE_TABLE_4,
    DATABASE_TABLE_5
};

class ResourceManager {
private:
    std::map<ResourceType, std::timed_mutex> resources;
    std::mutex stats_mutex;
    
    // Statistics
    int successful_acquisitions = 0;
    int failed_acquisitions = 0;
    int deadlocks_detected = 0;
    int deadlocks_recovered = 0;
    
public:
    ResourceManager() {
        // Initialize resources
        resources[ResourceType::DATABASE_TABLE_1];
        resources[ResourceType::DATABASE_TABLE_2];
        resources[ResourceType::DATABASE_TABLE_3];
        resources[ResourceType::DATABASE_TABLE_4];
        resources[ResourceType::DATABASE_TABLE_5];
    }
    
    // TODO: Implement acquire_resources with timeout
    bool acquire_resources(const std::vector<ResourceType>& needed_resources,
                          std::chrono::milliseconds timeout) {
        // Your code here
        // Try to acquire all resources with timeout
        // If timeout occurs, release all acquired resources and report deadlock
    }
    
    // TODO: Implement release_resources
    void release_resources(const std::vector<ResourceType>& resources) {
        // Your code here
    }
    
    // TODO: Implement deadlock recovery strategy
    void handle_deadlock(int thread_id, const std::vector<ResourceType>& resources) {
        // Your code here
        // Log deadlock, implement recovery (e.g., backoff and retry)
    }
    
    void print_statistics() {
        std::lock_guard<std::mutex> lock(stats_mutex);
        std::cout << "\n=== Resource Manager Statistics ===\n";
        std::cout << "Successful acquisitions: " << successful_acquisitions << "\n";
        std::cout << "Failed acquisitions: " << failed_acquisitions << "\n";
        std::cout << "Deadlocks detected: " << deadlocks_detected << "\n";
        std::cout << "Deadlocks recovered: " << deadlocks_recovered << "\n";
    }
};

// TODO: Implement transaction simulation
void transaction_worker(ResourceManager& manager, int thread_id,
                       const std::vector<ResourceType>& resources) {
    // Your code here
}

int main() {
    // TODO: Create scenarios that can cause deadlocks
    // TODO: Verify that deadlocks are detected and recovered
    return 0;
}
```

### Expected Output
```
Transaction 1: Requesting [TABLE_1, TABLE_2]
Transaction 2: Requesting [TABLE_2, TABLE_3]
Transaction 3: Requesting [TABLE_3, TABLE_1]

DEADLOCK DETECTED: Transaction 3 timed out waiting for TABLE_1
RECOVERY: Transaction 3 releasing all resources and retrying...

Transaction 3: Retry successful

=== Resource Manager Statistics ===
Successful acquisitions: 997
Failed acquisitions: 3
Deadlocks detected: 3
Deadlocks recovered: 3
```

---

## Assignment 4: Thread-Safe Queue with Condition Variables (Advanced)

### Objective
Implement a producer-consumer queue using condition variables for efficient thread synchronization.

### Requirements

1. **Blocking Queue Implementation:**
   - Fixed capacity thread-safe queue
   - Producers block when queue is full
   - Consumers block when queue is empty
   - Use `std::condition_variable` for efficient waiting

2. **Operations:**
   - `push(item)`: Add item (blocks if full)
   - `try_push(item, timeout)`: Try to add with timeout
   - `pop()`: Remove item (blocks if empty)
   - `try_pop(timeout)`: Try to remove with timeout
   - Support for shutdown/cancellation

3. **Advanced Features:**
   - Multiple producers and consumers
   - Graceful shutdown mechanism
   - Priority queue variant (optional)
   - Performance metrics (throughput, wait times)

### Real-World Scenario
Simulate a task processing system:
- 3 producer threads generating tasks at variable rates
- 5 consumer threads processing tasks (variable processing time)
- Queue capacity: 10 items
- Run for 30 seconds, then graceful shutdown
- Measure: tasks produced, tasks consumed, average wait times

### Starter Code
```cpp
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>

template<typename T>
class BlockingQueue {
private:
    std::queue<T> queue;
    size_t capacity;
    mutable std::mutex mtx;
    std::condition_variable cv_full;   // Notified when queue is not full
    std::condition_variable cv_empty;  // Notified when queue is not empty
    bool shutdown_flag = false;
    
    // Statistics
    std::atomic<size_t> total_pushed{0};
    std::atomic<size_t> total_popped{0};
    std::atomic<size_t> push_waits{0};
    std::atomic<size_t> pop_waits{0};
    
public:
    explicit BlockingQueue(size_t cap) : capacity(cap) {}
    
    // TODO: Implement blocking push
    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait while queue is full and not shutting down
        // Your code here
        
        // Add item and notify waiting consumers
        // Your code here
    }
    
    // TODO: Implement try_push with timeout
    bool try_push(const T& item, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait with timeout while queue is full
        // Your code here
        
        // Return false if timeout or shutdown
        // Otherwise add item and return true
    }
    
    // TODO: Implement blocking pop
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait while queue is empty and not shutting down
        // Your code here
        
        // Return std::nullopt if shutting down and queue empty
        // Otherwise remove and return item
    }
    
    // TODO: Implement try_pop with timeout
    std::optional<T> try_pop(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait with timeout while queue is empty
        // Your code here
    }
    
    // TODO: Implement shutdown
    void shutdown() {
        std::lock_guard<std::mutex> lock(mtx);
        shutdown_flag = true;
        cv_full.notify_all();
        cv_empty.notify_all();
    }
    
    bool is_shutdown() const {
        std::lock_guard<std::mutex> lock(mtx);
        return shutdown_flag;
    }
    
    void print_statistics() const {
        std::cout << "\n=== Queue Statistics ===\n";
        std::cout << "Total pushed: " << total_pushed << "\n";
        std::cout << "Total popped: " << total_popped << "\n";
        std::cout << "Push waits: " << push_waits << "\n";
        std::cout << "Pop waits: " << pop_waits << "\n";
    }
};

// TODO: Implement producer function
void producer(BlockingQueue<int>& queue, int producer_id, int num_items) {
    // Your code here
}

// TODO: Implement consumer function
void consumer(BlockingQueue<int>& queue, int consumer_id) {
    // Your code here
}

int main() {
    // TODO: Create producers and consumers
    // TODO: Run for specified duration
    // TODO: Shutdown gracefully
    // TODO: Print statistics
    return 0;
}
```

### Expected Behavior
- Producers never lose items
- Consumers process all produced items
- No deadlocks or race conditions
- Graceful shutdown with all threads terminating
- Balanced load across consumers

---

## Assignment 5: Lock-Free Data Structure with Atomic Operations (Advanced)

### Objective
Implement a lock-free stack using atomic operations, demonstrating when and how to avoid traditional locks.

### Requirements

1. **Lock-Free Stack:**
   - Use `std::atomic` for lock-free implementation
   - Compare-and-swap (CAS) operations
   - Handle ABA problem
   - No mutex or lock usage

2. **Operations:**
   - `push(value)`: Lock-free push
   - `pop()`: Lock-free pop
   - Correct behavior under heavy contention
   - Memory reclamation strategy

3. **Comparison:**
   - Implement both lock-based and lock-free versions
   - Benchmark performance under different contention levels
   - Measure throughput and latency

### Real-World Scenario
Simulate a high-frequency trading system where multiple threads:
- Push market data updates (lock-free)
- Pop data for processing (lock-free)
- Compare performance: lock-free vs mutex-based
- Test with 100,000+ operations per thread

### Starter Code
```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>

template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
        
        Node(const T& d) : data(d), next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    std::atomic<size_t> size_count{0};
    
public:
    LockFreeStack() : head(nullptr) {}
    
    ~LockFreeStack() {
        // TODO: Clean up all nodes
    }
    
    // TODO: Implement lock-free push
    void push(const T& data) {
        Node* new_node = new Node(data);
        
        // Use compare_exchange to atomically update head
        // Your code here
        // Hint: Loop until compare_exchange succeeds
    }
    
    // TODO: Implement lock-free pop
    std::shared_ptr<T> pop() {
        // Use compare_exchange to atomically remove head
        // Your code here
        // Handle empty stack case
        // Be careful with memory management!
    }
    
    bool empty() const {
        return head.load() == nullptr;
    }
    
    size_t size() const {
        return size_count.load();
    }
};

// TODO: Implement mutex-based stack for comparison
template<typename T>
class MutexBasedStack {
private:
    std::stack<T> stack;
    std::mutex mtx;
    
public:
    void push(const T& data) {
        // Your code here
    }
    
    std::shared_ptr<T> pop() {
        // Your code here
    }
    
    bool empty() const {
        // Your code here
    }
};

// TODO: Implement benchmark function
template<typename Stack>
void benchmark_stack(Stack& stack, int num_threads, int operations_per_thread) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    
    // Half threads push, half threads pop
    for (int i = 0; i < num_threads / 2; ++i) {
        threads.emplace_back([&stack, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                stack.push(j);
            }
        });
    }
    
    for (int i = 0; i < num_threads / 2; ++i) {
        threads.emplace_back([&stack, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                stack.pop();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << duration.count() << "ms\n";
    std::cout << "Throughput: " 
              << (num_threads * operations_per_thread * 1000.0 / duration.count()) 
              << " ops/sec\n";
}

int main() {
    // TODO: Benchmark both implementations
    // TODO: Compare performance under different thread counts
    return 0;
}
```

### Expected Output
```
=== Lock-Free Stack ===
Threads: 8, Operations: 100000
Time: 245ms
Throughput: 3265306 ops/sec

=== Mutex-Based Stack ===
Threads: 8, Operations: 100000
Time: 892ms
Throughput: 897533 ops/sec

Lock-free improvement: 3.64x faster
```

---

## Assignment 6: Hierarchical Mutex Lock Manager (Advanced)

### Objective
Implement a hierarchical mutex system that prevents deadlocks by enforcing a strict lock ordering hierarchy.

### Requirements

1. **Hierarchical Mutex:**
   - Each mutex has a hierarchy level (0-N)
   - Threads can only acquire mutexes in ascending order
   - Attempting to violate hierarchy throws an exception
   - Track per-thread lock hierarchy state

2. **Features:**
   - `hierarchical_mutex` class with lock level enforcement
   - Thread-local hierarchy tracking
   - Exception on hierarchy violation
   - Lock acquisition logging and validation

3. **Real-World Application:**
   - Implement a multi-layer cache system
   - L1 Cache (level 1000) -> L2 Cache (level 2000) -> Database (level 3000)
   - Multiple threads accessing all layers
   - Hierarchy prevents deadlocks automatically

### Starter Code
```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <stdexcept>

class hierarchical_mutex {
private:
    std::mutex internal_mutex;
    unsigned long const hierarchy_value;
    unsigned long previous_hierarchy_value;
    
    static thread_local unsigned long this_thread_hierarchy_value;
    
    void check_for_hierarchy_violation() {
        // TODO: Implement hierarchy check
        // Throw std::logic_error if attempting to lock out of order
    }
    
    void update_hierarchy_value() {
        // TODO: Update thread-local hierarchy tracking
    }
    
public:
    explicit hierarchical_mutex(unsigned long value)
        : hierarchy_value(value),
          previous_hierarchy_value(0)
    {}
    
    void lock() {
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchy_value();
    }
    
    void unlock() {
        // TODO: Restore previous hierarchy value
        internal_mutex.unlock();
    }
    
    bool try_lock() {
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock())
            return false;
        update_hierarchy_value();
        return true;
    }
};

// Initialize thread_local variable
thread_local unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);

// TODO: Implement multi-layer cache system using hierarchical mutexes

int main() {
    // TODO: Test hierarchical locking with correct and incorrect orders
    // TODO: Demonstrate deadlock prevention
    return 0;
}
```

---

## Evaluation Criteria

For each assignment, you will be evaluated on:

1. **Correctness (40%)**
   - Thread safety (no race conditions)
   - Deadlock-free operation
   - Correct synchronization primitive usage
   - Proper exception handling

2. **Performance (20%)**
   - Efficient lock usage (minimal critical sections)
   - Appropriate choice of synchronization mechanisms
   - Good throughput under high contention

3. **Code Quality (20%)**
   - Clean, readable code
   - Proper RAII usage
   - Good variable naming
   - Adequate comments

4. **Testing (20%)**
   - Comprehensive test cases
   - Stress testing with many threads
   - Edge case handling
   - Performance benchmarks

---

## Bonus Challenges

1. **Visualization:** Create a visual representation of lock acquisition/release timeline
2. **Metrics Dashboard:** Implement detailed performance monitoring
3. **Fuzzing:** Add random delays to expose rare race conditions
4. **Memory Analysis:** Use tools like Valgrind/ThreadSanitizer to verify thread safety
5. **Comparative Analysis:** Compare different synchronization approaches for the same problem

---

## Resources for Reference

- C++ Concurrency in Action (Chapter 3)
- cppreference.com - Thread support library
- Lock-free programming articles
- Consider using `-fsanitize=thread` for testing

---

## Submission Guidelines

1. Complete source code for each assignment
2. README with build/run instructions
3. Test output demonstrating correctness
4. Performance benchmarks (if applicable)
5. Brief writeup of design decisions and challenges faced

Good luck! Remember: **Thread safety is not optional—it's essential!**
