# Programming Assignment - Item 37: Make std::threads Unjoinable on All Paths

## Learning Objectives
- Understand thread lifecycle and joinability
- Implement RAII wrappers for threads
- Handle exception safety with threads
- Prevent thread resource leaks

---

## Exercise 1: Thread RAII Wrapper (Easy)

### Problem
Implement a basic RAII wrapper for std::thread that automatically joins on destruction.

### Requirements
```cpp
class ThreadGuard {
public:
    explicit ThreadGuard(std::thread&& t);
    ~ThreadGuard();
    
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
    
    ThreadGuard(ThreadGuard&&) noexcept;
    ThreadGuard& operator=(ThreadGuard&&) noexcept;
    
private:
    std::thread thread;
};
```

### Example
```cpp
void doWork() {
    std::cout << "Working...\n";
}

{
    ThreadGuard guard(std::thread(doWork));
    // Do other work...
}  // Thread automatically joined here
```

### Hints
- Store thread as member
- Check joinable() in destructor
- Call join() if joinable

---

## Exercise 2: Detaching ThreadGuard (Easy)

### Problem
Extend ThreadGuard to support both joining and detaching policies.

### Requirements
```cpp
enum class ThreadPolicy {
    Join,
    Detach
};

class ConfigurableThreadGuard {
public:
    ConfigurableThreadGuard(std::thread&& t, ThreadPolicy policy);
    ~ConfigurableThreadGuard();
    
    void changePolicy(ThreadPolicy newPolicy);
    ThreadPolicy getPolicy() const;
    
private:
    std::thread thread;
    ThreadPolicy policy;
};
```

### Example
```cpp
ConfigurableThreadGuard guard(
    std::thread(backgroundTask),
    ThreadPolicy::Detach
);

// Can change policy before destruction
guard.changePolicy(ThreadPolicy::Join);
```

### Hints
- Check policy in destructor
- Either join() or detach()
- Cannot detach if already joined

---

## Exercise 3: Exception Safety (Medium)

### Problem
Demonstrate the problem of thread leaks with exceptions and show how RAII solves it.

### Requirements
```cpp
class ExceptionSafetyDemo {
public:
    // Unsafe: thread may not be joined if exception thrown
    static void unsafeVersion();
    
    // Safe: RAII ensures join even with exceptions
    static void safeVersion();
    
    // Demonstrate both
    static void compareVersions();
};
```

### Example
```cpp
ExceptionSafetyDemo::compareVersions();
// Output:
// Unsafe version: Thread leak! (may crash)
// Safe version: Thread joined properly
```

### Hints
- Throw exception after thread creation
- Show thread not joined in unsafe version
- Use ThreadGuard in safe version

---

## Exercise 4: ThreadPool with RAII (Hard)

### Problem
Implement a thread pool where all threads are properly joined on destruction.

### Requirements
```cpp
class RAIIThreadPool {
public:
    explicit RAIIThreadPool(size_t numThreads);
    ~RAIIThreadPool();  // Must join all threads
    
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;
    
    void shutdown();
    bool isShutdown() const;
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop{false};
};
```

### Example
```cpp
{
    RAIIThreadPool pool(4);
    
    auto future = pool.submit([]() { return 42; });
    int result = future.get();
    
    // Exception thrown here
    throw std::runtime_error("Error!");
    
}  // Pool destructor still joins all threads!
```

### Hints
- Join all workers in destructor
- Set stop flag first
- Notify all waiting threads

---

## Exercise 5: Scoped Thread (Medium)

### Problem
Create a scoped thread class similar to C++20's std::jthread but for C++17.

### Requirements
```cpp
class ScopedThread {
public:
    template<typename F, typename... Args>
    explicit ScopedThread(F&& f, Args&&... args);
    
    ~ScopedThread();
    
    ScopedThread(const ScopedThread&) = delete;
    ScopedThread& operator=(const ScopedThread&) = delete;
    
    ScopedThread(ScopedThread&&) noexcept = default;
    ScopedThread& operator=(ScopedThread&&) noexcept = default;
    
    void join();
    bool joinable() const;
    
private:
    std::thread thread;
};
```

