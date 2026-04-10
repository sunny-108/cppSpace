# Chapter 4: Synchronizing Concurrent Operations - Part 1

## Introduction

Sometimes threads need to wait for something to happen before they can continue. Chapter 4 teaches you how to make threads wait efficiently without wasting CPU cycles, and how to communicate results between threads.

---

## Part 1: Waiting for Events and Futures

---

## 1. Waiting for an Event or Condition

### 1.1 The Problem: Busy-Waiting (Don't Do This!)

**Simple Explanation:**  
Imagine constantly checking your mailbox every second to see if mail arrived. You're wasting time and energy! This is called "busy-waiting" or "polling."

**Bad Example: Busy-Waiting**
```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

bool data_ready = false;
std::mutex mtx;

void wait_for_data_badly() {
    // TERRIBLE approach - wastes CPU!
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (data_ready) {
                break; // Data is ready!
            }
        }
        // Still wastes CPU checking repeatedly
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << "Data is ready! Processing...\n";
}

void prepare_data() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    {
        std::lock_guard<std::mutex> lock(mtx);
        data_ready = true;
    }
    std::cout << "Data prepared!\n";
}

int main() {
    std::thread t1(wait_for_data_badly);
    std::thread t2(prepare_data);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

**Why it's bad:**
- Wastes CPU cycles checking repeatedly
- Uses locks unnecessarily
- Inefficient use of resources

---

## 2. Condition Variables: The Right Way to Wait

### 2.1 What is a Condition Variable?

**Simple Explanation:**  
A condition variable is like a doorbell. Instead of constantly checking if someone is at the door, you wait for the doorbell to ring. When it rings, you wake up and answer the door.

**Key Components:**
- `std::condition_variable` - The doorbell
- `wait()` - Go to sleep until doorbell rings
- `notify_one()` - Ring doorbell for one waiting thread
- `notify_all()` - Ring doorbell for all waiting threads

### 2.2 Basic Condition Variable Example

**Example: Producer-Consumer with Condition Variable**
```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> data_queue;
bool done = false;

void producer() {
    for (int i = 1; i <= 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(i);
            std::cout << "Produced: " << i << std::endl;
        }
        
        cv.notify_one(); // Wake up one waiting consumer
    }
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        done = true;
    }
    cv.notify_all(); // Wake up all consumers to check 'done'
}

void consumer(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait until queue has data or producer is done
        cv.wait(lock, []{ return !data_queue.empty() || done; });
        
        if (!data_queue.empty()) {
            int value = data_queue.front();
            data_queue.pop();
            lock.unlock(); // Unlock before processing
            
            std::cout << "Consumer " << id << " consumed: " << value << std::endl;
        } else if (done) {
            break; // Producer finished and queue is empty
        }
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons1(consumer, 1);
    std::thread cons2(consumer, 2);
    
    prod.join();
    cons1.join();
    cons2.join();
    
    std::cout << "All done!\n";
    return 0;
}
```

**How it works:**
1. **Consumer calls `wait()`** → Releases the lock and goes to sleep
2. **Producer produces data** → Holds lock, adds to queue
3. **Producer calls `notify_one()`** → Wakes up one sleeping consumer
4. **Consumer wakes up** → Automatically re-acquires lock, checks condition
5. **If condition true** → Processes data
6. **If condition false** → Goes back to sleep (spurious wakeup)

### 2.3 Understanding wait() with Predicate

**Simple Explanation:**  
The predicate (lambda function) is the condition you're waiting for. `wait()` keeps checking it to handle spurious wakeups automatically.

**Syntax:**
```cpp
// Without predicate (manual loop needed)
while (!condition) {
    cv.wait(lock);
}

// With predicate (automatic loop)
cv.wait(lock, []{ return condition; });
```

**Example: Thread-Safe Queue with Condition Variable**
```cpp
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mtx;
    std::queue<T> data_queue;
    std::condition_variable cv;
    
public:
    ThreadSafeQueue() {}
    
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        data_queue.push(std::move(value));
        cv.notify_one(); // Wake up one waiting thread
    }
    
    // Wait until queue has data, then pop
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !data_queue.empty(); });
        value = std::move(data_queue.front());
        data_queue.pop();
    }
    
    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !data_queue.empty(); });
        std::shared_ptr<T> result(
            std::make_shared<T>(std::move(data_queue.front()))
        );
        data_queue.pop();
        return result;
    }
    
    // Try to pop without waiting
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (data_queue.empty()) {
            return false;
        }
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data_queue.empty();
    }
};
```

### 2.4 Important Notes about Condition Variables

**Spurious Wakeups:**
```cpp
// Condition variable might wake up even if nobody called notify!
// Always use a predicate or loop:

// WRONG - vulnerable to spurious wakeups:
cv.wait(lock);
process_data();

// RIGHT - protected against spurious wakeups:
cv.wait(lock, []{ return data_ready; });
process_data();
```

**Why use unique_lock instead of lock_guard?**
```cpp
// Condition variable needs to unlock and relock
// lock_guard can't do that, but unique_lock can

