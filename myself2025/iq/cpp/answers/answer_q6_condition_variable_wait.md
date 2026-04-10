# Q6: How does `std::condition_variable::wait()` work internally? Why must it be used with a mutex?

## Quick Answer

`std::condition_variable::wait()` atomically releases the mutex and puts the thread to sleep until notified. It **must** be used with a mutex to prevent race conditions between checking the condition and going to sleep.

---

## High-Level Design

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    Condition Variable Wait Mechanism                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   Thread A (Consumer)                    Thread B (Producer)                │
│   ──────────────────                    ───────────────────                 │
│                                                                             │
│   ┌─────────────────┐                   ┌─────────────────┐                 │
│   │ 1. Lock mutex   │                   │ 1. Lock mutex   │                 │
│   └────────┬────────┘                   └────────┬────────┘                 │
│            │                                     │                          │
│            ▼                                     ▼                          │
│   ┌─────────────────┐                   ┌─────────────────┐                 │
│   │ 2. Check        │                   │ 2. Modify       │                 │
│   │    condition    │                   │    shared data  │                 │
│   └────────┬────────┘                   └────────┬────────┘                 │
│            │                                     │                          │
│            ▼ (condition false)                   ▼                          │
│   ┌─────────────────────────┐           ┌─────────────────┐                 │
│   │ 3. ATOMIC OPERATION:    │           │ 3. Unlock mutex │                 │
│   │    - Release mutex      │           └────────┬────────┘                 │
│   │    - Add to wait queue  │                    │                          │
│   │    - Sleep              │                    ▼                          │
│   └────────┬────────────────┘           ┌─────────────────┐                 │
│            │                            │ 4. notify_one() │                 │
│            │ ◄────── WAKEUP ────────────│    or           │                 │
│            │                            │    notify_all() │                 │
│            ▼                            └─────────────────┘                 │
│   ┌─────────────────┐                                                       │
│   │ 4. Re-acquire   │                                                       │
│   │    mutex        │                                                       │
│   └────────┬────────┘                                                       │
│            │                                                                │
│            ▼                                                                │
│   ┌─────────────────┐                                                       │
│   │ 5. Re-check     │ ◄─── Handle spurious wakeups                          │
│   │    condition    │                                                       │
│   └─────────────────┘                                                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Internal Workings

### What `wait()` Does (Step-by-Step)

```cpp
// Conceptual pseudocode of what wait() does internally
void condition_variable::wait(unique_lock<mutex>& lock) {
    // 1. Add current thread to the wait queue
    add_to_wait_queue(this_thread);
    
    // 2. ATOMICALLY: release mutex AND go to sleep
    //    This is ONE atomic operation - critical for correctness!
    atomic_release_and_sleep(lock.mutex());
    
    // 3. Thread sleeps here until notified...
    //    (blocked, not consuming CPU)
    
    // 4. When woken up, re-acquire the mutex BEFORE returning
    lock.mutex().lock();
    
    // 5. Return to caller (mutex is now held)
}
```

### The Atomic Operation is Key

The critical insight is that **releasing the mutex and sleeping must be atomic**:

```
WITHOUT atomic operation (BROKEN):
─────────────────────────────────
Thread A                          Thread B
────────                          ────────
1. Check: queue empty
2. Release mutex
                                  3. Lock mutex
                                  4. Add item to queue
                                  5. notify_one()  ← Notification LOST!
                                  6. Unlock mutex
7. Sleep (waiting forever!)

WITH atomic operation (CORRECT):
────────────────────────────────
Thread A                          Thread B
────────                          ────────
1. Check: queue empty
2. [ATOMIC: Release mutex + Sleep]
                                  3. Lock mutex
                                  4. Add item to queue
                                  5. notify_one()  ← Thread A wakes up
                                  6. Unlock mutex
7. Wake up, re-acquire mutex
```

---

## Why Mutex is Required

### Reason 1: Protect Shared State

The condition being waited on typically involves shared data:

```cpp
// WRONG: No mutex protection
bool data_ready = false;  // shared state

// Thread A (Consumer)
while (!data_ready) {     // Race condition reading data_ready!
    cv.wait(lock);
}

// Thread B (Producer)  
data_ready = true;        // Race condition writing data_ready!
cv.notify_one();
```

### Reason 2: Prevent Lost Wakeups

Without mutex, notifications can be lost:

