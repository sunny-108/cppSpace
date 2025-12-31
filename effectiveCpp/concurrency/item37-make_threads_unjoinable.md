# Item 37: Make std::threads Unjoinable on All Paths

## Simple Explanation

**Main Idea**: Always ensure that every `std::thread` is either joined or detached before it's destroyed. If a joinable thread is destroyed, your program will terminate! Use RAII (Resource Acquisition Is Initialization) to guarantee this happens automatically.

Think of it like this:
- **Joinable thread**: A worker you hired but haven't settled with - you must either wait for them to finish (join) or release them to work independently (detach)
- **If you forget**: It's like the worker showing up at your door demanding to be dealt with, and your house explodes! 💥

## The Problem

### What Happens If You Don't Join or Detach?

```cpp
void oops() {
    std::thread t(doWork);
    // Oops! Neither join() nor detach() called
}  // ← BOOM! std::terminate() called, program crashes!
```

**Why?** The C++ standard says that if a `std::thread` destructor is called on a joinable thread (one that hasn't been joined or detached), the program must terminate.

## Understanding Joinable

A thread is **joinable** if it represents an actual thread of execution:

```cpp
std::thread t1;                    // Not joinable (default constructed)
std::thread t2(doWork);            // Joinable (running a task)
std::thread t3(std::move(t2));     // t3 is joinable, t2 is not (moved-from)

t3.join();                         // After join(), t3 is not joinable
```

## Two Ways to Make Threads Unjoinable

### Option 1: join() - Wait for Completion

```cpp
void example1() {
    std::thread t(doWork);
    t.join();  // Wait for thread to complete
    // Now t is unjoinable
}  // Safe! t's destructor won't terminate
```

### Option 2: detach() - Let It Run Independently

```cpp
void example2() {
    std::thread t(doWork);
    t.detach();  // Thread continues independently
    // Now t is unjoinable
}  // Safe! t's destructor won't terminate
```

## The Exception Safety Problem

### Unsafe Code (BAD!)

```cpp
void processData(const std::string& filename) {
    std::thread t(processInBackground, filename);
    
    // Do some work that might throw
    auto data = loadFile(filename);  // Might throw!
    doProcessing(data);              // Might throw!
    
    t.join();  // Never reached if exception thrown!
}  // ← CRASH! Thread still joinable
```

**Problem**: If an exception is thrown, `join()` is never called, and the program terminates.

## The RAII Solution

### ThreadGuard: A Simple RAII Wrapper

```cpp
class ThreadGuard {
public:
    explicit ThreadGuard(std::thread t) : thread(std::move(t)) {
        if (!thread.joinable()) {
            throw std::logic_error("No thread!");
        }
    }
    
    ~ThreadGuard() {
        if (thread.joinable()) {
            thread.join();  // Always join in destructor
        }
    }
    
    // Prevent copying
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
    
private:
    std::thread thread;
};
```

### Using ThreadGuard (GOOD!)

```cpp
void processDataSafely(const std::string& filename) {
    ThreadGuard guard(std::thread(processInBackground, filename));
    
    // Do work that might throw
    auto data = loadFile(filename);  // Exception? No problem!
    doProcessing(data);              // Exception? Still safe!
    
}  // ThreadGuard destructor automatically joins, even with exceptions!
```

## Complete Working Examples

### Example 1: The Danger

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void work() {
    std::cout << "Thread working...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Thread done!\n";
}

// ❌ DANGEROUS - Program will terminate!
void dangerousExample() {
    std::thread t(work);
    // Forgot to join or detach!
}  // std::terminate() called here

int main() {
    // Don't actually run this - it will crash!
    // dangerousExample();
    
    std::cout << "If we called dangerousExample(), ";
    std::cout << "the program would have terminated!\n";
    return 0;
}
```

### Example 2: Safe with Manual join()

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void work(int id) {
    std::cout << "Thread " << id << " working...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Thread " << id << " done!\n";
}

// ✅ SAFE - Manual join
void safeManualJoin() {
    std::thread t(work, 1);
    t.join();  // Wait for completion
}  // Safe!

int main() {
    std::cout << "=== Manual Join Example ===\n";
    safeManualJoin();
    std::cout << "Completed successfully!\n";
    return 0;
}
```

### Example 3: Exception Safety Problem

```cpp
#include <iostream>
#include <thread>
#include <stdexcept>
#include <chrono>

void work() {
    std::cout << "Thread working...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Thread done!\n";
}

// ❌ UNSAFE - Exception can cause crash
void unsafeWithException() {
    std::thread t(work);
    
    // Simulate work that might throw
    throw std::runtime_error("Something went wrong!");
    
    t.join();  // Never reached!
}  // CRASH! Thread still joinable

int main() {
    std::cout << "=== Exception Safety Problem ===\n";
    
    try {
        // Don't actually run this in real code!
        // unsafeWithException();
        std::cout << "If we ran unsafeWithException(), program would crash!\n";
    } catch (const std::exception& e) {
        std::cout << "Exception caught, but damage already done!\n";
    }
    
    return 0;
}
```

### Example 4: ThreadGuard - The Solution

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

class ThreadGuard {
public:
    explicit ThreadGuard(std::thread t) : thread(std::move(t)) {
        if (!thread.joinable()) {
            throw std::logic_error("No thread!");
        }
    }
    
    ~ThreadGuard() {
        if (thread.joinable()) {
            std::cout << "ThreadGuard: Joining thread...\n";
            thread.join();
        }
    }
    
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
    
private:
    std::thread thread;
};

void work(int id) {
    std::cout << "Thread " << id << " working...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Thread " << id << " done!\n";
}

// ✅ SAFE with ThreadGuard
void safeWithGuard() {
    ThreadGuard guard(std::thread(work, 1));
    
    // Do work...
    std::cout << "Main: Doing other work...\n";
    
}  // ThreadGuard destructor automatically joins!

// ✅ SAFE even with exceptions
void safeEvenWithException() {
    ThreadGuard guard(std::thread(work, 2));
    
    std::cout << "Main: About to throw exception...\n";
    throw std::runtime_error("Something went wrong!");
    
}  // ThreadGuard destructor still joins during stack unwinding!

int main() {
    std::cout << "=== ThreadGuard Example ===\n\n";
    
    std::cout << "Test 1: Normal flow\n";
    safeWithGuard();
    std::cout << "Test 1 completed!\n\n";
    
    std::cout << "Test 2: Exception thrown\n";
    try {
        safeEvenWithException();
    } catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << "\n";
        std::cout << "But thread was still joined safely!\n";
    }
    
    return 0;
}
```

### Example 5: Detaching Threads

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void backgroundWork() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Background work complete!\n";
}

// ✅ Using detach() - but be careful!
void detachExample() {
    std::thread t(backgroundWork);
    t.detach();  // Thread runs independently
    
    std::cout << "Main: Thread detached, continuing...\n";
}  // Safe! Thread is unjoinable

int main() {
    std::cout << "=== Detach Example ===\n";
    detachExample();
    
    std::cout << "Main: Doing other work...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Main: Exiting...\n";
    
    // Note: backgroundWork might not finish before main exits!
    return 0;
}
```

