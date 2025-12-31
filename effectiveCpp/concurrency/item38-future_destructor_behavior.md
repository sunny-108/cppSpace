# Item 38: Be Aware of Varying Thread Handle Destructor Behavior

## Simple Explanation

**Main Idea**: When you create a future (like from `std::async`), the destructor behavior changes depending on how it was created. Sometimes the destructor **blocks and waits**, sometimes it **doesn't**. This inconsistency can cause bugs if you don't know the rules!

Think of it like lending a book:
- **Normal case**: You return the book to the library, done immediately
- **Special case**: You must wait until the borrower returns it before you can leave the library

## The Surprising Problem

```cpp
{
    auto future = std::async(std::launch::async, longRunningTask);
    // Do some work...
}  // ← What happens here when future is destroyed?
```

**Answer**: It depends! The destructor might:
1. **Block** and wait for the task to complete (SURPRISING!)
2. **Return immediately** without waiting (EXPECTED)

## The Rules for std::future Destructors

A `std::future` destructor **BLOCKS** (waits for the task) if **ALL** these conditions are true:

1. ✅ The future refers to a **shared state** created by `std::async`
2. ✅ The task's launch policy is `std::launch::async` (runs in new thread)
3. ✅ This is the **last** future referring to the shared state

Otherwise, the destructor returns immediately.

### Visual Representation

```cpp
// BLOCKING destructor (meets all 3 conditions)
{
    auto future = std::async(std::launch::async, task);
    // 1. Created by std::async ✓
    // 2. Launch policy is async ✓
    // 3. Last reference ✓
}  // ← BLOCKS here until task completes!

// NON-BLOCKING destructor (doesn't meet all conditions)
{
    std::packaged_task<int()> pt(task);
    auto future = pt.get_future();
    std::thread t(std::move(pt));
    t.detach();
}  // ← Returns immediately (not created by std::async)
```

## Examples of Different Behaviors

### Example 1: Blocking Destructor (std::async)

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <chrono>

