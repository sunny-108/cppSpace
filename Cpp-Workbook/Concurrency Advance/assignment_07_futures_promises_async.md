# Assignment 07: Futures, Promises & Async Programming

## Overview
Master task-based parallelism using std::future, std::promise, std::async, and std::packaged_task. Design asynchronous systems and understand execution policies.

**Estimated Time:** 6-8 hours

---

## Learning Objectives
- Master std::future and std::promise
- Understand std::async execution policies
- Implement task-based parallelism patterns
- Design continuation-based workflows
- Build async pipelines and DAGs

---

## Part 1: Multiple Choice Questions (15 MCQs)

### Q1. `std::async` with `std::launch::async` guarantees:
A) Immediate execution  
B) Asynchronous execution in a separate thread  
C) Execution on the calling thread  
D) Parallel execution  

**Answer:** B

### Q2. The default launch policy for `std::async` is:
A) `std::launch::async`  
B) `std::launch::deferred`  
C) `std::launch::async | std::launch::deferred` (implementation chooses)  
D) No default  

**Answer:** C

### Q3. Calling `get()` on a std::future:
A) Can be called multiple times  
B) Can only be called once (moves the value out)  
C) Never blocks  
D) Requires a promise  

**Answer:** B

### Q4. `std::promise` is used to:
A) Create futures automatically  
B) Set a value that a future can retrieve  
C) Launch async tasks  
D) Create threads  

**Answer:** B

### Q5. `std::packaged_task` combines:
A) A function and a future  
B) Multiple futures  
C) Promises and threads  
D) Async and sync execution  

**Answer:** A

### Q6. `std::future::wait_for()` returns:
A) The value  
B) A status indicating ready/timeout/deferred  
C) A boolean  
D) void  

**Answer:** B

### Q7. If a std::promise is destroyed without setting a value:
A) Nothing happens  
B) The future throws `std::future_error` with `broken_promise`  
C) The program terminates  
D) The future waits forever  

**Answer:** B

### Q8. `std::shared_future` differs from `std::future` in that:
A) It's faster  
B) Multiple threads can call `get()` safely  
C) It doesn't block  
D) It uses less memory  

**Answer:** B

### Q9. With `std::launch::deferred`, the task executes:
A) Immediately  
B) In a thread pool  
C) When `get()` or `wait()` is called on the future  
D) Never  

**Answer:** C

### Q10. To pass exceptions across threads, use:
A) `throw` directly  
B) `std::promise::set_exception()` or `std::current_exception()`  
C) Global variables  
D) Thread-local storage  

**Answer:** B

### Q11. `std::async` returns:
A) `std::thread`  
B) `std::future`  
C) `std::promise`  
D) void  

**Answer:** B

### Q12. The main advantage of task-based parallelism over thread-based:
A) Always faster  
B) Decouples task creation from thread management  
C) Uses less memory  
D) Prevents deadlocks  

**Answer:** B

### Q13. `std::future::valid()` returns false when:
A) The value isn't ready yet  
B) After calling `get()` or moving the future  
C) Never  
D) When the promise is broken  

**Answer:** B

### Q14. To create a continuation (then) in C++, you:
A) Use std::future::then() (C++23 proposal)  
B) Must implement manually in C++17/20  
C) Use std::async recursively  
D) Use coroutines  

**Answer:** B

### Q15. `std::async` with many tasks can:
A) Always create unlimited threads  
B) Potentially exhaust resources (thread creation overhead)  
C) Never block  
D) Guarantee execution order  

**Answer:** B

---

## Part 2: Code Review Exercises

### Exercise 2.1: Promise/Future Misuse
```cpp
void problematicCode() {
    std::promise<int> promise;
    std::future<int> future = promise.get_future();
    
    std::thread t([&promise] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        promise.set_value(42);
        promise.set_value(100);  // Issue 1
    });
    
    int result = future.get();
    std::cout << result << "\n";
    
    int result2 = future.get();  // Issue 2
    std::cout << result2 << "\n";
    
    t.join();
}

void anotherIssue() {
    auto future = std::async(std::launch::async, [] {
        return 42;
    });
    // Issue 3: future destructor blocks here!
}

void danglingReference() {
    std::vector<int> data = {1, 2, 3, 4, 5};
    
    auto future = std::async(std::launch::deferred, [&data] {
        int sum = 0;
        for (int val : data) sum += val;
        return sum;
    });
    
    data.clear();  // Issue 4
    
    int result = future.get();
    std::cout << result << "\n";
}
```

**Questions:**
1. What happens with double `set_value()`?
2. What happens with double `get()`?
3. Explain the blocking destructor issue
4. Why is the reference dangerous?
5. Fix all issues

---

### Exercise 2.2: Async Exception Handling
```cpp
class DataProcessor {
public:
    std::future<int> processAsync(const std::string& data) {
        return std::async(std::launch::async, [data] {
            if (data.empty()) {
                throw std::invalid_argument("Empty data");
            }
            
            // Simulate processing
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return static_cast<int>(data.length());
        });
    }
};

void testExceptions() {
    DataProcessor processor;
    
    std::vector<std::future<int>> futures;
    futures.push_back(processor.processAsync("hello"));
    futures.push_back(processor.processAsync(""));
    futures.push_back(processor.processAsync("world"));
    
    for (auto& future : futures) {
        int result = future.get();  // How to handle exceptions properly?
        std::cout << "Result: " << result << "\n";
    }
}
```

