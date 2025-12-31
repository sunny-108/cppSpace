# Programming Assignment - Item 39: Consider void Futures for One-Shot Event Communication

## Learning Objectives
- Use void futures for simple thread synchronization
- Understand when void futures are better than condition variables
- Implement one-shot notification patterns
- Use shared_future for multiple waiters

---

## Exercise 1: Simple Start Signal (Easy)

### Problem
Implement a start signal where worker threads wait for a "GO!" signal from main thread.

### Requirements
```cpp
class StartSignal {
public:
    void waitForStart();  // Workers call this
    void go();            // Main thread calls this
    
private:
    // Use promise/future for signaling
};
```

### Example
```cpp
StartSignal signal;

std::thread worker([&signal]() {
    std::cout << "Worker ready\n";
    signal.waitForStart();
    std::cout << "Worker running!\n";
});

std::this_thread::sleep_for(1s);
std::cout << "Main: GO!\n";
signal.go();

worker.join();
```

### Hints
- Use std::promise<void>
- Call set_value() to signal
- Call wait() on future to wait

---

## Exercise 2: Comparison with Condition Variable (Medium)

### Problem
Implement the same functionality using both condition variable and void future approaches, then compare.

### Requirements
```cpp
class EventNotification {
public:
    // Condition variable version
    void waitCV();
    void signalCV();
    
    // void future version
    void waitFuture();
    void signalFuture();
    
    // Compare complexity and code size
    static void compareApproaches();
};
```

### Example
```cpp
EventNotification::compareApproaches();
// Output:
// Condition Variable: 15 lines, mutex+cv+flag
// void Future: 5 lines, promise+future only
```

### Hints
- Count lines of code
- Note complexity differences
- Document what can go wrong with CV

---

## Exercise 3: Multiple Waiters (Medium)

### Problem
Implement a "starting gate" where multiple threads wait for same signal.

### Requirements
```cpp
class StartingGate {
public:
    explicit StartingGate(size_t numWaiters);
    
    void waitAtGate();  // Threads call this
    void openGate();    // Main calls this
    
    size_t waitingCount() const;
    
private:
    // Use shared_future for multiple waiters
};
```

### Example
```cpp
StartingGate gate(5);

std::vector<std::thread> runners;
for (int i = 0; i < 5; ++i) {
    runners.emplace_back([&gate, i]() {
        std::cout << "Runner " << i << " at gate\n";
        gate.waitAtGate();
        std::cout << "Runner " << i << " GO!\n";
    });
}

std::this_thread::sleep_for(1s);
gate.openGate();  // All runners start together

for (auto& t : runners) t.join();
```

### Hints
- Use std::shared_future<void>
- All waiters share same future
- Single signal wakes all

---

## Exercise 4: Resource Initialization (Hard)

### Problem
Create a lazily-initialized resource where multiple threads can safely wait for initialization.

### Requirements
```cpp
template<typename T>
class LazyResource {
public:
    LazyResource(std::function<T()> initializer);
    
    T& get();  // Wait for initialization if needed
    
    bool isInitialized() const;
    
private:
    std::function<T()> init;
    std::shared_future<void> initComplete;
    std::unique_ptr<T> resource;
    std::once_flag initFlag;
};
```

### Example
```cpp
LazyResource<Database> db([]() {
    return Database::connect("localhost");
});

// Multiple threads
std::thread t1([&db]() {
    auto& connection = db.get();  // Waits if not ready
    connection.query("SELECT ...");
});

std::thread t2([&db]() {
    auto& connection = db.get();  // Waits for same init
    connection.query("INSERT ...");
});
```

### Hints
- Initialize once with std::call_once
- Use shared_future for waiting
- Signal when initialization completes

---

## Exercise 5: Task Completion Notification (Medium)

### Problem
Notify when a background task completes without needing to retrieve its result.

### Requirements
```cpp
class TaskNotifier {
public:
    template<typename F>
    void runTask(F&& task);
    
    void waitForCompletion();
    
    bool isComplete() const;
    
    template<typename Rep, typename Period>
    bool waitForCompletion(
        const std::chrono::duration<Rep, Period>& timeout);
    
private:
    std::promise<void> completion;
    std::future<void> completionFuture;
};
```

