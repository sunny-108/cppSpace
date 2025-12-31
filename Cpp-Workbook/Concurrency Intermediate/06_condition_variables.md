# Assignment 06: Condition Variables

## Overview
Master condition variables for thread synchronization, including producer-consumer patterns, spurious wakeups, and wait predicates.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 5-6 hours  
**Prerequisites:** Assignments 01-05

---

## Learning Objectives
- Understand condition_variable for thread coordination
- Implement producer-consumer patterns
- Handle spurious wakeups correctly
- Use wait with predicates
- Understand notify_one vs notify_all

---

## Part 1: Multiple Choice Questions (12 MCQs)

### Q1. What is a condition variable used for?
A) Locking mutexes  
B) Waiting for a condition to become true  
C) Creating threads  
D) Atomic operations  

**Answer:** B

### Q2. Spurious wakeups mean:
A) wait() can return even if no notify was called  
B) The program crashes  
C) Condition variable is broken  
D) Mutex is unlocked  

**Answer:** A - Always check condition in a loop

### Q3. Condition variable wait() must be used with:
A) Any mutex  
B) std::unique_lock<std::mutex>  
C) No lock needed  
D) std::lock_guard  

**Answer:** B - unique_lock because wait() needs to unlock/relock

### Q4. What does notify_one() do?
A) Wakes all waiting threads  
B) Wakes exactly one waiting thread  
C) Wakes no threads  
D) Locks the mutex  

**Answer:** B

### Q5. What does notify_all() do?
A) Wakes one thread  
B) Wakes all waiting threads  
C) Notifies all mutexes  
D) Blocks all threads  

**Answer:** B

### Q6. After wait() returns, the mutex is:
A) Unlocked  
B) Locked by the thread that called wait()  
C) Destroyed  
D) Available to all threads  

**Answer:** B

### Q7. To properly handle spurious wakeups, use:
A) notify_all() instead of notify_one()  
B) wait() with a predicate or manual loop  
C) More mutexes  
D) Atomic variables  

**Answer:** B

### Q8. The predicate form of wait (e.g., cv.wait(lock, []{ return ready; })) is equivalent to:
A) A single wait() call  
B) while(!ready) cv.wait(lock);  
C) if(!ready) cv.wait(lock);  
D) Nothing, they're different  

**Answer:** B

### Q9. If multiple threads are waiting and you notify_one():
A) All wake up  
B) One unspecified thread wakes up  
C) The oldest thread wakes up  
D) None wake up  

**Answer:** B - Which thread is unspecified

### Q10. Can you use condition_variable with std::lock_guard?
A) Yes  
B) No, wait() needs to unlock and relock  
C) Only notify operations  
D) In C++20 only  

**Answer:** B

### Q11. Lost wakeup problem occurs when:
A) notify() is called before wait()  
B) Too many threads  
C) Wrong mutex  
D) Using notify_all()  

**Answer:** A - Always check condition before waiting

### Q12. condition_variable_any differs from condition_variable by:
A) Faster performance  
B) Works with any lock type, not just unique_lock<mutex>  
C) No difference  
D) Better for atomics  

**Answer:** B - More flexible but slightly slower

---

## Part 2: Code Review Exercises

### Exercise 2.1: Missing Predicate Check

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void worker() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);  // BUG: No predicate check!
    
    // Assumes ready is true, but spurious wakeup possible!
    std::cout << "Worker proceeding\n";
}

void trigger() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    
    cv.notify_one();
}

int main() {
    std::thread t1(worker);
    std::thread t2(trigger);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

**Questions:**
1. What is wrong with this code?
2. What are spurious wakeups?
3. Fix using predicate-based wait

**Solution:**
```cpp
void worker() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return ready; });  // Predicate handles spurious wakeups
    std::cout << "Worker proceeding\n";
}
```

---

### Exercise 2.2: Lost Wakeup

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool dataReady = false;

void consumer() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Delay!
    
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Consumer waiting...\n";
    cv.wait(lock, []{ return dataReady; });
    std::cout << "Consumer got data\n";
}