void blockingExample() {
    std::cout << "Starting blocking example..." << std::endl;
    auto start = std::chrono::steady_clock::now();
    
    {
        auto future = std::async(std::launch::async, []() {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            std::cout << "Task finished!" << std::endl;
        });
        
        std::cout << "Future created, doing other work..." << std::endl;
        // Don't call get() or wait()
    }  // ← Destructor BLOCKS for 3 seconds here!
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "Blocked for " << duration.count() << " seconds" << std::endl;
}
```

**Output**:
```
Starting blocking example...
Future created, doing other work...
Task finished!
Blocked for 3 seconds
```

### Example 2: Non-Blocking Destructor (std::packaged_task)

```cpp
void nonBlockingExample() {
    std::cout << "\nStarting non-blocking example..." << std::endl;
    auto start = std::chrono::steady_clock::now();
    
    {
        std::packaged_task<void()> task([]() {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            std::cout << "Task finished (you might not see this)!" << std::endl;
        });
        
        auto future = task.get_future();
        std::thread t(std::move(task));
        t.detach();
        
        std::cout << "Future created, doing other work..." << std::endl;
    }  // ← Destructor returns IMMEDIATELY!
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Returned immediately after " << duration.count() << " ms" << std::endl;
    
    // Wait a bit to potentially see the task output
    std::this_thread::sleep_for(std::chrono::seconds(4));
}
```

**Output**:
```
Starting non-blocking example...
Future created, doing other work...
Returned immediately after 1 ms
Task finished (you might not see this)!
```

### Example 3: Non-Blocking with Deferred Policy

```cpp
void deferredExample() {
    std::cout << "\nDeferred example..." << std::endl;
    auto start = std::chrono::steady_clock::now();
    
    {
        auto future = std::async(std::launch::deferred, []() {
            std::cout << "This never runs!" << std::endl;
        });
        
        std::cout << "Future created with deferred policy..." << std::endl;
    }  // ← Returns immediately (deferred, not async)
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Returned after " << duration.count() << " ms" << std::endl;
}
```

### Example 4: Multiple Futures Sharing State

```cpp
void sharedStateExample() {
    std::cout << "\nShared state example..." << std::endl;
    
    auto future1 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 42;
    });
    
    auto future2 = future1.share();  // Create shared_future
    
    {
        auto future3 = future2;  // Copy shared_future
        std::cout << "future3 going out of scope..." << std::endl;
    }  // ← Doesn't block (not the LAST reference)
    
    std::cout << "future3 destroyed, but others still exist" << std::endl;
    
    {
        auto localFuture = std::move(future1);
        std::cout << "localFuture going out of scope..." << std::endl;
    }  // ← Doesn't block (moved, so it's empty)
    
    std::cout << "All done, waiting for future2..." << std::endl;
    std::cout << "Result: " << future2.get() << std::endl;
}
```

## Complete Working Example

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

void heavyTask(const std::string& name, int seconds) {
    std::cout << "  [" << name << "] Task started" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    std::cout << "  [" << name << "] Task completed" << std::endl;
}

void demonstrateBlockingBehavior() {
    std::cout << "\n=== 1. std::async with async policy (BLOCKS) ===" << std::endl;
    std::cout << "Creating future..." << std::endl;
    
    auto start = std::chrono::steady_clock::now();
    {
        auto fut = std::async(std::launch::async, heavyTask, "Async", 2);
        std::cout << "Exiting scope (destructor will block)..." << std::endl;
    }  // Blocks here!
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start).count();
    
    std::cout << "Destructor blocked for " << duration << " seconds\n";
}

void demonstrateNonBlockingBehavior() {
    std::cout << "\n=== 2. std::packaged_task (DOESN'T BLOCK) ===" << std::endl;
    std::cout << "Creating future..." << std::endl;
    
    auto start = std::chrono::steady_clock::now();
    {
        std::packaged_task<void(std::string, int)> task(heavyTask);
        auto fut = task.get_future();
        std::thread t(std::move(task), "Packaged", 2);
        t.detach();
        
        std::cout << "Exiting scope (destructor won't block)..." << std::endl;
    }  // Returns immediately!
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    
    std::cout << "Destructor returned after " << duration << " ms\n";
    std::this_thread::sleep_for(3s);  // Give task time to finish
}

void demonstrateDeferredBehavior() {
    std::cout << "\n=== 3. std::async with deferred policy (DOESN'T BLOCK) ===" << std::endl;
    std::cout << "Creating future..." << std::endl;
    
    auto start = std::chrono::steady_clock::now();
    {
        auto fut = std::async(std::launch::deferred, heavyTask, "Deferred", 2);
        std::cout << "Exiting scope (destructor won't block)..." << std::endl;
    }  // Returns immediately! (task never runs)
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    
    std::cout << "Destructor returned after " << duration << " ms\n";
}

void demonstrateMovedFuture() {
    std::cout << "\n=== 4. Moved future (DOESN'T BLOCK) ===" << std::endl;
    
    auto fut1 = std::async(std::launch::async, heavyTask, "Moved", 2);
    std::cout << "Future created, now moving it..." << std::endl;
    
    auto fut2 = std::move(fut1);  // fut1 is now empty
    
    std::cout << "Destroying original future (now empty)..." << std::endl;
    auto start = std::chrono::steady_clock::now();
    {
        auto temp = std::move(fut1);  // Move empty future
    }  // Doesn't block (no shared state)
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    
    std::cout << "Empty future destructor: " << duration << " ms\n";
    
    std::cout << "Now destroying the real future..." << std::endl;
    start = std::chrono::steady_clock::now();
    {
        auto temp = std::move(fut2);
    }  // This one BLOCKS!
    duration = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start).count();
    
    std::cout << "Real future destructor blocked for " << duration << " seconds\n";
}

int main() {
    demonstrateBlockingBehavior();
    demonstrateNonBlockingBehavior();
    demonstrateDeferredBehavior();
    demonstrateMovedFuture();
    
    return 0;
}
```

## Why This Design?

The C++ standard committee made `std::async` futures block on destruction to prevent:

### Problem Without Blocking:
```cpp
void badExample() {
    std::async(std::launch::async, []() {
        deleteAllFiles();  // Dangerous operation!
    });
    // If destructor doesn't block, this returns immediately
    // The task might still be running in the background!
}
// Function returns, but deleteAllFiles() is still running - DISASTER!
```

