# Assignment 09: Futures and Promises

## Overview
Master asynchronous programming with std::future, std::promise, std::async, and std::packaged_task for task-based parallelism.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 5-6 hours  
**Prerequisites:** Assignments 01-08

---

## Learning Objectives
- Use std::future and std::promise for async results
- Master std::async for easy parallelism
- Understand std::packaged_task
- Handle exceptions across threads
- Compare with manual thread management

---

## Part 1: Multiple Choice Questions (10 MCQs)

### Q1. std::future represents:
A) A past event  
B) A value that will be available in the future  
C) A thread  
D) A mutex  

**Answer:** B

### Q2. How many times can you call get() on a std::future?
A) Unlimited  
B) Once (moves the value out)  
C) Twice  
D) Never  

**Answer:** B

### Q3. std::promise is used to:
A) Get values from futures  
B) Set values that futures will retrieve  
C) Create threads  
D) Lock mutexes  

**Answer:** B

### Q4. std::async with std::launch::async policy:
A) Might run synchronously  
B) Guarantees a new thread  
C) Never runs  
D) Uses thread pool  

**Answer:** B

### Q5. std::async with std::launch::deferred policy:
A) Runs immediately  
B) Runs lazily when get() is called  
C) Never runs  
D) Runs in background  

**Answer:** B

### Q6. If an exception is thrown in a task, it:
A) Terminates the program  
B) Is stored and rethrown when get() is called  
C) Is ignored  
D) Crashes the thread  

**Answer:** B

### Q7. std::packaged_task is:
A) A callable object that stores its result in a future  
B) A thread  
C) A mutex  
D) Same as std::promise  

**Answer:** A

### Q8. future.wait() vs future.get():
A) Same thing  
B) wait() doesn't retrieve value, get() does  
C) wait() is faster  
D) get() doesn't block  

**Answer:** B

### Q9. future.valid() returns false when:
A) The result is not ready  
B) After get() has been called or future was moved  
C) There's an error  
D) Never  

**Answer:** B

### Q10. std::async default launch policy is:
A) Always async  
B) Always deferred  
C) Implementation-defined (async | deferred)  
D) Sequential  

**Answer:** C

---

## Part 2: Code Review Exercises

### Exercise 2.1: Basic Promise-Future

```cpp
#include <iostream>
#include <thread>
#include <future>

void compute(std::promise<int> prom) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    prom.set_value(42);  // Set the result
}

int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    std::thread t(compute, std::move(prom));
    
    std::cout << "Waiting for result...\n";
    int result = fut.get();  // Blocks until ready
    std::cout << "Result: " << result << "\n";
    
    t.join();
    return 0;
}
```

**Questions:**
1. Why move the promise?
2. What happens if set_value is never called?
3. Can you call get() twice?

---

### Exercise 2.2: Async vs Manual Threads

```cpp
#include <iostream>
#include <future>
#include <thread>

int compute() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 42;
}

void manualThreadWay() {
    int result;
    std::thread t([&result] {
        result = compute();
    });
    t.join();
    std::cout << "Manual: " << result << "\n";
}

void asyncWay() {
    std::future<int> fut = std::async(std::launch::async, compute);
    std::cout << "Async: " << fut.get() << "\n";  // Automatic join!
}

int main() {
    manualThreadWay();
    asyncWay();  // Much simpler!
    return 0;
}
```

**Questions:**
1. Which is simpler?
2. When does the async thread join?
3. What about exception handling?

---

### Exercise 2.3: Exception Propagation

```cpp
#include <iostream>
#include <future>
#include <stdexcept>

int riskyComputation() {
    throw std::runtime_error("Something went wrong!");
    return 42;
}

int main() {
    std::future<int> fut = std::async(std::launch::async, riskyComputation);
    
    try {
        int result = fut.get();  // Exception rethrown here!
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
    
    return 0;
}
```

**Questions:**
1. How is exception propagated?
2. What if you don't call get()?
3. Compare with manual thread exception handling

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Parallel Sum with Futures

```cpp
#include <iostream>
#include <vector>
#include <future>
#include <numeric>

int sumRange(const std::vector<int>& data, size_t start, size_t end) {
    return std::accumulate(data.begin() + start, data.begin() + end, 0);
}

int parallelSum(const std::vector<int>& data, size_t numThreads) {
    // Your implementation:
    // 1. Divide data into chunks
    // 2. Launch async tasks for each chunk
    // 3. Collect results from futures
    // 4. Return total sum
    
    std::vector<std::future<int>> futures;
    size_t chunkSize = data.size() / numThreads;
    
    for (size_t i = 0; i < numThreads; ++i) {
        size_t start = i * chunkSize;
        size_t end = (i == numThreads - 1) ? data.size() : (i + 1) * chunkSize;
        
        futures.push_back(std::async(std::launch::async, sumRange, 
                                      std::cref(data), start, end));
    }
    
    int total = 0;
    for (auto& fut : futures) {
        total += fut.get();
    }
    
    return total;
}

void test() {
    std::vector<int> data(10000000, 1);
    int result = parallelSum(data, 4);
    std::cout << "Sum: " << result << "\n";
}
```

