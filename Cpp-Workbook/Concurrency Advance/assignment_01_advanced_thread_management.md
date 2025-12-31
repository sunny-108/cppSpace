# Assignment 01: Advanced Thread Management

## Overview
This assignment focuses on advanced thread management techniques in C++, including thread lifecycle management, thread pools, thread-local storage, and hardware thread affinity. Designed for experienced C++ developers (7+ years).

**Estimated Time:** 6-8 hours

---

## Learning Objectives
- Master advanced thread lifecycle management
- Implement production-ready thread pools
- Understand and utilize thread-local storage effectively
- Work with hardware thread affinity and CPU pinning
- Optimize thread creation and destruction overhead

---

## Part 1: Multiple Choice Questions (MCQs)

### Q1. What happens when you call `std::thread::detach()` on a thread object?
A) The thread is immediately terminated  
B) The thread continues execution independently, and the thread object no longer manages it  
C) The thread is paused until `join()` is called  
D) The thread's resources are immediately released  

**Answer:** B

---

### Q2. Which of the following is TRUE about `std::jthread` (C++20)?
A) It cannot be detached  
B) It automatically joins in its destructor  
C) It has worse performance than `std::thread`  
D) It doesn't support stop tokens  

**Answer:** B

---

### Q3. Thread-local storage duration means:
A) The variable is created when the program starts  
B) Each thread gets its own instance of the variable  
C) The variable is shared across all threads  
D) The variable only exists for one function call  

**Answer:** B

---

### Q4. In a fixed-size thread pool, what strategy should be used when all threads are busy?
A) Create a new thread immediately  
B) Terminate the oldest thread and reuse it  
C) Queue the task and wait for a thread to become available  
D) Throw an exception  

**Answer:** C

---

### Q5. What is the risk of creating too many threads?
A) Deadlock  
B) Memory exhaustion and context switching overhead  
C) Race conditions  
D) Data corruption  

**Answer:** B

---

### Q6. `std::hardware_concurrency()` returns:
A) The number of currently running threads  
B) A hint for the number of concurrent threads supported by the hardware  
C) The maximum number of threads that can be created  
D) The optimal thread pool size for all applications  

**Answer:** B

---

### Q7. Which is the correct way to pass arguments to a thread constructor?
A) `std::thread t(func, &arg);` always passes by reference  
B) `std::thread t(func, std::ref(arg));` is needed to pass by reference  
C) Arguments are always passed by value, references are impossible  
D) `std::thread t(func, arg&);` passes by reference  

**Answer:** B

---

### Q8. What happens if a `std::thread` object with an active thread is destroyed?
A) The thread is automatically joined  
B) The thread is automatically detached  
C) `std::terminate()` is called  
D) The thread continues running normally  

**Answer:** C

---

### Q9. Thread affinity (CPU pinning) is useful for:
A) Preventing deadlocks  
B) Improving cache locality and reducing context switches  
C) Automatically distributing work across CPUs  
D) Creating more threads than available cores  

**Answer:** B

---

### Q10. In a work-stealing thread pool:
A) Idle threads steal tasks from busy threads' queues  
B) The main thread steals results from worker threads  
C) Threads steal CPU time from each other  
D) Tasks are stolen from completed work  

**Answer:** A

---

### Q11. What is a common pattern to signal a thread pool to shut down gracefully?
A) Call `std::terminate()` on all threads  
B) Use an atomic boolean flag checked by worker threads  
C) Force kill the process  
D) Detach all threads and exit main  

**Answer:** B

---

### Q12. Which C++20 feature allows cooperative cancellation of threads?
A) `std::stop_token` and `std::stop_source`  
B) `std::cancel_token`  
C) `std::thread::cancel()`  
D) `pthread_cancel`  

**Answer:** A

---

### Q13. Thread-local storage is allocated:
A) On the stack of the creating thread  
B) On the heap globally  
C) In a thread-specific storage area  
D) In the CPU cache  

**Answer:** C

---

