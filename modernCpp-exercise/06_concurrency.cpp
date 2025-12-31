/**
 * Modern C++ Exercise 06: Concurrency
 * 
 * Topics covered:
 * - std::thread basics
 * - Thread lifecycle and joining
 * - Mutexes and locks
 * - Condition variables
 * - Atomic operations
 * - std::async and std::future
 * - Thread-safe data structures
 * - Parallel patterns
 * 
 * Compile: g++ -std=c++20 -Wall -Wextra -pthread -o concurrency 06_concurrency.cpp
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>
#include <vector>
#include <queue>
#include <chrono>
#include <random>
#include <numeric>

// ============================================================================
// TASK 1: Basic thread creation and management
// ============================================================================

void simpleTask(int id) {
    std::cout << "Thread " << id << " is running\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Thread " << id << " finished\n";
}

void demonstrateBasicThreads() {
    std::cout << "\n=== Task 1: Basic Threads ===\n";
    
    // TODO: Create and start a thread
    
    // TODO: Join the thread to wait for completion
    
    // TODO: Create multiple threads in a vector
    
    // TODO: Join all threads
    
    // TODO: Pass arguments to thread function
    
    // TODO: Use lambda as thread function
    
    // Your code here
}

// ============================================================================
// TASK 2: Mutex and thread safety
// ============================================================================

class Counter {
private:
    int value_;
    std::mutex mutex_;
    
public:
    Counter() : value_(0) {}
    
    // TODO: Implement thread-safe increment using mutex
    void increment() {
        // Your code here
    }
    
    // TODO: Implement thread-safe decrement
    void decrement() {
        // Your code here
    }
    
    int getValue() const {
        return value_;
    }
};

void demonstrateMutex() {
    std::cout << "\n=== Task 2: Mutex and Thread Safety ===\n";
    
    Counter counter;
    
    // TODO: Create multiple threads that increment the counter
    
    // TODO: Create multiple threads that decrement the counter
    
    // TODO: Join all threads and print final value
    
    // Your code here
}

// ============================================================================
// TASK 3: Lock guards and unique locks
// ============================================================================

class BankAccount {
private:
    double balance_;
    mutable std::mutex mutex_;
    
public:
    BankAccount(double initial) : balance_(initial) {}
    
    // TODO: Implement deposit using std::lock_guard
    void deposit(double amount) {
        // Your code here
    }
    
    // TODO: Implement withdraw using std::unique_lock
    bool withdraw(double amount) {
        // Your code here
        return false;
    }
    
    // TODO: Implement transfer between accounts (requires locking two mutexes)
    // Use std::lock or std::scoped_lock to avoid deadlock
    static bool transfer(BankAccount& from, BankAccount& to, double amount) {
        // Your code here
        return false;
    }
    
    double getBalance() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return balance_;
    }
};

void demonstrateLocks() {
    std::cout << "\n=== Task 3: Lock Guards and Unique Locks ===\n";
    
    BankAccount account1(1000);
    BankAccount account2(500);
    
    // TODO: Create threads that perform deposits, withdrawals, and transfers
    
    // TODO: Print final balances
    
    // Your code here
}

// ============================================================================
// TASK 4: Condition variables
// ============================================================================

class MessageQueue {
private:
    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool done_ = false;
    
public:
    // TODO: Implement push (producer)
    void push(const std::string& message) {
        // Your code here
    }
    
    // TODO: Implement pop (consumer) - blocks until message available
    std::string pop() {
        // Your code here
        return "";
    }
    
    // TODO: Implement done signaling
    void setDone() {
        std::lock_guard<std::mutex> lock(mutex_);
        done_ = true;
        cv_.notify_all();
    }
    
    bool isDone() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return done_ && queue_.empty();
    }
};

void demonstrateConditionVariable() {
    std::cout << "\n=== Task 4: Condition Variables ===\n";
    
    MessageQueue queue;
    
    // TODO: Create producer thread that pushes messages
    
    // TODO: Create consumer threads that pop messages
    
    // TODO: Signal done and join all threads
    
    // Your code here
}

// ============================================================================
// TASK 5: Atomic operations
// ============================================================================

void demonstrateAtomics() {
    std::cout << "\n=== Task 5: Atomic Operations ===\n";
    
    std::atomic<int> counter(0);
    
    // TODO: Create threads that increment atomic counter
    // No mutex needed!
    
    // TODO: Demonstrate different memory orderings
    // std::memory_order_relaxed, seq_cst, acquire, release
    
    // TODO: Implement spin lock using atomic flag
    
    // Your code here
}

// ============================================================================
// TASK 6: std::async and std::future
// ============================================================================

// TODO: Implement a function that performs computation asynchronously
int computeSum(int start, int end) {
    int sum = 0;
    for (int i = start; i <= end; ++i) {
        sum += i;
    }
    return sum;
}

void demonstrateAsync() {
    std::cout << "\n=== Task 6: std::async and std::future ===\n";
    
    // TODO: Use std::async to compute sum asynchronously
    
    // TODO: Get result using future.get()
    
    // TODO: Use std::launch::async vs std::launch::deferred
    
    // TODO: Demonstrate std::promise and std::future
    
    // TODO: Use std::packaged_task
    
    // Your code here
}

// ============================================================================
// TASK 7: Thread-safe singleton
// ============================================================================

class ThreadSafeSingleton {
private:
    static std::unique_ptr<ThreadSafeSingleton> instance_;
    static std::once_flag initFlag_;
    
    int data_;
    
    ThreadSafeSingleton() : data_(42) {
        std::cout << "Singleton created\n";
    }
    
public:
    // Delete copy and move
    ThreadSafeSingleton(const ThreadSafeSingleton&) = delete;
    ThreadSafeSingleton& operator=(const ThreadSafeSingleton&) = delete;
    
    // TODO: Implement thread-safe getInstance using std::call_once
    static ThreadSafeSingleton& getInstance() {
        // Your code here
        static ThreadSafeSingleton instance;
        return instance;
    }
    
    int getData() const { return data_; }
};

void demonstrateSingleton() {
    std::cout << "\n=== Task 7: Thread-Safe Singleton ===\n";
    
    // TODO: Create multiple threads trying to get instance
    
    // TODO: Verify only one instance is created
    
    // Your code here
}

// ============================================================================
// TASK 8: Thread pool
// ============================================================================

class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;
    
public:
    ThreadPool(size_t numThreads) : stop_(false) {
        // TODO: Create worker threads
        // Your code here
    }
    
    ~ThreadPool() {
        // TODO: Stop all threads and join
        // Your code here
    }
    
    // TODO: Implement enqueue to add tasks
    template<typename F>
    void enqueue(F&& task) {
        // Your code here
    }
    
private:
    void workerThread() {
        // TODO: Implement worker thread logic
        // Your code here
    }
};

void demonstrateThreadPool() {
    std::cout << "\n=== Task 8: Thread Pool ===\n";
    
    // TODO: Create thread pool
    
    // TODO: Enqueue multiple tasks
    
    // TODO: Wait for completion
    
    // Your code here
}

// ============================================================================
// BONUS TASK: Producer-Consumer with bounded buffer
// ============================================================================

template<typename T>
class BoundedBuffer {
private:
    std::queue<T> queue_;
    size_t maxSize_;
    std::mutex mutex_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
    
public:
    BoundedBuffer(size_t maxSize) : maxSize_(maxSize) {}
    
    // TODO: Implement push - blocks when buffer is full
    void push(T item) {
        // Your code here
    }
    
    // TODO: Implement pop - blocks when buffer is empty
    T pop() {
        // Your code here
    }
};

void demonstrateBoundedBuffer() {
    std::cout << "\n=== Bonus: Bounded Buffer ===\n";
    
    BoundedBuffer<int> buffer(10);
    
    // TODO: Create fast producer
    
    // TODO: Create slow consumer
    
    // TODO: Observe blocking behavior
    
    // Your code here
}

// ============================================================================
// BONUS TASK: Parallel algorithms
// ============================================================================

void demonstrateParallelAlgorithms() {
    std::cout << "\n=== Bonus: Parallel Algorithms ===\n";
    
    const size_t size = 10000000;
    std::vector<int> data(size);
    
    // Initialize with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    
    for (auto& val : data) {
        val = dis(gen);
    }
    
    // TODO: Implement parallel sum using multiple threads
    auto parallelSum = [&data](size_t numThreads) {
        // Your code here
        return 0;
    };
    
    // TODO: Compare with sequential sum
    
    // TODO: Implement parallel map (transform)
    
    // TODO: Implement parallel reduce
    
    // Your code here
}

// ============================================================================
// PRACTICAL TASK: Concurrent web server simulation
// ============================================================================

class Request {
public:
    int id;
    std::string url;
    
    Request(int id, const std::string& url) : id(id), url(url) {}
};

class WebServer {
private:
    ThreadPool pool_;
    std::atomic<int> requestsProcessed_;
    std::mutex coutMutex_;
    
public:
    WebServer(size_t numThreads) : pool_(numThreads), requestsProcessed_(0) {}
    
    void handleRequest(const Request& req) {
        // TODO: Implement request handling
        // Simulate processing time
        // Update requestsProcessed counter
        // Print status (with mutex for cout)
        
        // Your code here
    }
    
    void start() {
        std::cout << "Server starting...\n";
        
        // TODO: Generate and handle multiple requests
        
        // Your code here
    }
    
    int getRequestsProcessed() const {
        return requestsProcessed_;
    }
};

void demonstratePracticalTask() {
    std::cout << "\n=== Practical Task: Web Server Simulation ===\n";
    
    // TODO: Create web server with thread pool
    
    // TODO: Start server and process requests
    
    // TODO: Print statistics
    
    // Your code here
}

// ============================================================================
// PRACTICAL TASK: Parallel file processing
// ============================================================================

void processFile(const std::string& filename) {
    // Simulate file processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Processed: " << filename << "\n";
}

void demonstrateParallelFileProcessing() {
    std::cout << "\n=== Practical: Parallel File Processing ===\n";
    
    std::vector<std::string> files = {
        "file1.txt", "file2.txt", "file3.txt", "file4.txt", "file5.txt",
        "file6.txt", "file7.txt", "file8.txt", "file9.txt", "file10.txt"
    };
    
    // TODO: Process files in parallel using futures
    
    // TODO: Wait for all to complete
    
    // Your code here
}

// ============================================================================
// Main function
// ============================================================================

int main() {
    std::cout << "Modern C++ Concurrency Exercise\n";
    std::cout << "================================\n";
    
    demonstrateBasicThreads();
    demonstrateMutex();
    demonstrateLocks();
    demonstrateConditionVariable();
    demonstrateAtomics();
    demonstrateAsync();
    demonstrateSingleton();
    demonstrateThreadPool();
    demonstrateBoundedBuffer();
    demonstrateParallelAlgorithms();
    demonstratePracticalTask();
    demonstrateParallelFileProcessing();
    
    std::cout << "\n=== All tasks completed! ===\n";
    return 0;
}

/*
 * KEY CONCEPTS TO REMEMBER:
 * 
 * 1. Always join or detach threads before destruction
 * 2. Use RAII locks (lock_guard, unique_lock) to avoid deadlocks
 * 3. Condition variables require a mutex and predicate check
 * 4. Atomics provide lock-free thread safety for simple types
 * 5. std::async is easier than raw threads for simple tasks
 * 6. Be careful with data races - use proper synchronization
 * 7. Thread pools reuse threads for better performance
 * 8. Consider memory ordering for advanced atomic operations
 * 
 * COMMON PITFALLS:
 * - Forgetting to join threads
 * - Deadlocks from incorrect lock ordering
 * - Spurious wakeups with condition variables
 * - Data races from improper synchronization
 * - Exception safety in threaded code
 */