---

### Exercise 3.2: Promise-Based Thread Pool

```cpp
#include <iostream>
#include <future>
#include <queue>
#include <thread>
#include <functional>
#include <vector>

class PromiseThreadPool {
public:
    PromiseThreadPool(size_t numThreads) : stop_(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                workerLoop();
            });
        }
    }
    
    ~PromiseThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            stop_ = true;
        }
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            worker.join();
        }
    }
    
    template<typename Func, typename... Args>
    auto submit(Func&& func, Args&&... args) 
        -> std::future<typename std::result_of<Func(Args...)>::type> {
        
        using ReturnType = typename std::result_of<Func(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );
        
        std::future<ReturnType> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            tasks_.emplace([task] { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
private:
    void workerLoop() {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                condition_.wait(lock, [this] {
                    return stop_ || !tasks_.empty();
                });
                
                if (stop_ && tasks_.empty()) {
                    return;
                }
                
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            
            task();
        }
    }
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_;
};

// Test
void testPromisePool() {
    PromiseThreadPool pool(4);
    
    std::vector<std::future<int>> results;
    
    for (int i = 0; i < 10; ++i) {
        results.push_back(pool.submit([i] {
            return i * i;
        }));
    }
    
    for (auto& fut : results) {
        std::cout << fut.get() << " ";
    }
    std::cout << "\n";
}
```

---

### Exercise 3.3: Async File Processing

```cpp
#include <iostream>
#include <fstream>
#include <future>
#include <vector>
#include <string>

std::string processFile(const std::string& filename) {
    // Simulate file processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return "Processed: " + filename;
}

void processFilesAsync(const std::vector<std::string>& files) {
    std::vector<std::future<std::string>> futures;
    
    // Launch all async tasks
    for (const auto& file : files) {
        futures.push_back(std::async(std::launch::async, processFile, file));
    }
    
    // Collect results
    for (auto& fut : futures) {
        std::cout << fut.get() << "\n";
    }
}

void test() {
    std::vector<std::string> files = {
        "file1.txt", "file2.txt", "file3.txt", "file4.txt"
    };
    
    processFilesAsync(files);
}
```

---

### Exercise 3.4: Timeout with Future

```cpp
#include <iostream>
#include <future>
#include <chrono>

int longComputation() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 42;
}

void testTimeout() {
    std::future<int> fut = std::async(std::launch::async, longComputation);
    
    // Wait with timeout
    auto status = fut.wait_for(std::chrono::seconds(2));
    
    if (status == std::future_status::ready) {
        std::cout << "Result: " << fut.get() << "\n";
    } else if (status == std::future_status::timeout) {
        std::cout << "Timeout! Still waiting...\n";
        // Can wait again or cancel (future will block on destruction)
    }
}
```

---

## Part 4: Debugging Exercises

### Exercise 4.1: Broken Promise

```cpp
#include <future>

void brokenPromise() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    // Promise destroyed without setting value!
    // Future will throw std::future_error
    
    try {
        int result = fut.get();
    } catch (const std::future_error& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}
```

**Questions:**
1. What happens when promise is destroyed without setting value?
2. How to handle this properly?

---

### Exercise 4.2: Double Get

```cpp
std::future<int> fut = std::async([] { return 42; });
int result1 = fut.get();  // OK
int result2 = fut.get();  // Throws std::future_error!
```

**Fix:** Only call get() once, or use shared_future.

---

## Part 5: Performance Analysis

### Exercise 5.1: Async vs Threads

Compare async vs manual threads for varying workloads.

---

### Exercise 5.2: Launch Policy Impact

```cpp
void benchmarkAsync() {
    std::future<int> fut = std::async(std::launch::async, compute);
    fut.get();
}

void benchmarkDeferred() {
    std::future<int> fut = std::async(std::launch::deferred, compute);
    fut.get();  // Runs synchronously here
}
```

---

## Submission Guidelines

Submit:
1. **answers.md** - MCQ answers
2. **code_review/** - Analysis
3. **implementations/** - All implementations
4. **debugging/** - Fixes
5. **performance/** - Benchmarks

---

## Evaluation Criteria

- **Correctness (35%):** Proper future/promise usage
- **Understanding (30%):** Clear explanations
- **Design (20%):** Good async patterns
- **Performance (15%):** Insightful analysis

---

## Key Takeaways

✅ std::async for simple parallelism  
✅ std::future for async results  
✅ std::promise for manual control  
✅ Exceptions propagated automatically  
✅ get() can only be called once  

---

## Common Pitfalls

❌ Calling get() multiple times  
❌ Not handling broken_promise exception  
❌ Forgetting to call get() (blocks on destruction)  
❌ Mixing manual threads when async suffices  

---

## Next Steps

Proceed to **Assignment 10: Thread-Safe Data Structures**.

---

## Resources

- [cppreference - std::future](https://en.cppreference.com/w/cpp/thread/future)
- [cppreference - std::async](https://en.cppreference.com/w/cpp/thread/async)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) - Chapter 4

Good luck!