void producer() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        dataReady = true;
    }
    cv.notify_one();  // Notifies before consumer is waiting!
    std::cout << "Producer notified\n";
}

int main() {
    std::thread c(consumer);
    std::thread p(producer);
    
    c.join();  // May hang!
    p.join();
    
    return 0;
}
```

**Questions:**
1. Why might consumer hang forever?
2. How does the predicate-based wait prevent this?
3. Is this code actually safe or can it still hang?

**Answer:** The predicate-based wait is safe because it checks `dataReady` immediately when entering wait(). If it's already true (notification happened early), wait() returns immediately without blocking.

---

### Exercise 2.3: Wrong Lock Type

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void worker() {
    std::lock_guard<std::mutex> lock(mtx);  // Wrong! Need unique_lock
    cv.wait(lock);  // Compilation error!
    std::cout << "Working\n";
}

int main() {
    std::thread t(worker);
    t.join();
    return 0;
}
```

**Questions:**
1. Why doesn't this compile?
2. Why is unique_lock required?
3. Fix the code

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Producer-Consumer Queue

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        // Your implementation:
        // 1. Lock mutex
        // 2. Add to queue
        // 3. Notify waiting consumer
    }
    
    T pop() {
        // Your implementation:
        // 1. Lock with unique_lock
        // 2. Wait until queue not empty
        // 3. Pop and return value
    }
    
    bool tryPop(T& value, std::chrono::milliseconds timeout) {
        // Your implementation:
        // Like pop() but with timeout
        // Return false if timeout expires
    }
    
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
};

// Test
void testProducerConsumer() {
    ThreadSafeQueue<int> queue;
    
    // Producer
    std::thread producer([&] {
        for (int i = 0; i < 10; ++i) {
            queue.push(i);
            std::cout << "Produced: " << i << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    // Consumer
    std::thread consumer([&] {
        for (int i = 0; i < 10; ++i) {
            int value = queue.pop();
            std::cout << "Consumed: " << value << "\n";
        }
    });
    
    producer.join();
    consumer.join();
}
```

**Requirements:**
- Thread-safe push and pop
- Consumer blocks when queue empty
- Handle spurious wakeups
- Bonus: Add bounded queue with max size

---

### Exercise 3.2: Barrier Synchronization

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

class Barrier {
public:
    explicit Barrier(size_t count) : threshold_(count), count_(count), generation_(0) {}
    
    void wait() {
        // Your implementation:
        // 1. Lock mutex
        // 2. Decrement count
        // 3. If count reaches 0:
        //    - Reset count
        //    - Increment generation
        //    - Notify all
        // 4. Else wait until generation changes
    }
    
private:
    std::mutex mutex_;
    std::condition_variable cond_;
    size_t threshold_;
    size_t count_;
    size_t generation_;  // Handles multiple barrier uses
};

// Test
void testBarrier() {
    const int numThreads = 5;
    Barrier barrier(numThreads);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&barrier, i] {
            std::cout << "Thread " << i << " working...\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            std::cout << "Thread " << i << " reached barrier\n";
            barrier.wait();  // All threads wait here
            
            std::cout << "Thread " << i << " passed barrier\n";
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}
```

---

### Exercise 3.3: Thread Pool with Task Queue

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop_(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                workerLoop();
            });
        }
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            stop_ = true;
        }
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            worker.join();
        }
    }
    
    void submit(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            tasks_.push(std::move(task));
        }
        condition_.notify_one();
    }
    
private:
    void workerLoop() {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                
                // Wait for task or stop signal
                condition_.wait(lock, [this] {
                    return stop_ || !tasks_.empty();
                });
                
                if (stop_ && tasks_.empty()) {
                    return;  // Exit thread
                }
                
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            
            task();  // Execute outside lock
        }
    }
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_;
};

