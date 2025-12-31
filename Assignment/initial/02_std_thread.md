# Assignment 02: std::thread Deep Dive

## Overview
Master the std::thread class, including advanced argument passing, move semantics, exception handling, and thread management patterns.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 4-5 hours  
**Prerequisites:** Assignment 01 (Thread Basics)

---

## Learning Objectives
- Master std::thread API and capabilities
- Handle complex argument passing scenarios
- Understand thread move semantics
- Manage thread exceptions and errors
- Implement common threading patterns

---

## Part 1: Multiple Choice Questions (10 MCQs)

### Q1. std::thread objects are:
A) Copyable and movable  
B) Copyable but not movable  
C) Movable but not copyable  
D) Neither copyable nor movable  

**Answer:** C - Threads can be moved but not copied

### Q2. What happens when you pass an argument by value to a thread?
A) It's passed by reference automatically  
B) A copy is made and passed to the thread  
C) It causes a compilation error  
D) It's converted to a pointer  

**Answer:** B

### Q3. To pass a reference to a thread, you must use:
A) `&` operator  
B) `std::ref()` or `std::cref()`  
C) Pointers only  
D) It's automatic  

**Answer:** B

### Q4. What is the purpose of `std::thread::joinable()`?
A) To check if the thread has finished  
B) To check if the thread can be joined (has an active thread of execution)  
C) To force a thread to be joinable  
D) To check thread priority  

**Answer:** B

### Q5. If you move a std::thread object, the moved-from object:
A) Still controls the thread  
B) Is in a valid but unspecified state and not joinable  
C) Causes undefined behavior  
D) Becomes nullptr  

**Answer:** B

### Q6. Can you get the ID of a std::thread?
A) No, thread IDs are not accessible  
B) Yes, using `std::thread::get_id()`  
C) Only after joining  
D) Only in debug mode  

**Answer:** B

### Q7. What is std::this_thread::sleep_for() used for?
A) To pause all threads  
B) To pause the calling thread for a specified duration  
C) To wait for another thread  
D) To hibernate the process  

**Answer:** B

### Q8. If an exception is thrown in a thread and not caught:
A) The main thread catches it  
B) `std::terminate()` is called for that thread  
C) The thread silently exits  
D) It propagates to main  

**Answer:** B

### Q9. std::thread::native_handle() returns:
A) The thread object  
B) A platform-specific thread handle (like pthread_t)  
C) The thread ID  
D) Always nullptr  

**Answer:** B

### Q10. Which is correct about thread arguments?
A) All arguments are evaluated in the calling thread before the new thread starts  
B) Arguments are evaluated in the new thread  
C) Only primitive types can be passed  
D) References are automatically handled  

**Answer:** A

---

## Part 2: Code Review Exercises

### Exercise 2.1: Argument Passing Bug

```cpp
#include <iostream>
#include <thread>
#include <string>

void printMessage(const std::string& msg) {
    std::cout << msg << std::endl;
}

int main() {
    char buffer[] = "Hello";
    std::thread t(printMessage, buffer);  // Potential issue!
    
    buffer[0] = 'J';  // Modified before thread reads it
    
    t.join();
    return 0;
}
```

**Questions:**
1. What could go wrong here?
2. Why is passing `buffer` directly problematic?
3. How would you fix this to ensure thread safety?
4. What is the difference between passing `buffer` and `std::string(buffer)`?

---

### Exercise 2.2: Move Semantics Bug

```cpp
#include <iostream>
#include <thread>
#include <vector>

void process(int id) {
    std::cout << "Processing " << id << "\n";
}

int main() {
    std::vector<std::thread> threads;
    
    std::thread t1(process, 1);
    threads.push_back(t1);  // Error: cannot copy thread!
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    return 0;
}
```

**Questions:**
1. Why doesn't this compile?
2. How do you properly add threads to a vector?
3. Provide the corrected code
4. What about using `emplace_back` vs `push_back`?

**Solution:**
```cpp
threads.push_back(std::move(t1));  // Move instead of copy
// Or better:
threads.emplace_back(process, 1);  // Construct in place
```

---

### Exercise 2.3: Reference and Const Issues

```cpp
#include <iostream>
#include <thread>

void increment(int& value) {
    ++value;
}

void printConst(const int& value) {
    std::cout << value << std::endl;
}

int main() {
    int x = 10;
    
    // Issue 1: Forgetting std::ref
    std::thread t1(increment, x);
    t1.join();
    std::cout << "x after t1: " << x << "\n";  // Still 10!
    
    // Issue 2: Temporary value
    std::thread t2(printConst, 42);  // Is this safe?
    t2.join();
    
    return 0;
}
```

