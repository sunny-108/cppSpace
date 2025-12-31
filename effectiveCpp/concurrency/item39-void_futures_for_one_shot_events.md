# Item 39: Consider void Futures for One-Shot Event Communication

## Simple Explanation

**Main Idea**: When you need one thread to signal another thread that an event has occurred (just once), consider using a `std::promise<void>` and `std::future<void>` pair. This is cleaner than using condition variables for simple one-time notifications.

Think of it like:
- **Condition Variable**: A bell that can ring multiple times, needs a mutex, and is complex
- **void Future**: A one-time signal flare - simple, clean, but only works once

## The Problem: Thread Communication

You have two threads:
- **Thread A**: Needs to wait for something to happen
- **Thread B**: Does work and signals when done

How do you communicate between them?

## Three Approaches

### Approach 1: Condition Variable (Traditional, Complex)

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::condition_variable cv;
std::mutex m;
bool dataReady = false;

void conditionVariableApproach() {
    std::cout << "=== Condition Variable Approach ===" << std::endl;
    
    // Waiting thread
    std::thread waiter([]() {
        std::cout << "Waiter: Waiting for data..." << std::endl;
        
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, []{ return dataReady; });  // Wait until dataReady is true
        
        std::cout << "Waiter: Data is ready!" << std::endl;
    });
    
    // Signaling thread
    std::thread signaler([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Signaler: Preparing data..." << std::endl;
        
        {
            std::lock_guard<std::mutex> lock(m);
            dataReady = true;
        }
        
        std::cout << "Signaler: Notifying waiter..." << std::endl;
        cv.notify_one();
    });
    
    waiter.join();
    signaler.join();
}
```

**Problems**:
- ❌ Need mutex, condition variable, AND a flag
- ❌ Easy to forget the flag (causes bugs)
- ❌ Verbose and error-prone
- ❌ Risk of spurious wakeups

### Approach 2: Shared Boolean with Polling (Bad!)

```cpp
std::atomic<bool> flag{false};

