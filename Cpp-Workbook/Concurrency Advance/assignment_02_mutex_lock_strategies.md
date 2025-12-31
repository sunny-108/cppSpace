# Assignment 02: Mutex & Lock Strategies

## Overview
This assignment explores advanced mutex and locking strategies in C++, including lock hierarchies, reader-writer locks, scoped locking, and fine-grained vs coarse-grained locking trade-offs. Designed for experienced C++ developers (7+ years).

**Estimated Time:** 6-8 hours

---

## Learning Objectives
- Master different mutex types and their use cases
- Implement and understand lock hierarchies for deadlock prevention
- Design systems with reader-writer locks for optimal concurrency
- Apply fine-grained locking strategies effectively
- Understand and use C++17 scoped locking mechanisms

---

## Part 1: Multiple Choice Questions (MCQs)

### Q1. What is the main advantage of `std::shared_mutex` over `std::mutex`?
A) It's faster for all operations  
B) Multiple readers can hold the lock simultaneously  
C) It prevents deadlocks automatically  
D) It uses less memory  

**Answer:** B

---

### Q2. `std::scoped_lock` (C++17) can lock multiple mutexes. How does it prevent deadlock?
A) It uses a global lock ordering  
B) It uses a deadlock detection algorithm at runtime  
C) It locks mutexes in a consistent order using a deadlock avoidance algorithm  
D) It doesn't prevent deadlocks  

**Answer:** C

---

### Q3. What is the purpose of `std::lock_guard`?
A) To try locking without blocking  
B) RAII-style mutex ownership with automatic unlock  
C) To lock multiple mutexes simultaneously  
D) To create recursive locks  

**Answer:** B

---

### Q4. When should you use `std::unique_lock` instead of `std::lock_guard`?
A) Never, `std::lock_guard` is always better  
B) When you need to manually unlock before scope end or transfer ownership  
C) When you want better performance  
D) When working with recursive mutexes  

**Answer:** B

---

### Q5. What is a lock hierarchy?
A) A tree structure of mutexes  
B) An ordering rule where locks must be acquired in a specific order  
C) A priority system for thread scheduling  
D) A type of recursive mutex  

**Answer:** B

---

### Q6. `std::recursive_mutex` allows:
A) Multiple threads to lock simultaneously  
B) The same thread to acquire the lock multiple times  
C) Faster locking than regular mutex  
D) Automatic deadlock prevention  

**Answer:** B

---

### Q7. What is the risk of fine-grained locking?
A) Better performance  
B) More memory usage  
C) Increased complexity and potential for deadlocks  
D) Reduced parallelism  

**Answer:** C

---

### Q8. `std::try_lock` on a mutex:
A) Blocks until the lock is acquired  
B) Attempts to lock and returns immediately with success/failure status  
C) Throws an exception if lock fails  
D) Automatically retries until successful  

**Answer:** B

---

### Q9. For a data structure with frequent reads and rare writes, which is best?
A) `std::mutex`  
B) `std::shared_mutex` with shared locks for reads  
C) No synchronization needed  
D) `std::recursive_mutex`  

**Answer:** B

---

### Q10. What does `std::adopt_lock` tag indicate to a lock guard?
A) Try to acquire the lock  
B) The mutex is already locked by the current thread  
C) Use a non-blocking lock attempt  
D) Create a new mutex  

**Answer:** B

---

### Q11. Coarse-grained locking typically means:
A) Using many fine locks for small data pieces  
B) Using fewer locks protecting larger regions  
C) Not using locks at all  
D) Using only reader-writer locks  

**Answer:** B

---

### Q12. What is "lock convoing"?
A) A convoy of threads waiting for the same lock, reducing throughput  
B) A deadlock situation  
C) A lock-free algorithm  
D) A type of mutex  

**Answer:** A

---

### Q13. `std::shared_lock` is used with `std::shared_mutex` to:
A) Acquire exclusive write access  
B) Acquire shared read access  
C) Try locking without blocking  
D) Recursively lock  