**Questions:**
1. Why is `x` still 10 after t1?
2. Is passing a temporary (42) to a const reference safe?
3. Fix the code properly
4. When should you use std::cref vs std::ref?

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Thread Manager Class

Implement a RAII thread manager that ensures threads are always joined:

```cpp
#include <thread>
#include <vector>
#include <stdexcept>

class ThreadManager {
public:
    ThreadManager() = default;
    
    // Add a thread to be managed
    template<typename Func, typename... Args>
    void addThread(Func&& func, Args&&... args) {
        // Your implementation
    }
    
    // Join all threads
    void joinAll() {
        // Your implementation
    }
    
    // Destructor ensures all threads are joined
    ~ThreadManager() {
        // Your implementation
    }
    
    // Non-copyable
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;
    
    // Movable
    ThreadManager(ThreadManager&&) = default;
    ThreadManager& operator=(ThreadManager&&) = default;
    
private:
    std::vector<std::thread> threads;
};

// Test the manager
void testThreadManager() {
    ThreadManager manager;
    
    manager.addThread([] {
        std::cout << "Thread 1\n";
    });
    
    manager.addThread([](int x) {
        std::cout << "Thread 2: " << x << "\n";
    }, 42);
    
    // Threads automatically joined when manager goes out of scope
}
```

**Requirements:**
- Exception safety: if adding thread throws, existing threads are still joined
- Support for any callable with any arguments
- Properly handle move semantics
- Add functionality to get thread count

---

### Exercise 3.2: Thread Pool (Simple Version)

Create a basic fixed-size thread pool:

```cpp
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class SimpleThreadPool {
public:
    SimpleThreadPool(size_t numThreads) {
        // Your implementation: create worker threads
    }
    
    ~SimpleThreadPool() {
        // Your implementation: shutdown and join all threads
    }
    
    void submit(std::function<void()> task) {
        // Your implementation: add task to queue
    }
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;
    
    void workerThread() {
        // Your implementation: worker loop
    }
};

// Test
void testThreadPool() {
    SimpleThreadPool pool(4);
    
    for (int i = 0; i < 10; ++i) {
        pool.submit([i] {
            std::cout << "Task " << i << " on thread " 
                      << std::this_thread::get_id() << "\n";
        });
    }
    
    // Pool destructor waits for all tasks
}
```

---

### Exercise 3.3: Thread with Return Value

Implement a wrapper that captures return values from threads:

```cpp
#include <thread>
#include <memory>
#include <stdexcept>

template<typename T>
class ThreadWithResult {
public:
    template<typename Func, typename... Args>
    ThreadWithResult(Func&& func, Args&&... args) {
        // Your implementation
        // Store result and any exception
    }
    
    T get() {
        // Wait for thread and return result
        // Rethrow any exception that occurred in thread
    }
    
private:
    std::thread thread;
    std::unique_ptr<T> result;
    std::exception_ptr exception;
};

// Usage example
int expensiveComputation(int x) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (x < 0) throw std::invalid_argument("Negative value");
    return x * x;
}

void testThreadWithResult() {
    ThreadWithResult<int> t(expensiveComputation, 5);
    
    // Do other work...
    
    try {
        int result = t.get();
        std::cout << "Result: " << result << "\n";
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
```

---

### Exercise 3.4: Thread Joiner (RAII)

```cpp
#include <thread>

class ThreadJoiner {
public:
    explicit ThreadJoiner(std::thread& t) : thread(t) {}
    
    ~ThreadJoiner() {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // Non-copyable, non-movable
    ThreadJoiner(const ThreadJoiner&) = delete;
    ThreadJoiner& operator=(const ThreadJoiner&) = delete;
    ThreadJoiner(ThreadJoiner&&) = delete;
    ThreadJoiner& operator=(ThreadJoiner&&) = delete;
    
private:
    std::thread& thread;
};

// Usage
void riskyFunction() {
    std::thread t([] {
        std::cout << "Working...\n";
    });
    
    ThreadJoiner joiner(t);
    
    // Even if exception thrown here, thread is joined
    if (someCondition) {
        throw std::runtime_error("Error!");
    }
    
    // Thread automatically joined when joiner destroyed
}
```

**Task:** Test this with exceptions and verify thread is always joined.

---

## Part 4: Debugging Exercises

### Exercise 4.1: Exception in Thread

