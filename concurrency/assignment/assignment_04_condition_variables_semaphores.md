# Assignment 04: Condition Variables & Semaphores

## Overview
Master advanced synchronization using condition variables and semaphores. Learn to implement complex coordination patterns, avoid spurious wakeups, and build efficient producer-consumer systems.

**Estimated Time:** 6-8 hours

---

## Learning Objectives
- Master condition variable usage and avoid common pitfalls
- Understand spurious wakeups and how to handle them
- Implement semaphores and use them effectively
- Build complex synchronization patterns
- Design efficient producer-consumer queues

---

## Part 1: Multiple Choice Questions (MCQs)

### Q1. Why must condition variables always be used with a mutex?
A) To improve performance  
B) To protect the predicate and ensure atomicity of wait operations  
C) To prevent deadlocks  
D) To enable broadcasting  

**Answer:** B

---

### Q2. What is a spurious wakeup?
A) A wakeup caused by an error  
B) A thread waking from wait without notification or timeout  
C) Waking multiple threads unnecessarily  
D) A performance optimization  

**Answer:** B

---

### Q3. The correct pattern for waiting on a condition variable is:
A) `cv.wait(lock);`  
B) `while (!condition) cv.wait(lock);`  
C) `if (!condition) cv.wait(lock);`  
D) `cv.wait(lock); if (condition) { ... }`  

**Answer:** B

---

### Q4. `notify_one()` vs `notify_all()`: when should you use `notify_all()`?
A) Always, it's safer  
B) When multiple threads might need to wake and check different conditions  
C) Never, it wastes CPU  
D) Only with broadcast patterns  

**Answer:** B

---

### Q5. What happens if you call `notify_one()` before any thread waits?
A) The notification is queued  
B) An exception is thrown  
C) The notification is lost  
D) All threads are notified  

**Answer:** C

---

### Q6. A binary semaphore is similar to:
A) `std::atomic<bool>`  
B) A mutex  
C) A condition variable  
D) A spinlock  

**Answer:** B

---

### Q7. C++20's `std::counting_semaphore<N>` allows:
A) Exactly N threads to proceed  
B) Up to N simultaneous acquisitions  
C) N notifications to be queued  
D) N mutexes to be locked together  

**Answer:** B

---

### Q8. In a producer-consumer queue, what should the producer do when the queue is full?
A) Overwrite old data  
B) Throw an exception  
C) Wait on a condition variable until space is available  
D) Create a larger queue  

**Answer:** C

---

### Q9. What's the difference between `wait()` and `wait_for()`?
A) `wait_for()` takes a predicate  
B) `wait_for()` has a timeout  
C) `wait_for()` is non-blocking  
D) No difference  

**Answer:** B

---

### Q10. The predicate in `cv.wait(lock, predicate)`:
A) Is checked once before waiting  
B) Is checked in a loop internally  
C) Must be a lambda function  
D) Is optional  

**Answer:** B

---

### Q11. When using `notify_one()` with multiple waiting threads:
A) All threads wake up  
B) Exactly one thread wakes up (unspecified which)  
C) The highest priority thread wakes up  
D) The thread that waited longest wakes up  

**Answer:** B

---

### Q12. A common pitfall when using condition variables is:
A) Holding the lock too long after notification  
B) Using the wrong mutex  
C) Checking the predicate with `if` instead of `while`  
D) All of the above  

**Answer:** D

---

### Q13. `std::condition_variable_any` differs from `std::condition_variable` in that:
A) It's faster  
B) It works with any lockable type, not just `std::unique_lock<std::mutex>`  
C) It doesn't have spurious wakeups  
D) It can notify multiple times  

**Answer:** B

---

### Q14. In a bounded buffer, you typically need:
A) One condition variable  
B) Two condition variables (not_full and not_empty)  
C) No condition variables, just mutexes  
D) One condition variable per producer/consumer  

**Answer:** B

---

### Q15. Semaphores are useful for:
A) Mutual exclusion only  
B) Resource counting and signaling  
C) Deadlock detection  
D) Memory ordering  

**Answer:** B

---

## Part 2: Code Review Exercises

### Exercise 2.1: Spurious Wakeup Bug

```cpp
class MessageQueue {
    std::queue<std::string> messages;
    std::mutex mtx;
    std::condition_variable cv;
    
public:
    void send(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        messages.push(msg);
        cv.notify_one();
    }
    
    std::string receive() {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (messages.empty()) {
            cv.wait(lock);
        }
        
        std::string msg = messages.front();
        messages.pop();
        return msg;
    }
    
    bool tryReceive(std::string& msg, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (messages.empty()) {
            if (cv.wait_for(lock, timeout) == std::cv_status::timeout) {
                return false;
            }
        }
        
        if (!messages.empty()) {
            msg = messages.front();
            messages.pop();
            return true;
        }
        
        return false;
    }
};
```