**Answer:** B

---

### Q14. What is double-checked locking typically used for?
A) Preventing all race conditions  
B) Lazy initialization with minimal locking overhead  
C) Acquiring two locks safely  
D) Reader-writer synchronization  

**Answer:** B

---

### Q15. The granularity of a lock refers to:
A) The size of the lock object in memory  
B) The amount of data or code protected by the lock  
C) The priority of the lock  
D) The type of mutex used  

**Answer:** B

---

## Part 2: Code Review Exercises

### Exercise 2.1: Identify Lock Hierarchy Violations

Review the following code and identify lock hierarchy violations:

```cpp
class BankAccount {
    mutable std::mutex mtx;
    double balance;
    int accountNumber;
    
public:
    BankAccount(int num, double bal) : accountNumber(num), balance(bal) {}
    
    void transfer(BankAccount& other, double amount) {
        std::lock_guard<std::mutex> lock1(mtx);
        std::lock_guard<std::mutex> lock2(other.mtx);
        
        if (balance >= amount) {
            balance -= amount;
            other.balance += amount;
        }
    }
    
    void deposit(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance += amount;
    }
    
    double getBalance() const {
        std::lock_guard<std::mutex> lock(mtx);
        return balance;
    }
};

void scenario1() {
    BankAccount acc1(1, 1000);
    BankAccount acc2(2, 2000);
    
    std::thread t1([&] { acc1.transfer(acc2, 100); });
    std::thread t2([&] { acc2.transfer(acc1, 150); });
    
    t1.join();
    t2.join();
}
```

**Questions:**
1. What is the potential deadlock scenario?
2. Why doesn't the current implementation prevent it?
3. How would you fix it using `std::scoped_lock`?
4. How would you fix it using lock hierarchy based on account numbers?
5. Write a corrected version using both approaches.

---

### Exercise 2.2: Reader-Writer Lock Misuse

```cpp
class Cache {
    mutable std::shared_mutex mtx;
    std::unordered_map<std::string, std::string> data;
    
public:
    std::string get(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        auto it = data.find(key);
        return (it != data.end()) ? it->second : "";
    }
    
    void put(const std::string& key, const std::string& value) {
        std::shared_lock<std::shared_mutex> lock(mtx);
        data[key] = value;
    }
    
    bool contains(const std::string& key) const {
        // No lock - read is atomic, right?
        return data.find(key) != data.end();
    }
    
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return data.size();
    }
};
```

**Questions:**
1. What locks are being used incorrectly?
2. Why is `contains()` unsafe?
3. What's wrong with `size()`?
4. How would you optimize this for a read-heavy workload?
5. Provide a corrected and optimized version.

---

### Exercise 2.3: Over-Engineering with Fine-Grained Locks

```cpp
class ThreadSafeVector {
    std::vector<int> data;
    std::vector<std::unique_ptr<std::mutex>> elementLocks;
    std::mutex sizeMutex;
    
public:
    ThreadSafeVector(size_t size) : data(size), elementLocks(size) {
        for (auto& lock : elementLocks) {
            lock = std::make_unique<std::mutex>();
        }
    }
    
    void set(size_t index, int value) {
        std::lock_guard<std::mutex> lock(*elementLocks[index]);
        data[index] = value;
    }
    
    int get(size_t index) const {
        std::lock_guard<std::mutex> lock(*elementLocks[index]);
        return data[index];
    }
    
    void push_back(int value) {
        std::lock_guard<std::mutex> lock(sizeMutex);
        data.push_back(value);
        elementLocks.push_back(std::make_unique<std::mutex>());
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(sizeMutex);
        return data.size();
    }
};
```

**Questions:**
1. What are the issues with this design?
2. Why is `push_back()` unsafe despite locking?
3. How does this approach compare to a single mutex?
4. When (if ever) would per-element locking make sense?
5. Benchmark and compare with a simple `std::mutex` approach.

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Hierarchical Mutex