### With Blocking:
```cpp
void goodExample() {
    std::async(std::launch::async, []() {
        deleteAllFiles();
    });
}  // Blocks here, ensures task completes before returning
// Safe: we know deleteAllFiles() has finished
```

## Decision Table

| How Future Was Created | Blocks on Destruction? |
|------------------------|------------------------|
| `std::async(std::launch::async, f)` | ✅ YES (last reference) |
| `std::async(std::launch::deferred, f)` | ❌ NO |
| `std::async(f)` (default) | Maybe (if runs async) |
| `std::packaged_task` | ❌ NO |
| `std::promise` | ❌ NO |
| Moved-from future | ❌ NO (empty) |
| Shared future (not last) | ❌ NO |

## Practical Implications

### 1. Implicit Joins Can Surprise You

```cpp
void processData() {
    auto future = std::async(std::launch::async, longComputation);
    
    if (errorCondition) {
        return;  // ← Blocks here! Waits for longComputation!
    }
    
    // More code...
}  // ← Also blocks here!
```

### 2. Exception Safety

```cpp
void riskyOperation() {
    auto future = std::async(std::launch::async, task);
    
    doSomethingRisky();  // Might throw
    
    // If exception thrown, destructor still blocks!
}  // ← Blocks even during stack unwinding
```

### 3. Be Explicit About Intent

```cpp
// ❌ Implicit blocking - unclear
{
    auto fut = std::async(std::launch::async, work);
}

// ✅ Explicit waiting - clear intent
{
    auto fut = std::async(std::launch::async, work);
    fut.wait();  // Make the wait explicit
}

// ✅ Or use the result
{
    auto fut = std::async(std::launch::async, work);
    auto result = fut.get();  // Clear we need the result
}
```

### 4. Detaching-Like Behavior

```cpp
// If you want "fire and forget" semantics:
std::thread(work).detach();  // Traditional way

// Or with async (but it WILL block on destruction):
auto fut = std::async(std::launch::async, work);
// Must ensure fut lives long enough

// Better: Keep future alive
std::vector<std::future<void>> futures;
futures.push_back(std::async(std::launch::async, work));
// Destructor of futures vector will block until all tasks complete
```

## Common Pitfalls

### Pitfall 1: Temporary Future

```cpp
// ❌ BAD: Immediate destruction and blocking!
std::async(std::launch::async, longTask);  // Blocks immediately!
// This is equivalent to:
// { auto temp = std::async(...); } // temp destroyed, blocks

// ✅ GOOD: Keep the future alive
auto future = std::async(std::launch::async, longTask);
```

### Pitfall 2: Container of Futures

```cpp
std::vector<std::future<int>> futures;

for (int i = 0; i < 10; ++i) {
    futures.push_back(std::async(std::launch::async, compute, i));
}

// All good here, tasks running concurrently

// ⚠️ CAREFUL: When futures destroyed, blocks until ALL complete
futures.clear();  // Blocks until all 10 tasks finish!
```

### Pitfall 3: Early Return

```cpp
void processItems(const std::vector<int>& items) {
    auto future = std::async(std::launch::async, processInBackground, items);
    
    if (items.empty()) {
        return;  // ← Blocks! Even though we're just checking emptiness
    }
    
    // Continue processing...
}
```

## Best Practices

1. **Be aware that `std::async` futures can block** on destruction
2. **Make waits explicit** with `.wait()` or `.get()` for clarity
3. **Don't create temporary futures** unless you want immediate blocking
4. **Use `std::launch::async` explicitly** to know blocking will occur
5. **Consider using `std::packaged_task`** if you want non-blocking destructors
6. **Document blocking behavior** in your code comments

## Summary

- `std::future` destructors from `std::async(std::launch::async, ...)` **BLOCK**
- Other futures (`packaged_task`, `promise`, `deferred`) **don't block**
- This prevents "fire and forget" tasks from causing problems
- Be explicit about waits to make code intent clear
- Understand the rules to avoid surprising performance issues

**Remember**: "std::async futures are special - they block on destruction!"