// Test
void testThreadPool() {
    ThreadPool pool(4);
    
    for (int i = 0; i < 20; ++i) {
        pool.submit([i] {
            std::cout << "Task " << i << " on thread " 
                      << std::this_thread::get_id() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Destructor waits for all tasks
}
```

---

### Exercise 3.4: Countdown Latch

```cpp
#include <mutex>
#include <condition_variable>

class CountdownLatch {
public:
    explicit CountdownLatch(int count) : count_(count) {}
    
    void countDown() {
        // Your implementation:
        // Decrement count, notify if reaches 0
    }
    
    void wait() {
        // Your implementation:
        // Wait until count reaches 0
    }
    
    bool waitFor(std::chrono::milliseconds timeout) {
        // Your implementation with timeout
    }
    
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int count_;
};

// Usage
void testLatch() {
    CountdownLatch latch(3);
    
    std::thread t1([&] {
        std::cout << "Task 1\n";
        latch.countDown();
    });
    
    std::thread t2([&] {
        std::cout << "Task 2\n";
        latch.countDown();
    });
    
    std::thread t3([&] {
        std::cout << "Task 3\n";
        latch.countDown();
    });
    
    latch.wait();  // Waits for all 3
    std::cout << "All tasks complete!\n";
    
    t1.join();
    t2.join();
    t3.join();
}
```

---

## Part 4: Debugging Exercises

### Exercise 4.1: Deadlock with Condition Variable

```cpp
std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void buggyWait() {
    std::lock_guard<std::mutex> lock(mtx);  // Lock held
    
    while (!ready) {
        // Can't wait with lock_guard!
        // Need to unlock before waiting
    }
}
```

**Fix:** Use unique_lock and wait()

---

### Exercise 4.2: Notify Before Lock Release

```cpp
void producer() {
    std::unique_lock<std::mutex> lock(mtx);
    data = newData;
    cv.notify_one();  // Notification under lock
    // Lock released here
}
```

**Question:** Is it better to notify inside or outside the lock? Why?

**Answer:** Generally outside is better (less contention), but inside is safer. Both work correctly.

---

## Part 5: Performance Analysis

### Exercise 5.1: notify_one vs notify_all

```cpp
#include <chrono>

void benchmarkNotifyOne() {
    // Multiple consumers
    // Producer uses notify_one
    // Measure wakeup latency
}

void benchmarkNotifyAll() {
    // Same setup
    // Producer uses notify_all
    // Compare performance
}
```

**Analysis:**
- When is notify_all necessary?
- Performance cost of waking all threads?

---

### Exercise 5.2: Condition Variable vs Busy Wait

```cpp
// Busy wait
void busyWait() {
    while (!ready) {
        std::this_thread::yield();
    }
}

// Condition variable
void conditionWait() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return ready; });
}
```

**Compare:**
- CPU usage
- Latency
- Power consumption

---

## Submission Guidelines

Submit:
1. **answers.md** - MCQ answers
2. **code_review/** - Fixed code
3. **implementations/** - All implementations
4. **debugging/** - Fixes
5. **performance/** - Analysis

---

## Evaluation Criteria

- **Correctness (35%):** Proper condition variable usage
- **Spurious Wakeups (25%):** Handled correctly everywhere
- **Understanding (20%):** Clear MCQ explanations
- **Performance (20%):** Insightful analysis

---

## Key Takeaways

✅ Always use unique_lock with condition_variable  
✅ Always check condition in a loop or use predicate  
✅ notify_one for single consumer, notify_all for broadcast  
✅ Condition variables prevent busy waiting  
✅ Check condition before waiting (lost wakeup)  

---

## Common Pitfalls

❌ Using lock_guard with condition_variable  
❌ Not checking condition in a loop  
❌ Notifying without holding lock (can work but risky)  
❌ Forgetting to notify after changing condition  

---

## Next Steps

Proceed to **Assignment 07: Atomic Operations**.

---

## Resources

- [cppreference - condition_variable](https://en.cppreference.com/w/cpp/thread/condition_variable)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) - Chapter 4

Good luck!