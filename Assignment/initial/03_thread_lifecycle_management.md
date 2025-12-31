# Assignment 03: Thread Lifecycle Management

## Overview
Master thread lifecycle management including join/detach operations, RAII patterns, exception safety, and best practices for thread ownership.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 5-6 hours  
**Prerequisites:** Assignments 01-02

---

## Learning Objectives
- Understand thread states and lifecycle
- Master join() and detach() operations
- Implement RAII wrappers for automatic thread management
- Handle exceptions safely with threads
- Understand ownership and transfer of threads

---

## Part 1: Multiple Choice Questions (12 MCQs)

### Q1. What happens if a std::thread object is destroyed while still joinable?
A) The thread is automatically joined  
B) The thread is automatically detached  
C) std::terminate() is called  
D) Nothing, it's safe  

**Answer:** C - Destroying a joinable thread terminates the program

### Q2. A thread becomes unjoinable when:
A) It finishes execution  
B) You call join() or detach() on it  
C) You move it to another thread object  
D) All of the above  

**Answer:** D

### Q3. Can you join a thread multiple times?
A) Yes, it's safe  
B) No, it causes undefined behavior  
C) Yes, but only twice  
D) It depends on the platform  

**Answer:** B - Joining an unjoinable thread causes undefined behavior

### Q4. What is the main risk of detaching a thread?
A) Memory leaks  
B) Accessing destroyed data if the thread outlives objects it references  
C) CPU overhead  
D) It cannot be detached  

**Answer:** B

### Q5. If you detach a thread, can you later join it?
A) Yes, using a special API  
B) No, once detached it cannot be joined  
C) Yes, but only within 1 second  
D) Only if it hasn't finished yet  

**Answer:** B

### Q6. std::thread::joinable() returns true when:
A) The thread has finished execution  
B) The thread represents an active thread of execution  
C) The thread is waiting  
D) The thread has been detached  

**Answer:** B

### Q7. Which RAII pattern ensures a thread is always joined?
A) Smart pointers  
B) Thread guard/joiner class in destructor  
C) std::unique_ptr<std::thread>  
D) std::shared_ptr<std::thread>  

**Answer:** B

### Q8. If an exception is thrown after creating a thread but before joining:
A) The thread is automatically joined  
B) The program may terminate if thread is still joinable  
C) The thread continues running  
D) The exception waits for the thread  

**Answer:** B

### Q9. Can you transfer ownership of a thread?
A) No, threads cannot be transferred  
B) Yes, using std::move()  
C) Yes, using copy constructor  
D) Only with detached threads  

**Answer:** B

### Q10. What is the effect of calling join() on a thread that has already finished?
A) It returns immediately  
B) It blocks forever  
C) Undefined behavior  
D) It throws an exception  

**Answer:** A - join() returns immediately if thread has finished

### Q11. Which is better for long-running background tasks?
A) Always use join()  
B) Always use detach()  
C) Depends on the use case and data lifetime  
D) Use neither  

**Answer:** C

### Q12. A thread's destructor is called when:
A) The thread finishes execution  
B) You call join() or detach()  
C) The thread object goes out of scope  
D) The program exits  

**Answer:** C

---

## Part 2: Code Review Exercises

### Exercise 2.1: Exception Safety Issue

```cpp
#include <iostream>
#include <thread>
#include <stdexcept>

void doWork() {
    std::cout << "Working...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Done!\n";
}

void riskyFunction() {
    std::thread worker(doWork);
    
    // Some risky operation
    if (rand() % 2 == 0) {
        throw std::runtime_error("Something went wrong!");
    }
    
    worker.join();
}

int main() {
    try {
        riskyFunction();
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
    
    std::cout << "Program finished\n";
    return 0;
}
```

**Questions:**
1. What is the critical bug here?
2. What happens if the exception is thrown?
3. Will the program crash?
4. Provide a RAII-based fix

**Solution Approach:**
```cpp
class ThreadGuard {
    std::thread& t;
public:
    explicit ThreadGuard(std::thread& t_) : t(t_) {}
    ~ThreadGuard() {
        if (t.joinable()) {
            t.join();
        }
    }
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};

void riskyFunction() {
    std::thread worker(doWork);
    ThreadGuard guard(worker);  // RAII ensures join
    
    if (rand() % 2 == 0) {
        throw std::runtime_error("Something went wrong!");
    }
    // worker joined automatically by guard destructor
}
```