**Questions:**
1. What happens when get() encounters an exception?
2. How do you handle exceptions from multiple futures?
3. Implement proper exception handling
4. Design a pattern to collect all results and errors
5. How would you implement a timeout?

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Task Scheduler with Futures
```cpp
class TaskScheduler {
public:
    TaskScheduler(size_t numThreads);
    ~TaskScheduler();
    
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>;
    
    // Submit with priority
    template<typename F, typename... Args>
    auto submitHigh(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;
    
    // Submit with dependency (run after another task completes)
    template<typename F>
    auto submitAfter(std::future<void>&& dependency, F&& f)
        -> std::future<typename std::invoke_result<F>::type>;
    
    void shutdown();
    void wait();  // Wait for all tasks
    
private:
    class Impl;  // Your implementation
    std::unique_ptr<Impl> pImpl;
};

// Test complex workflow
void testScheduler() {
    TaskScheduler scheduler(4);
    
    // Step 1: Load data
    auto loadFuture = scheduler.submit([] {
        std::cout << "Loading data...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return std::vector<int>{1, 2, 3, 4, 5};
    });
    
    // Step 2: Process (depends on load)
    auto processFuture = scheduler.submitAfter(
        std::async(std::launch::deferred, [&loadFuture] {
            loadFuture.wait();
        }),
        [data = loadFuture.get()] {
            std::cout << "Processing data...\n";
            int sum = 0;
            for (int v : data) sum += v;
            return sum;
        }
    );
    
    // Step 3: Save (depends on process)
    auto saveFuture = scheduler.submitAfter(
        std::async(std::launch::deferred, [&processFuture] {
            processFuture.wait();
        }),
        [result = processFuture.get()] {
            std::cout << "Saving result: " << result << "\n";
        }
    );
    
    saveFuture.wait();
    scheduler.shutdown();
}
```

**Requirements:**
- Implement using thread pool and futures
- Support task dependencies
- Handle exceptions properly
- Support priority queues
- Graceful shutdown

---

### Exercise 3.2: Async Pipeline
```cpp
template<typename T>
class AsyncPipeline {
public:
    using TransformFunc = std::function<T(const T&)>;
    
    AsyncPipeline& addStage(TransformFunc transform);
    
    std::future<T> process(const T& input);
    
    // Process multiple items in parallel
    std::vector<std::future<T>> processBatch(const std::vector<T>& inputs);
    
private:
    std::vector<TransformFunc> stages;
};

// Example usage
void testPipeline() {
    AsyncPipeline<int> pipeline;
    
    pipeline
        .addStage([](int x) { return x * 2; })
        .addStage([](int x) { return x + 10; })
        .addStage([](int x) { return x * x; });
    
    auto future = pipeline.process(5);
    std::cout << "Result: " << future.get() << "\n";  // ((5*2)+10)^2 = 400
    
    // Batch processing
    std::vector<int> inputs = {1, 2, 3, 4, 5};
    auto futures = pipeline.processBatch(inputs);
    
    for (auto& f : futures) {
        std::cout << f.get() << " ";
    }
    std::cout << "\n";
}
```

**Requirements:**
- Implement async pipeline stages
- Support batch processing
- Optimize for throughput
- Handle exceptions in any stage
- Benchmark vs sequential processing

---

### Exercise 3.3: Continuation Pattern (Manual Then)
```cpp
template<typename T>
class Future {
public:
    // Constructor from std::future
    explicit Future(std::future<T>&& f);
    
    // Get value (blocking)
    T get();
    
    // Then continuation
    template<typename F>
    auto then(F&& func) -> Future<typename std::invoke_result<F, T>::type>;
    
    // Error handling
    template<typename F>
    Future<T> onError(F&& errorHandler);
    
    // Wait for completion
    void wait();
    
    // Non-blocking check
    bool ready() const;
    
private:
    std::shared_future<T> future;
};

// Helper to create futures
template<typename F, typename... Args>
auto async(F&& f, Args&&... args) 
    -> Future<typename std::invoke_result<F, Args...>::type>;

// Example usage
void testContinuations() {
    auto result = async([] {
        return 42;
    })
    .then([](int x) {
        return x * 2;
    })
    .then([](int x) {
        return std::to_string(x);
    })
    .onError([](const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return std::string("error");
    });
    
    std::cout << "Final: " << result.get() << "\n";
}
```

**Requirements:**
- Implement continuation chaining
- Support error handling
- Avoid blocking intermediate stages
- Thread-safe implementation
- Compare with raw std::future

---

## Part 4: Debugging

### Exercise 4.1: Deadlock with Futures
```cpp
class CircularDependency {
    TaskScheduler scheduler;
    
public:
    void problematic() {
        auto future1 = scheduler.submit([this] {
            auto future2 = scheduler.submit([] {
                return 42;
            });
            return future2.get();  // Deadlock if thread pool full!
        });
        
        auto result = future1.get();
    }
};
```

**Tasks:**
1. Explain the deadlock scenario
2. Reproduce with small thread pool
3. Fix using nested task submission
4. Implement work stealing to avoid issue

---

## Part 5: Performance Optimization

### Exercise 5.1: Async vs Thread Pool
```cpp
class BenchmarkAsync {
public:
    // Naive approach: one async per task
    void benchmarkAsync(size_t numTasks);
    
    // Thread pool approach
    void benchmarkThreadPool(size_t numTasks);
    
    // Batch processing
    void benchmarkBatched(size_t numTasks, size_t batchSize);
    
    void runComparison();
};
```

**Analysis:**
1. Measure task overhead
2. Find optimal batch size
3. Test with various task durations
4. Graph results

---

## Submission Guidelines
Same as previous assignments.

---

## Evaluation Criteria
- **Correctness (30%):** Proper future/promise usage
- **Design (25%):** Task dependency handling
- **Performance (25%):** Efficient async patterns
- **Exception Handling (20%):** Robust error propagation

---

Good luck!