### Example
```cpp
TaskNotifier notifier;

std::thread worker([&notifier]() {
    notifier.runTask([]() {
        // Long running task
        std::this_thread::sleep_for(2s);
    });
});

std::cout << "Waiting for task...\n";
notifier.waitForCompletion();
std::cout << "Task complete!\n";

worker.join();
```

### Hints
- Task signals promise when done
- Main thread waits on future
- Support timeout with wait_for

---

## Exercise 6: Pipeline Stage Synchronization (Hard)

### Problem
Create a multi-stage pipeline where each stage signals when ready for next item.

### Requirements
```cpp
template<typename T>
class PipelineStage {
public:
    PipelineStage(
        std::function<T(T)> processor,
        PipelineStage* nextStage = nullptr);
    
    void process(T input);
    void signalComplete();
    void waitForComplete();
    
private:
    std::function<T(T)> processor;
    PipelineStage* next;
    std::promise<void> stageComplete;
    std::shared_future<void> completeFuture;
};
```

### Example
```cpp
PipelineStage<int> stage3([](int x) { return x * 3; });
PipelineStage<int> stage2([](int x) { return x * 2; }, &stage3);
PipelineStage<int> stage1([](int x) { return x + 1; }, &stage2);

std::thread t([&]() {
    stage1.process(10);
    stage1.signalComplete();
});

stage1.waitForComplete();
std::cout << "Pipeline complete!\n";
```

### Hints
- Each stage signals completion
- Next stage waits for signal
- Chain signals through pipeline

---

## Exercise 7: Graceful Shutdown (Medium)

### Problem
Implement graceful shutdown for a worker thread pool using void futures.

### Requirements
```cpp
class WorkerPool {
public:
    explicit WorkerPool(size_t numWorkers);
    ~WorkerPool();
    
    void submitTask(std::function<void()> task);
    
    void shutdown();  // Signal shutdown
    void waitForShutdown();
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable cv;
    
    std::promise<void> shutdownSignal;
    std::shared_future<void> shutdownFuture;
    std::atomic<bool> stopping{false};
};
```

### Example
```cpp
{
    WorkerPool pool(4);
    
    for (int i = 0; i < 100; ++i) {
        pool.submitTask([i]() {
            std::cout << "Task " << i << "\n";
        });
    }
    
    pool.shutdown();  // Signals workers
    pool.waitForShutdown();
}  // All workers gracefully stopped
```

### Hints
- Workers check shutdown future
- Use shared_future for all workers
- Combine with condition variable for tasks

---

## Exercise 8: Test Runner (Hard)

### Problem
Create a test runner where tests wait for setup, then all run simultaneously.

### Requirements
```cpp
class TestRunner {
public:
    void registerTest(
        const std::string& name,
        std::function<void()> test);
    
    void runAllTests();  // Setup, signal, run
    
    struct TestResult {
        std::string name;
        bool passed;
        std::string error;
    };
    
    std::vector<TestResult> getResults() const;
    
private:
    struct Test {
        std::string name;
        std::function<void()> func;
    };
    
    std::vector<Test> tests;
    std::promise<void> setupComplete;
    std::vector<TestResult> results;
};
```

### Example
```cpp
TestRunner runner;

runner.registerTest("Test1", []() { /* test */ });
runner.registerTest("Test2", []() { /* test */ });
runner.registerTest("Test3", []() { /* test */ });

runner.runAllTests();
// All tests wait for setup, then run in parallel

auto results = runner.getResults();
for (const auto& r : results) {
    std::cout << r.name << ": " 
              << (r.passed ? "PASS" : "FAIL") << "\n";
}
```

### Hints
- Global setup before signaling
- All tests share setup complete signal
- Collect results thread-safely

---

## Exercise 9: Cancellation Token (Advanced)

### Problem
Implement a cancellation token using void futures.