---

### Exercise 2.2: Detach Danger

```cpp
#include <iostream>
#include <thread>
#include <string>

void processString(const std::string& str) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Processing: " << str << "\n";
}

void dangerousDetach() {
    std::string data = "Important data";
    
    std::thread worker(processString, std::ref(data));
    worker.detach();  // Detach and return immediately
    
    // Function returns, 'data' is destroyed
    // But worker thread still references it!
}

int main() {
    dangerousDetach();
    
    std::cout << "Main continues...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}
```

**Questions:**
1. What is the bug here?
2. Why is it dangerous to detach in this case?
3. When is detach() actually safe to use?
4. How would you fix this specific case?

**Safe Alternatives:**
- Pass by value instead of reference
- Use shared ownership (std::shared_ptr)
- Don't detach; use join or thread pool

---

### Exercise 2.3: Move Semantics and Ownership

```cpp
#include <iostream>
#include <thread>
#include <vector>

void worker(int id) {
    std::cout << "Worker " << id << "\n";
}

std::thread createThread(int id) {
    return std::thread(worker, id);  // Return by value
}

int main() {
    std::vector<std::thread> threads;
    
    // Create threads and store them
    for (int i = 0; i < 5; ++i) {
        threads.push_back(createThread(i));  // Move semantics
    }
    
    // What about these threads?
    std::thread t1(worker, 100);
    std::thread t2 = std::move(t1);  // Transfer ownership
    
    // Is t1 joinable now?
    std::cout << "t1 joinable: " << t1.joinable() << "\n";
    std::cout << "t2 joinable: " << t2.joinable() << "\n";
    
    // Join all
    for (auto& t : threads) {
        t.join();
    }
    
    if (t2.joinable()) {
        t2.join();
    }
    
    return 0;
}
```

**Questions:**
1. Explain the ownership transfer from t1 to t2
2. What is the state of t1 after the move?
3. Is it safe to call join() on t1?
4. Why use std::move explicitly here?

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Scoped Thread (RAII Wrapper)

Implement a complete RAII thread wrapper:

```cpp
#include <thread>
#include <iostream>

class ScopedThread {
public:
    // Constructor: accept thread by move
    explicit ScopedThread(std::thread t) : t_(std::move(t)) {
        if (!t_.joinable()) {
            throw std::logic_error("No thread");
        }
    }
    
    // Destructor: automatically join
    ~ScopedThread() {
        // Your implementation
    }
    
    // Delete copy operations
    ScopedThread(const ScopedThread&) = delete;
    ScopedThread& operator=(const ScopedThread&) = delete;
    
    // Allow move operations (optional, advanced)
    ScopedThread(ScopedThread&& other) noexcept 
        : t_(std::move(other.t_)) {}
    
    ScopedThread& operator=(ScopedThread&& other) noexcept {
        // Your implementation:
        // Join current thread if joinable
        // Then move from other
    }
    
    // Get the underlying thread (advanced)
    std::thread& get() { return t_; }
    
private:
    std::thread t_;
};

// Test it
void testScopedThread() {
    ScopedThread st(std::thread([] {
        std::cout << "Worker thread\n";
    }));
    
    // Thread automatically joined when st goes out of scope
    
    throw std::runtime_error("Exception!"); // Thread still joined!
}
```

**Requirements:**
- Join thread in destructor
- Exception-safe
- Support move semantics
- Prevent double-join

---

### Exercise 3.2: Thread Pool with Lifecycle Management

Implement a thread pool that properly manages thread lifecycles:

```cpp
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPoolLifecycle {
public:
    ThreadPoolLifecycle(size_t numThreads) : stop_(false) {
        // Your implementation:
        // Create worker threads
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                this->workerLoop();
            });
        }
    }
    
    ~ThreadPoolLifecycle() {
        // Your implementation:
        // 1. Signal all threads to stop
        // 2. Wake up all waiting threads
        // 3. Join all threads (ensuring no exception)
    }
    
    void submit(std::function<void()> task) {
        // Your implementation:
        // Add task to queue with proper locking
    }
    
    size_t activeThreads() const {
        return activeCount_;
    }
    
private:
    void workerLoop() {
        // Your implementation:
        // 1. Wait for tasks
        // 2. Execute tasks
        // 3. Check stop flag
        // 4. Update active count
    }
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    
    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    std::atomic<size_t> activeCount_{0};
};

// Test
void testThreadPool() {
    ThreadPoolLifecycle pool(4);
    
    for (int i = 0; i < 20; ++i) {
        pool.submit([i] {
            std::cout << "Task " << i << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Destructor ensures all threads complete and are joined
}
```

**Key Points:**
- All threads must be joined before destruction
- No thread should be left running
- Handle shutdown gracefully
- Exception-safe

---

### Exercise 3.3: Joining Thread Wrapper

Implement a wrapper that automatically joins on destruction OR allows explicit join:

```cpp
#include <thread>
#include <optional>

class JoiningThread {
public:
    JoiningThread() = default;
    
    template<typename Func, typename... Args>
    explicit JoiningThread(Func&& func, Args&&... args)
        : thread_(std::forward<Func>(func), std::forward<Args>(args)...) {}
    
    JoiningThread(JoiningThread&& other) noexcept = default;
    JoiningThread& operator=(JoiningThread&& other) noexcept {
        // Your implementation:
        // Join current thread first, then move
    }
    
    ~JoiningThread() {
        // Your implementation: join if joinable
    }
    
    void join() {
        // Your implementation: explicit join
    }
    
    bool joinable() const {
        return thread_.joinable();
    }
    
    // Delete copy
    JoiningThread(const JoiningThread&) = delete;
    JoiningThread& operator=(const JoiningThread&) = delete;
    
private:
    std::thread thread_;
};

// Usage
void testJoiningThread() {
    JoiningThread jt([] {
        std::cout << "Working...\n";
    });
    
    // Option 1: explicit join
    jt.join();
    
    // Option 2: automatic join in destructor
    
    JoiningThread jt2([] {
        std::cout << "More work...\n";
    });
    // Automatically joined when jt2 goes out of scope
}
```

---

### Exercise 3.4: Thread State Monitor

Track thread states through lifecycle:

```cpp
#include <thread>
#include <iostream>
#include <chrono>
#include <string>

enum class ThreadState {
    Created,
    Running,
    Finished,
    Joined,
    Detached
};

class MonitoredThread {
public:
    template<typename Func, typename... Args>
    MonitoredThread(Func&& func, Args&&... args) 
        : state_(ThreadState::Created) {
        
        // Wrap function to update state
        thread_ = std::thread([this, func = std::forward<Func>(func), 
                                args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            state_ = ThreadState::Running;
            std::apply(func, std::move(args));
            state_ = ThreadState::Finished;
        });
    }
    
    void join() {
        if (thread_.joinable()) {
            thread_.join();
            state_ = ThreadState::Joined;
        }
    }
    
    void detach() {
        if (thread_.joinable()) {
            thread_.detach();
            state_ = ThreadState::Detached;
        }
    }
    
    ThreadState getState() const { return state_; }
    
    std::string getStateString() const {
        switch (state_) {
            case ThreadState::Created: return "Created";
            case ThreadState::Running: return "Running";
            case ThreadState::Finished: return "Finished";
            case ThreadState::Joined: return "Joined";
            case ThreadState::Detached: return "Detached";
        }
        return "Unknown";
    }
    
private:
    std::thread thread_;
    std::atomic<ThreadState> state_;
};

// Test
void testMonitoredThread() {
    MonitoredThread mt([] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    
    std::cout << "State: " << mt.getStateString() << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "State: " << mt.getStateString() << "\n";
    
    mt.join();
    std::cout << "State: " << mt.getStateString() << "\n";
}
```

---

## Part 4: Debugging Exercises

### Exercise 4.1: Find the Termination Bug