Implement a mutex that enforces lock hierarchy at runtime:

```cpp
#include <mutex>
#include <stdexcept>
#include <thread>

class HierarchicalMutex {
public:
    explicit HierarchicalMutex(unsigned level);
    
    void lock();
    void unlock();
    bool try_lock();
    
    unsigned getLevel() const { return hierarchyLevel; }
    
private:
    std::mutex internalMutex;
    unsigned const hierarchyLevel;
    unsigned previousHierarchyLevel;
    
    static thread_local unsigned thisThreadHierarchyLevel;
    
    void checkForHierarchyViolation();
    void updateHierarchyLevel();
};

// Initialize thread-local storage
thread_local unsigned HierarchicalMutex::thisThreadHierarchyLevel(UINT_MAX);

// Your implementation here

// Test cases
void testHierarchicalMutex() {
    HierarchicalMutex high(10000);
    HierarchicalMutex mid(6000);
    HierarchicalMutex low(5000);
    
    // Test 1: Correct order - should work
    try {
        std::lock_guard<HierarchicalMutex> lock1(high);
        std::lock_guard<HierarchicalMutex> lock2(mid);
        std::lock_guard<HierarchicalMutex> lock3(low);
        std::cout << "Test 1 passed: Correct hierarchy\n";
    } catch (const std::logic_error& e) {
        std::cout << "Test 1 failed: " << e.what() << "\n";
    }
    
    // Test 2: Wrong order - should throw
    try {
        std::lock_guard<HierarchicalMutex> lock1(low);
        std::lock_guard<HierarchicalMutex> lock2(high); // Should throw!
        std::cout << "Test 2 failed: Should have thrown\n";
    } catch (const std::logic_error& e) {
        std::cout << "Test 2 passed: Caught violation - " << e.what() << "\n";
    }
    
    // Test 3: Multiple threads with different hierarchies
    std::thread t1([&] {
        std::lock_guard<HierarchicalMutex> lock1(high);
        std::lock_guard<HierarchicalMutex> lock2(mid);
    });
    
    std::thread t2([&] {
        std::lock_guard<HierarchicalMutex> lock1(mid);
        std::lock_guard<HierarchicalMutex> lock2(low);
    });
    
    t1.join();
    t2.join();
    std::cout << "Test 3 passed: Multi-threaded hierarchy\n";
}
```

**Requirements:**
- Enforce that locks are acquired in decreasing order of hierarchy level
- Throw exception if hierarchy is violated
- Properly restore hierarchy level on unlock
- Support try_lock
- Be thread-safe and support multiple threads with independent hierarchies

---

### Exercise 3.2: Upgradeable Reader-Writer Lock

Implement a lock that supports upgrading from read to write lock:

```cpp
#include <shared_mutex>
#include <condition_variable>

class UpgradeableMutex {
public:
    // Read lock
    void lock_shared();
    void unlock_shared();
    
    // Write lock
    void lock();
    void unlock();
    
    // Upgradeable lock - can be upgraded to write lock
    void lock_upgrade();
    void unlock_upgrade();
    
    // Upgrade from upgradeable to exclusive
    void upgrade();
    
    // Downgrade from exclusive to upgradeable
    void downgrade();
    
private:
    std::mutex mtx;
    std::condition_variable cv;
    
    int readers = 0;
    bool writer = false;
    bool upgrader = false;
    int waitingWriters = 0;
    
    // Your implementation
};

// RAII wrappers
class SharedLock {
public:
    explicit SharedLock(UpgradeableMutex& m) : mutex(m) { mutex.lock_shared(); }
    ~SharedLock() { mutex.unlock_shared(); }
private:
    UpgradeableMutex& mutex;
};

class UpgradeableLock {
public:
    explicit UpgradeableLock(UpgradeableMutex& m) : mutex(m) { mutex.lock_upgrade(); }
    ~UpgradeableLock() { mutex.unlock_upgrade(); }
    
    void upgrade() { mutex.upgrade(); upgraded = true; }
    void downgrade() { if (upgraded) { mutex.downgrade(); upgraded = false; } }
    
private:
    UpgradeableMutex& mutex;
    bool upgraded = false;
};

// Test case: Cache with copy-on-write
class CopyOnWriteCache {
    UpgradeableMutex mutex;
    std::unordered_map<std::string, int> data;
    
public:
    int get(const std::string& key) {
        SharedLock lock(mutex);
        auto it = data.find(key);
        return (it != data.end()) ? it->second : -1;
    }
    
    void update(const std::string& key, int value) {
        UpgradeableLock lock(mutex);
        
        // Check if update needed (with read lock)
        auto it = data.find(key);
        if (it != data.end() && it->second == value) {
            return; // No change needed
        }
        
        // Upgrade to write lock for modification
        lock.upgrade();
        data[key] = value;
    }
};
```