### Q14. What is the main advantage of using a thread pool over creating threads on-demand?
A) Thread pools prevent race conditions  
B) Reduced overhead of thread creation/destruction  
C) Thread pools automatically prevent deadlocks  
D) Better compiler optimization  

**Answer:** B

---

### Q15. When should you use `std::async` instead of manually managing threads?
A) When you need fine-grained control over thread lifecycle  
B) When you want task-based parallelism with automatic result retrieval  
C) When you need to set thread affinity  
D) When working with thread-local storage  

**Answer:** B

---

## Part 2: Code Review Exercises

### Exercise 2.1: Identify Issues in Thread Pool Implementation

Review the following thread pool implementation and identify at least 5 issues:

```cpp
#include <thread>
#include <vector>
#include <queue>
#include <functional>

class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
public:
    ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    if (!tasks.empty()) {
                        task = tasks.front();
                        tasks.pop();
                        task();
                    }
                }
            });
        }
    }
    
    void submit(std::function<void()> task) {
        tasks.push(task);
    }
    
    ~ThreadPool() {
        for (auto& worker : workers) {
            worker.join();
        }
    }
};
```

**Issues to identify:**
1. No synchronization protecting the `tasks` queue (race condition)
2. Busy-wait loop consuming CPU when no tasks available
3. No shutdown mechanism - threads will run forever, `join()` will block indefinitely
4. No condition variable to notify workers when tasks are available
5. Potential exception safety issues if task throws
6. No way to wait for all tasks to complete
7. Workers may access `tasks` after the queue is destroyed

**Your Task:** Write a corrected version with proper synchronization, graceful shutdown, and exception handling.

---

### Exercise 2.2: Thread Lifecycle Management Bug

```cpp
class DataProcessor {
    std::thread worker;
    std::vector<int> data;
    bool done = false;
    
public:
    void start() {
        worker = std::thread([this] {
            while (!done) {
                processData();
            }
        });
    }
    
    void stop() {
        done = true;
    }
    
    void processData() {
        for (auto& item : data) {
            item *= 2;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

int main() {
    DataProcessor processor;
    processor.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    processor.stop();
    return 0;
}
```

**Questions:**
1. What is the critical bug in this code?
2. Why does the destructor of `DataProcessor` not join the thread?
3. How does this lead to undefined behavior?
4. What race conditions exist?
5. Fix all issues including proper synchronization.

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Production-Ready Thread Pool

Implement a complete thread pool with the following features:

**Requirements:**
- Fixed number of worker threads
- Thread-safe task queue with condition variable
- Support for task submission with future return values
- Graceful shutdown mechanism
- Exception handling in tasks
- Wait for all tasks to complete
- Optional: Task priority support

**Template:**

```cpp
#include <iostream>
#include <thread>
#include <queue>
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();
    
    // Submit a task and get a future for the result
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>;
    
    // Wait for all pending tasks to complete
    void wait();
    
    // Shutdown the pool
    void shutdown();
    
private:
    // Your implementation here
};

// Test your implementation
int main() {
    ThreadPool pool(4);
    
    // Test 1: Simple tasks
    auto future1 = pool.submit([] { return 42; });
    std::cout << "Result: " << future1.get() << std::endl;
    
    // Test 2: Multiple tasks
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.submit([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * i;
        }));
    }
    
    for (auto& f : futures) {
        std::cout << f.get() << " ";
    }
    std::cout << std::endl;
    
    // Test 3: Exception handling
    auto future2 = pool.submit([] {
        throw std::runtime_error("Task failed");
        return 0;
    });
    
    try {
        future2.get();
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << std::endl;
    }
    
    pool.shutdown();
    return 0;
}
```

---

### Exercise 3.2: Thread-Local Storage Cache

Implement a thread-local cache system for expensive computations:

```cpp
#include <unordered_map>
#include <string>
#include <functional>

class ThreadLocalCache {
public:
    using ComputeFunc = std::function<std::string(const std::string&)>;
    
    // Get value from cache or compute it
    std::string get(const std::string& key, ComputeFunc compute);
    
    // Clear current thread's cache
    void clear();
    
    // Get statistics for current thread
    struct Stats {
        size_t hits;
        size_t misses;
        size_t size;
    };
    Stats getStats() const;
    
private:
    // Your implementation using thread_local
};

// Expensive computation to cache
std::string expensiveComputation(const std::string& key) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return "computed_" + key;
}

// Test with multiple threads
int main() {
    ThreadLocalCache cache;
    
    auto worker = [&cache](int id) {
        for (int i = 0; i < 5; ++i) {
            std::string key = "key" + std::to_string(i % 3);
            auto result = cache.get(key, expensiveComputation);
            std::cout << "Thread " << id << ": " << result << std::endl;
        }
        auto stats = cache.getStats();
        std::cout << "Thread " << id << " - Hits: " << stats.hits 
                  << ", Misses: " << stats.misses << std::endl;
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

---

### Exercise 3.3: RAII Thread Manager

Create a RAII wrapper for thread management that ensures threads are always properly joined or detached:

```cpp
class ManagedThread {
public:
    enum class Policy { Join, Detach };
    
    // Constructor takes a thread and a policy
    template<typename F, typename... Args>
    ManagedThread(Policy policy, F&& f, Args&&... args);
    
    // Non-copyable but movable
    ManagedThread(const ManagedThread&) = delete;
    ManagedThread& operator=(const ManagedThread&) = delete;
    ManagedThread(ManagedThread&&) noexcept;
    ManagedThread& operator=(ManagedThread&&) noexcept;
    
    ~ManagedThread();
    
    // Check if thread is joinable
    bool joinable() const;
    
    // Explicitly join (only if policy is Join)
    void join();
    
private:
    // Your implementation
};

// Usage example
int main() {
    {
        ManagedThread t1(ManagedThread::Policy::Join, [] {
            std::cout << "Thread 1\n";
        });
        // Automatically joined on scope exit
    }
    
    {
        ManagedThread t2(ManagedThread::Policy::Detach, [] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Thread 2\n";
        });
        // Automatically detached on scope exit
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return 0;
}
```

---

## Part 4: Debugging Concurrent Code

### Exercise 4.1: Race Condition Hunt

The following code has subtle race conditions. Find them all and explain how they occur:

```cpp
#include <thread>
#include <vector>
#include <iostream>
#include <memory>

class Worker {
    int id;
    std::vector<int> data;
    bool running = false;
    std::thread thread;
    
public:
    Worker(int id) : id(id) {}
    
    void start() {
        running = true;
        thread = std::thread([this] {
            while (running) {
                process();
            }
        });
    }
    
    void stop() {
        running = false;
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    void addData(int value) {
        data.push_back(value);
    }
    
    void process() {
        if (!data.empty()) {
            int value = data.back();
            data.pop_back();
            std::cout << "Worker " << id << " processed: " << value << std::endl;
        }
    }
    
    ~Worker() {
        stop();
    }
};

int main() {
    std::vector<std::shared_ptr<Worker>> workers;
    
    for (int i = 0; i < 4; ++i) {
        auto worker = std::make_shared<Worker>(i);
        worker->start();
        workers.push_back(worker);
    }
    
    for (int i = 0; i < 100; ++i) {
        workers[i % 4]->addData(i);
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    return 0;
}
```

**Tasks:**
1. List all race conditions
2. Explain potential crashes or undefined behavior
3. Use thread sanitizer to verify: compile with `-fsanitize=thread`
4. Provide a corrected, thread-safe version

---

### Exercise 4.2: Debugging Thread Pool Hang

This thread pool occasionally hangs. Debug and fix the issue:

```cpp
class SimpleThreadPool {
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;
    std::atomic<int> activeTasks{0};
    
public:
    SimpleThreadPool(size_t n) {
        for (size_t i = 0; i < n; ++i) {
            threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] { return stop || !tasks.empty(); });
                        
                        if (stop && tasks.empty()) return;
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    activeTasks++;
                    task();
                    activeTasks--;
                }
            });
        }
    }
    
    void submit(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            tasks.push(std::move(task));
        }
        cv.notify_one();
    }
    
    void waitAll() {
        while (activeTasks > 0 || !tasks.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    ~SimpleThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto& t : threads) {
            t.join();
        }
    }
};
```

**Tasks:**
1. Identify why `waitAll()` can hang
2. Find the race condition with `activeTasks`
3. Explain the check `!tasks.empty()` race condition
4. Provide a corrected version

---

## Part 5: Performance Optimization

### Exercise 5.1: Thread Pool Sizing

Write a program that:
1. Benchmarks different thread pool sizes (1 to 2x hardware concurrency)
2. Tests with CPU-bound vs I/O-bound tasks
3. Measures throughput and latency
4. Determines optimal pool size for each workload type

```cpp
#include <chrono>
#include <random>