std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock); // Internally: unlock → sleep → wake up → lock
```

---

## 3. Waiting for One-Off Events with Futures

### 3.1 What is a Future?

**Simple Explanation:**  
A future is like an online order tracking system. You place an order (start a task), get a tracking number (future), and later check if your package (result) has arrived.

**Key Concepts:**
- **Future** (`std::future<T>`) - Receives the result
- **Promise** (`std::promise<T>`) - Provides the result
- One-time use: Once you get the result, the future is "consumed"

### 3.2 Using std::promise and std::future

**Example: Basic Promise and Future**
```cpp
#include <iostream>
#include <thread>
#include <future>
#include <chrono>

void compute_square(std::promise<int>& prom, int value) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    int result = value * value;
    prom.set_value(result); // Fulfill the promise
    
    std::cout << "Computation complete!\n";
}

int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future(); // Get the future
    
    std::thread t(compute_square, std::ref(prom), 10);
    
    std::cout << "Waiting for result...\n";
    
    // This blocks until the result is available
    int result = fut.get();
    
    std::cout << "Result: " << result << std::endl;
    
    t.join();
    return 0;
}
```

**Output:**
```
Waiting for result...
Computation complete!
Result: 100
```

### 3.3 Multiple Ways to Get Results from Threads

**Example: Comparing Different Approaches**
```cpp
#include <iostream>
#include <thread>
#include <future>

// Method 1: Using shared variable (needs synchronization)
void method1_shared_variable() {
    int result = 0;
    std::mutex mtx;
    
    std::thread t([&result, &mtx]() {
        std::lock_guard<std::mutex> lock(mtx);
        result = 42;
    });
    
    t.join();
    std::cout << "Method 1 result: " << result << std::endl;
}

// Method 2: Using promise and future (cleaner!)
void method2_promise_future() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    std::thread t([&prom]() {
        prom.set_value(42);
    });
    
    int result = fut.get(); // Automatically synchronizes!
    std::cout << "Method 2 result: " << result << std::endl;
    
    t.join();
}

int main() {
    method1_shared_variable();
    method2_promise_future();
    return 0;
}
```

### 3.4 Passing Exceptions with Futures

**Simple Explanation:**  
If something goes wrong in the thread, you can pass the exception through the future to the waiting thread.

**Example: Exception Handling with Futures**
```cpp
#include <iostream>
#include <future>
#include <stdexcept>

void risky_computation(std::promise<int>& prom, int value) {
    try {
        if (value < 0) {
            throw std::runtime_error("Negative values not allowed!");
        }
        prom.set_value(value * 2);
    } catch (...) {
        // Capture and store the exception
        prom.set_exception(std::current_exception());
    }
}

int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    std::thread t(risky_computation, std::ref(prom), -5);
    
    try {
        int result = fut.get(); // Exception re-thrown here!
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    t.join();
    return 0;
}
```

**Output:**
```
Caught exception: Negative values not allowed!
```

---

## 4. Making (std::)Futures from (std::)Promises

### 4.1 The Promise-Future Contract

**Simple Explanation:**  
Think of promise-future like a mailbox system:
- **Promise** = The person who will put mail in the mailbox
- **Future** = The person waiting to receive mail from the mailbox
- Each promise can have only ONE future

**Example: Promise-Future Lifecycle**
```cpp
#include <iostream>
#include <future>

int main() {
    // Create promise
    std::promise<std::string> prom;
    
    // Get future from promise (can only do this once!)
    std::future<std::string> fut = prom.get_future();
    
    // Create thread that will fulfill promise
    std::thread t([](std::promise<std::string> p) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        p.set_value("Hello from thread!");
    }, std::move(prom)); // Move promise into thread
    
    std::cout << "Waiting for message...\n";
    
    // Wait for result
    std::string message = fut.get();
    std::cout << "Received: " << message << std::endl;
    
    t.join();
    return 0;
}
```

### 4.2 Breaking the Promise

**Simple Explanation:**  
If the thread with the promise dies before setting a value, the future receives a `std::future_error` exception.

**Example: Broken Promise**
```cpp
#include <iostream>
#include <future>

int main() {
    std::future<int> fut;
    
    {
        std::promise<int> prom;
        fut = prom.get_future();
        
        // Promise goes out of scope without set_value!
    } // Promise destroyed here
    
    try {
        int value = fut.get(); // Will throw!
    } catch (const std::future_error& e) {
        std::cout << "Future error: " << e.what() << std::endl;
        std::cout << "Error code: " << e.code() << std::endl;
    }
    
    return 0;
}
```

---

## 5. Waiting from Multiple Threads (std::shared_future)

### 5.1 The Problem with std::future

**Simple Explanation:**  
`std::future` is like a one-time coupon – once you use it (call `get()`), it's gone. But what if multiple threads want the same result?

**Example: Problem with std::future**
```cpp
#include <future>