```cpp
#include <iostream>
#include <thread>
#include <stdexcept>

void mayThrow(int value) {
    if (value < 0) {
        throw std::invalid_argument("Negative value not allowed");
    }
    std::cout << "Value: " << value << "\n";
}

int main() {
    try {
        std::thread t(mayThrow, -5);
        t.join();
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
    
    std::cout << "Program continues...\n";
    return 0;
}
```

**Questions:**
1. Run this program. What happens?
2. Is the exception caught in main?
3. How can you propagate exceptions from threads?
4. Implement a solution using std::exception_ptr

---

### Exercise 4.2: Thread Lifecycle Issues

```cpp
#include <iostream>
#include <thread>
#include <vector>

void processData(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Processed " << id << "\n";
}

void problematicCode() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(processData, i);
    }
    
    // Join only first 3 threads
    for (int i = 0; i < 3; ++i) {
        threads[i].join();
    }
    
    // What about threads[3] and threads[4]?
}

int main() {
    try {
        problematicCode();
        std::cout << "Success!\n";
    } catch (...) {
        std::cout << "Error occurred\n";
    }
    
    return 0;
}
```

**Tasks:**
1. What happens to unjoined threads?
2. Fix this to ensure all threads are joined
3. Make it exception-safe

---

## Part 5: Performance Analysis

### Exercise 5.1: Thread Creation vs Reuse

Compare creating new threads vs reusing threads:

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

void simpleTask() {
    volatile int sum = 0;
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
}

void benchmarkThreadCreation(int numTasks) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numTasks; ++i) {
        std::thread t(simpleTask);
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Creating threads: " << duration.count() << "ms\n";
}

void benchmarkThreadPool(int numTasks) {
    SimpleThreadPool pool(4);  // From Exercise 3.2
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numTasks; ++i) {
        pool.submit(simpleTask);
    }
    
    // Wait for completion
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Thread pool: " << duration.count() << "ms\n";
}

int main() {
    const int numTasks = 100;
    
    benchmarkThreadCreation(numTasks);
    benchmarkThreadPool(numTasks);
    
    return 0;
}
```

**Analysis:**
1. Which approach is faster?
2. Why is there a difference?
3. When should you use each approach?

---

### Exercise 5.2: Optimal Thread Count

Find optimal thread count for parallel sum:

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <chrono>

void parallelSum(const std::vector<int>& data, size_t numThreads) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    std::vector<long long> partialSums(numThreads, 0);
    
    size_t chunkSize = data.size() / numThreads;
    
    for (size_t i = 0; i < numThreads; ++i) {
        size_t startIdx = i * chunkSize;
        size_t endIdx = (i == numThreads - 1) ? data.size() : (i + 1) * chunkSize;
        
        threads.emplace_back([&data, &partialSums, i, startIdx, endIdx] {
            for (size_t j = startIdx; j < endIdx; ++j) {
                partialSums[i] += data[j];
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    long long total = std::accumulate(partialSums.begin(), partialSums.end(), 0LL);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Threads: " << numThreads 
              << ", Time: " << duration.count() << "ms"
              << ", Result: " << total << "\n";
}

int main() {
    std::vector<int> data(10000000, 1);
    
    std::cout << "Hardware concurrency: " 
              << std::thread::hardware_concurrency() << "\n\n";
    
    for (size_t threads : {1, 2, 4, 8, 16, 32}) {
        parallelSum(data, threads);
    }
    
    return 0;
}
```

**Tasks:**
1. Run and analyze results
2. Plot speedup vs number of threads
3. Explain why performance plateaus
4. What is the optimal thread count?

---

## Submission Guidelines

Same format as Assignment 01:
- Source code with all implementations
- MCQ answers with explanations
- Performance analysis with graphs
- Bug fixes with explanations

---

## Evaluation Criteria

- **Correctness (35%):** All code works as specified
- **Understanding (30%):** MCQs and explanations show depth
- **Code Quality (20%):** Clean, well-organized code
- **Analysis (15%):** Insightful performance analysis

---

## Key Takeaways

✅ std::thread is move-only (not copyable)  
✅ Arguments are copied by default; use std::ref for references  
✅ Threads must be joined or detached before destruction  
✅ Exceptions in threads don't propagate to parent  
✅ Thread pools reduce overhead of frequent thread creation  

---

## Next Steps

Proceed to **Assignment 03: Thread Lifecycle Management** to learn about join, detach, and advanced lifecycle patterns.

---

## Resources

- [cppreference - std::thread](https://en.cppreference.com/w/cpp/thread/thread)
- [Effective Modern C++](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/) - Item 37
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) - Chapter 2

Good luck!