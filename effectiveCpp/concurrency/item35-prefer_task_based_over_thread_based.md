# Item 35: Prefer Task-Based Programming to Thread-Based

## Simple Explanation

**Main Idea**: Instead of creating threads directly using `std::thread`, use task-based approaches like `std::async`. Let the C++ runtime manage threads for you rather than doing it manually.

Think of it like this:

- **Thread-based**: You're a restaurant owner who manually assigns each waiter to each table
- **Task-based**: You just tell the restaurant "serve this customer" and let the manager decide which waiter is available

## The Two Approaches

### Thread-Based Approach (NOT Recommended)

```cpp
#include <thread>

int doAsyncWork() {
    // Some time-consuming work
    return 42;
}

void threadBasedExample() {
    std::thread t(doAsyncWork);  // Create a thread manually
    t.join();                     // Wait for it to finish
    // Problem: Can't get the return value easily!
}
```

### Task-Based Approach (✅ Recommended)

```cpp
#include <future>

int doAsyncWork() {
    return 42;
}

void taskBasedExample() {
    auto future = std::async(doAsyncWork);  // Create a task
    int result = future.get();               // Get the result
    std::cout << "Result: " << result << std::endl;
}
```

## Why Task-Based is Better

### 1. **Easy to Get Return Values**

**Thread-based** (complicated):

```cpp
int result;
std::thread t([&result]() {
    result = doAsyncWork();  // Need to capture by reference
});
t.join();
// Use result - but be careful with thread safety!
```

**Task-based** (simple):

```cpp
auto future = std::async(doAsyncWork);
int result = future.get();  // Clean and simple!
```

### 2. **Automatic Exception Handling**

**Thread-based**:

```cpp
std::thread t([]() {
    throw std::runtime_error("Oops!");  // Program might terminate!
});
t.join();
```

**Task-based**:

```cpp
auto future = std::async([]() {
    throw std::runtime_error("Oops!");
});

try {
    future.get();  // Exception is transported to here
} catch (const std::runtime_error& e) {
    std::cout << "Caught: " << e.what() << std::endl;
}
```

### 3. **Automatic Thread Management**

The system knows:

- How many CPU cores you have
- How many threads are already running
- Whether to create a new thread or reuse an existing one

**Thread-based**: You might create too many threads (oversubscription) or too few

```cpp
// What if you do this 10,000 times?
for (int i = 0; i < 10000; ++i) {
    std::thread t(doWork);  // Creating 10,000 threads! BAD!
    t.detach();
}
```

**Task-based**: The runtime manages this for you

```cpp
std::vector<std::future<void>> futures;
for (int i = 0; i < 10000; ++i) {
    futures.push_back(std::async(doWork));  // Runtime manages thread pool
}
```

## Complete Working Example

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <chrono>

// Simulates some work that takes time
int calculateSquare(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (x < 0) {
        throw std::invalid_argument("Negative number!");
    }
    return x * x;
}

// ❌ Thread-based approach (more complicated)
void threadBasedApproach() {
    std::cout << "=== Thread-Based Approach ===" << std::endl;
  
    std::vector<int> results(5);
    std::vector<std::thread> threads;
  
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i, &results]() {
            results[i] = calculateSquare(i);
        });
    }
  
    for (auto& t : threads) {
        t.join();
    }
  
    for (int i = 0; i < 5; ++i) {
        std::cout << i << "^2 = " << results[i] << std::endl;
    }
}

// ✅ Task-based approach (simpler and better)
void taskBasedApproach() {
    std::cout << "\n=== Task-Based Approach ===" << std::endl;
  
    std::vector<std::future<int>> futures;
  
    for (int i = 0; i < 5; ++i) {
        futures.push_back(std::async(std::launch::async, calculateSquare, i));
    }
  
    for (int i = 0; i < 5; ++i) {
        try {
            int result = futures[i].get();
            std::cout << i << "^2 = " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error for " << i << ": " << e.what() << std::endl;
        }
    }
}

int main() {
    threadBasedApproach();
    taskBasedApproach();
    return 0;
}
```

## When to Still Use std::thread

There are rare cases where you might need `std::thread`:

1. **Need access to the underlying native thread handle**

   ```cpp
   std::thread t(doWork);
   auto handle = t.native_handle();  // For platform-specific APIs
   ```
2. **Need to set thread priority or affinity**

   ```cpp
   std::thread t(doWork);
   // Set CPU affinity using OS-specific calls
   ```
3. **Your threading library requires threads** (very rare)
4. **Implementing your own thread pool** (advanced use case)

## Key Takeaways

| Aspect              | `std::thread` | `std::async`  |
| ------------------- | --------------- | --------------- |
| Return values       | Hard            | Easy            |
| Exceptions          | Terminates      | Transported     |
| Thread management   | Manual          | Automatic       |
| Resource management | You decide      | Runtime decides |
| Complexity          | Higher          | Lower           |

## Summary

- **Use `std::async`** for most concurrent tasks
- It's simpler, safer, and more efficient
- Only use `std::thread` when you need low-level control
- Let the C++ runtime be smart about thread management

**Remember**: "Don't manage threads, manage tasks!"