**Questions:**
1. What's wrong with the `receive()` method's wait pattern?
2. Why is spurious wakeup a problem here?
3. What happens if multiple threads call `receive()` simultaneously?
4. Is `tryReceive()` correct? What edge cases exist?
5. Fix all issues and provide comprehensive tests

---

### Exercise 2.2: Lost Wakeup Problem

```cpp
class EventNotifier {
    std::mutex mtx;
    std::condition_variable cv;
    bool eventOccurred = false;
    
public:
    void notify() {
        eventOccurred = true;
        cv.notify_one();  // Notification before locking?
    }
    
    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        while (!eventOccurred) {
            cv.wait(lock);
        }
        eventOccurred = false;
    }
    
    void waitFor(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, timeout, [this] { return eventOccurred; });
        eventOccurred = false;
    }
};

void testScenario() {
    EventNotifier notifier;
    
    std::thread waiter([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        notifier.wait();
        std::cout << "Event received!\n";
    });
    
    std::thread notifyThread([&] {
        notifier.notify();
        std::cout << "Event sent!\n";
    });
    
    notifyThread.join();
    waiter.join();
}
```

**Questions:**
1. Can the wakeup be lost? Under what conditions?
2. What's the race condition in `notify()`?
3. Is resetting `eventOccurred` safe?
4. How would you fix the lost wakeup?
5. Should this use `notify_all()` instead?

---

### Exercise 2.3: Inefficient Notification

```cpp
class TaskScheduler {
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> workers;
    bool shutdown = false;
    
public:
    TaskScheduler(size_t numWorkers) {
        for (size_t i = 0; i < numWorkers; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] { 
                            return shutdown || !tasks.empty(); 
                        });
                        
                        if (shutdown && tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    cv.notify_all();  // Notify after taking a task?
                    
                    task();
                }
            });
        }
    }
    
    void submit(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(std::move(task));
        }
        cv.notify_all();  // Wake all workers for each task?
    }
    
    ~TaskScheduler() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            shutdown = true;
        }
        cv.notify_all();
        
        for (auto& worker : workers) {
            worker.join();
        }
    }
};
```

**Questions:**
1. Why is `notify_all()` in the worker loop problematic?
2. Should `submit()` use `notify_one()` or `notify_all()`?
3. What's the performance impact of excessive notifications?
4. How can you optimize this?
5. Is the shutdown sequence correct?

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Bounded Buffer with Condition Variables

Implement a thread-safe bounded buffer (circular queue):

```cpp
template<typename T, size_t Capacity>
class BoundedBuffer {
public:
    BoundedBuffer() : head(0), tail(0), count(0) {}
    
    // Block until space is available
    void push(const T& item);
    void push(T&& item);
    
    // Block until item is available
    T pop();
    
    // Try to push with timeout
    bool tryPush(const T& item, std::chrono::milliseconds timeout);
    
    // Try to pop with timeout
    bool tryPop(T& item, std::chrono::milliseconds timeout);
    
    // Non-blocking operations
    bool tryPush(const T& item);
    bool tryPop(T& item);
    
    size_t size() const;
    bool empty() const;
    bool full() const;
    
private:
    std::array<T, Capacity> buffer;
    size_t head;
    size_t tail;
    size_t count;
    
    mutable std::mutex mtx;
    std::condition_variable notFull;
    std::condition_variable notEmpty;
    
    // Your implementation here
};

// Stress test
void testBoundedBuffer() {
    BoundedBuffer<int, 10> buffer;
    
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};
    std::atomic<bool> done{false};
    
    // Multiple producers
    auto producer = [&](int id) {
        for (int i = 0; i < 1000; ++i) {
            buffer.push(id * 1000 + i);
            produced++;
        }
    };
    
    // Multiple consumers
    auto consumer = [&](int id) {
        while (!done || !buffer.empty()) {
            int value;
            if (buffer.tryPop(value, std::chrono::milliseconds(10))) {
                consumed++;
            }
        }
    };
    
    std::vector<std::thread> producers;
    for (int i = 0; i < 4; ++i) {
        producers.emplace_back(producer, i);
    }
    
    std::vector<std::thread> consumers;
    for (int i = 0; i < 4; ++i) {
        consumers.emplace_back(consumer, i);
    }
    
    for (auto& t : producers) {
        t.join();
    }
    
    done = true;
    
    for (auto& t : consumers) {
        t.join();
    }
    
    std::cout << "Produced: " << produced << ", Consumed: " << consumed << "\n";
    assert(produced == consumed);
}
```