### Example 6: ThreadGuard with detach() Option

```cpp
#include <iostream>
#include <thread>
#include <chrono>

enum class ThreadAction {
    Join,
    Detach
};

class FlexibleThreadGuard {
public:
    FlexibleThreadGuard(std::thread t, ThreadAction action)
        : thread(std::move(t)), action(action) {
        if (!thread.joinable()) {
            throw std::logic_error("No thread!");
        }
    }
    
    ~FlexibleThreadGuard() {
        if (thread.joinable()) {
            if (action == ThreadAction::Join) {
                std::cout << "Joining thread...\n";
                thread.join();
            } else {
                std::cout << "Detaching thread...\n";
                thread.detach();
            }
        }
    }
    
    FlexibleThreadGuard(const FlexibleThreadGuard&) = delete;
    FlexibleThreadGuard& operator=(const FlexibleThreadGuard&) = delete;
    
private:
    std::thread thread;
    ThreadAction action;
};

void task(const std::string& name) {
    std::cout << name << " running\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << name << " done\n";
}

int main() {
    std::cout << "=== Flexible ThreadGuard ===\n\n";
    
    std::cout << "Test 1: Join policy\n";
    {
        FlexibleThreadGuard guard(
            std::thread(task, "Task1"), 
            ThreadAction::Join
        );
    }  // Waits for thread
    std::cout << "After scope 1\n\n";
    
    std::cout << "Test 2: Detach policy\n";
    {
        FlexibleThreadGuard guard(
            std::thread(task, "Task2"), 
            ThreadAction::Detach
        );
    }  // Doesn't wait
    std::cout << "After scope 2 (thread still running)\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return 0;
}
```

