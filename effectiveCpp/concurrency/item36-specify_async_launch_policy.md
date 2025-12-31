# Item 36: Specify std::launch::async if Asynchronicity is Essential

## Simple Explanation

**Main Idea**: When you use `std::async`, you should explicitly tell it HOW to run your task. By default, `std::async` might NOT run your code in a new thread - it might run it lazily when you call `.get()` or `.wait()`. This can cause unexpected behavior!

Think of it like ordering food:
- **Default `std::async`**: "I'll deliver it now OR you can pick it up later - I'll decide!"
- **`std::launch::async`**: "I'll start delivering it RIGHT NOW in a separate vehicle"
- **`std::launch::deferred`**: "I'll only make it when you arrive to pick it up"

## The Problem with Default std::async

### Default Behavior (Unpredictable!)
```cpp
auto future = std::async(doWork);  // What does this do?
```

This can do EITHER:
1. Run `doWork()` immediately in a new thread (asynchronous)
2. Run `doWork()` later when you call `future.get()` (deferred/lazy)

**The runtime decides**, and you don't know which!

## The Launch Policies

### 1. std::launch::async (Run NOW in new thread)
```cpp
auto future = std::async(std::launch::async, doWork);
// Guaranteed to start immediately in a separate thread
```

### 2. std::launch::deferred (Run LATER in current thread)
```cpp
auto future = std::async(std::launch::deferred, doWork);
// Won't run until you call future.get() or future.wait()
// Runs in the SAME thread that calls get()
```

### 3. Default (Runtime decides)
```cpp
auto future = std::async(doWork);
// Same as: std::async(std::launch::async | std::launch::deferred, doWork)
// Runtime picks one!
```

## Problems with Default Policy

### Problem 1: Thread-Local Storage is Unpredictable

```cpp
thread_local int threadLocalVar = 0;

void incrementThreadLocal() {
    ++threadLocalVar;
    std::cout << "Thread local value: " << threadLocalVar << std::endl;
}

// Default async - unpredictable behavior!
auto future = std::async(incrementThreadLocal);
future.get();
// Did it run in a new thread or this thread? You don't know!
```

### Problem 2: Timeout-Based Waits May Never Complete

```cpp
void doWork() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

auto future = std::async(doWork);  // Default policy

// This might wait FOREVER if deferred!
if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
    std::cout << "Done!" << std::endl;
} else {
    std::cout << "Still waiting..." << std::endl;  // Might print this!
}
```

**Why?** If the task was deferred, it never started! The timeout returns immediately with `std::future_status::deferred`, not `ready` or `timeout`.

### Problem 3: Task Might Never Run

```cpp
void criticalTask() {
    std::cout << "This MUST run!" << std::endl;
}

auto future = std::async(criticalTask);  // Default policy
// If we never call future.get() or future.wait(), and it's deferred,
// criticalTask() NEVER RUNS!

// Program ends here - task might not have executed!
```

## Complete Working Example

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <chrono>

void printThreadId(const std::string& msg) {
    std::cout << msg << " - Thread ID: " 
              << std::this_thread::get_id() << std::endl;
}

void simulateWork(const std::string& taskName, int seconds) {
    printThreadId(taskName + " started");
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    printThreadId(taskName + " finished");
}

// ❌ Problem: Default policy is unpredictable
void defaultPolicyProblem() {
    std::cout << "\n=== Default Policy (Unpredictable) ===" << std::endl;
    printThreadId("Main thread");
    
    auto future = std::async([]() {
        simulateWork("Default task", 1);
    });
    
    std::cout << "Checking if task is running..." << std::endl;
    auto status = future.wait_for(std::chrono::milliseconds(100));
    
    if (status == std::future_status::deferred) {
        std::cout << "Task is DEFERRED (will run on get())" << std::endl;
    } else if (status == std::future_status::ready) {
        std::cout << "Task is READY (already finished)" << std::endl;
    } else {
        std::cout << "Task is RUNNING (timeout)" << std::endl;
    }
    
    future.get();  // Actually run it (if deferred)
}

// ✅ Solution: Explicitly use std::launch::async
void explicitAsyncPolicy() {
    std::cout << "\n=== Explicit Async Policy ===" << std::endl;
    printThreadId("Main thread");
    
    auto future = std::async(std::launch::async, []() {
        simulateWork("Async task", 1);
    });
    
    std::cout << "Task started immediately in new thread!" << std::endl;
    auto status = future.wait_for(std::chrono::milliseconds(100));
    
    if (status == std::future_status::deferred) {
        std::cout << "Deferred (won't happen with async)" << std::endl;
    } else if (status == std::future_status::ready) {
        std::cout << "Already finished!" << std::endl;
    } else {
        std::cout << "Still running (this is expected)" << std::endl;
    }
    
    future.get();
}