void pollingApproach() {
    std::cout << "\n=== Polling Approach (BAD) ===" << std::endl;
    
    // Waiting thread - WASTES CPU!
    std::thread waiter([]() {
        std::cout << "Waiter: Polling for flag..." << std::endl;
        
        while (!flag) {
            // Busy waiting - wastes CPU cycles!
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::cout << "Waiter: Flag is set!" << std::endl;
    });
    
    // Signaling thread
    std::thread signaler([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Signaler: Setting flag..." << std::endl;
        flag = true;
    });
    
    waiter.join();
    signaler.join();
}
```

**Problems**:
- ❌ Wastes CPU with busy-waiting
- ❌ Delay in detection (depends on polling interval)
- ❌ Not efficient

### Approach 3: void Future (✅ Best for One-Shot!)

```cpp
void voidFutureApproach() {
    std::cout << "\n=== void Future Approach (BEST) ===" << std::endl;
    
    std::promise<void> p;
    
    // Waiting thread
    std::thread waiter([fut = p.get_future()]() mutable {
        std::cout << "Waiter: Waiting for signal..." << std::endl;
        
        fut.wait();  // Simple! Just wait.
        
        std::cout << "Waiter: Signal received!" << std::endl;
    });
    
    // Signaling thread
    std::thread signaler([&p]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Signaler: Sending signal..." << std::endl;
        
        p.set_value();  // Signal!
    });
    
    waiter.join();
    signaler.join();
}
```

**Advantages**:
- ✅ Clean and simple
- ✅ No mutex needed
- ✅ No boolean flag needed
- ✅ No spurious wakeups
- ✅ Perfect for one-shot notifications

## Complete Working Example

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;

// Example 1: Simple one-shot notification
void simpleNotification() {
    std::cout << "=== Simple One-Shot Notification ===" << std::endl;
    
    std::promise<void> ready;
    std::future<void> readyFuture = ready.get_future();
    
    // Worker thread waits for signal to start
    std::thread worker([fut = std::move(readyFuture)]() mutable {
        std::cout << "Worker: Waiting for start signal..." << std::endl;
        fut.wait();
        std::cout << "Worker: Starting work!" << std::endl;
        std::this_thread::sleep_for(500ms);
        std::cout << "Worker: Work completed!" << std::endl;
    });
    
    // Main thread does setup then signals
    std::cout << "Main: Doing setup..." << std::endl;
    std::this_thread::sleep_for(1s);
    
    std::cout << "Main: Setup complete, signaling worker..." << std::endl;
    ready.set_value();
    
    worker.join();
}

// Example 2: Multiple waiters (using shared_future)
void multipleWaiters() {
    std::cout << "\n=== Multiple Waiters ===" << std::endl;
    
    std::promise<void> go;
    std::shared_future<void> goFuture = go.get_future();
    
    // Create multiple threads all waiting for same signal
    std::vector<std::thread> workers;
    for (int i = 1; i <= 3; ++i) {
        workers.emplace_back([i, fut = goFuture]() {
            std::cout << "Worker " << i << ": Waiting at starting line..." << std::endl;
            fut.wait();  // All wait for same signal
            std::cout << "Worker " << i << ": GO! Running..." << std::endl;
        });
    }
    
    std::cout << "Main: Ready... Set..." << std::endl;
    std::this_thread::sleep_for(1s);
    std::cout << "Main: GO!" << std::endl;
    go.set_value();  // All threads start together!
    
    for (auto& t : workers) {
        t.join();
    }
}

// Example 3: Initialization pattern
class ThreadSafeResource {
private:
    std::promise<void> initPromise;
    std::shared_future<void> initFuture;
    bool initialized = false;
    
public:
    ThreadSafeResource() : initFuture(initPromise.get_future()) {}
    
    void initialize() {
        std::cout << "Resource: Initializing..." << std::endl;
        std::this_thread::sleep_for(1s);
        initialized = true;
        std::cout << "Resource: Initialization complete!" << std::endl;
        initPromise.set_value();  // Signal initialization done
    }
    
    void use(int id) {
        std::cout << "User " << id << ": Waiting for initialization..." << std::endl;
        initFuture.wait();  // Wait for initialization
        std::cout << "User " << id << ": Using resource!" << std::endl;
    }
};

void initializationPattern() {
    std::cout << "\n=== Initialization Pattern ===" << std::endl;
    
    ThreadSafeResource resource;
    
    // Multiple users trying to use resource
    std::vector<std::thread> users;
    for (int i = 1; i <= 3; ++i) {
        users.emplace_back([&resource, i]() {
            resource.use(i);
        });
    }
    
    // Initialize after users are waiting
    std::this_thread::sleep_for(500ms);
    std::thread initializer([&resource]() {
        resource.initialize();
    });
    
    for (auto& t : users) {
        t.join();
    }
    initializer.join();
}

// Example 4: Task completion notification
void taskCompletionNotification() {
    std::cout << "\n=== Task Completion Notification ===" << std::endl;
    
    std::promise<void> taskDone;
    
    std::thread worker([&taskDone]() {
        std::cout << "Worker: Starting long task..." << std::endl;
        std::this_thread::sleep_for(2s);
        std::cout << "Worker: Task finished!" << std::endl;
        taskDone.set_value();  // Notify completion
    });
    
    auto future = taskDone.get_future();
    
    std::cout << "Main: Doing other work while task runs..." << std::endl;
    
    // Can check status without blocking
    while (future.wait_for(500ms) != std::future_status::ready) {
        std::cout << "Main: Still waiting... doing other work..." << std::endl;
    }
    
    std::cout << "Main: Task is complete!" << std::endl;
    worker.join();
}

int main() {
    simpleNotification();
    multipleWaiters();
    initializationPattern();
    taskCompletionNotification();
    
    return 0;
}
```

## When to Use void Futures