**Requirements:**
- Multiple shared readers allowed
- Only one upgradeable lock holder at a time
- Upgradeable lock can coexist with shared readers
- Upgrade operation waits for readers to finish
- Prevent writer starvation
- Avoid deadlocks

---

### Exercise 3.3: Striped Lock Container

Implement a thread-safe hash map using lock striping:

```cpp
#include <vector>
#include <list>
#include <mutex>
#include <functional>

template<typename Key, typename Value, size_t NumStripes = 16>
class StripedHashMap {
public:
    StripedHashMap() : buckets(NumStripes), mutexes(NumStripes) {}
    
    void insert(const Key& key, const Value& value);
    bool find(const Key& key, Value& value) const;
    bool erase(const Key& key);
    
    size_t size() const;
    void clear();
    
    // Advanced: Implement iteration safely
    template<typename Func>
    void forEach(Func func);
    
private:
    struct Entry {
        Key key;
        Value value;
    };
    
    std::vector<std::list<Entry>> buckets;
    mutable std::vector<std::mutex> mutexes;
    
    size_t getBucketIndex(const Key& key) const {
        return std::hash<Key>{}(key) % NumStripes;
    }
    
    // Your implementation
};

// Test with high concurrency
void testStripedHashMap() {
    StripedHashMap<int, std::string> map;
    
    const int numThreads = 16;
    const int operationsPerThread = 10000;
    
    auto worker = [&](int id) {
        for (int i = 0; i < operationsPerThread; ++i) {
            int key = (id * operationsPerThread + i) % 1000;
            
            // Insert
            map.insert(key, "value_" + std::to_string(key));
            
            // Find
            std::string value;
            map.find(key, value);
            
            // Erase some
            if (i % 10 == 0) {
                map.erase(key);
            }
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Operations completed in " << duration.count() << "ms\n";
    std::cout << "Final size: " << map.size() << "\n";
}
```

**Requirements:**
- Use separate mutex for each stripe
- Minimize lock contention
- Implement safe iteration (lock all stripes)
- Handle size() efficiently
- Benchmark against single-mutex version

---

## Part 4: Debugging Concurrent Code

### Exercise 4.1: Deadlock Detection

This code has a subtle deadlock. Find and fix it:

```cpp
class Account {
    mutable std::mutex mtx;
    int balance;
    int id;
    
public:
    Account(int id, int bal) : id(id), balance(bal) {}
    
    void withdraw(int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance -= amount;
    }
    
    void deposit(int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance += amount;
    }
    
    int getBalance() const {
        std::lock_guard<std::mutex> lock(mtx);
        return balance;
    }
    
    bool transferTo(Account& other, int amount) {
        std::lock_guard<std::mutex> lock1(mtx);
        
        if (balance < amount) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock2(other.mtx);
        balance -= amount;
        other.balance += amount;
        return true;
    }
    
    // Compound operation: transfer and log
    void transferAndLog(Account& other, int amount) {
        if (transferTo(other, amount)) {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Transferred " << amount 
                      << " from " << id << " to " << other.id 
                      << ". New balance: " << balance << "\n";
        }
    }
};

void runTransfers() {
    Account a1(1, 1000);
    Account a2(2, 1000);
    
    std::thread t1([&] {
        for (int i = 0; i < 100; ++i) {
            a1.transferAndLog(a2, 10);
        }
    });
    
    std::thread t2([&] {
        for (int i = 0; i < 100; ++i) {
            a2.transferAndLog(a1, 15);
        }
    });
    
    t1.join();
    t2.join();
}
```

