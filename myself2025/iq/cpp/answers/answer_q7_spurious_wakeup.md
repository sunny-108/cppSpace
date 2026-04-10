# Q7: What is Spurious Wakeup? How do you handle it in your code?

## Quick Answer

A **spurious wakeup** occurs when a thread waiting on a condition variable wakes up without being explicitly notified. This can happen due to OS-level implementation details. Handle it by **always checking the condition in a loop** or using the predicate overload of `wait()`.

---

## High-Level Design

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         Spurious Wakeup Scenario                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   Expected Flow:                      Spurious Wakeup:                      │
│   ───────────────                     ─────────────────                     │
│                                                                             │
│   ┌──────────────┐                    ┌──────────────┐                      │
│   │ Thread waits │                    │ Thread waits │                      │
│   │ on CV        │                    │ on CV        │                      │
│   └──────┬───────┘                    └──────┬───────┘                      │
│          │                                   │                              │
│          │ notify_one()                      │ ??? (no notification)        │
│          ▼                                   ▼                              │
│   ┌──────────────┐                    ┌──────────────┐                      │
│   │ Thread wakes │                    │ Thread wakes │ ◄── SPURIOUS!        │
│   │ Condition:   │                    │ Condition:   │                      │
│   │ TRUE ✓       │                    │ FALSE ✗      │                      │
│   └──────┬───────┘                    └──────┬───────┘                      │
│          │                                   │                              │
│          ▼                                   ▼                              │
│   ┌──────────────┐                    ┌──────────────┐                      │
│   │ Process data │                    │ Must re-check│                      │
│   │              │                    │ and go back  │                      │
│   │              │                    │ to sleep!    │                      │
│   └──────────────┘                    └──────────────┘                      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Why Do Spurious Wakeups Happen?

### 1. OS/Kernel Implementation

```
┌─────────────────────────────────────────────────────────────────┐
│                    Under the Hood (Linux)                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   condition_variable::wait() uses futex() on Linux              │
│                                                                 │
│   ┌─────────────┐     ┌─────────────┐     ┌─────────────┐       │
│   │ User Space  │     │   Kernel    │     │  Hardware   │       │
│   │             │────▶│   futex()   │────▶│  Interrupt  │       │
│   │ cv.wait()   │     │             │     │             │       │
│   └─────────────┘     └─────────────┘     └─────────────┘       │
│                              │                                  │
│                              ▼                                  │
│                       Reasons for spurious wakeup:              │
│                       • Signal interruption (EINTR)             │
│                       • Kernel scheduling decisions             │
│                       • Memory contention                       │
│                       • Performance optimization                │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2. Performance Trade-off

The POSIX/C++ standard **allows** spurious wakeups because:
- Guaranteeing no spurious wakeups would require expensive synchronization
- It's cheaper to let applications handle rare spurious wakeups
- Allows kernel optimizations

### 3. Multi-Processor Complexities

On multi-core systems:
- Race conditions in the kernel's wait queue management
- Cache coherency issues
- Multiple threads waiting on same condition variable

---

## How to Handle Spurious Wakeups

### Pattern 1: While Loop (Classic Approach)

```cpp
std::mutex mtx;
std::condition_variable cv;
bool data_ready = false;

void consumer() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // CORRECT: Loop until condition is actually true
    while (!data_ready) {
        cv.wait(lock);  // May wake spuriously, loop handles it
    }
    
    // Safe to proceed - condition verified
    process_data();
}
```

### Pattern 2: Predicate Overload (Modern C++ - Recommended)

```cpp
void consumer() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // BEST: Predicate handles spurious wakeups internally
    cv.wait(lock, [] { return data_ready; });
    
    // Equivalent to:
    // while (!data_ready) { cv.wait(lock); }
    
    process_data();
}
```

### Pattern 3: wait_for / wait_until with Predicate

```cpp
void consumer_with_timeout() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Wait with timeout AND spurious wakeup protection
    bool success = cv.wait_for(
        lock,
        std::chrono::seconds(5),
        [] { return data_ready; }  // Predicate
    );
    
    if (success) {
        process_data();
    } else {
        handle_timeout();
    }
}
```

---

## Code Examples

### Example 1: Demonstrating Spurious Wakeup Vulnerability

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
int shared_data = 0;
bool ready = false;

// WRONG: Vulnerable to spurious wakeups
void bad_consumer() {
    std::unique_lock<std::mutex> lock(mtx);
    
    cv.wait(lock);  // NO condition check!
    
    // BUG: May execute even when ready == false
    std::cout << "Bad consumer: data = " << shared_data << std::endl;
    std::cout << "But ready = " << std::boolalpha << ready << std::endl;
}

// CORRECT: Protected against spurious wakeups
void good_consumer() {
    std::unique_lock<std::mutex> lock(mtx);
    
    cv.wait(lock, [] { return ready; });  // Predicate check
    
    // Safe: ready is guaranteed to be true
    std::cout << "Good consumer: data = " << shared_data << std::endl;
}

void producer() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        shared_data = 42;
        ready = true;
    }
    cv.notify_all();
}

int main() {
    std::thread t1(good_consumer);
    std::thread t2(producer);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Example 2: Thread-Safe Queue with Proper Wakeup Handling

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_ = false;

public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }
    
    // Blocking pop with spurious wakeup protection
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Predicate handles spurious wakeups
        cv_.wait(lock, [this] { 
            return !queue_.empty() || shutdown_; 
        });
        
        if (shutdown_ && queue_.empty()) {
            return std::nullopt;  // Queue closed
        }
        
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
    
    // Pop with timeout
    std::optional<T> try_pop_for(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // wait_for with predicate - handles both timeout AND spurious wakeups
        bool success = cv_.wait_for(lock, timeout, [this] {
            return !queue_.empty() || shutdown_;
        });
        
        if (!success || (shutdown_ && queue_.empty())) {
            return std::nullopt;
        }
        
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
    
    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }
};

int main() {
    ThreadSafeQueue<int> queue;
    
    // Consumer thread
    std::thread consumer([&queue] {
        while (auto value = queue.pop()) {
            std::cout << "Consumed: " << *value << std::endl;
        }
        std::cout << "Consumer finished" << std::endl;
    });
    
    // Producer
    for (int i = 1; i <= 5; ++i) {
        queue.push(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    queue.close();
    consumer.join();
    
    return 0;
}
```