## The Dangers of detach()

### Problem: Detached Thread Accessing Destroyed Data

```cpp
#include <iostream>
#include <thread>
#include <chrono>

// ❌ DANGEROUS - detached thread accesses local variable
void dangerousDetach() {
    int localData = 42;
    
    std::thread t([&localData]() {  // Captures by reference!
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Data: " << localData << "\n";  // Undefined behavior!
    });
    
    t.detach();  // Thread continues independently
    
}  // localData destroyed, but thread still tries to access it!

int main() {
    // Don't actually run this!
    // dangerousDetach();  // Undefined behavior
    
    std::cout << "dangerousDetach() would cause undefined behavior!\n";
    std::cout << "The thread would access destroyed local variable.\n";
    return 0;
}
```

### Solution: Pass by Value or Use join()

```cpp
#include <iostream>
#include <thread>
#include <chrono>

// ✅ SAFE - pass by value
void safeDetach() {
    int localData = 42;
    
    std::thread t([localData]() {  // Captures by value!
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Data: " << localData << "\n";
    });
    
    t.detach();  // Safe! Thread has its own copy
}

// ✅ SAFE - use join
void safeJoin() {
    int localData = 42;
    
    std::thread t([&localData]() {  // Reference is OK with join
        std::cout << "Data: " << localData << "\n";
    });
    
    t.join();  // Wait for completion before localData is destroyed
}

int main() {
    std::cout << "=== Safe Detach/Join ===\n";
    safeDetach();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    safeJoin();
    return 0;
}
```

## C++20 Alternative: std::jthread

In C++20, there's a better solution built into the standard library:

```cpp
#include <thread>  // C++20

void work() {
    // Do work
}

void modernApproach() {
    std::jthread t(work);  // Automatically joins in destructor!
    
    // Do other work...
    
}  // jthread automatically joined, no need for manual management!
```

## Best Practices Summary

### ✅ DO:
1. **Use RAII wrappers** (ThreadGuard) for automatic cleanup
2. **Always join or detach** before thread destruction
3. **Pass data by value** when using detach()
4. **Use std::jthread** in C++20 if available

### ❌ DON'T:
1. **Leave threads joinable** when they go out of scope
2. **Forget exception safety** - use RAII
3. **Detach threads that access local variables** by reference
4. **Assume join() will always be reached** in exception-prone code

## Quick Reference

| Scenario | Solution |
|----------|----------|
| Normal flow | Manual `join()` or `detach()` |
| Exception-prone code | RAII wrapper (ThreadGuard) |
| Background task, no data sharing | `detach()` is OK |
| Accessing local variables | Must `join()`, don't detach |
| C++20 code | Use `std::jthread` |

## Common Mistakes

### Mistake 1: Forgetting to join

```cpp
// ❌ BAD
void bad() {
    std::thread t(work);
    // Oops, no join or detach
}
```

### Mistake 2: join() after exception

```cpp
// ❌ BAD
void bad() {
    std::thread t(work);
    mightThrow();  // If throws, join() never called
    t.join();
}
```

### Mistake 3: Detaching with local references

```cpp
// ❌ BAD
void bad() {
    int data = 42;
    std::thread t([&data]() { useData(data); });
    t.detach();  // data will be destroyed!
}
```

## Summary

- **Every thread must be unjoinable before destruction** (via join or detach)
- **Use RAII** (like ThreadGuard) to ensure automatic cleanup
- **Be careful with detach()** - make sure the thread doesn't access local data
- **In C++20, use std::jthread** for automatic joining
- **Think of threads like hired workers** - you must settle up with them before they leave!

**Remember**: "Join or detach before destruct, or your program will self-destruct!" 💥