### Example
```cpp
void task(int id) {
    std::cout << "Task " << id << "\n";
}

{
    ScopedThread t1(task, 1);
    ScopedThread t2(task, 2);
    
    // Both automatically joined at end of scope
}
```

### Hints
- Forward arguments to std::thread
- Always join in destructor
- Handle move semantics

---

## Exercise 6: Parallel Algorithm with RAII (Medium)

### Problem
Implement parallel_for_each using RAII threads that are guaranteed to be joined.

### Requirements
```cpp
template<typename Iterator, typename Function>
class ParallelForEach {
public:
    ParallelForEach(Iterator first, Iterator last, 
                    Function f, size_t numThreads);
    
    void execute();
    
private:
    Iterator first, last;
    Function func;
    size_t numThreads;
    std::vector<ScopedThread> threads;
};
```

### Example
```cpp
std::vector<int> data(1000);
std::iota(data.begin(), data.end(), 0);

ParallelForEach processor(
    data.begin(), 
    data.end(),
    [](int& x) { x *= 2; },
    4
);

processor.execute();  // All threads joined before return
```

### Hints
- Divide range among threads
- Use ScopedThread for workers
- Automatic cleanup via RAII

---

## Exercise 7: Thread Lifecycle Manager (Hard)

### Problem
Create a manager that tracks thread lifecycles and ensures proper cleanup.

### Requirements
```cpp
class ThreadLifecycleManager {
public:
    struct ThreadInfo {
        std::thread::id id;
        std::string name;
        std::chrono::system_clock::time_point startTime;
        bool active;
    };
    
    std::thread::id createThread(
        const std::string& name,
        std::function<void()> f);
    
    void joinThread(std::thread::id id);
    void joinAll();
    
    std::vector<ThreadInfo> getActiveThreads() const;
    
    ~ThreadLifecycleManager();  // Join all active threads
    
private:
    struct ManagedThread {
        std::thread thread;
        ThreadInfo info;
    };
    
    std::vector<ManagedThread> threads;
    mutable std::mutex mutex;
};
```

### Example
```cpp
ThreadLifecycleManager manager;

auto id1 = manager.createThread("Worker1", task1);
auto id2 = manager.createThread("Worker2", task2);

auto active = manager.getActiveThreads();
// Shows 2 active threads with names and start times

// Destructor ensures all joined
```

### Hints
- Track all created threads
- Mark as inactive when joined
- Join all active threads in destructor

---

## Exercise 8: Exception-Safe Parallel Executor (Hard)

### Problem
Execute multiple tasks in parallel with strong exception safety guarantee.

### Requirements
```cpp
class ParallelExecutor {
public:
    template<typename... Functions>
    void execute(Functions&&... functions);
    
    // Throws if any task throws
    // Guarantees all threads joined even with exceptions
    
private:
    template<typename F>
    void executeOne(F&& f, std::exception_ptr& eptr);
};
```

### Example
```cpp
ParallelExecutor executor;

try {
    executor.execute(
        []() { task1(); },
        []() { throw std::runtime_error("Error!"); },
        []() { task3(); }
    );
} catch (const std::exception& e) {
    // All threads joined before exception propagated
    std::cout << "Caught: " << e.what() << "\n";
}
```

### Hints
- Use ThreadGuard for each task
- Collect exceptions from all threads
- Rethrow first exception after all joined

---

## Exercise 9: Interruptible Thread (Advanced)

### Problem
Create a thread that can be interrupted and will automatically join.