// CPU-bound task
int cpuBoundTask() {
    int result = 0;
    for (int i = 0; i < 1000000; ++i) {
        result += std::sqrt(i);
    }
    return result;
}

// I/O-bound task simulation
void ioBoundTask() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// Implement benchmarking framework
class ThreadPoolBenchmark {
public:
    struct Results {
        double throughput;  // tasks per second
        double avgLatency;  // milliseconds
        size_t poolSize;
    };
    
    Results benchmark(size_t poolSize, size_t numTasks, 
                     std::function<void()> task);
    
    void runFullBenchmark();
};
```

**Required Analysis:**
- Graph throughput vs pool size for both workload types
- Explain the results based on Amdahl's Law
- Recommend optimal pool sizes

---

### Exercise 5.2: Reduce Thread Creation Overhead

Optimize this code that creates many short-lived threads:

```cpp
void processItems(const std::vector<int>& items) {
    std::vector<std::thread> threads;
    
    for (const auto& item : items) {
        threads.emplace_back([item] {
            // Quick processing
            int result = item * 2;
            std::cout << result << std::endl;
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

int main() {
    std::vector<int> items(1000);
    std::iota(items.begin(), items.end(), 0);
    
    auto start = std::chrono::high_resolution_clock::now();
    processItems(items);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::cout << "Time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ms" << std::endl;
    
    return 0;
}
```

**Tasks:**
1. Measure baseline performance
2. Implement using thread pool
3. Implement using batch processing
4. Compare performance with graphs
5. Explain when each approach is best

---

## Submission Guidelines

1. **Code Files:**
   - All implementations must compile with C++17 or later
   - Include CMakeLists.txt or compilation instructions
   - Comment your code explaining design decisions

2. **Documentation:**
   - Answers to all MCQs
   - Detailed explanations for code review exercises
   - Performance analysis with data and graphs for optimization tasks
   - List of bugs found in debugging exercises with explanations

3. **Testing:**
   - Provide test cases for all implementations
   - Include stress tests for thread pools
   - Verify with thread sanitizer: `-fsanitize=thread`

4. **Performance Report:**
   - Benchmarking results for optimization exercises
   - Analysis of results
   - Recommendations

---

## Evaluation Criteria

- **Correctness (30%):** Thread-safe implementations, no race conditions
- **Code Quality (20%):** Clean, maintainable, well-documented code
- **Performance (20%):** Efficient implementations with proper optimizations
- **Problem Solving (20%):** Thoroughness in finding and fixing bugs
- **Analysis (10%):** Quality of explanations and performance analysis

---

## Additional Resources

- C++ Concurrency in Action (Anthony Williams)
- [cppreference.com - Thread support library](https://en.cppreference.com/w/cpp/thread)
- Intel Threading Building Blocks documentation
- [Herb Sutter - Effective Concurrency](http://herbsutter.com/category/effective-concurrency/)

---

## Time Expectations

- MCQs: 30-45 minutes
- Code Review: 1-2 hours
- Implementation: 3-4 hours
- Debugging: 1-2 hours
- Performance Optimization: 1-2 hours

Good luck! Remember to test thoroughly with thread sanitizer.