### Example 3: Event Class with Spurious Wakeup Protection

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

class Event {
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool signaled_ = false;
    bool auto_reset_;

public:
    explicit Event(bool auto_reset = false) 
        : auto_reset_(auto_reset) {}
    
    void signal() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            signaled_ = true;
        }
        cv_.notify_all();
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        signaled_ = false;
    }
    
    // Wait indefinitely - handles spurious wakeups
    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return signaled_; });
        
        if (auto_reset_) {
            signaled_ = false;
        }
    }
    
    // Wait with timeout - handles spurious wakeups
    bool wait_for(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        bool result = cv_.wait_for(lock, timeout, [this] { 
            return signaled_; 
        });
        
        if (result && auto_reset_) {
            signaled_ = false;
        }
        
        return result;
    }
};

int main() {
    Event event(true);  // auto-reset event
    
    std::thread worker([&event] {
        std::cout << "Worker: waiting for event..." << std::endl;
        
        if (event.wait_for(std::chrono::seconds(5))) {
            std::cout << "Worker: event received!" << std::endl;
        } else {
            std::cout << "Worker: timeout!" << std::endl;
        }
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Main: signaling event" << std::endl;
    event.signal();
    
    worker.join();
    return 0;
}
```

---

## Comparison: Handling Methods

| Method | Syntax | Spurious-Safe | Recommended |
|--------|--------|---------------|-------------|
| Raw wait() | `cv.wait(lock)` | ❌ No | ❌ Never use alone |
| While loop | `while(!cond) cv.wait(lock)` | ✅ Yes | ✅ OK |
| Predicate | `cv.wait(lock, predicate)` | ✅ Yes | ✅ Best |
| wait_for + pred | `cv.wait_for(lock, dur, pred)` | ✅ Yes | ✅ Best for timeouts |

---

## What the Standard Says

From the C++ Standard (since C++11):

> *"This function may wake up spuriously and return even if the predicate still evaluates to false."*

This is **by design** - implementations are allowed to have spurious wakeups for performance reasons.

---

## Common Anti-Patterns

### Anti-Pattern 1: Naked Wait

```cpp
// WRONG: No condition check
cv.wait(lock);
do_work();  // May run at wrong time!
```

### Anti-Pattern 2: If Instead of While

```cpp
// WRONG: Only checks once
if (!ready) {
    cv.wait(lock);
}
do_work();  // Still unsafe - spurious wakeup not handled!
```

### Anti-Pattern 3: Checking After Wait

```cpp
// WRONG: Check is outside the loop
cv.wait(lock);
if (ready) {  // Too late - already woke up
    do_work();
}
```

---

## Key Takeaways

| Aspect | Best Practice |
|--------|--------------|
| **Always assume** | Spurious wakeups can happen |
| **Always use** | Predicate or while-loop |
| **Preferred method** | `cv.wait(lock, predicate)` |
| **For timeouts** | `cv.wait_for(lock, duration, predicate)` |
| **Never use** | Naked `cv.wait(lock)` without condition |

---

## Related Questions
- [Q6: How does condition_variable::wait() work internally?](./answer_q6_condition_variable_wait.md)
- [Q8: Producer-Consumer Pattern](./answer_q8_producer_consumer.md)
- [Q9: notify_one() vs notify_all()](./answer_q9_notify_one_vs_all.md)