```cpp
// The "lost wakeup" problem without proper mutex usage:

// Thread A                      Thread B
// ────────                      ────────
if (!ready) {                    
    // <── Thread B runs here ──>
                                 ready = true;
                                 cv.notify_one(); // LOST! A not sleeping yet
    cv.wait(lock);  // Sleeps forever, missed the notification
}
```

### Reason 3: Atomic Unlock-and-Sleep

The mutex allows `wait()` to atomically:
1. Release the lock
2. Add thread to wait queue
3. Put thread to sleep

This atomicity prevents the race condition window.

---

## Code Examples

### Example 1: Basic Producer-Consumer

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> data_queue;
bool finished = false;

void producer() {
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(i);
            std::cout << "Produced: " << i << std::endl;
        }
        cv.notify_one();  // Notify AFTER releasing lock (best practice)
    }
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    cv.notify_one();
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait with predicate (handles spurious wakeups)
        cv.wait(lock, [] { 
            return !data_queue.empty() || finished; 
        });
        
        // Process all available items
        while (!data_queue.empty()) {
            int value = data_queue.front();
            data_queue.pop();
            std::cout << "Consumed: " << value << std::endl;
        }
        
        if (finished && data_queue.empty()) {
            break;
        }
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons(consumer);
    
    prod.join();
    cons.join();
    
    return 0;
}
```

### Example 2: Wait with Predicate (Recommended Pattern)

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
    
    // METHOD 1: Manual loop (handles spurious wakeups)
    while (!ready) {
        cv.wait(lock);
    }
    
    // METHOD 2: Predicate overload (preferred - cleaner and safer)
    cv.wait(lock, [] { return ready; });
    
    // Equivalent to:
    // while (!predicate()) {
    //     wait(lock);
    // }
    
    std::cout << "Worker proceeding - condition met!" << std::endl;
}

void signaler() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
        std::cout << "Signaler: condition set to true" << std::endl;
    }
    cv.notify_one();
}

int main() {
    std::thread t1(worker);
    std::thread t2(signaler);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Example 3: Thread Pool Task Queue

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;

public:
    ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        
                        // Wait until there's a task or we're stopping
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });
                        
                        if (stop && tasks.empty()) {
                            return;  // Exit thread
                        }
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    task();  // Execute task outside the lock
                }
            });
        }
    }
    
    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }
    
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();  // Wake all threads
        
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
};

int main() {
    ThreadPool pool(4);
    
    for (int i = 0; i < 8; ++i) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " executed by thread " 
                      << std::this_thread::get_id() << std::endl;
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
```

---

## Common Mistakes to Avoid

### Mistake 1: Using `lock_guard` Instead of `unique_lock`

```cpp
// WRONG: lock_guard cannot be unlocked/relocked
std::lock_guard<std::mutex> lock(mtx);
cv.wait(lock);  // Compilation error!

// CORRECT: unique_lock allows unlock/relock
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock);  // Works - wait() can release and reacquire
```

### Mistake 2: Forgetting to Handle Spurious Wakeups

```cpp
// WRONG: No spurious wakeup handling
cv.wait(lock);
process_data();  // May run even if condition not met!

// CORRECT: Always use a predicate or loop
cv.wait(lock, [] { return condition_met; });
process_data();
```

### Mistake 3: Notifying While Holding the Lock

```cpp
// WORKS but suboptimal (notified thread blocks on mutex)
{
    std::lock_guard<std::mutex> lock(mtx);
    ready = true;
    cv.notify_one();  // Thread wakes but immediately blocks on mutex
}

// BETTER: Notify after releasing lock
{
    std::lock_guard<std::mutex> lock(mtx);
    ready = true;
}
cv.notify_one();  // Thread can proceed immediately
```

---

## Key Takeaways

| Aspect | Details |
|--------|---------|
| **Atomic Operation** | `wait()` atomically releases mutex + sleeps |
| **Mutex Requirement** | Required for protecting shared state and atomic unlock-sleep |
| **Spurious Wakeups** | Always use predicate or while-loop |
| **Lock Type** | Must use `std::unique_lock`, not `std::lock_guard` |
| **Notification** | `notify_one()` wakes one thread, `notify_all()` wakes all |

---

## Related Questions
- [Q7: What is spurious wakeup?](./answer_q7_spurious_wakeup.md)
- [Q8: Producer-Consumer Pattern](./answer_q8_producer_consumer.md)
- [Q9: notify_one() vs notify_all()](./answer_q9_notify_one_vs_all.md)