### Requirements
```cpp
class InterruptibleThread {
public:
    template<typename F, typename... Args>
    InterruptibleThread(F&& f, Args&&... args);
    
    ~InterruptibleThread();
    
    void interrupt();
    bool isInterrupted() const;
    
    void join();
    
private:
    std::thread thread;
    std::atomic<bool> interrupted{false};
};

// Helper function to check interruption
void interruptionPoint();
```

### Example
```cpp
InterruptibleThread thread([]() {
    while (true) {
        interruptionPoint();  // Throws if interrupted
        doWork();
    }
});

std::this_thread::sleep_for(1s);
thread.interrupt();  // Thread will stop and be joined
```

### Hints
- Pass interrupt flag to thread
- Check flag at interruption points
- Join in destructor

---

## Exercise 10: Thread Container (Advanced)

### Problem
Create a container for threads that ensures all are joined on destruction.

### Requirements
```cpp
class ThreadContainer {
public:
    void addThread(std::thread&& t);
    
    template<typename F, typename... Args>
    void createThread(F&& f, Args&&... args);
    
    void joinAll();
    void detachAll();
    
    size_t activeCount() const;
    
    ~ThreadContainer();  // Join all remaining threads
    
private:
    std::vector<std::thread> threads;
    mutable std::mutex mutex;
};
```

### Example
```cpp
{
    ThreadContainer container;
    
    container.createThread(task1);
    container.createThread(task2);
    container.addThread(std::thread(task3));
    
    // Can explicitly join some
    container.joinAll();
    
    // Or let destructor handle it
}
```

### Hints
- Store threads in vector
- Iterate and join all in destructor
- Thread-safe add/remove operations

---

## Bonus Challenge: Thread Stack

### Problem
Implement a thread stack where threads are joined in LIFO order.

### Requirements
```cpp
class ThreadStack {
public:
    void push(std::thread&& t);
    void pop();  // Joins and removes top thread
    void clear();  // Joins all threads
    
    size_t size() const;
    bool empty() const;
    
    ~ThreadStack();
    
private:
    std::stack<std::thread> threads;
    std::mutex mutex;
};
```

---

## Testing Your Solutions

### Key Test Cases
```cpp
void testThreadGuard() {
    // Test 1: Normal flow
    {
        ThreadGuard guard(std::thread(task));
    }  // Should join
    
    // Test 2: Exception thrown
    try {
        ThreadGuard guard(std::thread(task));
        throw std::runtime_error("Error!");
    } catch (...) {
        // Thread should still be joined
    }
    
    // Test 3: Move semantics
    ThreadGuard guard1(std::thread(task));
    ThreadGuard guard2(std::move(guard1));  // Should work
}
```

### Verification Checklist
- [ ] Threads always joined or detached
- [ ] No thread leaks with exceptions
- [ ] Move semantics work correctly
- [ ] Thread sanitizer reports no issues
- [ ] Destructor order is correct
- [ ] No double join/detach

---

## Common Mistakes to Avoid

1. **Not checking joinable() before join/detach**
   ```cpp
   // ❌ Bad
   thread.join();  // May crash if not joinable
   
   // ✅ Good
   if (thread.joinable()) thread.join();
   ```

2. **Forgetting to handle moved-from threads**
   ```cpp
   // ❌ Bad
   auto t1 = std::thread(task);
   auto t2 = std::move(t1);
   t1.join();  // Crash! t1 is empty
   ```

3. **Not making destructors noexcept**
   ```cpp
   // ❌ Bad - can terminate if join throws
   ~ThreadGuard() {
       thread.join();
   }
   
   // ✅ Good
   ~ThreadGuard() noexcept {
       try {
           if (thread.joinable()) thread.join();
       } catch (...) { /* log error */ }
   }
   ```

---

## Grading Rubric

| Criteria | Points |
|----------|--------|
| RAII correctness | 35 |
| Exception safety | 25 |
| Move semantics | 15 |
| Code quality | 15 |
| Documentation | 10 |
| **Total** | **100** |