void try_multiple_gets() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    prom.set_value(42);
    
    int value1 = fut.get(); // OK
    // int value2 = fut.get(); // ERROR! Can't call get() twice!
}
```

### 5.2 Solution: std::shared_future

**Simple Explanation:**  
`std::shared_future` is like a broadcast – multiple threads can all receive the same result.

**Example: Multiple Threads Waiting**
```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>

int compute_value() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 42;
}

int main() {
    // Create a shared_future
    std::promise<int> prom;
    std::shared_future<int> shared_fut = prom.get_future().share();
    
    // Multiple threads can wait on the same shared_future
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([shared_fut, i]() {
            std::cout << "Thread " << i << " waiting...\n";
            int value = shared_fut.get(); // All can call get()!
            std::cout << "Thread " << i << " got: " << value << std::endl;
        });
    }
    
    // Fulfill the promise after threads are waiting
    std::this_thread::sleep_for(std::chrono::seconds(1));
    prom.set_value(compute_value());
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

**Output:**
```
Thread 0 waiting...
Thread 1 waiting...
Thread 2 waiting...
Thread 3 waiting...
Thread 4 waiting...
Thread 0 got: 42
Thread 1 got: 42
Thread 2 got: 42
Thread 3 got: 42
Thread 4 got: 42
```

### 5.3 Creating shared_future

**Three Ways to Create:**
```cpp
#include <future>

int main() {
    std::promise<int> prom;
    
    // Method 1: From std::future using share()
    std::future<int> fut = prom.get_future();
    std::shared_future<int> shared_fut1 = fut.share();
    
    // Method 2: Implicit conversion
    std::promise<int> prom2;
    std::shared_future<int> shared_fut2 = prom2.get_future();
    
    // Method 3: Using auto with share()
    std::promise<int> prom3;
    auto shared_fut3 = prom3.get_future().share();
    
    return 0;
}
```

---

## 6. Real-World Example: Parallel Data Processing

**Example: Processing Multiple Files with Futures**
```cpp
#include <iostream>
#include <future>
#include <vector>
#include <string>
#include <fstream>
#include <numeric>

// Simulate processing a file
int process_file(const std::string& filename) {
    std::cout << "Processing: " << filename << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Return simulated line count
    return filename.length() * 100;
}

int main() {
    std::vector<std::string> files = {
        "data1.txt", "data2.txt", "data3.txt", 
        "data4.txt", "data5.txt"
    };
    
    // Create promises and futures for each file
    std::vector<std::promise<int>> promises(files.size());
    std::vector<std::future<int>> futures;
    
    for (auto& prom : promises) {
        futures.push_back(prom.get_future());
    }
    
    // Launch threads to process files
    std::vector<std::thread> threads;
    for (size_t i = 0; i < files.size(); ++i) {
        threads.emplace_back([&promises, &files, i]() {
            int result = process_file(files[i]);
            promises[i].set_value(result);
        });
    }
    
    // Collect results
    int total_lines = 0;
    for (auto& fut : futures) {
        total_lines += fut.get();
    }
    
    std::cout << "\nTotal lines processed: " << total_lines << std::endl;
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

---

## Summary of Part 1

| Concept | Purpose | When to Use |
|---------|---------|-------------|
| **Condition Variable** | Efficient waiting for events | Producer-consumer, event notification |
| **std::promise** | Set a value from one thread | Return result from thread |
| **std::future** | Retrieve value in another thread | Single consumer of result |
| **std::shared_future** | Multiple threads get same value | Multiple consumers of result |

**Key Takeaways:**
1. **Never busy-wait** – Use condition variables or futures
2. **Condition variables** need unique_lock and predicates
3. **Futures** provide clean thread synchronization
4. **Promises** can transfer exceptions, not just values
5. **shared_future** allows multiple threads to access the same result

---

## Common Patterns

### Pattern 1: Event Notification
```cpp
std::condition_variable event;
std::mutex mtx;
bool flag = false;

// Waiter
std::unique_lock<std::mutex> lock(mtx);
event.wait(lock, []{ return flag; });

// Notifier
{
    std::lock_guard<std::mutex> lock(mtx);
    flag = true;
}
event.notify_all();
```

### Pattern 2: Thread Return Value
```cpp
std::promise<ResultType> prom;
std::future<ResultType> fut = prom.get_future();

std::thread t([](std::promise<ResultType> p) {
    ResultType result = compute();
    p.set_value(result);
}, std::move(prom));

ResultType value = fut.get();
t.join();
```

### Pattern 3: Broadcasting Results
```cpp
std::promise<int> prom;
auto shared_fut = prom.get_future().share();

// Multiple threads
for (int i = 0; i < 10; ++i) {
    threads.emplace_back([shared_fut]() {
        int value = shared_fut.get();
        // All threads get the same value
    });
}

prom.set_value(42); // Broadcast to all
```

Ready to move on to Part 2 for more advanced topics!