**Tasks:**
1. Explain the deadlock scenario step-by-step
2. Why does `transferAndLog` make it worse?
3. Use a thread debugger or add logging to confirm
4. Provide multiple solutions (lock ordering, `std::scoped_lock`, lock-free approach)
5. Compare performance of each solution

---

### Exercise 4.2: Lock Granularity Bug

```cpp
class ThreadSafeQueue {
    std::queue<int> queue;
    mutable std::mutex mtx;
    std::condition_variable cv;
    
public:
    void push(int value) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(value);
        }
        cv.notify_one(); // Outside lock - is this safe?
    }
    
    int pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        
        int value = queue.front();
        queue.pop();
        return value;
    }
    
    // Batch operation
    std::vector<int> popAll() {
        std::vector<int> result;
        std::lock_guard<std::mutex> lock(mtx);
        
        while (!queue.empty()) {
            result.push_back(queue.front());
            queue.pop();
        }
        
        return result;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }
};

void testQueue() {
    ThreadSafeQueue q;
    std::atomic<bool> done{false};
    
    // Producer
    std::thread producer([&] {
        for (int i = 0; i < 1000; ++i) {
            q.push(i);
        }
        done = true;
    });
    
    // Consumer 1: pop one at a time
    std::thread consumer1([&] {
        while (!done || q.size() > 0) {
            if (q.size() > 0) {
                int val = q.pop();
                // Process...
            }
        }
    });
    
    // Consumer 2: pop in batches
    std::thread consumer2([&] {
        while (!done || q.size() > 0) {
            auto batch = q.popAll();
            // Process batch...
        }
    });
    
    producer.join();
    consumer1.join();
    consumer2.join();
}
```

**Questions:**
1. Is `notify_one()` outside the lock safe? Why or why not?
2. What's the issue with checking `q.size() > 0` then calling `pop()`?
3. Can `popAll()` and `pop()` cause issues together?
4. How would you fix the consumers' logic?
5. Is there a risk of missed notifications?

---

## Part 5: Performance Optimization

### Exercise 5.1: Lock Contention Analysis

Implement a performance comparison framework:

```cpp
class LockBenchmark {
public:
    struct Config {
        size_t numThreads;
        size_t operationsPerThread;
        double readRatio; // 0.0 to 1.0
        size_t contentionLevel; // 0 = no contention, 10 = high
    };
    
    struct Results {
        double throughput; // ops per second
        double avgLatency; // microseconds
        std::string lockType;
    };
    
    // Benchmark different lock strategies
    Results benchmarkMutex(const Config& config);
    Results benchmarkSharedMutex(const Config& config);
    Results benchmarkSpinlock(const Config& config);
    Results benchmarkNoLock(const Config& config); // baseline
    
    void runFullComparison();
};
```

**Tasks:**
1. Implement all benchmark methods
2. Vary read/write ratio from 90/10 to 10/90
3. Test with 1 to 32 threads
4. Graph results: throughput vs threads for each lock type
5. Analyze when each lock type performs best
6. Measure lock contention using `perf` or similar tools

---

### Exercise 5.2: Optimize Lock-Heavy Code

Optimize this lock-heavy implementation:

```cpp
class MetricsCollector {
    mutable std::mutex mtx;
    std::map<std::string, int64_t> counters;
    std::map<std::string, std::vector<double>> histograms;
    
public:
    void incrementCounter(const std::string& name, int64_t delta = 1) {
        std::lock_guard<std::mutex> lock(mtx);
        counters[name] += delta;
    }
    
    void recordValue(const std::string& name, double value) {
        std::lock_guard<std::mutex> lock(mtx);
        histograms[name].push_back(value);
    }
    
    int64_t getCounter(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = counters.find(name);
        return (it != counters.end()) ? it->second : 0;
    }
    
    std::vector<double> getHistogram(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = histograms.find(name);
        return (it != histograms.end()) ? it->second : std::vector<double>();
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(mtx);
        counters.clear();
        histograms.clear();
    }
};

// High-frequency usage
void simulateHighLoad(MetricsCollector& metrics) {
    const int numThreads = 16;
    const int opsPerThread = 100000;
    
    auto worker = [&](int id) {
        for (int i = 0; i < opsPerThread; ++i) {
            metrics.incrementCounter("requests");
            metrics.recordValue("latency", id * 0.1 + i * 0.001);
            
            if (i % 1000 == 0) {
                auto count = metrics.getCounter("requests");
            }
        }
    };
    
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Duration: " << duration.count() << "ms\n";
}
```

**Optimization Tasks:**
1. Measure baseline performance
2. Implement using reader-writer locks
3. Implement using lock striping (separate locks for counters/histograms)
4. Implement using thread-local aggregation with periodic merging
5. Compare all approaches with detailed metrics
6. Provide recommendations for different use cases

---

### Exercise 5.3: Reduce Lock Hold Time

```cpp
class Logger {
    std::mutex mtx;
    std::ofstream file;
    
public:
    Logger(const std::string& filename) : file(filename) {}
    
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);
        
        // Format timestamp
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::string timestamp = std::ctime(&time);
        timestamp.pop_back(); // Remove newline
        
        // Format message
        std::string formatted = "[" + timestamp + "] " + message + "\n";
        
        // Write to file
        file << formatted;
        file.flush();
    }
};

void testLogger() {
    Logger logger("test.log");
    
    const int numThreads = 10;
    const int messagesPerThread = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i] {
            for (int j = 0; j < messagesPerThread; ++j) {
                logger.log("Thread " + std::to_string(i) + 
                          " message " + std::to_string(j));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Logging took: " << duration.count() << "ms\n";
}
```

**Tasks:**
1. Identify operations that don't need to be in critical section
2. Implement with minimal lock hold time
3. Consider using a queue and background writer thread
4. Benchmark improvements
5. Ensure message ordering is preserved

---

## Submission Guidelines

1. **Source Code:**
   - Compile with C++17 minimum (`-std=c++17`)
   - Include comprehensive test cases
   - Document design decisions

2. **Analysis Document:**
   - MCQ answers with explanations
   - Code review findings
   - Performance analysis with graphs
   - Comparisons of different approaches

3. **Build System:**
   - CMakeLists.txt or Makefile
   - Instructions for building and running
   - Thread sanitizer support: `-fsanitize=thread`

4. **Performance Data:**
   - Benchmark results in CSV format
   - Graphs comparing approaches
   - Hardware specifications used for testing

---

## Evaluation Criteria

- **Correctness (30%):** Proper lock usage, no race conditions
- **Performance (25%):** Optimal lock strategies for given scenarios
- **Design (20%):** Appropriate lock granularity and patterns
- **Analysis (15%):** Thorough performance analysis
- **Code Quality (10%):** Clean, maintainable code

---

## Additional Resources

- [C++ Reference - Mutex library](https://en.cppreference.com/w/cpp/thread#Mutual_exclusion)
- C++ Concurrency in Action, 2nd Edition - Chapters 3-4
- [Lock-Free Programming](https://preshing.com/20120612/an-introduction-to-lock-free-programming/)
- [Intel VTune Profiler](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/vtune-profiler.html)

---

Good luck with the assignment!