```cpp
#include <iostream>
#include <thread>
#include <vector>

void worker(int id) {
    std::cout << "Worker " << id << " starting\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Worker " << id << " done\n";
}

void buggyCode() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker, i);
    }
    
    // Oops, forgot to join threads!
    // What happens when threads vector is destroyed?
}

int main() {
    buggyCode();
    std::cout << "This line may never execute!\n";
    return 0;
}
```

**Tasks:**
1. Run this program. What happens?
2. Why does std::terminate() get called?
3. Fix it properly with RAII
4. Add exception handling

---

### Exercise 4.2: Double Join Bug

```cpp
#include <iostream>
#include <thread>

void work() {
    std::cout << "Working...\n";
}

int main() {
    std::thread t(work);
    
    t.join();
    std::cout << "First join completed\n";
    
    // Accidentally join again
    if (t.joinable()) {  // This check helps!
        t.join();
    } else {
        std::cout << "Thread not joinable\n";
    }
    
    return 0;
}
```

**Questions:**
1. What prevents the double join here?
2. What if we didn't check joinable()?
3. How does RAII help prevent this?

---

## Part 5: Performance Analysis

### Exercise 5.1: Join vs Detach Performance

Compare performance implications:

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

void quickTask() {
    volatile int sum = 0;
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
}

void benchmarkJoin(int numThreads) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(quickTask);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Join approach: " << duration.count() << " μs\n";
}

void benchmarkDetach(int numThreads) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numThreads; ++i) {
        std::thread t(quickTask);
        t.detach();
    }
    
    // Give detached threads time to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Detach approach: " << duration.count() << " μs\n";
}

int main() {
    const int numThreads = 100;
    
    benchmarkJoin(numThreads);
    benchmarkDetach(numThreads);
    
    return 0;
}
```

**Analysis:**
1. Which is faster and why?
2. What are the trade-offs?
3. When is each appropriate?

---

### Exercise 5.2: RAII Overhead

Measure overhead of RAII wrappers:

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

void emptyTask() {}

void benchmarkRawThreads(int count) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < count; ++i) {
        threads.emplace_back(emptyTask);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Raw threads: " << duration.count() << " μs\n";
}

void benchmarkScopedThreads(int count) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<ScopedThread> threads;  // From Exercise 3.1
    for (int i = 0; i < count; ++i) {
        threads.emplace_back(std::thread(emptyTask));
    }
    
    // Automatic join via destructors
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Scoped threads: " << duration.count() << " μs\n";
}

int main() {
    benchmarkRawThreads(1000);
    benchmarkScopedThreads(1000);
    
    return 0;
}
```

**Questions:**
1. Is there measurable overhead?
2. Is the safety worth any overhead?
3. Does compiler optimization eliminate the difference?

---

## Submission Guidelines

Submit in a single ZIP file:
1. **answers.md** - MCQ answers with explanations
2. **code_review/** - Fixed code for Part 2
3. **implementations/** - All Part 3 implementations
4. **debugging/** - Part 4 fixes with explanations
5. **performance/** - Part 5 code, data, and analysis report

---

## Evaluation Criteria

- **Correctness (35%):** Code works correctly and safely
- **Exception Safety (25%):** Proper RAII and exception handling
- **Understanding (20%):** MCQs and explanations demonstrate depth
- **Code Quality (20%):** Clean, well-designed implementations

---

## Key Takeaways

✅ Always join or detach before thread destruction  
✅ Use RAII for automatic thread management  
✅ Detach only when thread doesn't access local data  
✅ Check joinable() before join/detach  
✅ std::move for transferring thread ownership  
✅ Exception safety requires careful lifecycle management  

---

## Common Pitfalls

❌ Forgetting to join/detach threads  
❌ Detaching threads that reference local variables  
❌ Double-joining a thread  
❌ Not checking joinable() status  
❌ Poor exception safety around threads  

---

## Next Steps

Proceed to **Assignment 04: Mutexes and Locks** to learn synchronization primitives.

---

## Resources

- [cppreference - std::thread](https://en.cppreference.com/w/cpp/thread/thread)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) - Chapter 2
- [Herb Sutter - Prefer Using Active Objects Instead of Naked Threads](https://herbsutter.com/2010/07/12/prefer-using-active-objects-instead-of-naked-threads/)

Good luck!