### Requirements
```cpp
class CancellationToken {
public:
    CancellationToken();
    
    void cancel();
    bool isCancelled() const;
    
    void throwIfCancelled() const;
    
    template<typename Rep, typename Period>
    bool waitForCancellation(
        const std::chrono::duration<Rep, Period>& timeout);
    
    std::shared_future<void> getFuture() const;
    
private:
    std::promise<void> cancelPromise;
    std::shared_future<void> cancelFuture;
    std::atomic<bool> cancelled{false};
};
```

### Example
```cpp
CancellationToken token;

std::thread worker([&token]() {
    while (true) {
        token.throwIfCancelled();
        doWork();
    }
});

std::this_thread::sleep_for(5s);
token.cancel();  // Worker will stop

worker.join();
```

### Hints
- Set promise on cancel
- Check future status for polling
- Throw exception for immediate cancel

---

## Exercise 10: Barrier Implementation (Advanced)

### Problem
Implement a simple barrier using void futures (similar to C++20 std::barrier).

### Requirements
```cpp
class SimpleBarrier {
public:
    explicit SimpleBarrier(size_t count);
    
    void arriveAndWait();  // Thread arrives and waits
    
    size_t getCount() const;
    
    void reset();  // For reuse (creates new promise/future)
    
private:
    size_t expectedCount;
    std::atomic<size_t> arrived{0};
    std::promise<void> barrierPromise;
    std::shared_future<void> barrierFuture;
    std::mutex mutex;
};
```

### Example
```cpp
SimpleBarrier barrier(3);

auto worker = [&barrier](int id) {
    std::cout << id << " working...\n";
    std::this_thread::sleep_for(1s);
    
    std::cout << id << " at barrier\n";
    barrier.arriveAndWait();
    
    std::cout << id << " continuing\n";
};

std::thread t1(worker, 1);
std::thread t2(worker, 2);
std::thread t3(worker, 3);

// All wait at barrier until all 3 arrive
```

### Hints
- Count arrivals atomically
- Last thread signals promise
- All wait on shared_future
- Reset for next use

---

## Bonus Challenge: Event Bus

### Problem
Create an event bus where subscribers wait for specific events.

### Requirements
```cpp
class EventBus {
public:
    void subscribe(const std::string& event, 
                   std::shared_future<void>& future);
    
    void publish(const std::string& event);
    
    size_t subscriberCount(const std::string& event) const;
    
private:
    struct EventChannel {
        std::promise<void> promise;
        std::vector<std::shared_future<void>*> subscribers;
    };
    
    std::map<std::string, EventChannel> channels;
    std::mutex mutex;
};
```

---

## Testing Your Solutions

### Key Test Cases
```cpp
void testOneShot() {
    std::promise<void> p;
    auto f = p.get_future();
    
    bool signaled = false;
    std::thread t([&f, &signaled]() {
        f.wait();
        signaled = true;
    });
    
    std::this_thread::sleep_for(100ms);
    assert(!signaled);
    
    p.set_value();
    t.join();
    assert(signaled);
}

void testMultipleWaiters() {
    std::promise<void> p;
    auto sf = p.get_future().share();
    
    std::atomic<int> count{0};
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([sf, &count]() {
            sf.wait();
            ++count;
        });
    }
    
    std::this_thread::sleep_for(100ms);
    p.set_value();
    
    for (auto& t : threads) t.join();
    assert(count == 5);
}
```

### Verification Checklist
- [ ] One-shot signaling works
- [ ] Multiple waiters work with shared_future
- [ ] Cannot signal twice (would throw)
- [ ] Simpler than condition variables
- [ ] No spurious wakeups
- [ ] Clean shutdown behavior

---

## When NOT to Use void Futures

Remember void futures are one-shot only:
- ❌ Need repeated signaling → use condition_variable
- ❌ Need to reset → use condition_variable
- ❌ Complex conditions → use condition_variable
- ✅ Simple one-time notification → void future perfect!

---

## Grading Rubric

| Criteria | Points |
|----------|--------|
| Correct usage of void futures | 35 |
| Understanding limitations | 25 |
| Shared_future for multiple waiters | 20 |
| Code quality | 12 |
| Documentation | 8 |
| **Total** | **100** |