**Requirements:**
- Correct synchronization with two condition variables
- Handle spurious wakeups properly
- Efficient (minimal lock contention)
- Exception-safe
- Support multiple producers and consumers

---

### Exercise 3.2: Implement Counting Semaphore (C++20 style)

```cpp
class CountingSemaphore {
public:
    explicit CountingSemaphore(ptrdiff_t desired);
    
    // Acquire (decrement)
    void acquire();
    
    // Try to acquire without blocking
    bool try_acquire();
    
    // Try to acquire with timeout
    template<typename Rep, typename Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& timeout);
    
    template<typename Clock, typename Duration>
    bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& timepoint);
    
    // Release (increment)
    void release(ptrdiff_t update = 1);
    
    // Get current count (for testing)
    ptrdiff_t getCount() const;
    
private:
    ptrdiff_t count;
    mutable std::mutex mtx;
    std::condition_variable cv;
    
    // Your implementation
};

// Test: Rate limiter using semaphore
class RateLimiter {
    CountingSemaphore sem;
    std::thread refiller;
    std::atomic<bool> running{true};
    
public:
    RateLimiter(int requestsPerSecond) : sem(requestsPerSecond) {
        refiller = std::thread([this, requestsPerSecond] {
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / requestsPerSecond));
                sem.release();
            }
        });
    }
    
    ~RateLimiter() {
        running = false;
        if (refiller.joinable()) {
            refiller.join();
        }
    }
    
    bool tryRequest(std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) {
        return sem.try_acquire_for(timeout);
    }
    
    void request() {
        sem.acquire();
    }
};

void testRateLimiter() {
    RateLimiter limiter(10); // 10 requests per second
    
    auto start = std::chrono::steady_clock::now();
    int successfulRequests = 0;
    
    for (int i = 0; i < 50; ++i) {
        if (limiter.tryRequest(std::chrono::milliseconds(200))) {
            successfulRequests++;
            std::cout << "Request " << i << " granted\n";
        } else {
            std::cout << "Request " << i << " denied\n";
        }
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    
    std::cout << "Successful requests: " << successfulRequests 
              << " in " << duration.count() << " seconds\n";
}
```

**Requirements:**
- Correct semaphore semantics
- Efficient implementation
- Support all timeout variants
- Thread-safe
- No spurious failures

---

### Exercise 3.3: Barrier Synchronization

Implement a reusable barrier for thread synchronization:

```cpp
class Barrier {
public:
    explicit Barrier(size_t numThreads);
    
    // Block until all threads arrive, then release all
    void arrive_and_wait();
    
    // Arrive and execute function when all threads arrive (one thread only)
    template<typename F>
    void arrive_and_wait(F&& onCompletion);
    
    // Drop one thread from future barriers
    void arrive_and_drop();
    
private:
    size_t threshold;
    size_t count;
    size_t generation;
    
    std::mutex mtx;
    std::condition_variable cv;
    
    // Your implementation
};

// Test: Parallel pipeline stages
class PipelineStage {
    std::vector<int> data;
    Barrier& barrier;
    int stage;
    
public:
    PipelineStage(Barrier& b, int s) : barrier(b), stage(s) {}
    
    void process(int threadId, int numThreads, const std::vector<int>& input) {
        // Each thread processes its portion
        size_t chunkSize = input.size() / numThreads;
        size_t start = threadId * chunkSize;
        size_t end = (threadId == numThreads - 1) ? input.size() : start + chunkSize;
        
        for (size_t i = start; i < end; ++i) {
            data.push_back(input[i] * stage);
        }
        
        // Wait for all threads to complete this stage
        barrier.arrive_and_wait([this] {
            std::cout << "Stage " << stage << " completed\n";
        });
    }
    
    const std::vector<int>& getData() const { return data; }
};

void testPipeline() {
    const int numThreads = 4;
    Barrier barrier(numThreads);
    
    std::vector<int> input(1000);
    std::iota(input.begin(), input.end(), 0);
    
    auto worker = [&](int id) {
        PipelineStage stage1(barrier, 1);
        PipelineStage stage2(barrier, 2);
        PipelineStage stage3(barrier, 3);
        
        stage1.process(id, numThreads, input);
        stage2.process(id, numThreads, stage1.getData());
        stage3.process(id, numThreads, stage2.getData());
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
}
```

**Requirements:**
- Reusable (can be called multiple times)
- Support completion callback
- Handle generation correctly for reuse
- Support dropping threads
- Efficient (minimal spurious wakeups)

---

## Part 4: Debugging Concurrent Code

### Exercise 4.1: Deadlock with Condition Variables