### ✅ Good Use Cases:
1. **One-time initialization** - Signal when resource is ready
2. **Start signal** - Multiple threads wait for "GO!"
3. **Completion notification** - Worker signals when done
4. **Suspend/resume** - Pause thread until signal received
5. **Gate/barrier** - Threads wait at checkpoint

### ❌ When NOT to Use:
1. **Multiple signals** - void future only works once!
2. **Need to check condition** - condition variable is better
3. **Complex synchronization** - need more sophisticated tools
4. **Need to reset** - can't reuse a promise/future pair

## Comparison Table

| Feature | Condition Variable | void Future |
|---------|-------------------|-------------|
| Simplicity | Complex (mutex + cv + flag) | Simple (promise + future) |
| One-shot events | ✅ Works but overkill | ✅ Perfect! |
| Multiple signals | ✅ Yes | ❌ No (one-shot only) |
| Spurious wakeups | ⚠️ Possible | ✅ None |
| Multiple waiters | ✅ notify_all() | ✅ shared_future |
| Reusable | ✅ Yes | ❌ No |

## Important Limitations

### Limitation 1: One-Shot Only

```cpp
std::promise<void> p;
auto fut = p.get_future();

p.set_value();  // First signal - OK
// p.set_value();  // ERROR! Can't signal twice
```

### Limitation 2: Can't Reset

```cpp
// ❌ Can't do this:
std::promise<void> p;
auto fut = p.get_future();
p.set_value();
// No way to "reset" p to reuse it

// ✅ Need to create new promise/future pair:
p = std::promise<void>();  // New promise
fut = p.get_future();      // New future
```

### Limitation 3: Shared State Overhead

```cpp
// void futures have some overhead from shared state
// For extremely performance-critical code, might use atomic
std::atomic<bool> flag{false};  // Lower overhead
// But loses benefits of futures (blocking wait, exception transport)
```

## Advanced Pattern: Cancellation Token

```cpp
class Task {
private:
    std::promise<void> cancelPromise;
    std::shared_future<void> cancelFuture;
    
public:
    Task() : cancelFuture(cancelPromise.get_future()) {}
    
    void run() {
        std::cout << "Task running..." << std::endl;
        
        // Check for cancellation periodically
        while (cancelFuture.wait_for(100ms) != std::future_status::ready) {
            std::cout << "Task: Working..." << std::endl;
            // Do work...
        }
        
        std::cout << "Task: Cancelled!" << std::endl;
    }
    
    void cancel() {
        std::cout << "Requesting cancellation..." << std::endl;
        cancelPromise.set_value();
    }
};

void cancellationExample() {
    std::cout << "\n=== Cancellation Pattern ===" << std::endl;
    
    Task task;
    
    std::thread worker([&task]() {
        task.run();
    });
    
    std::this_thread::sleep_for(500ms);
    task.cancel();
    
    worker.join();
}
```

## Best Practices

1. **Use void futures for simple one-shot signals** between threads
2. **Use shared_future** when multiple threads need the same signal
3. **Prefer condition variables** when you need multiple signals or complex conditions
4. **Document that it's one-shot** so others know it can't be reused
5. **Consider std::latch or std::barrier** (C++20) for more sophisticated patterns

## C++20 Alternatives

In C++20, there are better alternatives for some use cases:

```cpp
#include <latch>
#include <barrier>

// One-shot countdown synchronization
std::latch startLatch(1);
startLatch.wait();        // Wait
startLatch.count_down();  // Signal

// Reusable synchronization point
std::barrier syncPoint(3);
syncPoint.arrive_and_wait();  // Reusable!
```

## Summary

- **void futures are perfect for one-shot event communication**
- Much simpler than condition variables for simple cases
- No mutex, no flag, no spurious wakeups
- Use `shared_future<void>` for multiple waiters
- Remember: **one-shot only** - can't reuse
- For repeated signals, use condition variables or C++20 primitives

**Remember**: "For one-time 'go' signals, void futures are your friend!"