// ✅ Using deferred explicitly
void explicitDeferredPolicy() {
    std::cout << "\n=== Explicit Deferred Policy ===" << std::endl;
    printThreadId("Main thread");
    
    auto future = std::async(std::launch::deferred, []() {
        simulateWork("Deferred task", 0);
    });
    
    std::cout << "Task NOT started yet (deferred)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Now calling get() - task will run NOW:" << std::endl;
    future.get();  // Runs in THIS thread
}

// Real-world example: Timeout with async
void timeoutExample() {
    std::cout << "\n=== Timeout Example ===" << std::endl;
    
    auto future = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 42;
    });
    
    std::cout << "Waiting up to 1 second..." << std::endl;
    
    if (future.wait_for(std::chrono::seconds(1)) == std::future_status::ready) {
        std::cout << "Result: " << future.get() << std::endl;
    } else {
        std::cout << "Timeout! Task still running..." << std::endl;
        // Still need to get() to clean up
        std::cout << "Waiting for completion..." << std::endl;
        std::cout << "Result: " << future.get() << std::endl;
    }
}

int main() {
    defaultPolicyProblem();
    explicitAsyncPolicy();
    explicitDeferredPolicy();
    timeoutExample();
    
    return 0;
}
```

## How to Handle Default Policy Safely

If you MUST use the default policy, check if it's deferred:

```cpp
auto future = std::async(doWork);

// Check if it's deferred
if (future.wait_for(std::chrono::seconds(0)) == std::future_status::deferred) {
    // It's deferred - runs on get()
    std::cout << "Task is deferred" << std::endl;
    future.get();
} else {
    // It's async - already running
    while (future.wait_for(std::chrono::milliseconds(100)) != 
           std::future_status::ready) {
        std::cout << "Still waiting..." << std::endl;
    }
    future.get();
}
```

## When to Use Each Policy

### Use `std::launch::async` when:
- ✅ You need the task to run concurrently RIGHT NOW
- ✅ You're using timeouts (`wait_for`, `wait_until`)
- ✅ You need the task to run on a different thread
- ✅ The task must run even if you might not call `get()`
- ✅ You're using thread-local storage

```cpp
// Must start immediately
auto future = std::async(std::launch::async, urgentTask);
```

### Use `std::launch::deferred` when:
- ✅ You want lazy evaluation
- ✅ You might not need the result
- ✅ You want to control WHEN the task runs
- ✅ You don't need a separate thread

```cpp
// Lazy evaluation - only compute if needed
auto future = std::async(std::launch::deferred, expensiveCalculation);

if (needResult) {
    auto result = future.get();  // Compute now
}
// If needResult is false, expensiveCalculation never runs!
```

### Avoid default policy when:
- ❌ Using `wait_for` or `wait_until`
- ❌ Using thread-local variables
- ❌ The task MUST execute
- ❌ You need predictable behavior

## Practical Helper Function

```cpp
// Helper to force async execution
template<typename F, typename... Args>
inline auto reallyAsync(F&& f, Args&&... args) {
    return std::async(std::launch::async,
                     std::forward<F>(f),
                     std::forward<Args>(args)...);
}

// Usage
auto future = reallyAsync(doWork);  // Always async!
```

## Summary Table

| Policy | When It Runs | Which Thread | Use Case |
|--------|--------------|--------------|----------|
| `std::launch::async` | Immediately | New thread | Concurrent execution needed |
| `std::launch::deferred` | On `get()`/`wait()` | Calling thread | Lazy evaluation |
| Default | Runtime decides | Depends | Generally avoid |

## Key Takeaways

1. **Always specify the launch policy** for predictable behavior
2. **Use `std::launch::async`** when you need true asynchronous execution
3. **Default policy can cause bugs** with timeouts and thread-local storage
4. **Deferred tasks don't run** until you call `get()` or `wait()`
5. **Check for deferred status** if using default policy

**Remember**: "If asynchronicity is essential, specify std::launch::async!"

## Common Pitfall

```cpp
// ❌ BAD: Might never timeout!
auto future = std::async(longTask);
if (future.wait_for(5s) == std::future_status::timeout) {
    // Might never reach here if deferred!
}

// ✅ GOOD: Guaranteed to timeout if needed
auto future = std::async(std::launch::async, longTask);
if (future.wait_for(5s) == std::future_status::timeout) {
    std::cout << "Task taking too long!" << std::endl;
}
```