```cpp
class SharedResource {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
    int data = 0;
    
public:
    void produce(int value) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait for consumer to be ready
        cv.wait(lock, [this] { return !ready; });
        
        data = value;
        ready = true;
        
        cv.notify_one();
    }
    
    int consume() {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait for data to be ready
        cv.wait(lock, [this] { return ready; });
        
        int value = data;
        ready = false;
        
        cv.notify_one();
        
        return value;
    }
};

void testDeadlock() {
    SharedResource resource;
    
    std::thread producer([&] {
        for (int i = 0; i < 10; ++i) {
            resource.produce(i);
            std::cout << "Produced: " << i << "\n";
        }
    });
    
    std::thread consumer([&] {
        for (int i = 0; i < 10; ++i) {
            int value = resource.consume();
            std::cout << "Consumed: " << value << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    producer.join();
    consumer.join();
}
```

**Tasks:**
1. Why does this occasionally deadlock?
2. What happens on the first produce() call?
3. Trace through a deadlock scenario step-by-step
4. Fix the issue
5. Add proper initialization

---

### Exercise 4.2: Condition Variable with Multiple Predicates

```cpp
class WorkQueue {
    std::queue<int> highPriority;
    std::queue<int> lowPriority;
    std::mutex mtx;
    std::condition_variable cv;
    bool shutdown = false;
    
public:
    void addHigh(int item) {
        std::lock_guard<std::mutex> lock(mtx);
        highPriority.push(item);
        cv.notify_one();
    }
    
    void addLow(int item) {
        std::lock_guard<std::mutex> lock(mtx);
        lowPriority.push(item);
        cv.notify_one();
    }
    
    int get() {
        std::unique_lock<std::mutex> lock(mtx);
        
        cv.wait(lock, [this] {
            return shutdown || !highPriority.empty() || !lowPriority.empty();
        });
        
        if (shutdown) {
            throw std::runtime_error("Shutdown");
        }
        
        int item;
        if (!highPriority.empty()) {
            item = highPriority.front();
            highPriority.pop();
        } else {
            item = lowPriority.front();
            lowPriority.pop();
        }
        
        return item;
    }
    
    void stop() {
        std::lock_guard<std::mutex> lock(mtx);
        shutdown = true;
        cv.notify_all();
    }
};
```

**Questions:**
1. Is the priority handling correct?
2. Can low priority items starve?
3. What happens with multiple consumers?
4. Should this use separate condition variables?
5. Optimize and fix any issues

---

## Part 5: Performance Optimization

### Exercise 5.1: Optimize Producer-Consumer

Compare different synchronization strategies:

```cpp
class BenchmarkProducerConsumer {
public:
    struct Config {
        size_t numProducers;
        size_t numConsumers;
        size_t itemsPerProducer;
        size_t bufferSize;
    };
    
    struct Results {
        std::string strategy;
        double throughput;  // items per second
        double latency;     // microseconds
    };
    
    // Version 1: Condition variables
    Results benchmarkConditionVariable(const Config& cfg);
    
    // Version 2: Semaphores
    Results benchmarkSemaphore(const Config& cfg);
    
    // Version 3: Lock-free queue
    Results benchmarkLockFree(const Config& cfg);
    
    // Version 4: Busy-wait with atomics
    Results benchmarkBusyWait(const Config& cfg);
    
    void runComparison();
};
```

**Analysis:**
1. Implement all versions
2. Benchmark with various configurations
3. Measure CPU usage
4. Graph throughput vs number of threads
5. Provide recommendations

---

### Exercise 5.2: Reduce Notification Overhead

```cpp
// Current version with excessive notifications
class EventBusV1 {
    std::vector<std::function<void(int)>> subscribers;
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<int> events;
    
public:
    void publish(int event) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            events.push(event);
        }
        cv.notify_all();  // Wakes all subscribers
    }
    
    void subscribe(std::function<void(int)> handler) {
        std::thread([this, handler] {
            while (true) {
                int event;
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this] { return !events.empty(); });
                    
                    if (events.empty()) continue;
                    
                    event = events.front();
                    events.pop();
                }
                handler(event);
            }
        }).detach();
    }
};

// Your optimized version
class EventBusV2 {
    // Optimize notification strategy
    // Consider: batching, per-subscriber queues, etc.
};
```

**Tasks:**
1. Profile V1's notification overhead
2. Implement optimized versions
3. Benchmark with 1-100 subscribers
4. Measure wakeup efficiency
5. Document trade-offs

---

## Submission Guidelines

Same as previous assignments: code, tests, documentation, performance analysis.

---

## Evaluation Criteria

- **Correctness (35%):** Proper predicate checking, no spurious failures
- **Design (25%):** Appropriate use of condition variables vs semaphores
- **Performance (20%):** Efficient notification strategies
- **Robustness (15%):** Exception safety, edge case handling
- **Code Quality (10%):** Clean, well-documented code

---

Good luck!
